//
// Created by JarvisChu on 2022/4/25.
//

#ifndef WAVE_FILE_H_
#define WAVE_FILE_H_

#include <cstdint>
#include <string>
#include <vector>

#define MAKE_FOURCC(a,b,c,d) ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#define CPY_FIELD(dst, field) { \
    memcpy(dst, &field, sizeof(field));\
    dst += sizeof(field);\
}

#define WaveAudioFormatUnknown   0 // unknown format
#define WaveAudioFormatPCM       1 // PCM                       [fmt chunk size: 16, no  fact chunk]
#define WaveAudioFormatMSADPCM   2 // Microsoft ADPCM           [fmt chunk size: 18, has fact chunk]
#define WaveAudioFormatIeeeFloat 3 // IEEE float.               [fmt chunk size: 18, has fact chunk]
#define WaveAudioFormatALaw      6 // 8-bit ITU-T G.711 A-law.  [fmt chunk size: 18, has fact chunk] 欧洲和其它
#define WaveAudioFormatMuLaw     7 // 8-bit ITU-T G.711 mu-law. [fmt chunk size: 18, has fact chunk] 北美日本
#define WaveAudioFormatGSM      49 // GSM 6.10.                 [fmt chunk size: 20, has fact chunk]
#define WaveAudioFormatG721     64 // ITU G.721 ADPCM           [fmt chunk size: 20, has fact chunk]

// 使用扩展区中的sub_format来决定音频的数据的编码方式。在以下几种情况下必须要使用 WAVE_FORMAT_EXTENSIBLE
// - PCM数据的量化位数大于16
// - 音频的采样声道大于2
// - 实际的量化位数不是8的倍数
// - 存储顺序和播放顺序不一致，需要指定从声道顺序到声卡播放顺序的映射情况
#define WaveAudioFormatExtensible 65534 // 0xFFFE

namespace WaveCodec {

    // Wave Header 相关定义
    struct ChunkHeader {
        uint32_t fourcc; // 块id，大端存储，如 "RIFF", "fmt ", "fact", "data"
        uint32_t size;   // 块大小，不包含 fourcc 和 size字段
    };

    // fmt 子块
    struct SubChunkFmt {
        ChunkHeader header;          // fourcc 固定为 "fmt "
        uint16_t    audio_format;    // 编码格式(Audio Format)，详见 WaveAudioFormat 宏定义
        uint16_t    channels;        // 声道数(Channels)，1或2
        uint32_t    sample_rate;     // 采样率(Sample Rate)
        uint32_t    byte_rate;       // 传输速率(Byte Rate)，每秒数据字节数，SampleRate * Channels * BitsPerSample / 8
        uint16_t    block_align;     // 每个采样所需的字节数BlockAlign，BitsPerSample*Channels / 8
        uint16_t    bits_per_sample; // 单个采样位深(Bits Per Sample)，可选8、16或32
        uint16_t    ex_size;         // [可选] 扩展块的大小，附加块的大小【PCM 格式时，无该字段】
    };

    // [可选] fact 子块
    struct SubChunkFact {
        ChunkHeader header;  // fourcc 固定为 "fact"
        uint32_t    samples; // 采样总数 (每个声道)
    };

    // data 子块，最后一个子块
    struct SubChunkData {
        ChunkHeader header; // fourcc 固定为 "data"
    };

    // riff 块，Wave文件本质就是一个 RIFF Chunk
    struct RIFFChunk{
        ChunkHeader header;  // fourcc 固定为 "RIFF"
        uint32_t form_type;  // 固定为WAVE，大端存储，类型码(Form Type)，WAV文件格式标记，即"WAVE"四个字母

        // RIFF Chunk 可以包含多个 Sub Chunk
        SubChunkFmt  fmt;  // fmt  子块，必选
        SubChunkFact fact; // fact 子块，[可选]，采用压缩编码的WAV文件，必须要有 fact chunk，该块中只有一个数据，为每个声道的采样总数
        SubChunkData data; // data 子块，必选
    };

    // Wave 文件头
    struct WaveHeader {
        RIFFChunk riff; // Wave文件本质就是一个 RIFF Chunk

