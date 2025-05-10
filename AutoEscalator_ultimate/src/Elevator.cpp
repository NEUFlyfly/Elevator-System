#include "Elevator.h"
#include "Constants.h"
#include <algorithm>

Elevator::Elevator() 
    : currentFloor(1)  // 初始在1楼
    , capacity(ElevatorConfig::MAX_CAPACITY)
    , state(ElevatorState::IDLE)
    , idleTimer(0.0)
{
}

void Elevator::move() {
    switch (state) {
        case ElevatorState::MOVING_UP:
            if (currentFloor < ElevatorConfig::FLOOR_COUNT) {
                currentFloor++;
            }
            break;
        case ElevatorState::MOVING_DOWN:
            if (currentFloor > 1) {
                currentFloor--;
            }
            break;
        default:
            break;
    }
}

bool Elevator::addPassenger(const Passenger& passenger) {
    if (passengers.size() >= capacity) {
        return false;
    }
    passengers.push_back(passenger);
    return true;
}

void Elevator::removePassenger(int floor) {
    passengers.erase(
        std::remove_if(passengers.begin(), passengers.end(),
            [floor](const Passenger& p) { return p.targetFloor == floor; }),
        passengers.end()
    );
}

void Elevator::update(double deltaTime) {
    if (state == ElevatorState::IDLE) {
        idleTimer += deltaTime;
        if (idleTimer >= ElevatorConfig::IDLE_MAX_TIME) {
            if (currentFloor != 1) {
                state = ElevatorState::MOVING_DOWN;
            }
            idleTimer = 0;
        }
    }
}

int Elevator::getCurrentFloor() const {
    return currentFloor;
}

int Elevator::getCurrentLoad() const {
    return passengers.size();
}

ElevatorState Elevator::getState() const {
    return state;
}

void Elevator::setState(ElevatorState newState) {
    state = newState;
    if (state != ElevatorState::IDLE) {
        idleTimer = 0.0;
    }
}

bool Elevator::hasStopRequest(int floor) const {
    for (const auto& passenger : passengers) {
        if (passenger.targetFloor == floor) {
            return true;
        }
    }
    return false;
}

void Elevator::updateMovement(double deltaTime) {
    static double moveTimer = 0.0;
    moveTimer += deltaTime;

    if (moveTimer >= ElevatorConfig::FLOOR_TIME) {
        moveTimer = 0.0;
        move();
        
        if (hasStopRequest(currentFloor)) {
            setState(ElevatorState::STOPPED);
            removePassenger(currentFloor);
        }
    }
} 