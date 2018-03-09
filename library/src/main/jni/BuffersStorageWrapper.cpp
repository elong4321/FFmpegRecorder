//
// Created by Elong on 2017/12/26.
//

#include <jni.h>
#include "BuffersStorage.h"

extern "C" {

jint getFreeBuffer(JNIEnv *env, jclass cl, jint size) {
    return (int)(BuffersStorage::getInstance().getFreeBuffer(size));
}

void reuse(JNIEnv *env, jclass cl, jint buffer) {
    BuffersStorage::getInstance().reuse((NativeByteBuffer *) buffer);
}

int registerBuffersStorageNative(JavaVM *javaVM, JNIEnv *env) {
    char *className = "com/elong/ffmpeg/BuffersStorage";
    jclass jclass_BuffersStorage = env->FindClass(className);
    JNINativeMethod nativeMethods[] = {
            {"native_getFreeBuffer", "(I)I", (void *) getFreeBuffer},
            {"native_reuse",         "(I)I", (void *) reuse},
    };
    if(!env->RegisterNatives(jclass_BuffersStorage, nativeMethods,
                         sizeof(nativeMethods) / sizeof(nativeMethods[0]))){
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

}