        WaveHeader(){};

        int GetHeaderSize(){
            if(riff.fmt.audio_format == WaveAudioFormatPCM) {
                return 44;
            }
            return 58; // 58 = 44 + 2 (fmt.ex_size) + 12(fact)
        }

        void FormatPCMWaveHeader(uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, uint32_t data_len){
            // riff
            riff.header.fourcc = MAKE_FOURCC('R', 'I', 'F', 'F');
            riff.header.size = 36 + data_len; // 36 = 44 (header size) - 8(sizeof chunk + sizeof chunk_size)
            riff.form_type = MAKE_FOURCC('W', 'A', 'V', 'E');

            // fmt
            riff.fmt.header.fourcc = MAKE_FOURCC('f', 'm', 't', ' ');
            riff.fmt.header.size = 16; // no ex_size field
            riff.fmt.audio_format = WaveAudioFormatPCM;
            riff.fmt.channels = channels;
            riff.fmt.sample_rate = sample_rate;
            riff.fmt.byte_rate = sample_rate*channels*sample_bits / 8;
            riff.fmt.block_align = channels*sample_bits / 8;
            riff.fmt.bits_per_sample = sample_bits;

            // no need riff.fmt.ex_size
            // no need fact sub trunk

            // data
            riff.data.header.fourcc = MAKE_FOURCC('d', 'a', 't', 'a');
            riff.data.header.size   = data_len;
        }

        // audio_format: only support WaveAudioFormatALaw/WaveAudioFormatMuLaw
        void FormatG711WaveHeader(uint16_t audio_format, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, uint32_t data_len){
            // riff
            riff.header.fourcc = MAKE_FOURCC('R', 'I', 'F', 'F');
            riff.header.size = 38 + data_len; // 38 = 46 (header size) - 8(sizeof chunk + sizeof chunk_size)
            riff.form_type = MAKE_FOURCC('W', 'A', 'V', 'E');

            // fmt
            riff.fmt.header.fourcc = MAKE_FOURCC('f', 'm', 't', ' ');
            riff.fmt.header.size = 18; // has ex_size field
            riff.fmt.audio_format = WaveAudioFormatPCM;
            riff.fmt.channels = channels;
            riff.fmt.sample_rate = sample_rate;
            riff.fmt.byte_rate = sample_rate*channels*sample_bits / 8;
            riff.fmt.block_align = channels*sample_bits / 8;
            riff.fmt.bits_per_sample = sample_bits;
            riff.fmt.ex_size = 0;

            // fact
            riff.fact.header.fourcc = MAKE_FOURCC('f', 'a', 'c', 't');
            riff.fact.header.size = 4;
            riff.fact.samples = data_len / (channels * sample_bits/8);

            // data
            riff.data.header.fourcc = MAKE_FOURCC('d', 'a', 't', 'a');
            riff.data.header.size   = data_len;
        }

        void ToBuffer(std::vector<uint8_t>& bufferOut){
            bufferOut.resize(GetHeaderSize());
            uint8_t *p = &bufferOut[0];

            // riff
            CPY_FIELD(p, riff.header.fourcc);
            CPY_FIELD(p, riff.header.size);
            CPY_FIELD(p, riff.form_type);

            // fmt
            CPY_FIELD(p, riff.fmt.header.fourcc);
            CPY_FIELD(p, riff.fmt.header.size);
            CPY_FIELD(p, riff.fmt.audio_format);
            CPY_FIELD(p, riff.fmt.channels);
            CPY_FIELD(p, riff.fmt.sample_rate);
            CPY_FIELD(p, riff.fmt.byte_rate);
            CPY_FIELD(p, riff.fmt.block_align);
            CPY_FIELD(p, riff.fmt.bits_per_sample);

            if(riff.fmt.audio_format != WaveAudioFormatPCM){
                CPY_FIELD(p, riff.fmt.ex_size);
            }

            // fact
            if(riff.fmt.audio_format != WaveAudioFormatPCM){
                CPY_FIELD(p, riff.fact.header.fourcc);
                CPY_FIELD(p, riff.fact.header.size);
                CPY_FIELD(p, riff.fact.samples);
            }

            // data
            CPY_FIELD(p, riff.data.header.fourcc)
            CPY_FIELD(p, riff.data.header.size)
        }
    };

