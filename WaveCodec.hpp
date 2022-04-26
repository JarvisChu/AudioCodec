//
// Created by JarvisChu on 2022/4/25.
//

#ifndef WAVECODEC_HPP
#define WAVECODEC_HPP

#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>

#define MAKE_FOURCC(a,b,c,d) \
( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )

namespace WaveCodec {

    // format 子块
    // size = 26
    struct SubChunkFormat {
        uint32_t chunk;           // 固定为fmt ，大端存储
        uint32_t sub_chunk_size;  // 子块的大小，不包含chunk+sub_chunk_size字段
        uint16_t audio_format;    // 编码格式(Audio Format)，1代表PCM无损格式
        uint16_t channels;        // 声道数(Channels)，1或2
        uint32_t sample_rate;     // 采样率(Sample Rate)
        uint32_t byte_rate;       // 传输速率(Byte Rate)，每秒数据字节数，SampleRate * Channels * BitsPerSample / 8
        uint16_t block_align;     // 每个采样所需的字节数BlockAlign，BitsPerSample*Channels / 8
        uint16_t bits_per_sample; // 单个采样位深(Bits Per Sample)，可选8、16或32
        uint16_t ex_size;         // 扩展块的大小，附加块的大小
    };

    // data 子块
    // size = 8
    struct SubChunkData {
        uint32_t chunk;          // 固定为data，大端存储
        uint32_t sub_chunk_size; // 子块的大小，不包含chunk+sub_chunk_size字段
    };

    // Wave 文件头
    // size = 46 = 4 + 4 + 4 + 26 + 8
    struct WaveHeader {
        uint32_t fourcc;     // 固定为RIFF，大端存储，所以需要使用MAKE_FOURCC宏处理
        uint32_t chunk_size; // 文件长度，不包含fourcc和chunk_size，即总文件长度-8字节
        uint32_t form_type;  // 固定为WAVE，大端存储，类型码(Form Type)，WAV文件格式标记，即"WAVE"四个字母
        SubChunkFormat sub_chunk_format; // fmt  子块
        SubChunkData sub_chunk_data;     // data 子块

        WaveHeader(){};

        WaveHeader(uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, uint32_t pcm_len) {
            FormatFrom(sample_rate, sample_bits, channels, pcm_len);
        };

        void FormatFrom(uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, uint32_t pcm_len){
            fourcc = MAKE_FOURCC('R', 'I', 'F', 'F');
            chunk_size = 38 + pcm_len; // 38 = 46 (header size) - 8(sizeof chunk + sizeof chunk_size)
            form_type = MAKE_FOURCC('W', 'A', 'V', 'E');

            sub_chunk_format.chunk = MAKE_FOURCC('f', 'm', 't', ' ');
            sub_chunk_format.sub_chunk_size = 18;
            sub_chunk_format.audio_format = 1;
            sub_chunk_format.channels = channels;
            sub_chunk_format.sample_rate = sample_rate;
            sub_chunk_format.byte_rate = sample_rate*channels*sample_bits / 8;
            sub_chunk_format.block_align = channels*sample_bits / 8;
            sub_chunk_format.bits_per_sample = sample_bits;
            sub_chunk_format.ex_size = 0;

            sub_chunk_data.chunk = MAKE_FOURCC('d', 'a', 't', 'a');
            sub_chunk_data.sub_chunk_size = pcm_len;
        }

