//
// Created by Elong on 2017/12/21.
//

#ifndef FFMPEGRECORDER_BYTEBUFFERPROVIDER_H
#define FFMPEGRECORDER_BYTEBUFFERPROVIDER_H

#include <stdint.h>
#include <vector>
#include <pthread.h>

class NativeByteBuffer;

class BuffersStorage{
public:
    BuffersStorage(bool threadSafe);
    NativeByteBuffer* getFreeBuffer(uint32_t size);
    void reuse(NativeByteBuffer* buffer);
    void clear();
    static BuffersStorage& getInstance();

private:
//    std::vector<NativeByteBuffer*> mBufferPool;
    std::vector<NativeByteBuffer*> freeBuffers8;
    std::vector<NativeByteBuffer*> freeBuffers128;
    std::vector<NativeByteBuffer*> freeBuffers1024;
    std::vector<NativeByteBuffer*> freeBuffers4096;
    std::vector<NativeByteBuffer*> freeBuffers16384;
    std::vector<NativeByteBuffer*> freeBuffers32768;
    std::vector<NativeByteBuffer*> freeBuffersBig;
    bool mThreadSafe = true;
    pthread_mutex_t mMutex;
};

#endif //FFMPEGRECORDER_BYTEBUFFERPROVIDER_H
