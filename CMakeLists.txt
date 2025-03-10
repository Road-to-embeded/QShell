cmake_minimum_required(VERSION 3.28)

project(qshell VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Widgets module
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

# Sources
set(SOURCES src/main.cpp)


# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)


