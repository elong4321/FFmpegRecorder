//
// Created by Elong on 2017/10/22.
//

#include <memory.h>
#include <stdlib.h>
#include "NativeByteBuffer.h"
#include "AndroidUtils.h"

extern JavaVM *javaVM;

extern jclass jclass_ByteBuffer;
extern jmethodID jclass_ByteBuffer_allocateDirect;


int NewGlobalCount = 0;

NativeByteBuffer::NativeByteBuffer(uint32_t size) {
    if (jclass_ByteBuffer != nullptr) {
        JNIEnv *env = 0;
        if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
            LOGE("new don't get jnienv");
            exit(1);
        }
        javaByteBuffer = env->CallStaticObjectMethod(jclass_ByteBuffer,
                                                     jclass_ByteBuffer_allocateDirect, size);
        if (javaByteBuffer == nullptr) {
            LOGE("can't create javaByteBuffer");
            exit(1);
        }
        jobject globalRef = env->NewGlobalRef(javaByteBuffer);
        NewGlobalCount++;
        LOGD("get NewGlobalCount= %d, ref=%d", NewGlobalCount, this);
        env->DeleteLocalRef(javaByteBuffer);
        javaByteBuffer = globalRef;
        buffer = (uint8_t *) env->GetDirectBufferAddress(javaByteBuffer);
        bufferOwner = false;
    } else {
        buffer = new uint8_t[size];
        bufferOwner = true;
    }
    if (buffer == nullptr) {
        LOGE("can't allocate NativeByteBuffer buffer");
        exit(1);
    }
    _limit = _capacity = size;
}

NativeByteBuffer::~NativeByteBuffer() {
    if (javaByteBuffer != nullptr) {
        JNIEnv *env = 0;
        if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
            LOGE("can't get jnienv");
            exit(1);
        }
        env->DeleteGlobalRef(javaByteBuffer);
        javaByteBuffer = nullptr;
    }
    if (bufferOwner && buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
    }

}


bool NativeByteBuffer::writeBytes(uint8_t *b, uint32_t length) {
    writeBytes(b, 0, length);
}

bool NativeByteBuffer::writeBytes(uint8_t *b, uint32_t offset, uint32_t length) {
    if (_position + length > _capacity) {
        return false;
    }
    writeBytesInternal(b, 0, length);
    return true;
}

void NativeByteBuffer::writeBytesInternal(uint8_t *b, uint32_t offset, uint32_t length) {
    memcpy(buffer + _position, b + offset, sizeof(uint8_t) * length);
    _position += length;
}

void NativeByteBuffer::position(uint32_t position) {
    _position = position;
}

uint32_t NativeByteBuffer::position() {
    return _position;
}

void NativeByteBuffer::limit(uint32_t count) {
    _limit = count;
}

uint32_t NativeByteBuffer::limit() {
    return _limit;
}

uint32_t NativeByteBuffer::capacity() {
    return _capacity;
}

uint32_t NativeByteBuffer::remaining() {
    return _limit - _position;
}

uint8_t *NativeByteBuffer::bytes() {
    return buffer;
}

void NativeByteBuffer::writeInt64(int64_t x) {
    if (_position + 8 > _limit) {
        return;
    }
    buffer[_position++] = (uint8_t) x;
    buffer[_position++] = (uint8_t) (x >> 8);
    buffer[_position++] = (uint8_t) (x >> 16);
    buffer[_position++] = (uint8_t) (x >> 24);
    buffer[_position++] = (uint8_t) (x >> 32);
    buffer[_position++] = (uint8_t) (x >> 40);
    buffer[_position++] = (uint8_t) (x >> 48);
    buffer[_position++] = (uint8_t) (x >> 56);
}

int64_t NativeByteBuffer::readInt64() {
    if (_position + 8 > _limit) {
        return 0;
    }
    int64_t result = ((int64_t) (buffer[_position] & 0xff)) |
                     ((int64_t) (buffer[_position + 1] & 0xff) << 8) |
                     ((int64_t) (buffer[_position + 2] & 0xff) << 16) |
                     ((int64_t) (buffer[_position + 3] & 0xff) << 24) |
                     ((int64_t) (buffer[_position + 4] & 0xff) << 32) |
                     ((int64_t) (buffer[_position + 5] & 0xff) << 40) |
                     ((int64_t) (buffer[_position + 6] & 0xff) << 48) |
                     ((int64_t) (buffer[_position + 7] & 0xff) << 56);

    _position += 8;
    return result;
}

bool NativeByteBuffer::hasRemainder() {
    return _position < _limit;
}

void NativeByteBuffer::rewind() {
    _position = 0;
}

void NativeByteBuffer::flip() {
    _limit = _position;
    _position = 0;
}

void NativeByteBuffer::clear() {
    _position = 0;
    _limit = _capacity;
}

jobject NativeByteBuffer::getJavaByteBuffer() {
    if (javaByteBuffer == nullptr && javaVM != nullptr) {
        JNIEnv *env = 0;
        if (javaVM->GetEnv((void **) env, JNI_VERSION_1_6) != JNI_OK) {
            LOGE("can't get jnienv");
            exit(1);
        }
        javaByteBuffer = env->NewDirectByteBuffer(buffer, _capacity);
        if (javaByteBuffer == nullptr) {
            LOGE("can't allocate NativeByteBuffer buffer");
            exit(1);
        }
        jobject globalRef = env->NewGlobalRef(javaByteBuffer);
        NewGlobalCount++;
        LOGD("NewGlobalCount= %d, ref=%d", NewGlobalCount, this);
        env->DeleteLocalRef(javaByteBuffer);
        javaByteBuffer = globalRef;
    }
    return javaByteBuffer;
}

