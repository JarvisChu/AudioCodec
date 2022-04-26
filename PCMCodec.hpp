//
// Created by JarvisChu on 2022/4/25.
//

#ifndef PCMCODEC_HPP
#define PCMCODEC_HPP

#include <string>
#include <vector>
#include <cstdint>
#include "WaveCodec.hpp"

namespace PCMCodec {
    // 降采样
    int DownSampling();

    // 取特定通道
    int AbstractChannel();

    // 混音
    int Mixing();
};

#endif //PCMCODEC_HPP