        void ToBuffer(uint8_t buffer[46]){
            uint8_t* p = buffer;

            // wave header
            memcpy(p, &fourcc, sizeof(fourcc));
            p += sizeof(fourcc);

            memcpy(p, &chunk_size, sizeof(chunk_size));
            p += sizeof(chunk_size);

            memcpy(p, &form_type, sizeof(form_type));
            p += sizeof(form_type);

            // sub_chunk_format
            memcpy(p, &sub_chunk_format.chunk, sizeof(sub_chunk_format.chunk));
            p += sizeof(sub_chunk_format.chunk);

            memcpy(p, &sub_chunk_format.sub_chunk_size, sizeof(sub_chunk_format.sub_chunk_size));
            p += sizeof(sub_chunk_format.sub_chunk_size);

            memcpy(p, &sub_chunk_format.audio_format, sizeof(sub_chunk_format.audio_format));
            p += sizeof(sub_chunk_format.audio_format);

            memcpy(p, &sub_chunk_format.channels, sizeof(sub_chunk_format.channels));
            p += sizeof(sub_chunk_format.channels);

            memcpy(p, &sub_chunk_format.sample_rate, sizeof(sub_chunk_format.sample_rate));
            p += sizeof(sub_chunk_format.sample_rate);

            memcpy(p, &sub_chunk_format.byte_rate, sizeof(sub_chunk_format.byte_rate));
            p += sizeof(sub_chunk_format.byte_rate);

            memcpy(p, &sub_chunk_format.block_align, sizeof(sub_chunk_format.block_align));
            p += sizeof(sub_chunk_format.block_align);

            memcpy(p, &sub_chunk_format.bits_per_sample, sizeof(sub_chunk_format.bits_per_sample));
            p += sizeof(sub_chunk_format.bits_per_sample);

            memcpy(p, &sub_chunk_format.ex_size, sizeof(sub_chunk_format.ex_size));
            p += sizeof(sub_chunk_format.ex_size);

            // sub_chunk_data
            memcpy(p, &sub_chunk_data.chunk, sizeof(sub_chunk_data.chunk));
            p += sizeof(sub_chunk_data.chunk);

            memcpy(p, &sub_chunk_data.sub_chunk_size, sizeof(sub_chunk_data.sub_chunk_size));
        }

        void ToBuffer(std::vector<uint8_t>& bufferOut){
            uint8_t buffer[46];
            ToBuffer(buffer);
            bufferOut.insert(bufferOut.end(), buffer, buffer + 46);
        }
    };

    // Wave 文件
    // TODO  wave file Encoder
    // TODO  wave file Decoder
    class WaveFile {
    public:
        WaveFile(){ m_fp = nullptr; };
        ~WaveFile(){ Close(); };

        // 打开 Wave 文件
        bool Open(const std::string& path, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels){
            if (path.size() == 0) return false;
            if (m_fp) return false;
            m_fp = fopen(path.c_str(), "wb");
            if(!m_fp){
                printf("open wave file failed, %s\n", path.c_str());
                return false;
            }

            InitHeader(sample_rate, sample_bits, channels);
            return true;
        };

        // 写数据到 Wave 文件
        void Write(const std::vector<unsigned char>& data, size_t size){
            if (m_fp && size > 0 && data.size() >= size) {
                fwrite(&data[0], sizeof(unsigned char), size, m_fp);
                m_len += size;
            }
        };

        // 写数据到 Wave 文件
        void Write(const unsigned char* data, size_t size){
            if (m_fp && data && size > 0) {
                fwrite(data, sizeof(unsigned char), size, m_fp);
                m_len += size;
            }
        };

        // 关闭 Wave 文件
        void Close(){
            if (m_fp) {
                // 回填长度字段
                m_header.chunk_size = 38 + m_len; // 38 = 46 (header size) - 8(sizeof chunk+ sizeof chunk_size)
                fseek(m_fp, 4, SEEK_SET);
                fwrite(&m_header.chunk_size, sizeof(char), sizeof(m_header.chunk_size), m_fp);

                m_header.sub_chunk_data.sub_chunk_size = m_len;
                fseek(m_fp, 46 - 4, SEEK_SET);
                fwrite(&m_header.sub_chunk_data.sub_chunk_size, sizeof(char), sizeof(m_header.sub_chunk_data.sub_chunk_size), m_fp);

                fclose(m_fp);
                m_fp = nullptr;
            }
        };
    private:
        void InitHeader(uint32_t sample_rate, uint16_t sample_bits, uint16_t channels){
            if (!m_fp) return;
            m_header.FormatFrom(sample_rate, sample_bits, channels, 0); // PCM数据长度先填 0，等待文件写完之后，再回填
            uint8_t headerBuffer[46];
            m_header.ToBuffer(headerBuffer);
            fwrite(headerBuffer, sizeof(uint8_t), 46, m_fp);
        };

    private:
        WaveHeader m_header;
        FILE* m_fp = nullptr;
        uint32_t m_len = 0;
    };

