cmake_minimum_required(VERSION 3.28)

project(qshell VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Widgets module
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
qt_standard_project_setup()

# Sources
set(SOURCES 
  src/main.cpp
  src/QShellUI.cpp
  src/ProcessManager.cpp
)

# Headers
set(HEADERS 
  includes/QShellUI.h
  includes/ProcessManager.h
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add includes dir
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/includes)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network)


