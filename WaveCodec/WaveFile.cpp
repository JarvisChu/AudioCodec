//
// Created by JarvisChu on 2022/4/25.
//

#include "WaveFile.h"

#define READ_AND_CHECK(ptr, size, cnt, fp) { \
      size_t read_cnt = fread(ptr, size, cnt, fp); \
      if(read_cnt < cnt) {                \
          return false;                                     \
      }\
}

namespace WaveCodec {

    std::string GetWaveAudioFormatString(uint16_t audio_format){
        if(audio_format == WaveAudioFormatPCM) return "PCM";
        if(audio_format == WaveAudioFormatMSADPCM) return "Microsoft ADPCM";
        if(audio_format == WaveAudioFormatIeeeFloat) return "IEEE float";
        if(audio_format == WaveAudioFormatALaw) return "8-bit ITU-T G.711 A-law";
        if(audio_format == WaveAudioFormatMuLaw) return "8-bit ITU-T G.711 mu-law";
        if(audio_format == WaveAudioFormatGSM) return "GSM 6.10";
        if(audio_format == WaveAudioFormatG721) return "ITU G.721 ADPCM";
        if(audio_format == WaveAudioFormatExtensible) return "Extensible";

        return "unknown";
    }

    ///////////////////////////////////////////////////
    // WaveFileReader
    WaveFileReader::WaveFileReader() {

    }

    WaveFileReader::~WaveFileReader() {
        Close();
    }

    bool WaveFileReader::Open(const std::string& waveFilePath){
        if (waveFilePath.size() == 0) return false;
        if (m_fp) return false;

        m_fp = fopen(waveFilePath.c_str(), "rb");
        if(!m_fp){
            printf("open file failed\n");
            return false;
        }

        return true;
    }

