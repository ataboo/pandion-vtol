cmake_minimum_required(VERSION 3.10)

add_definitions(-DATALOGGER_IOSTREAM)

add_library(AtaLogger AtaLogger.cpp)

target_include_directories(AtaLogger PUBLIC ./)
