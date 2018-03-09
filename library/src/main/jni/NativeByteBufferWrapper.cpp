//
// Created by Elong on 2017/12/18.
//

#include <jni.h>

#include "NativeByteBuffer.h"
#include "BuffersStorage.h"

extern "C" {

jclass jclass_ByteBuffer = nullptr;
jmethodID jclass_ByteBuffer_allocateDirect = nullptr;

jint limit(JNIEnv *env, jclass c, jint address);
jint position(JNIEnv *env, jclass c, jint address);
void setPosition(JNIEnv *env, jclass c, jint address, jint position);
void flip(JNIEnv *env, jclass c, jint address);
jobject getJavaByteBuffer(JNIEnv *env, jclass c, jint address);


int registerNativeByteBufferFunctions(JavaVM *vm, JNIEnv *env) {
    const char *nativeByteBufferClassPathName = "com/elong/ffmpeg/NativeByteBuffer";
    JNINativeMethod nativeByteBufferMethods[] = {
            {"native_limit",             "(I)I",                     (void *) limit},
            {"native_position",          "(I)I",                     (void *) position},
            {"native_position",          "(II)V",                     (void *) setPosition},
            {"native_flip",              "(I)V",                     (void *) flip},
            {"native_getJavaByteBuffer", "(I)Ljava/nio/ByteBuffer;", (void *) getJavaByteBuffer},
    };
    jclass clazz = env->FindClass(nativeByteBufferClassPathName);
    if (clazz == nullptr) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, nativeByteBufferMethods, sizeof(nativeByteBufferMethods) /
                                                             sizeof(nativeByteBufferMethods[0]))) {
        return JNI_FALSE;
    }
    jclass_ByteBuffer = (jclass) env->NewGlobalRef(env->FindClass("java/nio/ByteBuffer"));
    jclass_ByteBuffer_allocateDirect = env->GetStaticMethodID(jclass_ByteBuffer, "allocateDirect",
                                                              "(I)Ljava/nio/ByteBuffer;");
    return JNI_TRUE;
}

jint limit(JNIEnv *env, jclass c, jint address) {
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    return buffer->limit();
}

jint position(JNIEnv *env, jclass c, jint address) {
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    return buffer->position();
}

void setPosition(JNIEnv *env, jclass c, jint address, jint position) {
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    buffer->position(position);
}

void flip(JNIEnv *env, jclass c, jint address){
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    buffer->flip();
}

jobject getJavaByteBuffer(JNIEnv *env, jclass c, jint address) {
    NativeByteBuffer *buffer = (NativeByteBuffer *) address;
    return buffer->getJavaByteBuffer();
}

}