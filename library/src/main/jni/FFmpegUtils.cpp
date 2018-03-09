//
// Created by Elong on 2017/11/28.
//
extern "C" {
#include "AndroidUtils.h"
#include "libavutil/log.h"

void onError(int code, const char* msg) {
    int size = 512;
    char* errBuf = new char[size];
    av_strerror(code, errBuf, size);
    LOGE("%s, due to %s\n", msg, errBuf);
    delete[](errBuf);
}

bool checkError(int ret, const char *errMsg) {
    if(ret < 0){
        onError(ret, errMsg);
    }
    return ret < 0;
}

void av_log_to_logcat(void *ptr, int level, const char *fmt, va_list vl) {
    int ffplv;
    switch (level) {
        case AV_LOG_ERROR:
            ffplv = ANDROID_LOG_ERROR;
            break;
        case AV_LOG_WARNING:
            ffplv = ANDROID_LOG_WARN;
            break;
        case AV_LOG_INFO:
            ffplv = ANDROID_LOG_INFO;
            break;
        case AV_LOG_VERBOSE:
            ffplv = ANDROID_LOG_VERBOSE;
        default:
            ffplv = ANDROID_LOG_DEBUG;
            break;
    }
    va_list vl2;
    char line[1024];
    static int print_prefix = 1;
    va_copy(vl2, vl);
    av_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);
    va_end(vl2);
    LOG(ffplv, "ffmpeg", "%s", line);
}

}