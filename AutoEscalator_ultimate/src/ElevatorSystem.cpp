#include "ElevatorSystem.h"
#include "Constants.h"
#include <fstream>
#include <random>
#include <iostream>
#include <sstream>
#include "Logger.h"
#include <iomanip>

ElevatorSystem::ElevatorSystem() 
    : currentTime(0.0)
{
    elevators.resize(ElevatorConfig::ELEVATOR_COUNT);
    floorRequests.resize(ElevatorConfig::FLOOR_COUNT, 0);
    hourlyRequests.resize(24, 0);
    totalRequests = 0;
    timeoutRequests = 0;
    totalWaitTime = 0.0;
}

void ElevatorSystem::start() {
    currentTime = 0.0;
    Logger::log("系统启动");
    while (!waitingPassengers.empty()) {
        waitingPassengers.pop();
    }
}

void ElevatorSystem::reset() {
    elevators.clear();
    elevators.resize(ElevatorConfig::ELEVATOR_COUNT);
    std::fill(floorRequests.begin(), floorRequests.end(), 0);
    std::fill(hourlyRequests.begin(), hourlyRequests.end(), 0);
    totalRequests = 0;
    currentTime = 0.0;
    while (!waitingPassengers.empty()) {
        waitingPassengers.pop();
    }
}

void ElevatorSystem::update(double deltaTime) {
    currentTime += deltaTime;
    
    for (auto& elevator : elevators) {
        elevator.update(deltaTime);
        elevator.updateMovement(deltaTime);
    }
    
    processWaitingPassengers();
    
    updateStatistics();
}

void ElevatorSystem::loadRandomRequests() {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    generatePeakTimeRequests(6.0, 8.0, true, gen);  
    generatePeakTimeRequests(11.0, 12.0, true, gen);
    generatePeakTimeRequests(13.0, 14.0, false, gen);
    generatePeakTimeRequests(17.0, 18.0, false, gen);
    generateNormalTimeRequests(gen);
}

void ElevatorSystem::loadFileRequests(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        int hour, minute, second;
        int from, to, count;
        char colon1, colon2;
        
        std::istringstream iss(line);
        iss >> hour >> colon1 >> minute >> colon2 >> second;
        iss >> from >> to >> count;
        
        if (iss.fail()) {
            std::cerr << "无效的输入行: " << line << std::endl;
            continue;
        }
        
        double time = (hour + minute/60.0 + second/3600.0);
        
        addManualRequest(from, to, count, time);
    }
}

void ElevatorSystem::addManualRequest(int from, int to, int count, double time) {
    int hour = static_cast<int>(time) % 24;
    hourlyRequests[hour] += count;
    floorRequests[from - 1] += count;
    floorRequests[to - 1] += count;
    totalRequests += count;
    
    for (int i = 0; i < count; ++i) {
        waitingPassengers.push(Passenger(from, to, time, time + 60.0));
    }
}

void ElevatorSystem::printStatistics() const {
    std::cout << "\n=== 电梯使用统计 ===\n";
    
    std::cout << "楼层请求统计：\n";
    for (int i = 0; i < ElevatorConfig::FLOOR_COUNT; ++i) {
        std::cout << "第 " << std::setw(2) << (i + 1) << " 层：" 
                 << std::setw(4) << floorRequests[i] << " 次请求\n";
    }
    
    std::cout << "\n时段请求统计：\n";
    for (int i = 0; i < 24; ++i) {
        double requestRate = totalRequests > 0 ? 
            (static_cast<double>(hourlyRequests[i]) / totalRequests * 100) : 0.0;
        
        std::cout << std::setw(2) << i << ":00 - " << std::setw(2) << (i + 1) 
                 << ":00：" << std::fixed << std::setprecision(1) 
                 << std::setw(5) << requestRate << "% 的乘客请求\n";
    }

    std::cout << "\n高峰期分析：\n";
    int morningRequests = 0;
    for (int i = 6; i < 8; ++i) {
        morningRequests += hourlyRequests[i];
    }
    double morningRate = totalRequests > 0 ? 
        (static_cast<double>(morningRequests) / totalRequests * 100) : 0.0;
    
    double noonRate = totalRequests > 0 ? 
        (static_cast<double>(hourlyRequests[11]) / totalRequests * 100) : 0.0;
    
    double lunchRate = totalRequests > 0 ? 
        (static_cast<double>(hourlyRequests[13]) / totalRequests * 100) : 0.0;
    
    double eveningRate = totalRequests > 0 ? 
        (static_cast<double>(hourlyRequests[17]) / totalRequests * 100) : 0.0;
    
    std::cout << "早高峰 (6:00-8:00)   请求比例：" << std::setprecision(1) << morningRate << "%\n"
              << "午高峰 (11:00-12:00) 请求比例：" << noonRate << "%\n"
              << "午休高峰 (13:00-14:00) 请求比例：" << lunchRate << "%\n"
              << "晚高峰 (17:00-18:00) 请求比例：" << eveningRate << "%\n"
              << "\n总请求数：" << totalRequests << " 次\n";
}

