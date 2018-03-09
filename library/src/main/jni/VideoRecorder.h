//
// Created by Elong on 2017/12/8.
//

#ifndef FFMPEGRECORDER_VIDEORECORDER_H
#define FFMPEGRECORDER_VIDEORECORDER_H

#include "libswscale/swscale.h"
#include "IRecorder.h"

class VideoRecorder : public IRecorder {
public:
    VideoRecorder(uint32_t srcWidth, uint32_t srcHeight, AVPixelFormat format, int frameRate, uint32_t dstWidth, uint32_t dstHeight)
    :mSrcWidth(srcWidth), mSrcHeight(srcHeight), mDstWidth(dstWidth), mDstHeight(dstHeight),
    mSrcFmt(format), mFrameRate(frameRate){};
    ~VideoRecorder(){};
    virtual bool prepare(AVFormatContext* formatContext);
    virtual bool encodeAndRecord(NativeByteBuffer* data);
    virtual void release();
    virtual void end();

private:
    AVCodecID  mCodecID = AV_CODEC_ID_H264;
    AVPixelFormat mSrcFmt, mDstFmt = AV_PIX_FMT_YUV420P;
    uint32_t mSrcWidth, mSrcHeight, mDstWidth, mDstHeight;
    int mFrameRate;
    int mBitRate = 400000;
    int mSampleRate = 90000;
    int mGopSize = 50;
    int mMaxBFrames = 0;
    int mAlign = 1;
    AVCodecContext *mCodecContext;
    AVCodec *mCodec;
    AVStream *mStream;
    AVFrame* mSrcFrame;
    AVFrame* mDstFrame;
    AVPacket* mPacket;
    SwsContext *mSwsContext;
    uint64_t mVideoPts = 1;

    void fillVideoFrame(NativeByteBuffer* data);
};


#endif //FFMPEGRECORDER_VIDEORECORDER_H
