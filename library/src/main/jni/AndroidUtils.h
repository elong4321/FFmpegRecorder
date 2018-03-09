//
// Created by Elong on 2017/10/19.
//

#ifndef SOCKET_ANDROIDUTILS_H
#define SOCKET_ANDROIDUTILS_H

#include <android/log.h>

#define LOG_TAG "ffmpeg_re"
#define LOG(level, TAG, ...) __android_log_print(level, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)

#endif //SOCKET_ANDROIDUTILS_H