    // PCM 文件转 Wave 文件
    bool PCM2WaveFile(const std::string& pcmFilePath, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, const std::string& waveFilePath){
        WaveFile waveFile;
        if(!waveFile.Open(waveFilePath, sample_rate, sample_bits, channels)){
            return false;
        }

        FILE* fpPCM = fopen(pcmFilePath.c_str(), "rb");
        if(!fpPCM){
            printf("open pcm file failed\n");
            return false;
        }

        while (true) {
            uint8_t buff[2048] = {0};
            int read_cnt = fread(buff, sizeof(uint8_t), 2048, fpPCM);
            if (read_cnt <= 0) {
                break;
            }
            waveFile.Write(buff, read_cnt);
        }

        fclose(fpPCM);
        waveFile.Close();
        return true;
    }

    // PCM 数据转换为 Wave 数据 (携带 Wave 文件头)
    void PCMToWaveBuffer(const uint8_t* pcmBuffer, uint32_t pcmBufferSize, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, std::vector<uint8_t>& waveBufferOut){
        WaveHeader header(sample_rate, sample_bits, channels, pcmBufferSize);
        uint8_t headerBuffer[46];
        header.ToBuffer(headerBuffer);

        waveBufferOut.insert(waveBufferOut.end(), headerBuffer, headerBuffer + 46);
        waveBufferOut.insert(waveBufferOut.end(), pcmBuffer, pcmBuffer + pcmBufferSize);
    }

    void PCMToWaveBuffer(const std::vector<uint8_t> pcmBuffer, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, std::vector<uint8_t>& waveBufferOut){
        PCMToWaveBuffer((const uint8_t*) &pcmBuffer[0], (uint32_t) pcmBuffer.size(), sample_rate, sample_bits, channels, waveBufferOut);
    }

