//
// Created by JarvisChu on 2022/4/25.
//

#ifndef PCM_CODEC_H
#define PCM_CODEC_H

#include <string>
#include <vector>
#include <cstdint>

namespace PCMCodec {
    // AbstractChannel: 从 16bits 双声道的PCM数据中，分离出左右声道数据。
    // 支持各种参数形式：指针或者vector，uint8_t 或者 uint16_t
    // * pcmBuffer       : 原始的PCM数据，必须是 16bit 双声道的PCM
    // * pcmBufferSize   : pcmBuffer为指针时，表明原始PCM数据的长度
    // * leftChannelOut  : 分离出的左声道数据
    // * rightChannelOut : 分离出的右声道数据
    void AbstractChannel(const uint8_t *pcmBuffer, uint32_t pcmBufferSize, std::vector<uint8_t>& leftChannelOut, std::vector<uint8_t>& rightChannelOut);
    void AbstractChannel(const std::vector<uint8_t>& pcmBuffer, std::vector<uint8_t>& leftChannelOut, std::vector<uint8_t>& rightChannelOut);
    void AbstractChannel(const uint16_t *pcmBuffer, uint32_t pcmBufferSize, std::vector<uint16_t>& leftChannelOut, std::vector<uint16_t>& rightChannelOut);
    void AbstractChannel(const std::vector<uint16_t>& pcmBuffer, std::vector<uint16_t>& leftChannelOut, std::vector<uint16_t>& rightChannelOut);

    // AbstractChannel2File: 从 16bits 双声道的PCM文件中，分离出左右声道数据，并保存到文件中
    // * srcPCMFilePath   : 原始的PCM音频文件路径
    // * leftPCMFilePath  : 分离出的左声道数据要保存到的文件路径，空表示不保存左声道
    // * rightPCMFilePath : 分离出的右声道数据要保存到的文件路径，空表示不保存右声道
    bool AbstractChannel2File(const std::string& srcPCMFilePath, const std::string& leftPCMFilePath, const std::string& rightPCMFilePath);

    // 混音，TODO
    int Mixing();

    // 降采样，TODO
    int DownSampling();
};

#endif //PCM_CODEC_H
