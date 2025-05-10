#pragma once
#include <vector>
#include <queue>
#include <random>
#include "Elevator.h"

enum class InputMode {
    RANDOM,
    FILE,
    MANUAL
};

enum class ElevatorStrategy {
    NEAREST_FIRST,
    SCAN,
    LOOK
};

class ElevatorSystem {
private:
    std::vector<Elevator> elevators;
    std::queue<Passenger> waitingPassengers;
    double currentTime;
    std::vector<int> floorRequests;
    std::vector<int> hourlyRequests;
    int totalRequests = 0;
    int timeoutRequests = 0;
    double totalWaitTime = 0.0;

    struct RequestConfig {
        int peakTimeRequests = 100;
        int normalTimeRequests = 50;
    } requestConfig;

    ElevatorStrategy currentStrategy = ElevatorStrategy::NEAREST_FIRST;

    void processWaitingPassengers();
    void generatePeakTimeRequests(double startHour, double endHour, bool isUpPeak, std::mt19937& gen);
    void generateNormalTimeRequests(std::mt19937& gen);
    void updateStatistics();
    void assignElevator(const Passenger& passenger);
    bool isElevatorAvailable(const Elevator& elevator, const Passenger& passenger) const;
    int findBestElevator(const Passenger& passenger) const;
    int findNearestElevator(const Passenger& passenger) const;
    int findScanElevator(const Passenger& passenger) const;
    int findLookElevator(const Passenger& passenger) const;

public:
    ElevatorSystem();
    void start();
    void reset();
    void update(double deltaTime);
    void loadRandomRequests();
    void loadFileRequests(const std::string& filename);
    void addManualRequest(int from, int to, int count, double time);
    void printStatistics() const;
    void printCurrentStatus() const;
    void setElevatorSpeed(double speed);
    void setMaxWaitTime(double time);
    void setMaxIdleTime(double time);
    void setRequestCounts(int peakCount, int normalCount);
    int getPeakRequestCount() const { return requestConfig.peakTimeRequests; }
    int getNormalRequestCount() const { return requestConfig.normalTimeRequests; }
    void setStrategy(ElevatorStrategy strategy);
    ElevatorStrategy getStrategy() const { return currentStrategy; }
}; 