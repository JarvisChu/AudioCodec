//
// Created by JarvisChu on 2022/4/25.
//


#include "PCMCodec.h"

namespace PCMCodec {
    void AbstractChannel(const uint8_t *pcmBuffer, uint32_t pcmBufferSize, std::vector<uint8_t>& leftChannelOut, std::vector<uint8_t>& rightChannelOut) {
        bool leftFlag = true;
        const uint8_t* p = pcmBuffer;
        size_t index = 0;

        while(index + 2 < pcmBufferSize){
            if(leftFlag){
                leftChannelOut.insert(leftChannelOut.end(), p + index, p+index+2);
                leftFlag = false;
            }else{
                rightChannelOut.insert(rightChannelOut.end(), p + index, p+index+2);
                leftFlag = true;
            }

            index += 2;
        }
    }

    void AbstractChannel(const std::vector<uint8_t>& pcmBuffer, std::vector<uint8_t>& leftChannelOut, std::vector<uint8_t>& rightChannelOut){
        AbstractChannel((const uint8_t*) &pcmBuffer[0], pcmBuffer.size(), leftChannelOut, rightChannelOut);
    }

    void AbstractChannel(const uint16_t *pcmBuffer, uint32_t pcmBufferSize, std::vector<uint16_t>& leftChannelOut, std::vector<uint16_t>& rightChannelOut) {
        bool leftFlag = true;
        const uint16_t* p = pcmBuffer;
        size_t index = 0;

        while(index + 1 < pcmBufferSize){
            if(leftFlag){
                leftChannelOut.insert(leftChannelOut.end(), p + index, p+index+1);
                leftFlag = false;
            }else{
                rightChannelOut.insert(rightChannelOut.end(), p + index, p+index+1);
                leftFlag = true;
            }

            index ++;
        }
    }

    void AbstractChannel(const std::vector<uint16_t>& pcmBuffer, std::vector<uint16_t>& leftChannelOut, std::vector<uint16_t>& rightChannelOut){
        AbstractChannel((const uint16_t*) &pcmBuffer[0], pcmBuffer.size(), leftChannelOut, rightChannelOut);
    }

    bool AbstractChannel2File(const std::string& srcPCMFilePath, const std::string& leftPCMFilePath, const std::string& rightPCMFilePath){
        FILE* fpSrc = fopen(srcPCMFilePath.c_str(), "rb");
        if(!fpSrc){
            printf("open src pcm file failed\n");
            return false;
        }

        FILE* fpLeft = nullptr;
        if(leftPCMFilePath.size() > 0){
            fpLeft = fopen(leftPCMFilePath.c_str(), "wb");
            if(!fpLeft){
                printf("open left pcm file failed\n");
                return false;
            }
        }

        FILE* fpRight = nullptr;
        if(rightPCMFilePath.size() > 0){
            fpRight = fopen(rightPCMFilePath.c_str(), "wb");
            if(!fpRight){
                printf("open right pcm file failed\n");
                return false;
            }
        }

        // 如果左右声道都不用存文件，则直接返回
        if(!fpLeft && !fpRight) {
            fclose(fpSrc);
            return true;
        }

        std::vector<uint8_t> leftChannel;
        std::vector<uint8_t> rightChannel;
        while(true){
            uint8_t buff[2048] = {0};
            int read_cnt = fread(buff, sizeof(uint8_t), 2048, fpSrc);
            if (read_cnt <= 0) {
                break;
            }

            leftChannel.clear();
            leftChannel.reserve(1024);
            rightChannel.clear();
            rightChannel.reserve(1024);
            AbstractChannel(buff, read_cnt, leftChannel, rightChannel);

            if(fpLeft) fwrite((const uint8_t*) &leftChannel[0], sizeof(uint8_t), leftChannel.size(), fpLeft);
            if(fpRight) fwrite((const uint8_t*) &rightChannel[0], sizeof(uint8_t), rightChannel.size(), fpRight);
        }

        fclose(fpSrc);
        if(fpLeft) fclose(fpLeft);
        if(fpRight) fclose(fpRight);

        return true;
    }
}
