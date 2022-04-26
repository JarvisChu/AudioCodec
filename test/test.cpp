#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

#include "PCMCodec.hpp"

int main()
{
    // test WaveCodec::PCM2WaveFile
    WaveCodec::PCM2WaveFile("/Users/jarvischu/code/github/PCMCodec/test/test_8000_16_1.pcm",
                            8000, 16, 1,
                            "/Users/jarvischu/code/github/PCMCodec/test/test_8000_16_1.wav");

    // test WaveCodec::PCMToWaveBuffer
    // (1) read whole pcm data to srcFileContent
    // (2) pcm2wave, srcFileContent => wavBufferOut
    // (3) write wavBufferOut to wave file
    std::ifstream srcFile("/Users/jarvischu/code/github/PCMCodec/test/test_8000_16_1.pcm", std::ios::binary);
    std::vector<char> srcFileContent( (std::istreambuf_iterator<char>(srcFile)), std::istreambuf_iterator<char>());

    std::vector<uint8_t> wavBufferOut;
    WaveCodec::PCMToWaveBuffer( (const uint8_t *) &srcFileContent[0], srcFileContent.size(), 8000, 16, 1, wavBufferOut);

    std::ofstream tgtFile("/Users/jarvischu/code/github/PCMCodec/test/test_8000_16_1_1.wav", std::ios::binary);
    std::ostream_iterator<char> output_iterator(tgtFile);
    std::copy(wavBufferOut.begin(), wavBufferOut.end(), output_iterator);
    tgtFile.flush();

    // test WaveCodec::Wave2PCMFile
    uint32_t sample_rate_out = 0;
    uint16_t sample_bits_out = 0;
    uint16_t channels_out = 0;
    WaveCodec::Wave2PCMFile("/Users/jarvischu/code/github/PCMCodec/test/test_8000_16_1_1.wav",
                            "/Users/jarvischu/code/github/PCMCodec/test/test_8000_16_1_1.pcm",
                            sample_rate_out, sample_bits_out, channels_out);
    std::cout << "sample_rate_out: " << sample_rate_out << ", sample_bits_out: " << sample_bits_out << ", channels_out: " << channels_out << std::endl;


    // test PCMCodec::AbstractChannel2File
    PCMCodec::AbstractChannel2File("/Users/jarvischu/code/github/PCMCodec/test/test_48000_16_2.pcm",
                                   "/Users/jarvischu/code/github/PCMCodec/test/test_48000_16_2_left.pcm",
                                   "/Users/jarvischu/code/github/PCMCodec/test/test_48000_16_2_right.pcm"
                                   );

    return 0;
}
