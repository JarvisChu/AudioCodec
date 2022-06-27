cmake_minimum_required(VERSION 3.19)
project(Example)

add_executable(WaveCodecExample WaveCodecExample.cpp)
target_include_directories(WaveCodecExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(WaveCodecExample WaveCodec)

add_executable(PCMCodecExample PCMCodecExample.cpp)
target_include_directories(PCMCodecExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(PCMCodecExample PCMCodec)