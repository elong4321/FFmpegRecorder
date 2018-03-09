//
// Created by Elong on 2017/1/2.
//

#ifndef FFMPEGRECORDER_FFMPEGRECORDER_H
#define FFMPEGRECORDER_FFMPEGRECORDER_H

#include <jni.h>
#include <pthread.h>
#include <map>
#include <list>

#include "libavformat/avformat.h"
#include "libavutil/samplefmt.h"
#include "NativeByteBuffer.h"
#include "IRecorder.h"

class FFmpegRecorder {

public:
    pthread_t recordThread;
    JNIEnv* jniEnv;
    jobject startLatch;

    FFmpegRecorder(){}
    ~FFmpegRecorder(){}
    void recordVideo(NativeByteBuffer* data);
    void recordAudio(NativeByteBuffer* data);
    void setFormatParams(char *type, char *filePath);
    void setVideoParams(AVPixelFormat format, int frameRate, int srcWidth, int srcHeight, int dstWidth, int dstHeight);
    void setAudioParams(AVSampleFormat format, int frameSize, int channels, uint64_t channelLayout, int sampleRate);

    void run();
    void stop();
    void tryRelease();

private:
    pthread_mutex_t mutex;
    volatile bool recording = false;
    volatile bool releaseMarked = false;
    volatile bool ended = false;
    int wakeEventFd = 0;
    int64_t startTime;

    AVFormatContext* formatContext = nullptr;
    std::list<NativeByteBuffer*> videoQueue;
    IRecorder* mVieoRecorder;
    std::list<NativeByteBuffer*> audioQueue;
    IRecorder* mAudioRecorder;


    char* mFileType = nullptr;
    char* mFilePath = nullptr;
    uint32_t mSrcWidth, mSrcHeight, mDstWidth, mDstHeight;
    AVPixelFormat mPixelFormat;
    int mFrameRate;

    AVSampleFormat mAudioFormat;
    int mFrameSize;
    int mAudioChannels = 1;
    uint64_t mAudioChannelLayout =  AV_CH_LAYOUT_MONO;
    int mAudioSampleRate = 44100;

    bool prepare();
    void record();
    void waitForData();
    void wakeUp();
    NativeByteBuffer* dequeue(std::list<NativeByteBuffer*>* queue, bool* empty);

    void end();
    void release();
};


#endif //FFMPEGRECORDER_FFMPEGRECORDER_H
