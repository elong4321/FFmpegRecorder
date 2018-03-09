//
// Created by Elong on 2017/12/9.
//

#ifndef FFMPEGRECORDER_IENCODER_H
#define FFMPEGRECORDER_IENCODER_H

#include <libavformat/avformat.h>
#include "NativeByteBuffer.h"

#define TIME_STAMP_LEN 8

class IRecorder {
public:
    IRecorder(){};
    virtual ~IRecorder(){};
    virtual bool prepare(AVFormatContext* formatContext) = 0;
    virtual bool encodeAndRecord(NativeByteBuffer* buffer) = 0;
    int64_t getDataTimestamp(NativeByteBuffer* data);
    int64_t getDataPts(NativeByteBuffer* data, AVRational* timeBase);
    bool fillAndWritePacket(AVCodecContext *codecContext, AVStream* stream, AVPacket *avPacket, AVFrame *frame, int64_t pts);
    void writePacket(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* stream, AVPacket* packet);
    virtual void end() = 0;
    virtual void release() = 0;

protected:
    AVFormatContext* mFormatContext;
    AVRational mRecordTimsBase = AV_TIME_BASE_Q;
    AVRational mStreamTimeBase = AV_TIME_BASE_Q;
    bool mInterleave = true;

};


#endif //FFMPEGRECORDER_IENCODER_H
