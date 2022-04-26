cmake_minimum_required(VERSION 3.19)
project(PCMCodec)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(PCMCodec
        PCMCodec.hpp
        test/test.cpp
        WaveCodec.hpp)
