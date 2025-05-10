#pragma once
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>

class Logger {
private:
    static std::ofstream logFile;
    static bool initialized;

public:
    static void init(const std::string& filename = "elevator.log") {
        if (!initialized) {
            logFile.open(filename);
            initialized = true;
        }
    }

    static void log(const std::string& message) {
        if (!initialized) init();

        auto now = std::time(nullptr);
        auto* timeinfo = std::localtime(&now);

        logFile << std::put_time(timeinfo, "[%Y-%m-%d %H:%M:%S] ")
                << message << std::endl;
    }

    static void close() {
        if (initialized) {
            logFile.close();
            initialized = false;
        }
    }
}; 