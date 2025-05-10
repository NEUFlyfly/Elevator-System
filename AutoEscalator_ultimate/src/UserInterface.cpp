#include "UserInterface.h"
#include "Constants.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <sstream>

void UserInterface::showMainMenu() {
    while (true) {
        std::cout << "\n=== 电梯模拟系统 ===\n"
                 << "1. 开始模拟\n"
                 << "2. 配置系统\n"
                 << "3. 查看当前状态\n"
                 << "4. 重置系统\n"
                 << "5. 退出\n"
                 << "请选择：";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                system.start();
                runSimulation();
                break;
            case 2:
                configureSystem();
                break;
            case 3:
                system.printCurrentStatus();
                break;
            case 4:
                system.reset();
                std::cout << "系统已重置\n";
                break;
            case 5:
                return;
            default:
                std::cout << "无效选择，请重试\n";
        }
    }
}

void UserInterface::handleInput() {
    std::cout << "\n选择输入模式：\n"
              << "1. 随机生成\n"
              << "2. 文件输入\n"
              << "3. 手动输入\n"
              << "请选择：";
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1: {
            currentMode = InputMode::RANDOM;
            std::cout << "\n=== 随机请求配置 ===\n";
            
            int peakCount, normalCount;
            
            std::cout << "请输入高峰期请求数量（默认100）：";
            std::cin >> peakCount;
            
            if (std::cin.fail() || peakCount <= 0) {
                std::cout << "无效输入，使用默认值100\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                peakCount = 100;
            }
            
            std::cout << "请输入平时请求数量（默认50）：";
            std::cin >> normalCount;
            
            if (std::cin.fail() || normalCount <= 0) {
                std::cout << "无效输入，使用默认值50\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                normalCount = 50;
            }
            
            system.setRequestCounts(peakCount, normalCount);
            system.loadRandomRequests();
            std::cout << "随机请求已生成\n";
            break;
        }
        case 2: {
            currentMode = InputMode::FILE;
            std::vector<std::string> dataFiles;
            
            std::filesystem::path exePath = std::filesystem::current_path();
            std::filesystem::path dataPath = exePath.parent_path().parent_path() / "data";
            
            if (!std::filesystem::exists(dataPath)) {
                dataPath = exePath.parent_path() / "data";
                
                if (!std::filesystem::exists(dataPath)) {
                    std::cout << u8"错误：找不到data目录，已尝试以下路径：\n"
                             << "1. " << (exePath.parent_path().parent_path() / "data") << "\n"
                             << "2. " << (exePath.parent_path() / "data") << "\n";
                    return;
                }
            }
            
            if (!std::filesystem::is_directory(dataPath)) {
                std::cout << u8"错误：data不是一个目录\n";
                return;
            }
            
            for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                    dataFiles.push_back(entry.path().filename().string());
                }
            }
            
            if (dataFiles.empty()) {
                std::cout << u8"未找到数据文件，请确保data目录下有.txt格式的请求文件\n";
                return;
            }
            
            std::sort(dataFiles.begin(), dataFiles.end());
            
            std::cout << u8"\n可用的请求文件：\n";
            for (size_t i = 0; i < dataFiles.size(); ++i) {
                std::cout << (i + 1) << ". " << dataFiles[i] << "\n";
            }
            
            std::cout << u8"请选择文件编号（1-" << dataFiles.size() << "）：";
            int fileChoice;
            std::cin >> fileChoice;
            
            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << u8"无效的输入\n";
                return;
            }
            
            if (fileChoice < 1 || fileChoice > static_cast<int>(dataFiles.size())) {
                std::cout << u8"无效的选择\n";
                return;
            }
            
            std::string filename = (dataPath / dataFiles[fileChoice - 1]).string();
            try {
                system.loadFileRequests(filename);
                std::cout << u8"成功加载文件：" << dataFiles[fileChoice - 1] << "\n";
            } catch (const std::exception& e) {
                std::cout << u8"加载文件失败：" << e.what() << "\n";
            }
            break;
        }
        case 3:
            currentMode = InputMode::MANUAL;
            handleManualInput();
            break;
        default:
            std::cout << "无效选择，使用随机模式\n";
            currentMode = InputMode::RANDOM;
            system.loadRandomRequests();
    }
}

void UserInterface::displayStatus() {
    system.printCurrentStatus();
}

