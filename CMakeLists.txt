cmake_minimum_required(VERSION 3.10)
project(SpectralGraphAnalysis)

set(CMAKE_CXX_STANDARD 14)

# 1. Look for headers in the "include" folder
include_directories(include)

# 2. Look for Eigen3 (adjust path if needed, this is standard for Homebrew on Mac)
include_directories("/opt/homebrew/include/eigen3")

# 3. Compile the source files from the "src" folder
add_executable(SpectralSolver src/main.cpp src/Graph.cpp)