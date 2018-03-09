//
// Created by Elong on 2017/12/21.
//

#include "NativeByteBuffer.h"
#include "BuffersStorage.h"

BuffersStorage& BuffersStorage::getInstance() {
    static BuffersStorage instance(true);
    return instance;
}

BuffersStorage::BuffersStorage(bool threadSafe) {
    mThreadSafe = threadSafe;
    if(threadSafe){
        pthread_mutex_init(&mMutex, NULL);
    }
//    for(uint32_t a=0; a<4; a++){
//        freeBuffers1024.push_back(new NativeByteBuffer(1024));
//    }
//    for(int a=0; a<4; a++){
//        freeBuffers4096.push_back(new NativeByteBuffer(4096));
//    }
}

NativeByteBuffer *BuffersStorage::getFreeBuffer(uint32_t size) {
    uint32_t byteCount = 0;
    std::vector<NativeByteBuffer *> *arrayToGetFrom = nullptr;
    NativeByteBuffer *buffer = nullptr;
    if (size <= 8) {
        arrayToGetFrom = &freeBuffers8;
        byteCount = 8;
    } else if (size <= 128) {
        arrayToGetFrom = &freeBuffers128;
        byteCount = 128;
    } else if (size <= 1024 + 200) {
        arrayToGetFrom = &freeBuffers1024;
        byteCount = 1024 + 200;
    } else if (size <= 4096 + 200) {
        arrayToGetFrom = &freeBuffers4096;
        byteCount = 4096 + 200;
    } else if (size <= 16384 + 200) {
        arrayToGetFrom = &freeBuffers16384;
        byteCount = 16384 + 200;
    } else if (size <= 40000) {
        arrayToGetFrom = &freeBuffers32768;
        byteCount = 40000 + 200;
    } else {
        arrayToGetFrom = &freeBuffersBig;
        byteCount = size;
    }

    if (arrayToGetFrom != nullptr) {
        if (mThreadSafe) {
            pthread_mutex_lock(&mMutex);
        }
        if (arrayToGetFrom->size() > 0) {
            std::vector<NativeByteBuffer *>::iterator iterator = arrayToGetFrom->begin();
            buffer = *iterator;
            arrayToGetFrom->erase(iterator);
        }
        if (mThreadSafe) {
            pthread_mutex_unlock(&mMutex);
        }
    }
    if (buffer == nullptr) {
        buffer = new NativeByteBuffer(byteCount);
    }
    buffer->limit(size);
    buffer->rewind();
    return buffer;
}

void BuffersStorage::reuse(NativeByteBuffer* buffer){
    if(buffer == nullptr){
        return;
    }
    std::vector<NativeByteBuffer*>* arrayToReuse = nullptr;
    uint32_t capacity = buffer->capacity();
    uint32_t maxCount = 10;
    if(capacity == 8){
        arrayToReuse = &freeBuffers8;
    }else if(capacity == 128){
        arrayToReuse = &freeBuffers128;
    }else if(capacity == 1024 + 200){
        arrayToReuse = &freeBuffers1024;
    }else if(capacity == 4096 + 200){
        arrayToReuse = &freeBuffers4096;
    }else if(capacity == 16384 + 200){
        arrayToReuse = &freeBuffers16384;
    }else if(capacity == 40000){
        arrayToReuse = &freeBuffers32768;
    }else if(capacity == 160000){
        arrayToReuse = &freeBuffersBig;
    }
    if(arrayToReuse != nullptr){
        if(mThreadSafe){
            pthread_mutex_lock(&mMutex);
        }
        if(arrayToReuse->size() < maxCount){
            buffer->clear();
            arrayToReuse->push_back(buffer);
        }else{
            delete buffer;
        }
        if(mThreadSafe){
            pthread_mutex_unlock(&mMutex);
        }
    }else{
        delete buffer;
    }
}

void BuffersStorage::clear(){
    freeBuffers8.clear();
    freeBuffers128.clear();
    freeBuffers1024.clear();
    freeBuffers4096.clear();
    freeBuffers16384.clear();
    freeBuffers32768.clear();
    freeBuffersBig.clear();
}




