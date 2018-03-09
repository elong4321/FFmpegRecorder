//
// Created by Elong on 2017/1/2.
//

#include <sys/eventfd.h>


extern "C" {
#include "libavformat/avformat.h"
#include <libavutil/time.h>
}

#include "FFmpegRecorder.h"
#include "AndroidUtils.h"
#include "VideoRecorder.h"
#include "AudioRecorder.h"
#include "IRecorder.h"
#include "BuffersStorage.h"

extern "C" void onError(int code, const char* msg);
extern "C" bool checkError(int ret, const char *errMsg);

extern JavaVM* javaVM;

void* onThreadCreate(void* data){
    LOGI("onThreadCreate");
    FFmpegRecorder* recorder = (FFmpegRecorder *) data;
    javaVM->AttachCurrentThread(&recorder->jniEnv, nullptr);
    recorder->run();
    javaVM->DetachCurrentThread();
    recorder->jniEnv = nullptr;
    return 0;
}

void startRecord(JNIEnv * env, FFmpegRecorder* recorder, jobject latch){
    recorder->startLatch = env->NewGlobalRef(latch);
    pthread_create(&recorder->recordThread, NULL, onThreadCreate, recorder);
}

void FFmpegRecorder::run() {
    if(prepare()) {
        recording = true;
        while (recording) {
            record();
            waitForData();
        }
        recording = false;
    }
    end();
}

bool FFmpegRecorder::prepare() {
    mVieoRecorder = new VideoRecorder(mSrcWidth, mSrcHeight, mPixelFormat, mFrameRate, mDstWidth, mDstHeight);
    mAudioRecorder = new AudioRecorder(mAudioFormat, mFrameSize, mAudioChannels, mAudioChannelLayout, mAudioSampleRate);

    av_register_all();
    int ret = avformat_alloc_output_context2(&formatContext, NULL, NULL, mFilePath);
    if(checkError(ret, "avformat_alloc_output_context2 failed")) return false;


    ret = avio_open(&formatContext->pb, formatContext->filename, AVIO_FLAG_WRITE);
    if(checkError(ret, "avio_open failed")) return false;

    mVieoRecorder->prepare(formatContext);
    mAudioRecorder->prepare(formatContext);

    ret = avformat_write_header(formatContext, NULL);
    if(checkError(ret, "avformat_write_header failed")) return false;

    wakeEventFd = eventfd(0, 0);
    pthread_mutex_init(&mutex,  NULL);

    startTime = av_gettime();

    //启动完成，通知java端
    jclass class_CountDownLatch = jniEnv->FindClass("java/util/concurrent/CountDownLatch");
    jmethodID  method_countDown = jniEnv->GetMethodID(class_CountDownLatch, "countDown", "()V");
    jniEnv->CallVoidMethod(startLatch, method_countDown);
    jniEnv->DeleteGlobalRef(startLatch);
    startLatch = nullptr;
    jniEnv->DeleteLocalRef(class_CountDownLatch);

    return true;
}

void FFmpegRecorder::record(){
    LOGD("record");
    bool videoEnd = false, audioEnd = false;
    std::list<NativeByteBuffer*>::iterator videoIter;
    std::list<NativeByteBuffer*>::iterator audioIter;
    NativeByteBuffer* videoData;
    NativeByteBuffer* audioData;
    bool vQueueEmpty;
    bool aQueueEmpty;
    do{
        pthread_mutex_lock(&mutex);
        videoData = dequeue(&videoQueue, &vQueueEmpty);
        audioData = dequeue(&audioQueue, &aQueueEmpty);
        pthread_mutex_unlock(&mutex);
        if(videoData != nullptr && mVieoRecorder != nullptr){
            mVieoRecorder->encodeAndRecord(videoData);
        }else if(mVieoRecorder == nullptr){
            BuffersStorage::getInstance().reuse(videoData);
        }
        if(audioData != nullptr && mAudioRecorder != nullptr){
            mAudioRecorder->encodeAndRecord(audioData);
        }else if(mAudioRecorder == nullptr){
            BuffersStorage::getInstance().reuse(audioData);
        }
    }while(!vQueueEmpty || !aQueueEmpty);
}

void FFmpegRecorder::wakeUp(){
    LOGD("wakeUp");
    eventfd_write(wakeEventFd, 1);
}

