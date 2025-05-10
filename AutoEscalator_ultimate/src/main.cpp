#include "UserInterface.h"
#include "Logger.h"
#include <filesystem>

int main() {
    std::filesystem::path dataPath = std::filesystem::current_path() / "data";
    if (!std::filesystem::exists(dataPath)) {
        std::filesystem::create_directory(dataPath);
    }
    
    Logger::init("elevator.log");
    UserInterface ui;
    ui.showMainMenu();
    Logger::close();
    return 0;
}