    bool WaveFileReader::ReadWaveHeader(WaveHeader& header){
        if(!m_fp) return false;

        // get file size
        fseek(m_fp, 0L, SEEK_END);
        long file_size = ftell(m_fp);
        fseek(m_fp, 0L, SEEK_SET);

        // read riff chunk fourcc
        uint8_t fourcc[4];
        READ_AND_CHECK(fourcc,sizeof(uint8_t), 4, m_fp )
        if (fourcc[0] != 'R' || fourcc[1] != 'I' || fourcc[2] != 'F' || fourcc[3] != 'F') {
            printf("invalid wave file, riff fourcc error\n");
            return false;
        }
        m_header.riff.header.fourcc = MAKE_FOURCC('R', 'I', 'F', 'F');

        // read RIFF chunk size
        uint32_t riff_chunk_size = 0;
        READ_AND_CHECK(&riff_chunk_size, sizeof(uint32_t), 1, m_fp);

        // check RIFF chunk size
        if ((uint32_t)file_size != riff_chunk_size + sizeof(fourcc) + sizeof(riff_chunk_size)) {
            printf("warning: invalid wave file, size not match, ignore\n");
            //return false;
        }
        m_header.riff.header.size = riff_chunk_size;

        // read RIFF chunk form type
        uint8_t form_type[4];
        READ_AND_CHECK(form_type, sizeof(uint8_t), 4, m_fp);
        if (form_type[0] != 'W' || form_type[1] != 'A' || form_type[2] != 'V' || form_type[3] != 'E') {
            printf("RIFF not WAVE, invalid wave file\n");
            return false;
        }
        m_header.riff.form_type = MAKE_FOURCC('W', 'A', 'V', 'E');

        // Find all sub chunks
        while(true){
            // sub chunk name
            uint8_t sub_chunk_name[4];
            READ_AND_CHECK(sub_chunk_name, sizeof(uint8_t), 4, m_fp);
            printf("sub chunk found: %c%c%c%c\n", sub_chunk_name[0], sub_chunk_name[1], sub_chunk_name[2], sub_chunk_name[3]);

            // sub chunk size
            uint32_t sub_chunk_size = 0;
            READ_AND_CHECK(&sub_chunk_size, sizeof(uint32_t), 1, m_fp);
            printf("sub chunk size: %d\n", sub_chunk_size);

            // Handle sub chunk

            // sub chunk fmt
            if(sub_chunk_name[0] == 'f' && sub_chunk_name[1] == 'm' && sub_chunk_name[2] == 't' && sub_chunk_name[3] == ' '){
                m_header.riff.fmt.header.fourcc = MAKE_FOURCC('f', 'm', 't', ' ');
                m_header.riff.fmt.header.size = sub_chunk_size;

                // audio format
                uint16_t audio_format = WaveAudioFormatUnknown;
                READ_AND_CHECK(&audio_format, sizeof(uint16_t), 1, m_fp);
                m_header.riff.fmt.audio_format = audio_format;

                // channels
                uint16_t channels = 0;
                READ_AND_CHECK(&channels, sizeof(uint16_t), 1, m_fp);
                m_header.riff.fmt.channels = channels;

                // sample_rate
                uint32_t sample_rate = 0;
                READ_AND_CHECK(&sample_rate, sizeof(uint32_t), 1, m_fp);
                m_header.riff.fmt.sample_rate = sample_rate;

                // byte_rate
                uint32_t byte_rate = 0;
                READ_AND_CHECK(&byte_rate, sizeof(uint32_t), 1, m_fp);
                m_header.riff.fmt.byte_rate = byte_rate;

                // block_align
                uint16_t block_align = 0;
                READ_AND_CHECK(&block_align, sizeof(uint16_t), 1, m_fp);
                m_header.riff.fmt.block_align = block_align;

                // sample_bits
                uint16_t sample_bits  = 0;
                READ_AND_CHECK(&sample_bits, sizeof(uint16_t), 1, m_fp);
                m_header.riff.fmt.bits_per_sample = sample_bits;

                // ex_size
                uint32_t left = sub_chunk_size - 16;
                if(left < 0) {
                    return false;
                }else if(left == 0){
                    m_header.riff.fmt.ex_size = 0;
                }else{
                    // left > 0
                    if(left < 2) return false;
                    uint16_t ex_size = 0;
                    READ_AND_CHECK(&ex_size, sizeof(uint16_t), 1, m_fp);
                    m_header.riff.fmt.ex_size = ex_size;

                    if(ex_size < 0) return false;

                    // skip ex data
                    if(ex_size > 0){
                        fseek(m_fp, ex_size, SEEK_CUR);
                    }
                }

                printf("audio_format:%d(%s), sample_rate:%d, sample_bits:%d, channels:%d\n", audio_format,
                      GetWaveAudioFormatString(audio_format).c_str(), sample_rate, sample_bits, channels);
            }

            // sub chunk fact
            else if(sub_chunk_name[0] == 'f' && sub_chunk_name[1] == 'a' && sub_chunk_name[2] == 'c' && sub_chunk_name[3] == 't'){
                m_header.riff.fact.header.fourcc = MAKE_FOURCC('f', 'a', 'c', 't');
                m_header.riff.fact.header.size = sub_chunk_size;

                // samples
                uint32_t samples = 0;
                READ_AND_CHECK(&samples, sizeof(uint32_t), 1, m_fp);
                m_header.riff.fact.samples = samples;

                uint32_t left = sub_chunk_size - 16;
                if(left > 0){
                    fseek(m_fp, left, SEEK_CUR);
                }
            }

            // sub chunk data, always the last sub chunk
            else if (sub_chunk_name[0] == 'd' && sub_chunk_name[1] == 'a' && sub_chunk_name[2] == 't' && sub_chunk_name[3] == 'a') {
                m_header.riff.data.header.fourcc = MAKE_FOURCC('d', 'a', 't', 'a');
                m_header.riff.data.header.size = sub_chunk_size;
                break;
            }

            // other sub trunk, skip
            else{
                fseek(m_fp, sub_chunk_size, SEEK_CUR);
            }
        }

        memcpy(&header, &m_header, sizeof(m_header));
        return true;
    }

