#include "WaveCodec/WaveFile.h"

void print_usage(){
    printf("WaveCodecExample <option> [params...] \n");
    printf("e.g.\n");
    printf("  WaveCodecExample decode in.wav out.pcm\n");
    printf("  WaveCodecExample encode in.pcm out.wav 8000 16 1\n");
}

void decode(int argc, char** argv){
    if(argc < 4){
        printf("invalid params\n");
        return;
    }

    std::string wavPath(argv[2]);
    std::string pcmPath(argv[3]);

    if(WaveCodec::Wave2PCMFile(wavPath, pcmPath)){
        printf("Wave2PCMFile success, wavPath:%s, pcmPath:%s\n", wavPath.c_str(), pcmPath.c_str());
    }else{
        printf("Wave2PCMFile failed, wavPath:%s, pcmPath:%s\n", wavPath.c_str(), pcmPath.c_str());
    }
}

void encode(int argc, char** argv){
    if(argc < 7){
        printf("invalid params\n");
        return;
    }

    std::string pcmPath(argv[2]);
    std::string wavPath(argv[3]);
    uint32_t sampleRate = std::stoi(argv[4]);
    uint16_t sampleBits = std::stoi(argv[5]);
    uint16_t channels = std::stoi(argv[6]);

    if(WaveCodec::PCM2WaveFile(pcmPath, sampleRate, sampleBits, channels, wavPath)){
        printf("PCM2WaveFile success, pcmPath:%s, wavPath:%s, sampleRate:%d, sampleBits:%d, channels:%d\n",
               pcmPath.c_str(),wavPath.c_str(), sampleRate, sampleBits, channels);
    }else{
        printf("PCM2WaveFile success, pcmPath:%s, wavPath:%s, sampleRate:%d, sampleBits:%d, channels:%d\n",
               pcmPath.c_str(),wavPath.c_str(), sampleRate, sampleBits, channels);
    }
}

int main(int argc, char** argv)
{
    if(argc < 2){
        print_usage();
        return 0;
    }

    std::string option = argv[1];
    if(option == "decode"){
        decode(argc, argv);
    }else if(option == "encode"){
        encode(argc, argv);
    }else{
        printf("invalid option\n");
    }

    return 0;
}

