cmake_minimum_required(VERSION 3.14)
project(SpectralGraphAnalysis CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build the unit test suite" ON)
option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# 1. Look for Eigen3
find_package(Eigen3 3.3 QUIET NO_MODULE)
if(NOT TARGET Eigen3::Eigen)
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
    add_library(Eigen3::Eigen INTERFACE IMPORTED)
    set_target_properties(Eigen3::Eigen PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIR}")
endif()

# 2. Compiler warnings, applied only to this project's targets
add_library(project_warnings INTERFACE)
if(MSVC)
    target_compile_options(project_warnings INTERFACE /W4)
    if(WARNINGS_AS_ERRORS)
        target_compile_options(project_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow)
    if(WARNINGS_AS_ERRORS)
        target_compile_options(project_warnings INTERFACE -Werror)
    endif()
endif()

# 3. Compile the source files from the "src" folder
add_library(GraphLib src/Graph.cpp)
target_include_directories(GraphLib PUBLIC include)
target_link_libraries(GraphLib PUBLIC Eigen3::Eigen PRIVATE project_warnings)

add_executable(SpectralSolver src/main.cpp)
target_link_libraries(SpectralSolver PRIVATE GraphLib project_warnings)

# 4. Tests (uses an installed GoogleTest, otherwise downloads it at configure time)
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
    target_link_libraries(GraphTests PRIVATE GraphLib GTest::gtest_main project_warnings)

    include(GoogleTest)
    gtest_discover_tests(GraphTests)
endif()