    // GetWaveAudioFormatString: 获取 Wave 音频格式的描述
    // * audio_format: Wave音频格式的枚举
    // * 返回值       : audio_format 对于的描述
    std::string GetWaveAudioFormatString(uint16_t audio_format);

    class WaveFileReader{
    public:
        WaveFileReader();
        ~WaveFileReader();

        // Open: 打开Wave文件
        // * waveFilePath: Wave 文件的路径
        // * 返回值       : 打开文件是否成功
        bool Open(const std::string& waveFilePath);

        // GetWaveHeader: 获取 wave 的 header信息
        // * header: 用于返回 wave header 信息
        // * 返回值 : 如果wave格式不合法，返回false，否则返回true
        bool ReadWaveHeader(WaveHeader& header);

        // ReadBytes: 从Wave文件中读取指定字节数的音频数据（不包含WaveHeader）
        // 如果剩余数据不足，则全部读取到bytes中
        // * bytes2Read : 要读取的字节数量
        // * bytes      : 存放读取到的字节数据，会存放在 bytes 中
        // * 返回值      : 实际读取到的字节数
        size_t ReadBytes(uint32_t bytes2Read, std::vector<uint8_t>& bytes);

        // ReadShorts: 从Wave文件中读取指定数量的short类型音频数据（不包含WaveHeader）
        // 即按 short 类型读取Wave，并保存到shorts中
        // 如果剩余数据不足，则全部读取到shorts中
        // * shorts2Read : 要读取的short数量
        // * shorts      : 存放读取到的short数据，会存放在 shorts 中
        // * 返回值      : 实际读取到的 short 个数
        size_t ReadShorts(uint32_t shorts2Read, std::vector<uint16_t>& shorts);

        // ReadDuration: 读取指定时长的音频数据，仅支持 PCM/ALaw/ULaw 格式，其它格式返回-1
        // 如果剩余数据不足 durationMs，则全部读取到data中，即如果data返回长度为0，则表明全部读取完了
        // * durationMs: 要读取的时长，单位毫秒
        // * data      : 读取到的音频数据，会存放在 data 中
        // * 返回值     : 实际读取到的数据个数，即 uint8_t 或者 uint16_t 的个数。出错返回-1
        size_t ReadDuration(uint32_t durationMs, std::vector<uint8_t>& data);
        size_t ReadDuration(uint32_t durationMs, std::vector<uint16_t>& data);

        // Close: 关闭PCM文件
        void Close();
    private:
        FILE* m_fp = nullptr;
        WaveHeader m_header;
    };

    class WaveFileWriter{
    public:
        WaveFileWriter();
        ~WaveFileWriter();

        // Open: 打开Wave文件
        // * waveFilePath: Wave 文件的路径
        // * audio_format: Wave文件的音频格式，目前仅支持WaveAudioFormatPCM/WaveAudioFormatALaw/WaveAudioFormatMuLaw
        // * sample_rate : 采样率
        // * sample_bits : 采样位深，仅支持8/16/32
        // * channels    : 声道数，仅支持1/2
        // * 返回值       : 打开文件是否成功
        bool Open(const std::string& waveFilePath, uint16_t audio_format, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels);

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

    private:
        FILE* m_fp = nullptr;
        WaveHeader m_header;
        uint32_t m_data_len;
    };

    // PCM 文件转 Wave 文件
    bool PCM2WaveFile(const std::string& pcmFilePath, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, const std::string& waveFilePath);

    // Wave 文件转 PCM 文件，同时返回音频的编码参数
    bool Wave2PCMFile(const std::string& waveFilePath, const std::string& pcmFilePath, uint32_t& sample_rate_out, uint16_t& sample_bits_out, uint16_t& channels_out);

    bool Wave2PCMFile(const std::string& waveFilePath, const std::string& pcmFilePath);
}

#endif //WAVE_FILE_H_
