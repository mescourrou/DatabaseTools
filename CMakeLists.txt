cmake_minimum_required(VERSION 3.10)
project(DatabaseTools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TEST "Build the tests" ON)
option(BUILD_USE_FILESYSTEM "Use c++17 filesystem lib" ON)

set(FILESYSTEM_LIB)
if(BUILD_USE_FILESYSTEM)
    set(FILESYSTEM_LIB stdc++fs)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(DB_TOOLS_THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/thirdparty")

find_package(glog REQUIRED)
find_package(sqlite REQUIRED)

add_library(DatabaseTools
    src/Database.cpp src/Database.hpp
    src/Query.cpp src/Query.hpp)
target_link_libraries(DatabaseTools
    glog
    sqlite
    ${FILESYSTEM_LIB})
target_include_directories(DatabaseTools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
if (BUILD_USE_FILESYSTEM)
    target_compile_definitions(DatabaseTools PRIVATE BUILD_USE_FILESYSTEM)
endif()

if (BUILD_TEST)
    find_package(gtest REQUIRED)
    target_link_libraries(DatabaseTools gtest)
    target_compile_definitions(DatabaseTools PUBLIC BUILD_TEST)
    add_subdirectory(unit_tests)
endif()

