#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

#include "PCMCodec/PCMFile.h"
#include "PCMCodec/PCMCodec.h"


void print_usage(){
    printf("PCMCodecExample <option> [param...] \n");
    printf("e.g.\n");
    printf("  # PCMCodecExample copy in.pcm out.pcm sampleRate sampleBits channels startMs endMs\n");
    printf("  # copy in.pcm (from startMs to endMs) to out.pcm\n");
    printf("  PCMCodecExample copy in.pcm out.pcm 8000 16 1 1000 3000\n");
    printf("  # abstract the left and right channel of in.pcm, save to out_left.pcm and out_right.pcm\n");
    printf("  # in.pcm: channels must be 2, and sampleBits must be 16\n");
    printf("  PCMCodecExample abstract in.pcm out_left.pcm out_right.pcm\n");
}

void doCopy(int argc, char** argv){
    if(argc < 9){
        printf("invalid param\n");
        return;
    }

    std::string inPCMPath(argv[2]);
    std::string outPCMPath(argv[3]);
    uint32_t sampleRate = std::stoi(argv[4]);
    uint16_t sampleBits = std::stoi(argv[5]);
    uint16_t channels = std::stoi(argv[6]);
    uint32_t startMs = std::stoi(argv[7]);
    uint32_t endMs = std::stoi(argv[8]);

    PCMCodec::PCMFileReader reader;
    if(!reader.Open(inPCMPath, sampleRate, sampleBits, channels)){
        return;
    }

    PCMCodec::PCMFileWriter writer;
    if(!writer.Open(outPCMPath)){
        return;
    }

    reader.SeekToTime(startMs);

    std::vector<uint8_t> buffer;
    reader.ReadDuration(endMs-startMs, buffer);
    writer.Write(buffer);

    reader.Close();
    writer.Close();

    printf("copy success\n");
}

void abstract(int argc, char** argv){
    if(argc < 5){
        printf("invalid param\n");
        return;
    }

    std::string inPCMPath(argv[2]);
    std::string leftPCMPath(argv[3]);
    std::string rightPCMPath(argv[4]);

    if(!PCMCodec::AbstractChannel2File(inPCMPath, leftPCMPath, rightPCMPath)){
        printf("abstract failed\n");
        return;
    }
    printf("abstract success\n");
}

int main(int argc, char** argv)
{
    if(argc < 3){
        print_usage();
        return 0;
    }

    std::string option = argv[1];
    if(option == "copy"){
        doCopy(argc, argv);
    }else if(option == "abstract"){
        abstract(argc, argv);
    }else{
        printf("invalid option\n");
    }

    return 0;
}

