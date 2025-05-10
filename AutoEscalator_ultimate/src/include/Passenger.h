#pragma once

struct Passenger {
    int sourceFloor;
    int targetFloor;
    double requestTime;
    double waitTimeout;
    
    Passenger(int from, int to, double time, double timeout)
        : sourceFloor(from), targetFloor(to), requestTime(time), waitTimeout(timeout) {}
}; 