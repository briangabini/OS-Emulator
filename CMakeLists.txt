cmake_minimum_required(VERSION 3.29)
project(csopesy)

set(CMAKE_CXX_STANDARD 20)

include_directories(os_emu_vs)

add_executable(csopesy
    os_emu_vs/os_emu_vs.cpp
    os_emu_vs/os_emu_vs.h
    os_emu_vs/util.h)