NativeByteBuffer* FFmpegRecorder::dequeue(std::list<NativeByteBuffer*>* queue, bool* empty){
    NativeByteBuffer* buffer = nullptr;
    int size = queue->size();
    *empty = size < 2;     //出队一个数据以后是否空
    if(size > 0){
        buffer = *queue->begin();
        queue->erase(queue->begin());
        return buffer;
    }
    return nullptr;
}

void FFmpegRecorder::waitForData(){
    LOGD("waitForData");
    eventfd_t counter;
    eventfd_read(wakeEventFd, &counter);
}

void FFmpegRecorder::end(){
    LOGD("end");
    if(mVieoRecorder != nullptr){
        mAudioRecorder->end();
    }
    if(mAudioRecorder != nullptr){
        mAudioRecorder->end();
    }
    if(formatContext != nullptr){
        int ret = av_write_trailer(formatContext);
        checkError(ret, "av_write_trailer failed");
    }
    ended = true;
    release();
}

void FFmpegRecorder::tryRelease() {
    releaseMarked = true;
    release();
}

void FFmpegRecorder::release() {
    if(!ended || !releaseMarked) return;
    LOGD("release");

    if(formatContext != nullptr){
        if(formatContext->pb){
            avio_close(formatContext->pb);
        }
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }

    if(mVieoRecorder != nullptr){
        mVieoRecorder->release();
        delete mVieoRecorder;
        mVieoRecorder = nullptr;
    }
    if(mAudioRecorder != nullptr){
        mAudioRecorder->release();
        delete mAudioRecorder;
        mAudioRecorder = nullptr;
    }

    std::list<NativeByteBuffer*>::iterator bufferIter;
    NativeByteBuffer* buffer;
    for(bufferIter = videoQueue.begin(); bufferIter != videoQueue.end(); bufferIter++){
        buffer = *bufferIter;
        BuffersStorage::getInstance().reuse(buffer);
    }
    videoQueue.clear();
    for(bufferIter = audioQueue.begin(); bufferIter != audioQueue.end(); bufferIter++){
        buffer = *bufferIter;
        BuffersStorage::getInstance().reuse(buffer);
    }
    audioQueue.clear();

    if(mFileType != nullptr){
        free(mFileType);
        mFileType = nullptr;
    }
    if(mFilePath != nullptr){
        free(mFilePath);
        mFilePath = nullptr;
    }
    pthread_mutex_destroy(&mutex);

}

void FFmpegRecorder::stop() {
    recording = false;
    wakeUp();
    checkError(0, "stopRecord 1");
//    if(recordThread != NULL){
//        pthread_join(recordThread, nullptr);
//        checkError(0, "stopRecord 2");
//    }
}

void FFmpegRecorder::recordVideo(NativeByteBuffer* data){
    data->writeInt64(av_gettime() - startTime);
    data->flip();
    pthread_mutex_lock(&mutex);
    videoQueue.push_back(data);
    pthread_mutex_unlock(&mutex);
    wakeUp();
}

void FFmpegRecorder::recordAudio(NativeByteBuffer* data){
    data->writeInt64(av_gettime() - startTime);
    data->flip();
    pthread_mutex_lock(&mutex);
    audioQueue.push_back(data);
    pthread_mutex_unlock(&mutex);
    wakeUp();
}

void FFmpegRecorder::setFormatParams(char *type, char *filePath) {
    mFileType = type;
    mFilePath = filePath;
}

void FFmpegRecorder::setVideoParams(AVPixelFormat format, int frameRate, int srcWidth, int srcHeight, int dstWidth, int dstHeight){
    this->mPixelFormat = format;
    this->mFrameRate = frameRate;
    this->mSrcWidth = srcWidth;
    this->mSrcHeight = srcHeight;
    this->mDstWidth = dstWidth;
    this->mDstHeight = dstHeight;
}

void FFmpegRecorder::setAudioParams(AVSampleFormat format, int frameSize, int channels, uint64_t channelLayout, int sampleRate) {
    mAudioFormat = format;
    mFrameSize = frameSize;
    mAudioChannels = channels;
    mAudioChannelLayout = channelLayout;
    mAudioSampleRate = sampleRate;
}
