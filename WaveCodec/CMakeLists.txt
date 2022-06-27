cmake_minimum_required(VERSION 3.19)
project(WaveCodec)

aux_source_directory(. WAVE_CODEC_SRCS)
add_library(${PROJECT_NAME} STATIC ${WAVE_CODEC_SRCS})