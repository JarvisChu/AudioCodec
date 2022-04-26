# PCMCodec

header only PCMCodec/WaveCodec

## Usage

```cpp
#include "PCMCodec.hpp"

WaveCodec::PCM2WaveFile(const std::string& pcmFilePath, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, const std::string& waveFilePath);
WaveCodec::PCMToWaveBuffer(const uint8_t* pcmBuffer, uint32_t pcmBufferSize, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, std::vector<uint8_t>& waveBufferOut);
WaveCodec::PCMToWaveBuffer(const std::vector<uint8_t> pcmBuffer, uint32_t sample_rate, uint16_t sample_bits, uint16_t channels, std::vector<uint8_t>& waveBufferOut);
WaveCodec::Wave2PCMFile(const std::string& waveFilePath, const std::string& pcmFilePath);
WaveCodec::Wave2PCMFile(const std::string& waveFilePath, const std::string& pcmFilePath, uint32_t& sample_rate_out, uint16_t& sample_bits_out, uint16_t& channels_out);

WaveCodec::WaveFile waveFile;
waveFile.Open(xxx);
waveFile.Write(xxx);
waveFile.Close(xxx);

```
