//
// Created by Elong on 2017/12/10
//

#ifndef FFMPEGRECORDER_AUDIORECORDER_H
#define FFMPEGRECORDER_AUDIORECORDER_H



extern "C" {
#include "libavutil/samplefmt.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};

#include "IRecorder.h"

class AudioRecorder : public IRecorder{
public:
    AudioRecorder(AVSampleFormat format, int frameSize, int channels, uint64_t channelLayout, int sampleRate)
    :mSrcFmt(format), mSrcFrameSize(frameSize),  mSrcChannels(channels), mSrcChannelLayout(channelLayout), mSrcSampleRate(sampleRate){};
    ~AudioRecorder(){};
    virtual bool prepare(AVFormatContext* formatContext);
    virtual bool encodeAndRecord(NativeByteBuffer* data);
    virtual void end();
    virtual void release();

private:
    AVSampleFormat mSrcFmt = AV_SAMPLE_FMT_NONE;
    AVSampleFormat mDstFmt = AV_SAMPLE_FMT_NONE;
    AVCodecID mCodecID = AV_CODEC_ID_AAC;
    int mSrcFrameSize;
    int mSrcChannels = 1;
    uint64_t mSrcChannelLayout = AV_CH_LAYOUT_MONO;
    uint64_t mDstChannelLayout = AV_CH_LAYOUT_MONO;
    int mBitRate = 64000;
    int mSrcSampleRate = 44100, mDstSampleRate = 44100;
    AVCodec* mCodec = nullptr;
    AVCodecContext* mCodecContext = nullptr;
    SwrContext* swrContext = nullptr;
    AVStream* mStream = nullptr;
    AVFrame* mSrcFrame = nullptr;
    AVFrame* mDstFrame = nullptr;
    AVPacket* mPacket = nullptr;

    bool checkSampleFmt(const AVCodec* codec, AVSampleFormat sampleFormat);
    int selectSampleRate(const AVCodec* codec);
    int selectChannelLayout(const AVCodec *codec);
};


#endif //FFMPEGRECORDER_AUDIORECORDER_H