void UserInterface::configureSystem() {
    while (true) {
        std::cout << "\n=== 系统配置 ===\n"
                  << "1. 配置输入模式\n"
                  << "2. 配置电梯参数\n"
                  << "3. 配置电梯策略\n"
                  << "4. 查看当前配置\n"
                  << "5. 返回主菜单\n"
                  << "请选择：";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                handleInput();
                break;
            case 2:
                configureElevatorParams();
                break;
            case 3:
                configureElevatorMode();
                break;
            case 4:
                showCurrentConfig();
                break;
            case 5:
                return;
            default:
                std::cout << "无效选择\n";
        }
    }
}

void UserInterface::configureElevatorParams() {
    std::cout << "\n=== 电梯参数配置 ===\n"
              << "当前设置：\n"
              << "1. 电梯运行速度：" << ElevatorConfig::FLOOR_TIME << " 模拟秒/层\n"
              << "2. 最大等待时间：" << ElevatorConfig::MAX_WAIT_TIME << " 模拟秒\n"
              << "3. 空闲等待时间：" << ElevatorConfig::IDLE_MAX_TIME << " 模拟秒\n"
              << "4. 模拟时间比例：" << ElevatorConfig::DAY_SIMULATION_TIME << " 秒/天\n"
              << "5. 返回\n\n"
              << "时间说明：\n"
              << "- 1模拟秒 = 1小时 = 3600真实秒\n"
              << "- 所有时间输入均使用模拟时间单位\n"
              << "请选择要修改的项目：";
    
    int choice;
    std::cin >> choice;
    double value;
    
    switch (choice) {
        case 1:
            std::cout << "请输入每层楼运行时间（模拟秒）：";
            std::cin >> value;
            if (value > 0) system.setElevatorSpeed(value);
            break;
        case 2:
            std::cout << "请输入最大等待时间（模拟秒）：";
            std::cin >> value;
            if (value > 0) system.setMaxWaitTime(value);
            break;
        case 3:
            std::cout << "请输入空闲等待时间（模拟秒）：";
            std::cin >> value;
            if (value > 0) system.setMaxIdleTime(value);
            break;
        case 4: {
            std::cout << "\n当前模拟时间比例说明：\n"
                     << "- 1模拟秒 = 1小时 = 3600真实秒\n"
                     << "- 24模拟秒 = 1天 = 86400真实秒\n"
                     << "- 默认值：24（表示1天的时间被压缩到24秒）\n\n"
                     << "请输入模拟时间比例（秒/天，建议值：24）：";
            std::cin >> value;
            if (value > 0) {
                ElevatorConfig::setDaySimulationTime(value);
                std::cout << "已设置模拟时间比例：1天 = " << value << " 秒\n"
                         << "即：1模拟秒 = " << (24.0/value) << " 小时\n";
            }
            break;
        }
        case 5:
            return;
        default:
            std::cout << "无效选择\n";
    }
}

void UserInterface::showCurrentConfig() {
    std::cout << "\n=== 当前配置 ===\n"
              << u8"输入模式：";
    switch (currentMode) {
        case InputMode::RANDOM: std::cout << u8"随机生成\n"; break;
        case InputMode::FILE: std::cout << u8"文件输入\n"; break;
        case InputMode::MANUAL: std::cout << u8"手动输入\n"; break;
    }
    
    std::cout << u8"电梯运行速度：" << ElevatorConfig::FLOOR_TIME << u8" 模拟秒/层 "
              << "(" << (ElevatorConfig::FLOOR_TIME * 3600) << u8" 真实秒/层)\n"
              << u8"空闲等待时间：" << ElevatorConfig::IDLE_MAX_TIME << u8" 模拟秒 "
              << "(" << (ElevatorConfig::IDLE_MAX_TIME * 3600) << u8" 真实秒)\n"
              << u8"最大等待时间：" << ElevatorConfig::MAX_WAIT_TIME << u8" 模拟秒 "
              << "(" << (ElevatorConfig::MAX_WAIT_TIME * 3600) << u8" 真实秒)\n"
              << u8"模拟时间比例：1模拟秒 = 1小时 = 3600真实秒\n"
              << u8"总模拟时长：" << ElevatorConfig::DAY_SIMULATION_TIME << u8" 秒 = 1天\n"
              << "\n随机请求配��：\n"
              << "高峰期请求数：" << system.getPeakRequestCount() << "\n"
              << "平时请求数：" << system.getNormalRequestCount() << "\n"
              << "\n电梯运行策略：";
    switch (system.getStrategy()) {
        case ElevatorStrategy::NEAREST_FIRST:
            std::cout << "就近优先\n";
            break;
        case ElevatorStrategy::SCAN:
            std::cout << "扫描算法\n";
            break;
        case ElevatorStrategy::LOOK:
            std::cout << "LOOK算法\n";
            break;
    }
}

