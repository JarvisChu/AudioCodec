cmake_minimum_required(VERSION 3.19)
project(PCMCodec)

aux_source_directory(. PCM_CODEC_SRCS)
add_library(${PROJECT_NAME} STATIC ${PCM_CODEC_SRCS})