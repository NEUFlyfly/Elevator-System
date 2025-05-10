#pragma once
#include "ElevatorSystem.h"

class UserInterface {
private:
    ElevatorSystem system;
    InputMode currentMode;

    void runSimulation();
    void handleManualInput();
    void configureElevatorParams();
    void showCurrentConfig();
    void configureTimeSettings();
    void configureElevatorMode();

public:
    void showMainMenu();
    void handleInput();
    void displayStatus();
    void configureSystem();
}; 