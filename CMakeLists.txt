cmake_minimum_required(VERSION 3.10)
project(SpectralGraphAnalysis)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

find_package(Eigen3 REQUIRED)

add_executable(SpectralSolver src/main.cpp src/Graph.cpp)

target_link_libraries(SpectralSolver PRIVATE Eigen3::Eigen)