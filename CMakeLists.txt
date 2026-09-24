cmake_minimum_required(VERSION 3.14)
project(SpectralGraphAnalysis)

set(CMAKE_CXX_STANDARD 14)

# 1. Look for headers in the "include" folder
include_directories(include)

# 2. Look for Eigen3
find_package(Eigen3 3.3 QUIET NO_MODULE)
if(Eigen3_FOUND)
    include_directories(${EIGEN3_INCLUDE_DIR})
else()
    # Fall back to searching common install locations (Homebrew, Linux package managers, etc.)
    find_path(EIGEN3_INCLUDE_DIR
        NAMES signature_of_eigen3_matrix_library
        PATHS
            /opt/homebrew/include/eigen3
            /usr/local/include/eigen3
            /usr/include/eigen3
    )
    if(NOT EIGEN3_INCLUDE_DIR)
        message(FATAL_ERROR "Eigen3 not found. Install it (e.g. 'brew install eigen' or 'apt install libeigen3-dev') or set EIGEN3_INCLUDE_DIR manually.")
    endif()
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

# 3. Compile the source files from the "src" folder
add_library(GraphLib src/Graph.cpp)
add_executable(SpectralSolver src/main.cpp)
target_link_libraries(SpectralSolver PRIVATE GraphLib)

# 4. Tests (uses an installed GoogleTest, otherwise downloads it at configure time)
option(BUILD_TESTS "Build the unit test suite" ON)
if(BUILD_TESTS)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    enable_testing()
    add_executable(GraphTests tests/GraphTests.cpp)
    target_link_libraries(GraphTests PRIVATE GraphLib GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(GraphTests)
endif()