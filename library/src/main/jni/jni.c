//
// Created by Elong on 2017/12/20.
//

#include <jni.h>
#include <unistd.h>
#include "AndroidUtils.h"

extern int registerNativeByteBufferFunctions(JavaVM* vm, JNIEnv* env);
extern int registerFFmpegRecorderNative(JavaVM* vm, JNIEnv* env);
extern int registerBuffersStorageNative(JavaVM* vm, JNIEnv* env);

JavaVM* javaVM;

jint JNI_OnLoad(JavaVM* vm, void* reserved){
    LOGD("JNI_OnLoad");
    JNIEnv* env = 0;
    javaVM = vm;
    if((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK){
        return -1;
    }
    registerFFmpegRecorderNative(vm, env);
    registerNativeByteBufferFunctions(vm, env);
    registerBuffersStorageNative(vm, env);
    return JNI_VERSION_1_6;
}