void ElevatorSystem::printCurrentStatus() const {
    std::cout << "\n=== 当前电梯状态 ===\n";
    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        std::cout << "电梯 " << (i + 1) << "：\n"
                 << "  当前楼层：" << elevator.getCurrentFloor() << "\n"
                 << "  载客数量：" << elevator.getCurrentLoad() << "\n"
                 << "  状态：";
        
        switch (elevator.getState()) {
            case ElevatorState::IDLE: std::cout << "空闲"; break;
            case ElevatorState::MOVING_UP: std::cout << "上行"; break;
            case ElevatorState::MOVING_DOWN: std::cout << "下行"; break;
            case ElevatorState::STOPPED: std::cout << "停止"; break;
        }
        std::cout << "\n\n";
    }

    int hour = static_cast<int>(currentTime);
    int minute = static_cast<int>((currentTime - hour) * 60);
    int second = static_cast<int>((currentTime - hour - minute/60.0) * 3600);
    
    std::cout << "当前时间：" 
              << std::setfill('0') << std::setw(2) << hour << ":"
              << std::setfill('0') << std::setw(2) << minute << ":"
              << std::setfill('0') << std::setw(2) << second << "\n";
}

void ElevatorSystem::processWaitingPassengers() {
    if (waitingPassengers.empty()) return;

    while (!waitingPassengers.empty()) {
        const auto& passenger = waitingPassengers.front();
        if (currentTime - passenger.requestTime > passenger.waitTimeout) {
            timeoutRequests++;
            std::string msg = "乘客请求超时：从" + std::to_string(passenger.sourceFloor) 
                             + "层到" + std::to_string(passenger.targetFloor) + "层";
            Logger::log(msg);
            waitingPassengers.pop();
            continue;
        }
        totalWaitTime += (currentTime - passenger.requestTime);
        break;
    }

    for (auto& elevator : elevators) {
        if (elevator.getState() == ElevatorState::IDLE) {
            if (!waitingPassengers.empty()) {
                const auto& passenger = waitingPassengers.front();
                if (elevator.getCurrentFloor() == passenger.sourceFloor) {
                    if (elevator.addPassenger(passenger)) {
                        waitingPassengers.pop();
                        elevator.setState(passenger.targetFloor > passenger.sourceFloor ? 
                            ElevatorState::MOVING_UP : ElevatorState::MOVING_DOWN);
                    }
                }
            }
        }
    }
}

void ElevatorSystem::generatePeakTimeRequests(double startHour, double endHour, bool isUpPeak, std::mt19937& gen) {
    double timeRange = (endHour - startHour) * 3600;
    std::uniform_real_distribution<> timeDist(0, timeRange);
    std::uniform_int_distribution<> floorDist(2, ElevatorConfig::FLOOR_COUNT);
    std::uniform_int_distribution<> countDist(1, 4);

    for (int i = 0; i < requestConfig.peakTimeRequests; ++i) {
        double relativeTime = timeDist(gen);
        double time = startHour + (relativeTime / 3600.0);
        
        int count = countDist(gen);
        
        if (isUpPeak) {
            int targetFloor = floorDist(gen);
            addManualRequest(1, targetFloor, count, time);
        } else {
            int sourceFloor = floorDist(gen);
            addManualRequest(sourceFloor, 1, count, time);
        }
    }
}

void ElevatorSystem::generateNormalTimeRequests(std::mt19937& gen) {
    std::vector<std::pair<double, double>> normalHours = {
        {0.0, 6.0},    // 凌晨
        {8.0, 11.0},   // 上午
        {12.0, 13.0},  // 中午
        {14.0, 17.0},  // 下午
        {18.0, 24.0}   // 晚上
    };
    
    std::uniform_int_distribution<> floorDist(1, ElevatorConfig::FLOOR_COUNT);
    std::uniform_int_distribution<> countDist(1, 3);
    
    for (int i = 0; i < requestConfig.normalTimeRequests; ++i) {
        int periodIndex = std::uniform_int_distribution<>(0, normalHours.size() - 1)(gen);
        auto period = normalHours[periodIndex];
        
        double time = std::uniform_real_distribution<>(period.first, period.second)(gen);
        
        int sourceFloor = floorDist(gen);
        int targetFloor;
        do {
            targetFloor = floorDist(gen);
        } while (targetFloor == sourceFloor);
        
        int count = countDist(gen);
        addManualRequest(sourceFloor, targetFloor, count, time);
    }
}

void ElevatorSystem::updateStatistics() {
    for (const auto& elevator : elevators) {
        int floor = elevator.getCurrentFloor();
        if (elevator.getState() != ElevatorState::IDLE) {
            floorRequests[floor - 1]++;
        }
    }

    int currentHour = static_cast<int>(currentTime) % 24;
    int activeElevators = 0;
    for (const auto& elevator : elevators) {
        if (elevator.getState() != ElevatorState::IDLE) {
            activeElevators++;
        }
    }
    
    if (activeElevators > 0) {
        hourlyRequests[currentHour]++;
    }
}

