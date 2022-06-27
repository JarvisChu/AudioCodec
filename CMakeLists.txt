cmake_minimum_required(VERSION 3.19)
project(AudioCodec)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(PCMCodec)
add_subdirectory(WaveCodec)
add_subdirectory(example bin)


