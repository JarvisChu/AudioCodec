//
// Created by JarvisChu on 2022/4/25.
//

#ifndef PCM_FILE_H
#define PCM_FILE_H

#include <string>
#include <vector>
#include <cstdint>

namespace PCMCodec {

    // PCMFileReader: PCM 音频文件的读取类
    class PCMFileReader{
    public:
        PCMFileReader();
        ~PCMFileReader();

        // Open: 打开PCM文件
        // * pcmFilePath: PCM 文件的路径
        // * 返回值      : 打开文件是否成功
        bool Open(const std::string& pcmFilePath);

        // Open: 打开PCM文件，同时指定PCM的采样参数
        // 只有指定了采样参数，才能使用 ReadDuration 函数
        // * pcmFilePath: PCM 文件的路径
        // * sampleRate : 采样频率
        // * sampleBits : 采样位深
        // * channelCnt : 声道数量
        // * 返回值      : 打开文件是否成功
        bool Open(const std::string& pcmFilePath, uint32_t sampleRate, uint32_t sampleBits, uint16_t channelCnt);

        // ReadBytes: 从PCM文件中读取指定字节数的数据
        // 如果剩余数据不足，则全部读取到bytes中
        // * bytes2Read : 要读取的字节数量
        // * bytes      : 存放读取到的字节数据，会存放在 bytes 中
        // * 返回值      : 实际读取到的字节数
        size_t ReadBytes(uint32_t bytes2Read, std::vector<uint8_t>& bytes);

        // ReadShorts: 从PCM文件中读取指定数量的short类型数据
        // 即按 short 类型读取PCM，并保存到shorts中
        // 如果剩余数据不足，则全部读取到shorts中
        // * shorts2Read : 要读取的short数量
        // * shorts      : 存放读取到的short数据，会存放在 shorts 中
        // * 返回值      : 实际读取到的 short 个数
        size_t ReadShorts(uint32_t shorts2Read, std::vector<uint16_t>& shorts);

        // ReadDuration: 读取指定时长的音频数据
        // 调用此函数时，必须是已指定了PCM的采样参数，即 Open 时指定了采样参数，如果没有，则返回失败
        // 如果剩余数据不足 durationMs，则全部读取到data中，即如果data返回长度为0，则表明全部读取完了
        // * durationMs: 要读取的时长，单位毫秒
        // * data      : 读取到的音频数据，会存放在 data 中
        // * 返回值      : 实际读取到的数据个数，即 uint8_t 或者 uint16_t 的个数
        size_t ReadDuration(uint32_t durationMs, std::vector<uint8_t>& data);
        size_t ReadDuration(uint32_t durationMs, std::vector<uint16_t>& data);

        // SeekToTime: 将文件指针移动到指定的时间处
        // 如果时间超过了文件时长，则移动末尾
        // * tmMs: 要移动的时间点，单位毫秒
        void SeekToTime(uint32_t tmMs);

        // GetFileSize: 获取文件的大小
        long GetFileSize();

        // Close: 关闭PCM文件
        void Close();
    private:
        FILE* m_fp = nullptr;
        long m_fileSize = 0;
        uint32_t m_sampleRate = 0;
        uint32_t m_sampleBits = 0;
        uint16_t m_channelCnt = 0;
    };

    // PCMFileWriter: 写 PCM数据
    class PCMFileWriter{
    public:
        PCMFileWriter();
        ~PCMFileWriter();

        // Open: 打开PCM文件
        // * pcmFilePath: PCM 文件的路径
        // * 返回值      : 打开文件是否成功
        bool Open(const std::string& pcmFilePath);

        // Write: 将数据写入PCM文件
        // 支持各种参数类型
        // * data: 要写入的数据
        // * len : 要写入的长度
        void Write(const uint8_t* data, uint32_t len);
        void Write(const uint16_t* data, uint32_t len);
        void Write(const std::vector<uint8_t>& data);
        void Write(const std::vector<uint8_t>& data, size_t len);
        void Write(const std::vector<uint16_t>& data);
        void Write(const std::vector<uint16_t>& data, size_t len);

        // Close: 关闭PCM文件
        void Close();
    private:
        FILE* m_fp = nullptr;
    };
};

#endif //PCM_FILE_H
