//
// Created by Elong on 2017/12/18.
//

#include <jni.h>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/log.h"
}

#include "FFmpegRecorder.h"
#include "AndroidUtils.h"
#include "BuffersStorage.h"

extern "C" {
extern void av_log_to_logcat(void *ptr, int level, const char *fmt, va_list vl);
}

extern void startRecord(JNIEnv* env, FFmpegRecorder* recorder, jobject latch);

void setFormatParams(JNIEnv *env, jobject obj, jint ptr, jstring jType, jstring jFilePath){
    jsize len = env->GetStringLength(jType);;
    char* type = (char *) malloc(len);
    env->GetStringUTFRegion(jType, 0, len, type);
    len = env->GetStringLength(jFilePath);
    char* filePath = (char *) malloc(len);
    //TODO: release
    env->GetStringUTFRegion(jFilePath, 0, len, filePath);

    ((FFmpegRecorder*)ptr)->setFormatParams(type, filePath);
}

void setVideoParams(JNIEnv *env, jobject obj, jint ptr, jint format, jint frameRate, jint srcWidth, jint srcHeight, jint dstWidth, jint dstHeight){
    int nativeFormat;
    switch (format){
        case 35:    //ImageFormat.YUV_420_888
            nativeFormat = AV_PIX_FMT_YUV420P;
            break;
        case 17:    //ImageFormat.NV21
        default:
            nativeFormat = AV_PIX_FMT_NV21;
            break;
    }
    format = AV_PIX_FMT_NV21;
    ((FFmpegRecorder*)ptr)->setVideoParams((AVPixelFormat) nativeFormat, frameRate, srcWidth, srcHeight, dstWidth, dstHeight);
}

jint setAudioParams(JNIEnv *env, jobject obj, jint ptr, jint format, jint frameSize, jint channels, jint channelLayout, jint sampleRate){
    //TODO: format, channelLayout 根据java端的对应转换
    AVSampleFormat fmt = AV_SAMPLE_FMT_S16;
    channelLayout = AV_CH_LAYOUT_MONO;
    ((FFmpegRecorder*)ptr)->setAudioParams(fmt, frameSize, channels, (uint64_t)channelLayout, sampleRate);
    int size = av_samples_get_buffer_size(NULL, channels, frameSize / channels, fmt, 0);
    LOGD("predit buffer size: %d", size);
    return 2048;
}

jint newRecorder(JNIEnv *env, jobject obj){
    FFmpegRecorder* fFmpegRecorder = new FFmpegRecorder();
    return (jint)fFmpegRecorder;
}

void start(JNIEnv *env, jobject obj, jint ptr, jobject latch){
    startRecord(env, ((FFmpegRecorder*)ptr), latch);
}

void stop(JNIEnv *env, jobject obj, jint ptr){
    ((FFmpegRecorder*)ptr)->stop();
}

void release(JNIEnv *env, jobject obj, jint ptr){
    FFmpegRecorder* recorder = (FFmpegRecorder*)ptr;
    recorder->tryRelease();
    delete recorder;
    BuffersStorage::getInstance().clear();
}

void recordVideo(JNIEnv *env, jobject obj, jint ptr, jint data){
    NativeByteBuffer* buffer = (NativeByteBuffer*)data;
    ((FFmpegRecorder*)ptr)->recordVideo(buffer);
}

void recordAudio(JNIEnv *env, jobject obj, jint ptr, jint data){
    NativeByteBuffer* buffer = (NativeByteBuffer*)data;
    ((FFmpegRecorder*)ptr)->recordAudio(buffer);
}

extern "C" int registerFFmpegRecorderNative(JavaVM *vm, JNIEnv *env){
    jclass jclass_FFmpegRecorder = env->FindClass("com/elong/ffmpeg/FFmpegRecorder");
    JNINativeMethod nativeMethods[] = {
            {"native_setFormatParams", "(ILjava/lang/String;Ljava/lang/String;)V", (void *) setFormatParams},
            {"native_setVideoParams", "(IIIIIII)V",                               (void *) setVideoParams},
            {"native_setAudioParams", "(IIIIII)I",                                  (void *) setAudioParams},
            {"native_new", "()I",                                                (void*)newRecorder},
            {"native_start", "(ILjava/util/concurrent/CountDownLatch;)V",          (void*)start},
            {"native_stop", "(I)V",                                                (void*)stop},
            {"native_release", "(I)V",                                                (void*)release},
            {"native_recordVideo", "(II)V",                                         (void*)recordVideo},
            {"native_recordAudio", "(II)V",                                         (void*)recordAudio},
    };
    if(!env->RegisterNatives(jclass_FFmpegRecorder, nativeMethods, sizeof(nativeMethods) / sizeof(nativeMethods[0]))){
        return JNI_FALSE;
    }

    av_log_set_level(AV_LOG_TRACE);
    av_log_set_callback(av_log_to_logcat);

    return JNI_TRUE;
}

