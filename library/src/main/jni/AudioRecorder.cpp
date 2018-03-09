//
// Created by Elong on 2017/12/10.
//

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#include "AudioRecorder.h"
#include "AndroidUtils.h"
#include "BuffersStorage.h"


extern "C" void onError(int code, const char* msg);
extern "C" bool checkError(int ret, const char *errMsg);

bool AudioRecorder::prepare(AVFormatContext* formatContext){

    this->mFormatContext = formatContext;
    mCodec = avcodec_find_encoder(mCodecID);
    if(mCodec == nullptr){
        onError(0, "can't find audioEncoder");
        return false;
    }
    formatContext->audio_codec = mCodec;
    mCodecContext = avcodec_alloc_context3(mCodec);
    mCodecContext->codec = mCodec;
    mCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
    if ((mCodec->capabilities & CODEC_CAP_EXPERIMENTAL) != 0) {
        mCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    }
    mCodecContext->sample_rate = mDstSampleRate;
    if(mDstFmt == AV_SAMPLE_FMT_NONE){
        if (mCodec->id == AV_CODEC_ID_AAC &&
            (mCodec->capabilities & CODEC_CAP_EXPERIMENTAL) != 0) {
            mDstFmt = AV_SAMPLE_FMT_FLTP;
        } else {
            mDstFmt = AV_SAMPLE_FMT_S16;
        }
    }
    if(!checkSampleFmt(mCodec, mDstFmt)){
        onError(0, "Encoder does not support sample format");
        return false;
    }
//    mCodecContext->sample_fmt = mDstFmt;
//    mCodecContext->bit_rate = mBitRate;
//    mCodecContext->time_base = AVRational{1, mCodecContext->sample_rate};
//    mCodecContext->channel_layout = mDstChannelLayout;
//    mCodecContext->channels = av_get_channel_layout_nb_channels(mCodecContext->channel_layout);

    mCodecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    mCodecContext->sample_rate = 44100;
    mCodecContext->thread_count = 4;
//    mCodecContext->bit_rate = 50*1024*8;
    mCodecContext->channels = 1;
    mCodecContext->frame_size = 2048;
    mCodecContext->time_base = AVRational{1, mCodecContext->sample_rate};
    mCodecContext->channel_layout = mDstChannelLayout;
    if((formatContext->flags & AVFMT_GLOBALHEADER) != 0){
        mCodecContext->flags = mCodecContext->flags | CODEC_FLAG_GLOBAL_HEADER;
    }
    if((mCodec->capabilities & CODEC_CAP_EXPERIMENTAL) != 0){
        mCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    }


    mStream = avformat_new_stream(formatContext, mCodec);
    if(mStream == nullptr){
        onError(0, "avformat_new_stream for audio failed");
        return false;
    }

    int ret = avcodec_parameters_from_context(mStream->codecpar, mCodecContext);
    if(checkError(ret, "avcodec_parameters_from_context, failed")) return false;

    ret = avcodec_open2(mCodecContext, mCodec, NULL);
    if(checkError(ret, "avcodec_open2 audio failed")) return false;

    mStream->time_base = mStreamTimeBase;
//    mStream->time_base = mCodecContext->time_base;

    LOGD("audiorecorder->time_base:%d,%d", mStream->time_base.num, mStream->time_base.den);

    av_dump_format(mFormatContext, mStream->index, mFormatContext->filename, 1);

    swrContext = swr_alloc_set_opts(swrContext, mCodecContext->channel_layout, mCodecContext->sample_fmt, mCodecContext->sample_rate,
    mCodecContext->channel_layout, mSrcFmt, mCodecContext->sample_rate, 0, NULL);
    if(swrContext == nullptr){
        onError(ret, "swr_alloc failed");
        return false;
    }
    //TODO:需要转换时调用
//    ret = swr_init(swrContext);
    if(checkError(ret, "swr_init failed")) return false;

    mSrcFrame = av_frame_alloc();
    if(mSrcFrame == nullptr){
        onError(0, "av_frame_alloc failed for audio failed");
        return false;
    }
    mSrcFrame->format = mSrcFmt;
    //TODO:
//    int src_nb_samples = mSrcFrameSize / (av_get_bytes_per_sample(mSrcFmt) * mSrcChannels);
    int src_nb_samples = mSrcFrameSize;

    int dst_nb_samples = 0;
    if(mCodec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
        dst_nb_samples = 10000;
    } else {
        dst_nb_samples = mCodecContext->frame_size;
    }

    mSrcFrame->nb_samples = src_nb_samples;
    mSrcFrame->sample_rate = mSrcSampleRate;
    mSrcFrame->channel_layout = mSrcChannelLayout;
    mSrcFrame->channels = mSrcChannels;
    ret = av_frame_get_buffer(mSrcFrame, 0);
    if(checkError(ret, "av_frame_get_buffer for audio mSrcFrame failed")) return false;

    mDstFrame = av_frame_alloc();
    if(mDstFrame == nullptr){
        onError(0, "av_frame_alloc failed for audio failed");
        return false;
    }
    mDstFrame->format = mCodecContext->sample_fmt;
    mDstFrame->sample_rate = mCodecContext->sample_rate;
    mDstFrame->nb_samples = 1024;
    mDstFrame->channel_layout =  mCodecContext->channel_layout;
    mDstFrame->channels = mCodecContext->channels;
    ret = av_frame_get_buffer(mDstFrame, 0);
    if(checkError(ret, "av_frame_get_buffer for audio mDstFrame failed")) return false;

    mPacket = av_packet_alloc();
    if(mPacket == nullptr){
        onError(0, "av_packet_alloc failed");
        return false;
    }

    return true;
}


