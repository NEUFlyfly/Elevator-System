#pragma once

namespace ElevatorConfig {
    constexpr int FLOOR_COUNT = 14;
    constexpr int ELEVATOR_COUNT = 4;
    constexpr int MAX_CAPACITY = 12;
    
    extern double FLOOR_TIME;
    extern double IDLE_MAX_TIME;
    extern double MAX_WAIT_TIME;
    extern double DAY_SIMULATION_TIME;
    
    constexpr double REAL_SECONDS_PER_SIM_SECOND = 3600.0;
    constexpr double SIM_SECONDS_PER_DAY = 24.0;
    
    void setFloorTime(double time);
    void setIdleMaxTime(double time);
    void setMaxWaitTime(double time);
    void setDaySimulationTime(double time);
    
    inline double realTimeToSimTime(double realSeconds) {
        return realSeconds / REAL_SECONDS_PER_SIM_SECOND;
    }
    
    inline double simTimeToRealTime(double simSeconds) {
        return simSeconds * REAL_SECONDS_PER_SIM_SECOND;
    }
} 