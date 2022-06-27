# AudioCodec

常见的音频编码和算法

## 目录结构

- PCMCodec：PCM 相关的编码算法和文件读写
  * PCMFile.h/PCMFile.cpp
    - PCMFileReader <sup>[class]</sup>
      * Open
      * ReadBytes/ReadShorts/ReadDuration
      * SeekToTime
      * GetFileSize
      * Close
    - PCMFileWriter <sup>[class]</sup>
      * Open
      * Write
      * Close
  * PCMCodec.h/PCMCodec.cpp
    - AbstractChannel <sup>[function]</sup> : 分离左右声道，提取某个声道数据
    - AbstractChannel2File <sup>[function]</sup> : 分离左右声道，保存到文件
    - Mix: 混音，TODO
    - Resampling: 重采样，TODO
- WaveCodec: Wave 相关的编解码和文件读写
  * WaveFile.h/WaveFile.cpp
    - WaveHeader <sup>[struct]</sup> : Wave Header 格式定义
    - WaveFileReader <sup>[class]</sup> : wave 文件读取类
      * Open
      * ReadWaveHeader
      * ReadBytes/ReadShorts/ReadDuration
      * Close
    - WaveFileWriter <sup>[class]</sup> : wave 文件写入类
      * Open
      * Write
      * Close
    - Wave2PCMFile <sup>[function]</sup> : 将Wave文件转换为PCM文件
    - PCM2WaveFile <sup>[function]</sup> : 将PCM文件转换为Wave文件
  
## Usage

**编译**

```
cmake -B build
cmake --build build
```

**运行 Example**

```
cd build/bin
./PCMCodecExample
./WaveCodecExample
```

> 测试需要的音频文件，可以在 [这里](https://github.com/jarvischu/audio) 下载

## Example

详见 example 目录
