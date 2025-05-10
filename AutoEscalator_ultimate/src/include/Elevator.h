#pragma once
#include <vector>
#include "Passenger.h"

enum class ElevatorState {
    IDLE,
    MOVING_UP,
    MOVING_DOWN,
    STOPPED
};

class Elevator {
private:
    int currentFloor;
    int capacity;
    std::vector<Passenger> passengers;
    ElevatorState state;
    double idleTimer;

public:
    Elevator();
    void move();
    bool addPassenger(const Passenger& passenger);
    void removePassenger(int floor);
    void update(double deltaTime);
    int getCurrentFloor() const;
    int getCurrentLoad() const;
    ElevatorState getState() const;
    void setState(ElevatorState newState);
    bool hasStopRequest(int floor) const;
    void updateMovement(double deltaTime);
}; 