//
// Created by JarvisChu on 2022/4/25.
//

#include "PCMFile.h"

namespace PCMCodec {
    ///////////////////////////////////////////////////
    // PCMFileReader
    PCMFileReader::PCMFileReader() {}

    PCMFileReader::~PCMFileReader() {
        Close();
    }

    bool PCMFileReader::Open(const std::string& pcmFilePath){
        if (pcmFilePath.size() == 0) return false;
        if (m_fp) return false;

        m_fp = fopen(pcmFilePath.c_str(), "rb");
        if(!m_fp){
            printf("open file failed\n");
            return false;
        }

        fseek(m_fp, 0L, SEEK_END);
        m_fileSize = ftell(m_fp);
        fseek(m_fp, 0L, SEEK_SET);
        return true;
    }

    bool PCMFileReader::Open(const std::string& pcmFilePath, uint32_t sampleRate, uint32_t sampleBits, uint16_t channelCnt){
        if(!Open(pcmFilePath)) {
            return false;
        }

        m_sampleRate = sampleRate;
        m_sampleBits = sampleBits;
        m_channelCnt = channelCnt;
        return true;
    }

    size_t PCMFileReader::ReadBytes(uint32_t bytes2Read, std::vector<uint8_t>& bytes){
        if(!m_fp) return 0;

        bytes.resize(bytes2Read);
        size_t nRead = fread(&bytes[0], sizeof(uint8_t), bytes2Read, m_fp);
        if(nRead < bytes2Read){
            bytes.resize(nRead);
        }
        return nRead;
    }

    size_t PCMFileReader::ReadShorts(uint32_t shorts2Read, std::vector<uint16_t>& shorts){
        if(!m_fp) return 0;

        shorts.resize(shorts2Read);
        size_t nRead = fread(&shorts[0], sizeof(uint16_t), shorts2Read, m_fp);
        if(nRead < shorts2Read){
            shorts.resize(nRead);
        }
        return nRead;
    }

    size_t PCMFileReader::ReadDuration(uint32_t durationMs, std::vector<uint8_t>& data){
        if(!m_fp) return 0;
        if(m_sampleRate == 0 || m_sampleBits == 0 || m_channelCnt == 0) return 0;

        uint32_t bytesPerMs = (m_sampleRate * m_sampleBits/8 * m_channelCnt) / 1000; // 每 ms 的字节数
        uint32_t bytesPerDuration = bytesPerMs * durationMs;
        return ReadBytes(bytesPerDuration, data);
    }

    size_t PCMFileReader::ReadDuration(uint32_t durationMs, std::vector<uint16_t>& data){
        if(!m_fp) return 0;
        if(m_sampleRate == 0 || m_sampleBits == 0 || m_channelCnt == 0) return 0;

        uint32_t shortsPerMs = (m_sampleRate * m_sampleBits/16 * m_channelCnt) / 1000; // 每 ms 的 short 个数
        uint32_t shortsPerDuration = shortsPerMs * durationMs;
        return ReadShorts(shortsPerDuration, data);
    }

    void PCMFileReader::SeekToTime(uint32_t tmMs){
        long bytesPerMs = (m_sampleRate * m_sampleBits/8 * m_channelCnt) / 1000; // 每 ms 的字节数
        long bytesAll = bytesPerMs * tmMs; // tmMs 时刻的字节数
        if(bytesAll >= m_fileSize){
            fseek(m_fp, 0, SEEK_END); // 超过了文件时长，直接移动到末尾
        }else{
            fseek(m_fp, bytesAll, SEEK_SET);
        }
    }

    long PCMFileReader::GetFileSize(){
        return m_fileSize;
    }

    void PCMFileReader::Close(){
        if(m_fp){
            fclose(m_fp);
            m_fp = nullptr;
            m_fileSize = 0;
            m_sampleRate = 0;
            m_sampleBits = 0;
            m_channelCnt = 0;
        }
    }

    ///////////////////////////////////////////////////
    // PCMFileWriter
    PCMFileWriter::PCMFileWriter(){}
    PCMFileWriter::~PCMFileWriter(){
        Close();
    }

    bool PCMFileWriter::Open(const std::string& pcmFilePath){
        if (pcmFilePath.size() == 0) return false;
        if (m_fp) return false;

        m_fp = fopen(pcmFilePath.c_str(), "wb");
        if(!m_fp){
            printf("open file failed\n");
            return false;
        }

        return true;
    }

    void PCMFileWriter::Write(const uint8_t* data, uint32_t len){
        if(!m_fp) return;
        if(!data) return;

        fwrite(data, sizeof(uint8_t), len, m_fp);
    }

    void PCMFileWriter::Write(const uint16_t* data, uint32_t len){
        if(!m_fp) return;
        if(!data) return;

        fwrite(data, sizeof(uint16_t), len, m_fp);
    }

    void PCMFileWriter::Write(const std::vector<uint8_t>& data){
        Write(&data[0], data.size());
    }

    void PCMFileWriter::Write(const std::vector<uint8_t>& data, size_t len){
        Write(&data[0], len);
    }

    void PCMFileWriter::Write(const std::vector<uint16_t>& data){
        Write(&data[0], data.size());
    }

    void PCMFileWriter::Write(const std::vector<uint16_t>& data, size_t len){
        Write(&data[0], len);
    }

    void PCMFileWriter::Close(){
        if(m_fp){
            fclose(m_fp);
            m_fp = nullptr;
        }
    }

}
