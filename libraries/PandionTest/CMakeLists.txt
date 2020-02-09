cmake_minimum_required(VERSION 3.10)

project(PandionTest)

add_subdirectory(../AtaLogger AtaLogger)

add_executable(PandionTest main.cpp)
target_include_directories(PandionTest PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/AtaLogger)

target_link_libraries(PandionTest AtaLogger)