    size_t WaveFileReader::ReadBytes(uint32_t bytes2Read, std::vector<uint8_t>& bytes){
        if(!m_fp) return 0;
        if(bytes2Read == 0) return 0;

        bytes.resize(bytes2Read);
        size_t nRead = fread(&bytes[0], sizeof(uint8_t), bytes2Read, m_fp);
        if(nRead < bytes2Read){
            bytes.resize(nRead);
        }

        return nRead;
    }

    size_t WaveFileReader::ReadShorts(uint32_t shorts2Read, std::vector<uint16_t>& shorts){
        if(!m_fp) return 0;
        if(shorts2Read == 0) return 0;

        shorts.resize(shorts2Read);
        size_t nRead = fread(&shorts[0], sizeof(uint16_t), shorts2Read, m_fp);
        if(nRead < shorts2Read){
            shorts.resize(nRead);
        }
        return nRead;
    }

    size_t WaveFileReader::ReadDuration(uint32_t durationMs, std::vector<uint8_t>& data){
        if(!m_fp) return 0;

        if( m_header.riff.fmt.audio_format != WaveAudioFormatPCM
            && m_header.riff.fmt.audio_format != WaveAudioFormatALaw
            && m_header.riff.fmt.audio_format != WaveAudioFormatMuLaw){
            return -1;
        }

        uint32_t bytesPerMs = (m_header.riff.fmt.sample_rate * m_header.riff.fmt.bits_per_sample/8 * m_header.riff.fmt.channels) / 1000; // 每 ms 的字节数
        uint32_t bytesPerDuration = bytesPerMs * durationMs;
        return ReadBytes(bytesPerDuration, data);
    }

    size_t WaveFileReader::ReadDuration(uint32_t durationMs, std::vector<uint16_t>& data){
        if(!m_fp) return 0;

        if( m_header.riff.fmt.audio_format != WaveAudioFormatPCM
            && m_header.riff.fmt.audio_format != WaveAudioFormatALaw
            && m_header.riff.fmt.audio_format != WaveAudioFormatMuLaw){
            return -1;
        }

        uint32_t shortsPerMs = (m_header.riff.fmt.sample_rate * m_header.riff.fmt.bits_per_sample/16 * m_header.riff.fmt.channels) / 1000; // 每 ms 的 short 个数
        uint32_t shortsPerDuration = shortsPerMs * durationMs;
        return ReadShorts(shortsPerDuration, data);
    }

    void WaveFileReader::Close(){
        if(m_fp){
            fclose(m_fp);
            m_fp = nullptr;
        }
    }

    ///////////////////////////////////////////////////
    // WaveFileWriter
    WaveFileWriter::WaveFileWriter(){}
    WaveFileWriter::~WaveFileWriter(){
        Close();
    }

    bool WaveFileWriter::Open(const std::string& waveFilePath, uint16_t audio_format, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels){
        if (waveFilePath.size() == 0) return false;
        if (audio_format != WaveAudioFormatPCM && audio_format != WaveAudioFormatALaw && audio_format != WaveAudioFormatMuLaw){
            return false;
        }
        if (sample_bits != 8 && sample_bits != 16 && sample_bits != 32){
            return false;
        }
        if (channels != 1 && channels != 2){
            return false;
        }

        if (m_fp) return false;
        m_fp = fopen(waveFilePath.c_str(), "wb");
        if(!m_fp){
            printf("open file failed\n");
            return false;
        }

        m_data_len = 0;
        m_header.riff.fmt.audio_format = audio_format;
        m_header.riff.fmt.sample_rate = sample_rate;
        m_header.riff.fmt.bits_per_sample = sample_bits;
        m_header.riff.fmt.channels = channels;

        // 文件开头保留 header_size 字节，用于回填 wave header
        fseek(m_fp, m_header.GetHeaderSize(), SEEK_CUR);

        return true;
    }

