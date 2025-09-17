cmake_minimum_required(VERSION 3.16)
project(hpcr_project LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system thread)

# Find glog
find_package(glog REQUIRED)

# Find yaml-cpp
find_package(yaml-cpp REQUIRED)

# Build output directory (optional)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Server binary
add_executable(hpcr hpcr.cpp)
target_link_libraries(hpcr PRIVATE Threads::Threads Boost::system Boost::thread glog::glog yaml-cpp)

# Client binary
add_executable(clientApp client.cpp)
target_link_libraries(clientApp PRIVATE Threads::Threads Boost::system Boost::thread glog::glog yaml-cpp)

# Config file install path (can be overridden)
set(HPCR_CONFIG_INSTALL_DIR "/etc/hpcr" CACHE PATH "Default configuration directory")

install(FILES config.yaml DESTINATION ${HPCR_CONFIG_INSTALL_DIR})
