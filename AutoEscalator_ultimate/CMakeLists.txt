cmake_minimum_required(VERSION 3.10)

project(elevator_simulation LANGUAGES CXX)

if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-fexec-charset=UTF-8)
    add_compile_options(-finput-charset=UTF-8)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES
    src/main.cpp
    src/Elevator.cpp
    src/ElevatorSystem.cpp
    src/UserInterface.cpp
    src/Constants.cpp
    src/Logger.cpp
)

set(HEADER_DIR src/include)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${HEADER_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_custom_target(copy_data ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    ${CMAKE_BINARY_DIR}/../data
)

add_dependencies(${PROJECT_NAME} copy_data)



