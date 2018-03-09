//
// Created by Elong on 2017/10/22.
//
#ifndef FFMPEGRECORDER_NativeByteBuffer_H
#define FFMPEGRECORDER_NativeByteBuffer_H

#include <stdint.h>
#include <jni.h>

class NativeByteBuffer{

public:
    NativeByteBuffer(uint32_t size);
    ~NativeByteBuffer();
    void position(uint32_t position);
    uint32_t position();
    void limit(uint32_t count);
    uint32_t limit();
    uint32_t capacity();
    uint32_t remaining();
    bool hasRemainder();
    uint8_t* bytes();
    bool writeBytes(uint8_t* b, uint32_t length);
    bool writeBytes(uint8_t* b, uint32_t offset, uint32_t length);
    void writeInt64(int64_t x);
    int64_t readInt64();
    void rewind();
    void flip();
    void clear();
    jobject getJavaByteBuffer();


private:

    void writeBytesInternal(uint8_t* b, uint32_t offset, uint32_t length);

    bool bufferOwner = true;
    uint8_t* buffer;
    uint32_t _position = 0;
    uint32_t _limit = 0;
    uint32_t _capacity = 0;
    jobject javaByteBuffer = nullptr;

};

#endif FFMPEGRECORDER_NativeByteBuffer_H