    void WaveFileWriter::Write(const uint8_t* data, uint32_t len){
        if(!m_fp) return;
        if(!data) return;

        fwrite(data, sizeof(uint8_t), len, m_fp);
        m_data_len += len;
    }

    void WaveFileWriter::Write(const uint16_t* data, uint32_t len){
        if(!m_fp) return;
        if(!data) return;

        fwrite(data, sizeof(uint16_t), len, m_fp);
        m_data_len += (len*2);
    }

    void WaveFileWriter::Write(const std::vector<uint8_t>& data){
        Write(&data[0], data.size());
    }

    void WaveFileWriter::Write(const std::vector<uint8_t>& data, size_t len){
        Write(&data[0], len);
    }

    void WaveFileWriter::Write(const std::vector<uint16_t>& data){
        Write(&data[0], data.size());
    }

    void WaveFileWriter::Write(const std::vector<uint16_t>& data, size_t len){
        Write(&data[0], len);
    }

    void WaveFileWriter::Close(){
        if(m_fp){

            // 生成 wave header
            if(m_header.riff.fmt.audio_format == WaveAudioFormatPCM){
                m_header.FormatPCMWaveHeader(m_header.riff.fmt.sample_rate, m_header.riff.fmt.bits_per_sample, m_header.riff.fmt.channels, m_data_len);
            }else if(m_header.riff.fmt.audio_format == WaveAudioFormatALaw || m_header.riff.fmt.audio_format == WaveAudioFormatMuLaw){
                m_header.FormatG711WaveHeader(m_header.riff.fmt.audio_format, m_header.riff.fmt.sample_rate, m_header.riff.fmt.bits_per_sample, m_header.riff.fmt.channels, m_data_len);
            }

            // 回填 wave header 到文件开头
            fseek(m_fp, 0, SEEK_SET);
            std::vector<uint8_t> buffer;
            m_header.ToBuffer(buffer);
            fwrite(&buffer[0], sizeof(uint8_t), buffer.size(), m_fp);

            fclose(m_fp);
            m_fp = nullptr;
        }
    }


    // PCM 文件转 Wave 文件
    bool PCM2WaveFile(const std::string& pcmFilePath, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, const std::string& waveFilePath){
        WaveFileWriter writer;
        if(!writer.Open(waveFilePath, WaveAudioFormatPCM, sample_rate, sample_bits, channels)){
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
            writer.Write(buff, read_cnt);
        }

        fclose(fpPCM);
        writer.Close();
        return true;
    }

    // Wave 文件转 PCM 文件，同时返回音频的编码参数
    bool Wave2PCMFile(const std::string& waveFilePath, const std::string& pcmFilePath, uint32_t& sample_rate_out, uint16_t& sample_bits_out, uint16_t& channels_out){
        WaveFileReader reader;
        if(!reader.Open(waveFilePath)){
            return false;
        }

        WaveHeader header;
        if(!reader.ReadWaveHeader(header)){
            return false;
        }

        sample_rate_out = header.riff.fmt.sample_rate;
        sample_bits_out = header.riff.fmt.bits_per_sample;
        channels_out    = header.riff.fmt.channels;

        FILE *fpPCM = fopen(pcmFilePath.c_str(), "wb");
        if (!fpPCM) {
            printf("open pcm file failed, %s\n", pcmFilePath.c_str());
            return false;
        }

        std::vector<uint8_t> buffer;
        while(true){
            int read_cnt = reader.ReadBytes(1024, buffer);
            if(read_cnt > 0){
                fwrite(&buffer[0], sizeof(uint8_t), read_cnt, fpPCM);
            }else{
                break;
            }
        }

        return true;
    }

    bool Wave2PCMFile(const std::string& waveFilePath, const std::string& pcmFilePath) {
        uint32_t sample_rate_out;
        uint16_t sample_bits_out;
        uint16_t channels_out;

        return Wave2PCMFile(waveFilePath, pcmFilePath, sample_rate_out, sample_bits_out, channels_out);
    }
}