    // Wave 文件转 PCM 文件，同时返回音频的编码参数
    bool Wave2PCMFile(const std::string& waveFilePath, const std::string& pcmFilePath, uint32_t& sample_rate_out, uint16_t& sample_bits_out, uint16_t& channels_out){
        FILE *fpWav = fopen(waveFilePath.c_str(), "rb");
        if (!fpWav) {
            printf("open wave file failed, %s\n", waveFilePath.c_str());
            return false;
        }

        FILE *fpPCM = fopen(pcmFilePath.c_str(), "wb");
        if (!fpPCM) {
            printf("open pcm file failed, %s\n", pcmFilePath.c_str());
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        // parse wave file

        // Get pcm file size
        fseek(fpWav, 0L, SEEK_END);
        long file_size = ftell(fpWav);
        fseek(fpWav, 0L, SEEK_SET);

        // Chunk RIFF
        uint8_t fourcc[4];
        size_t read_cnt = fread(fourcc, sizeof(uint8_t), 4, fpWav);
        if (fourcc[0] != 'R' || fourcc[1] != 'I' || fourcc[2] != 'F' || fourcc[3] != 'F') {
            printf("invalid wave file, %s\n", waveFilePath.c_str());
            return false;
        }

        // Chunk RIFF size
        uint32_t riff_chunk_size = 0;
        read_cnt = fread(&riff_chunk_size, sizeof(uint32_t), 1, fpWav);
        if (read_cnt < 1){
            printf("invalid wave file, %s\n", waveFilePath.c_str());
            return false;
        }

        if ((uint32_t)file_size != riff_chunk_size + sizeof(fourcc) + sizeof(riff_chunk_size)) {
            printf("invalid wave file, size not match, error ignored, %s\n", waveFilePath.c_str());
            //return false;
        }

        // Chunk RIFF form type
        uint8_t form_type[4];
        read_cnt = fread(form_type, sizeof(uint8_t), 4, fpWav);
        if (form_type[0] != 'W' || form_type[1] != 'A' || form_type[2] != 'V' || form_type[3] != 'E') {
            printf("invalid wave file, %s\n", waveFilePath.c_str());
            return false;
        }

        // 遍历所有的 sub chunk
        while (true) {

            // sub chunk name
            uint8_t sub_chunk_name[4];
            read_cnt = fread(sub_chunk_name, sizeof(uint8_t), 4, fpWav);
            if (read_cnt < 4) {
                printf("read sub chunk failed\n");
                return false;
            }
            printf("sub chunk found: %c%c%c%c\n", sub_chunk_name[0], sub_chunk_name[1], sub_chunk_name[2], sub_chunk_name[3]);

            // sub chunk size
            uint32_t sub_chunk_size = 0;
            read_cnt = fread(&sub_chunk_size, sizeof(uint32_t), 1, fpWav);
            if (read_cnt < 1) {
                printf("invalid wave file, %s\n", waveFilePath.c_str());
                return false;
            }
            printf("sub chunk size: %d\n", sub_chunk_size);

            // handle sub chunk

            // data sub chunk => get pcm data
            if (sub_chunk_name[0] == 'd' && sub_chunk_name[1] == 'a' && sub_chunk_name[2] == 't' && sub_chunk_name[3] == 'a') {

                while (sub_chunk_size > 0) {
                    uint8_t buff[2048];
                    int len = (sub_chunk_size <= 2048) ? sub_chunk_size:2048;
                    read_cnt = fread(buff, sizeof(uint8_t), len, fpWav);
                    if (read_cnt != len) {
                        printf("invalid wave file, error ignored, %s\n", waveFilePath.c_str());
                        fwrite(buff, sizeof(uint8_t), read_cnt, fpPCM);
                        break;
                    }

                    fwrite(buff, sizeof(uint8_t), len, fpPCM);
                    sub_chunk_size -= len;
                }

                // data sub chunk is the last sub chunk, stop reading
                break;
            }

            // fmt sub chunk => get sample_rate/sample_bits/channels
            else if(sub_chunk_name[0] == 'f' && sub_chunk_name[1] == 'm' && sub_chunk_name[2] == 't' && sub_chunk_name[3] == ' '){
                uint16_t audio_format = 0; // 编码格式(Audio Format)，1代表PCM无损格式
                read_cnt = fread(&audio_format, sizeof(uint16_t), 1, fpWav);
                if (read_cnt != 1) {
                    printf("invalid wave file, %s\n", waveFilePath.c_str());
                    return false;
                }

                if(audio_format != 1) {
                    printf("invalid wave file, not pcm, audio_format:%d, %s\n", audio_format, waveFilePath.c_str());
                    return false;
                }

                uint16_t channels = 0; // 声道数(Channels)，1或2
                read_cnt = fread(&channels, sizeof(uint16_t), 1, fpWav);
                if (read_cnt != 1) {
                    printf("invalid wave file, %s\n", waveFilePath.c_str());
                    return false;
                }
                channels_out = channels;

                uint32_t sample_rate = 0;     // 采样率(Sample Rate)
                read_cnt = fread(&sample_rate, sizeof(uint32_t), 1, fpWav);
                if (read_cnt != 1) {
                    printf("invalid wave file, %s\n", waveFilePath.c_str());
                    return false;
                }
                sample_rate_out = sample_rate;

                uint32_t byte_rate = 0;       // 传输速率(Byte Rate)，每秒数据字节数，SampleRate * Channels * BitsPerSample / 8
                read_cnt = fread(&byte_rate, sizeof(uint32_t), 1, fpWav);
                if (read_cnt != 1) {
                    printf("invalid wave file, %s\n", waveFilePath.c_str());
                    return false;
                }

                uint16_t block_align = 0;     // 每个采样所需的字节数BlockAlign，BitsPerSample*Channels / 8
                read_cnt = fread(&block_align, sizeof(uint16_t), 1, fpWav);
                if (read_cnt != 1) {
                    printf("invalid wave file, %s\n", waveFilePath.c_str());
                    return false;
                }

                uint16_t bits_per_sample = 0; // 单个采样位深(Bits Per Sample)，可选8、16或32
                read_cnt = fread(&block_align, sizeof(uint16_t), 1, fpWav);
                if (read_cnt != 1) {
                    printf("invalid wave file, %s\n", waveFilePath.c_str());
                    return false;
                }
                sample_bits_out = bits_per_sample;

                // skip remain bytes of this sub chunk
                uint32_t left = sub_chunk_size - 16;
                if(left > 0){
                    fseek(fpWav, left, SEEK_CUR);
                }
            }

            // other sub chunk, skip it
            else {
                fseek(fpWav, sub_chunk_size, SEEK_CUR);
            }
        }

        fclose(fpWav);
        fclose(fpPCM);
        printf("convert %s to %s success!\n", waveFilePath.c_str(), pcmFilePath.c_str());
        return true;
    }

    bool Wave2PCMFile(const std::string& waveFilePath, const std::string& pcmFilePath) {
        uint32_t sample_rate_out;
        uint16_t sample_bits_out;
        uint16_t channels_out;

        return Wave2PCMFile(waveFilePath, pcmFilePath, sample_rate_out, sample_bits_out, channels_out);
    }
}

#endif //WAVECODEC_HPP