void UserInterface::runSimulation() {
    // 设置实际的时间步长 (1/(60*60) 秒，即1秒=1小时)
    const double timeStep = 1.0 / (60.0 * 60.0);
    double simulationTime = 0.0;
    
    // 用于进度条显示的计数器
    int progressBarWidth = 50;  // 进度条宽度
    double displayUpdateInterval = 0.1;  // 显示更新间隔（0.1秒）
    double nextDisplayUpdate = displayUpdateInterval;
    
    // ANSI转义序列
    const std::string clearScreen = "\033[2J\033[H";
    const std::string saveCursor = "\033[s";
    const std::string restoreCursor = "\033[u";
    const std::string hideCursor = "\033[?25l";
    const std::string showCursor = "\033[?25h";
    
    // 隐藏光标
    std::cout << hideCursor;
    
    while (simulationTime < ElevatorConfig::DAY_SIMULATION_TIME) {
        system.update(timeStep);
        simulationTime += timeStep;
        
        // 每0.1秒更新一次显示
        if (simulationTime >= nextDisplayUpdate) {
            // 清屏并��置光标位置
            std::cout << clearScreen;
            
            // 显示电梯状态
            system.printCurrentStatus();
            
            // 显示模拟时间（24小时制）
            int hours = static_cast<int>(simulationTime * 24 / ElevatorConfig::DAY_SIMULATION_TIME);
            int minutes = static_cast<int>((simulationTime * 24 * 60 / ElevatorConfig::DAY_SIMULATION_TIME) - (hours * 60));
            int seconds = static_cast<int>((simulationTime * 24 * 3600 / ElevatorConfig::DAY_SIMULATION_TIME) - (hours * 3600 + minutes * 60));
            
            std::cout << "\n=== 模拟状态 ===\n"
                      << "当前时间: " 
                      << std::setfill('0') << std::setw(2) << hours << ":"
                      << std::setfill('0') << std::setw(2) << minutes << ":"
                      << std::setfill('0') << std::setw(2) << seconds 
                      << " (" << std::fixed << std::setprecision(2) 
                      << (simulationTime / ElevatorConfig::DAY_SIMULATION_TIME * 24.0) << " 小时)\n"
                      << "模拟速度: 1秒 = " << (24.0/ElevatorConfig::DAY_SIMULATION_TIME) << " 小时\n\n";
            
            // 显示进度条
            double progress = simulationTime / ElevatorConfig::DAY_SIMULATION_TIME;
            int pos = static_cast<int>(progressBarWidth * progress);
            
            // 显示高峰段标记
            std::cout << "时段标记: ";
            for (int i = 0; i < progressBarWidth; ++i) {
                double hour = i * 24.0 / progressBarWidth;
                if ((hour >= 6 && hour < 8) || (hour >= 11 && hour < 12) ||
                    (hour >= 13 && hour < 14) || (hour >= 17 && hour < 18)) {
                    std::cout << "^";  // 高峰时段标记
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "\n";
            
            // 显示进度条
            std::cout << "进度: [";
            for (int i = 0; i < progressBarWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << std::fixed << std::setprecision(1) 
                     << (progress * 100.0) << "%\n";
            
            // 显示时间刻度
            std::cout << "时间: 0";
            for (int i = 1; i < 24; ++i) {
                int tickPos = static_cast<int>(progressBarWidth * i / 24.0);
                for (int j = 0; j < (tickPos - (i == 1 ? 0 : static_cast<int>(progressBarWidth * (i-1) / 24.0))); ++j) {
                    std::cout << "-";
                }
                if (i < 10) std::cout << " ";  // 对齐个位数
                std::cout << i;
            }
            std::cout << "\n";
            
            nextDisplayUpdate = simulationTime + displayUpdateInterval;
            
            // 短暂延时以控制显示刷新率
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    // 显示光标
    std::cout << showCursor;
    
    // 模拟结束，显示统计信息
    system.printStatistics();
}

void UserInterface::handleManualInput() {
    std::cout << "\n=== 手动输入请求 ===\n"
              << "输入格式说明：\n"
              << "1. 时间格式：HH:MM:SS（24小时制，如 07:00:00）\n"
              << "2. 楼层范围：1-" << ElevatorConfig::FLOOR_COUNT << "层\n"
              << "3. 输入顺序：时间 起始楼层 目标楼层 人数\n"
              << "4. 示例：07:00:00 1 5 2  表示早上7点从1楼到5楼有2人\n"
              << "5. 输入 -1 结束输入\n\n";
    
    while (true) {
        std::string timeStr;
        int from, to, count;
        
        // 首先读取时间或结束标记
        std::cout << "请输入（时间或-1结束）：";
        std::cin >> timeStr;
        
        if (timeStr == "-1") break;
        
        // 解析间
        int hour, minute, second;
        char colon1, colon2;
        std::istringstream iss(timeStr);
        iss >> hour >> colon1 >> minute >> colon2 >> second;
        
        if (iss.fail() || hour < 0 || hour >= 24 || minute < 0 || minute >= 60 || second < 0 || second >= 60) {
            std::cout << "无效的时间格式，请使用 HH:MM:SS 格式\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        
        // 读取其他参数
        std::cout << "请输入：起始楼层 目标楼层 人数：";
        std::cin >> from >> to >> count;
        
        if (std::cin.fail() || from < 1 || from > ElevatorConfig::FLOOR_COUNT || 
            to < 1 || to > ElevatorConfig::FLOOR_COUNT || count <= 0) {
            std::cout << "无效的输入参数\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        
        // 转换时间到模拟时间（直接使用小时作为模拟时间）
        double time = hour + minute/60.0 + second/3600.0;
        
        // 添加请求
        system.addManualRequest(from, to, count, time);
        std::cout << "已添加请求：" 
                 << std::setfill('0') << std::setw(2) << hour << ":"
                 << std::setfill('0') << std::setw(2) << minute << ":"
                 << std::setfill('0') << std::setw(2) << second 
                 << " 从" << from << "层到" << to << "层，" << count << "人\n";
    }
}

void UserInterface::configureTimeSettings() {
    std::cout << "\n=== 时间设置 ===\n"
              << "当前设置：\n"
              << "1. 电梯运行时间：" << ElevatorConfig::FLOOR_TIME << " 秒/层\n"
              << "2. 空闲等待时间：" << ElevatorConfig::IDLE_MAX_TIME << " 秒\n"
              << "3. 乘客最大等待时间：" << ElevatorConfig::MAX_WAIT_TIME << " 秒\n"
              << "4. 模拟时间比例：" << ElevatorConfig::DAY_SIMULATION_TIME << " 秒/天\n"
              << "5. 返回\n\n"
              << "请选择要修改的项目：";
    
    int choice;
    std::cin >> choice;
    double value;
    
    switch (choice) {
        case 1:
            std::cout << "请输入电梯运行时间（秒/层）：";
            std::cin >> value;
            if (value > 0) system.setElevatorSpeed(value);
            break;
        case 2:
            std::cout << "请输入空闲等待时间（秒）：";
            std::cin >> value;
            if (value > 0) system.setMaxIdleTime(value);
            break;
        case 3:
            std::cout << "请输入乘客最大等待时间（秒）：";
            std::cin >> value;
            if (value > 0) system.setMaxWaitTime(value);
            break;
        case 4:
            std::cout << "请输入模拟时间比例（秒/天）：";
            std::cin >> value;
            if (value > 0) ElevatorConfig::setDaySimulationTime(value);
            break;
        case 5:
            return;
        default:
            std::cout << "无效选择\n";
    }
}

void UserInterface::configureElevatorMode() {
    std::cout << "\n=== 电梯运行策略 ===\n"
              << "当前策略：";
    
    switch (system.getStrategy()) {
        case ElevatorStrategy::NEAREST_FIRST:
            std::cout << "就近优先\n";
            break;
        case ElevatorStrategy::SCAN:
            std::cout << "扫描算法\n";
            break;
        case ElevatorStrategy::LOOK:
            std::cout << "LOOK算法\n";
            break;
    }
    
    std::cout << "\n策略说明：\n"
              << "1. 就近优先：选择距离乘客最近的电梯响应请求\n"
              << "2. 扫描算法：电梯会先到达当前方向的最远请求楼层\n"
              << "3. LOOK算法：遇到没有请求时立即改变方向\n\n"
              << "请选择策略（1-3）：";
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1:
            system.setStrategy(ElevatorStrategy::NEAREST_FIRST);
            std::cout << "已设置为就近优先策略\n";
            break;
        case 2:
            system.setStrategy(ElevatorStrategy::SCAN);
            std::cout << "已设置为扫描算法\n";
            break;
        case 3:
            system.setStrategy(ElevatorStrategy::LOOK);
            std::cout << "已设置为LOOK算法\n";
            break;
        default:
            std::cout << "无效选择，使用默认的就近优先策略\n";
            system.setStrategy(ElevatorStrategy::NEAREST_FIRST);
    }
} 