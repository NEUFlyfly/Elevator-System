#include "Constants.h"

namespace ElevatorConfig {
    double FLOOR_TIME = 5.0;
    double IDLE_MAX_TIME = 10.0;
    double MAX_WAIT_TIME = 60.0;
    double DAY_SIMULATION_TIME = 24.0;

    void setFloorTime(double time) {
        if (time > 0) {
            FLOOR_TIME = time;
        }
    }

    void setIdleMaxTime(double time) {
        if (time > 0) {
            IDLE_MAX_TIME = time;
        }
    }

    void setMaxWaitTime(double time) {
        if (time > 0) {
            MAX_WAIT_TIME = time;
        }
    }

    void setDaySimulationTime(double time) {
        if (time > 0) {
            DAY_SIMULATION_TIME = time;
        }
    }
} 