void ElevatorSystem::assignElevator(const Passenger& passenger) {
    int bestElevatorIndex = findBestElevator(passenger);
    if (bestElevatorIndex >= 0) {
        auto& elevator = elevators[bestElevatorIndex];
        if (elevator.addPassenger(passenger)) {
            if (elevator.getState() == ElevatorState::IDLE) {
                elevator.setState(passenger.targetFloor > passenger.sourceFloor ? 
                    ElevatorState::MOVING_UP : ElevatorState::MOVING_DOWN);
            }
        }
    }
}

bool ElevatorSystem::isElevatorAvailable(const Elevator& elevator, const Passenger& passenger) const {
    if (elevator.getCurrentLoad() >= ElevatorConfig::MAX_CAPACITY) {
        return false;
    }

    switch (elevator.getState()) {
        case ElevatorState::IDLE:
            return true;
        case ElevatorState::MOVING_UP:
            return passenger.targetFloor > elevator.getCurrentFloor();
        case ElevatorState::MOVING_DOWN:
            return passenger.targetFloor < elevator.getCurrentFloor();
        default:
            return false;
    }
}

int ElevatorSystem::findBestElevator(const Passenger& passenger) const {
    switch (currentStrategy) {
        case ElevatorStrategy::NEAREST_FIRST:
            return findNearestElevator(passenger);
        case ElevatorStrategy::SCAN:
            return findScanElevator(passenger);
        case ElevatorStrategy::LOOK:
            return findLookElevator(passenger);
        default:
            return findNearestElevator(passenger);
    }
}

int ElevatorSystem::findNearestElevator(const Passenger& passenger) const {
    int bestIndex = -1;
    int minDistance = INT_MAX;

    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        if (!isElevatorAvailable(elevator, passenger)) {
            continue;
        }

        int distance = std::abs(elevator.getCurrentFloor() - passenger.sourceFloor);
        if (distance < minDistance) {
            minDistance = distance;
            bestIndex = i;
        }
    }

    return bestIndex;
}

int ElevatorSystem::findScanElevator(const Passenger& passenger) const {
    int bestIndex = -1;
    int minDistance = INT_MAX;

    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        if (!isElevatorAvailable(elevator, passenger)) {
            continue;
        }

        if (elevator.getState() == ElevatorState::MOVING_UP && 
            passenger.targetFloor > elevator.getCurrentFloor()) {
            int distance = std::abs(elevator.getCurrentFloor() - passenger.sourceFloor);
            if (distance < minDistance) {
                minDistance = distance;
                bestIndex = i;
            }
        }
        else if (elevator.getState() == ElevatorState::MOVING_DOWN && 
                 passenger.targetFloor < elevator.getCurrentFloor()) {
            int distance = std::abs(elevator.getCurrentFloor() - passenger.sourceFloor);
            if (distance < minDistance) {
                minDistance = distance;
                bestIndex = i;
            }
        }
    }

    return bestIndex >= 0 ? bestIndex : findNearestElevator(passenger);
}

int ElevatorSystem::findLookElevator(const Passenger& passenger) const {
    int bestIndex = -1;
    int minDistance = INT_MAX;

    for (size_t i = 0; i < elevators.size(); ++i) {
        const auto& elevator = elevators[i];
        if (!isElevatorAvailable(elevator, passenger)) {
            continue;
        }

        if ((elevator.getState() == ElevatorState::MOVING_UP && 
             passenger.sourceFloor >= elevator.getCurrentFloor()) ||
            (elevator.getState() == ElevatorState::MOVING_DOWN && 
             passenger.sourceFloor <= elevator.getCurrentFloor())) {
            int distance = std::abs(elevator.getCurrentFloor() - passenger.sourceFloor);
            if (distance < minDistance) {
                minDistance = distance;
                bestIndex = i;
            }
        }
    }

    return bestIndex >= 0 ? bestIndex : findNearestElevator(passenger);
}

void ElevatorSystem::setElevatorSpeed(double speed) {
    ElevatorConfig::setFloorTime(speed);
}

void ElevatorSystem::setMaxWaitTime(double time) {
    ElevatorConfig::setMaxWaitTime(time);
}

void ElevatorSystem::setMaxIdleTime(double time) {
    ElevatorConfig::setIdleMaxTime(time);
}

void ElevatorSystem::setRequestCounts(int peakCount, int normalCount) {
    if (peakCount > 0) requestConfig.peakTimeRequests = peakCount;
    if (normalCount > 0) requestConfig.normalTimeRequests = normalCount;
}

void ElevatorSystem::setStrategy(ElevatorStrategy strategy) {
    currentStrategy = strategy;
    
    std::string strategyName;
    switch (strategy) {
        case ElevatorStrategy::NEAREST_FIRST:
            strategyName = "就近优先";
            break;
        case ElevatorStrategy::SCAN:
            strategyName = "扫描算法";
            break;
        case ElevatorStrategy::LOOK:
            strategyName = "LOOK算法";
            break;
    }
    Logger::log("电梯策略已更改为: " + strategyName);
}
 