bool AudioRecorder::encodeAndRecord(NativeByteBuffer* data){
    int ret = avcodec_fill_audio_frame(mDstFrame, mSrcChannels, mSrcFmt, data->bytes(), data->limit(), 0);
    int64_t pts = getDataPts(data, &mCodecContext->time_base);
    mDstFrame->pts = pts;
    mDstFrame->quality = mCodecContext->global_quality;
    LOGD("audio pts=%lld", mDstFrame->pts);
    LOGD("audiorecorder->time_base:%d,%d", mStream->time_base.num, mStream->time_base.den);
    int outSize = av_samples_get_buffer_size(NULL, mDstFrame->channels, mDstFrame->nb_samples,
                                            (AVSampleFormat) mDstFrame->format, 0);
    LOGD("audio inputSize:%d, fill_in_frame:%d, outBufSize:%d", data->limit(), ret, outSize);
    BuffersStorage::getInstance().reuse(data);
    if(checkError(ret, "avcodec_fill_audio_frame failed")) return false;

//    LOGD("audio srcFormat:%d, dstFrame format:%d, channels:%d, channelLayout:%d, sampleRate:%d", mSrcFrame->format, mDstFrame->format, mDstFrame->channels, (int)mDstFrame->channel_layout, mDstFrame->sample_rate);

    //TODO: 如果格式不一，或采样比不一再做重采样
//    ret = swr_convert_frame(swrContext, mDstFrame, mSrcFrame);
    if(checkError(ret, "swr_convert_frame failed")) return false;

    LOGD("audio mDstFrame->pts=%lld, stream.time_base:%d,%d", mDstFrame->pts, mStream->time_base.num, mStream->time_base.den);
    fillAndWritePacket(mCodecContext, mStream, mPacket, mDstFrame, pts);
    return true;
}

void AudioRecorder::end(){
}

void AudioRecorder::release(){
    mCodec = nullptr;
    if(mCodecContext != nullptr) {
        avcodec_close(mCodecContext);
        avcodec_free_context(&mCodecContext);
        mCodecContext = nullptr;
    }
    if(mSrcFrame != nullptr){
        av_frame_free(&mSrcFrame);
        mSrcFrame = nullptr;
    }
    if(mDstFrame != nullptr){
        av_frame_free(&mDstFrame);
        mDstFrame = nullptr;
    }
    if(mPacket != nullptr){
        av_packet_unref(mPacket);
        av_packet_free(&mPacket);
        mPacket = nullptr;
    }
}

bool AudioRecorder::checkSampleFmt(const AVCodec* codec, AVSampleFormat sampleFormat){
    const AVSampleFormat *fmt = codec->sample_fmts;
    while(*fmt != AV_SAMPLE_FMT_NONE){
        if(*fmt == sampleFormat){
            return true;
        }
        ++fmt;
    }
    return false;
}

int AudioRecorder::selectSampleRate(const AVCodec* codec){
    const int* p;
    int bestSampleRate = 0;
    if(!codec->supported_samplerates){
        return 44100;
    }
    p = codec->supported_samplerates;
    while(*p){
        if(!bestSampleRate || abs(44100 - *p ) < abs(44100 - bestSampleRate)){
            bestSampleRate = *p;
        }
        ++p;
    }
    return bestSampleRate;
}

int AudioRecorder::selectChannelLayout(const AVCodec *codec) {
    const uint64_t *p;
    uint64_t bestChLayout = 0;
    int bestNbChannels = 0;
    if(!codec->channel_layouts){
        return AV_CH_LAYOUT_STEREO;
    }
    p = codec->channel_layouts;
    while(*p){
        int nbChannels = av_get_channel_layout_nb_channels(*p);
        if(nbChannels > bestChLayout){
            bestChLayout = *p;
            bestNbChannels = nbChannels;
        }
        ++p;
    }
    return bestChLayout;
}

