//
// Created by Elong on 2017/12/8.
//

extern "C" {
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#include <libavutil/time.h>
#include "BuffersStorage.h"
#include "VideoRecorder.h"
#include "IRecorder.h"
#include "AndroidUtils.h"

extern "C" void onError(int code, const char* msg);
extern "C" bool checkError(int ret, const char *errMsg);


bool VideoRecorder::prepare(AVFormatContext *formatContext) {
    this->mFormatContext = formatContext;
    mCodec = avcodec_find_encoder(mCodecID);
    if(mCodec == nullptr){
        onError(0, "can't find encoder");
        return false;
    }
    formatContext->video_codec = mCodec;
    formatContext->oformat->video_codec = mCodec->id;
    mCodecContext = avcodec_alloc_context3(mCodec);
    if(mCodecContext == nullptr){
        onError(0, "avcodec_alloc_context3 failed");
        return false;
    }
    mCodecContext->codec = mCodec;
    mCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    if (mCodec->id == AV_CODEC_ID_H264){
        mCodecContext->profile = FF_PROFILE_H264_CONSTRAINED_BASELINE;
        av_opt_set(mCodecContext->priv_data, "preset", "ultrafast", 0);//编码器的速度会影响推流音视频同步,所以这里需要设置下
//    av_dict_set(&opts, "tune", "zerolatency", 0);//如果开0延迟可能会影响视频质量
        av_opt_set(mCodecContext->priv_data, "profile", "baseline", 0);//I/P帧
        av_opt_set(mCodecContext->priv_data, "bf", "4", 0);
        av_opt_set(mCodecContext->priv_data, "qmin", "30", 0);
        av_opt_set(mCodecContext->priv_data, "qdiff", "10", 0);
        av_opt_set(mCodecContext->priv_data, "x264opts", "keyint=50:min-keyint=25:chroma-qp-offset=5:direct=spatial:ipratio=1.4:pbratio=1.2", 0);
        av_opt_set(mCodecContext->priv_data, "x264-params", "cabac=1", 0);
        av_opt_set(mCodecContext->priv_data, "me_method", "hex", 0);
        av_opt_set(mCodecContext->priv_data, "partitions", "all", 0);
    }
    if((mCodec->capabilities & CODEC_CAP_EXPERIMENTAL) != 0){
        mCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    }
    if((formatContext->flags & AVFMT_GLOBALHEADER) != 0){
        mCodecContext->flags = mCodecContext->flags | CODEC_FLAG_GLOBAL_HEADER;
    }

    mCodecContext->pix_fmt = mDstFmt;
    mCodecContext->gop_size = mGopSize;
    mCodecContext->max_b_frames = mMaxBFrames;
    AVRational fr{mFrameRate, 1};
    const AVRational* supportedFramerates = mCodec->supported_framerates;
    if (supportedFramerates != NULL) {
        int idx = av_find_nearest_q_idx(fr, supportedFramerates);
        fr = supportedFramerates[idx];
    }
    mCodecContext->framerate = fr;
    //TODO: 这个才是正确的？
    mCodecContext->time_base = AVRational{fr.den, fr.num};
    mCodecContext->width = mDstWidth;
    mCodecContext->height = mDstHeight;
//    mCodecContext->bit_rate = mBitRate;
    mCodecContext->thread_count = 4;

    mStream = avformat_new_stream(formatContext, mCodec);

    int ret = avcodec_parameters_from_context(mStream->codecpar, mCodecContext);
    if(checkError(ret, "avcodec_parameters_from_context, failed")) return false;

    ret = avcodec_open2(mCodecContext, mCodec, NULL);
    if(checkError(ret, "avcodec_open2 failed")) return false;

    mStream->time_base = mStreamTimeBase;

//    av_dump_format(mFormatContext, mStream->index, mFormatContext->filename, 1);

    mSwsContext = sws_getContext(mSrcWidth, mSrcHeight, mSrcFmt, mDstWidth, mDstHeight, mDstFmt, SWS_BICUBIC, NULL, NULL, NULL);
//    videoSwsContext = sws_getContext(srcWidth, srcHeight, srcFmt, dstWidth, dstHeight, srcFmt, SWS_BICUBIC, NULL, NULL, NULL);

    mSrcFrame = av_frame_alloc();
    mSrcFrame->format = mSrcFmt;
    mSrcFrame->width = mSrcWidth;
    mSrcFrame->height = mSrcHeight;
    av_frame_get_buffer(mSrcFrame, mAlign);

    mDstFrame = av_frame_alloc();
    mDstFrame->format = mDstFmt;
    mDstFrame->width = mDstWidth;
    mDstFrame->height = mDstHeight;
    av_frame_get_buffer(mDstFrame, mAlign);

    mPacket = av_packet_alloc();

    return true;
}

bool VideoRecorder::encodeAndRecord(NativeByteBuffer* data){
    LOGD("video encodeAndRecord");
    fillVideoFrame(data);
    int64_t ts = getDataPts(data, &mCodecContext->time_base);
    mDstFrame->pts = ts;
    LOGD("audio mDstFrame->pts=%lld", mDstFrame->pts);
    BuffersStorage::getInstance().reuse(data);
    bool received = fillAndWritePacket(mCodecContext, mStream, mPacket, mDstFrame, ts);
    return true;
}

void VideoRecorder::fillVideoFrame(NativeByteBuffer* data) {
    mSrcFrame->quality = mCodecContext->global_quality;
    mDstFrame->quality = mCodecContext->global_quality;
    int ret = av_image_fill_arrays(mSrcFrame->data, mSrcFrame->linesize, data->bytes(), mSrcFmt, this->mSrcWidth, this->mSrcHeight, mAlign);
    if(ret < 0){
        onError(ret, "fillVideoFrame failed");
        return;
    }
    ret = sws_scale(mSwsContext, (const uint8_t* const*)mSrcFrame->data, mSrcFrame->linesize, 0, mSrcHeight, mDstFrame->data, mDstFrame->linesize);
    if(ret < 0){
        onError(ret, "sws_scale failed");
        return;
    }
}

void VideoRecorder::end(){
}

void VideoRecorder::release(){
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
    if(mSwsContext != nullptr) {
        sws_freeContext(mSwsContext);
        mSwsContext = nullptr;
    }
}