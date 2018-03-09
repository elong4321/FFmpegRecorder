//
// Created by Elong on 2017/12/9.
//

extern "C" {
#include "IRecorder.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avstring.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"
}

#include "AndroidUtils.h"

extern "C"  void onError(int code, const char* msg);
extern "C"  bool checkError(int ret, const char *errMsg);

int64_t IRecorder::getDataTimestamp(NativeByteBuffer* data){
    LOGD("video getDataTimestamp, position:%d, limit:%d", data->position(), data->limit());
    data->position(data->limit() - TIME_STAMP_LEN);
    LOGD("video getDataTimestamp, position:%d, limit:%d", data->position(), data->limit());

    return data->readInt64();;
}

int64_t IRecorder::getDataPts(NativeByteBuffer* data, AVRational* timeBase){
    int64_t ts = getDataTimestamp(data);
    int64_t pts = av_rescale_q(ts, mRecordTimsBase, *timeBase);
    LOGD("getDataPts, ts:%lld, pts:%lld", ts, pts);
    return pts;
}

bool IRecorder::fillAndWritePacket(AVCodecContext *codecContext, AVStream* stream, AVPacket *packet,
                                        AVFrame *frame, int64_t pts){
    bool received = false;
    if(frame){
        LOGD("fillAndWritePacket frame.pts=%lld", frame->pts);
    }
    int ret = avcodec_send_frame(codecContext, frame);
    if(ret < 0){
        checkError(ret, "Error sending a frame for encoding");
        return false;
    }
    while(ret >= 0){
        av_init_packet(packet);
        ret = avcodec_receive_packet(codecContext, packet);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return received;
        }else if(ret < 0){
            onError(ret, "Error during encoding\n");
            return false;
        }
//        packet->pts = pts;
        writePacket(mFormatContext, codecContext, stream, packet);
        received = true;
    }
    return received;
};

void IRecorder::writePacket(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* stream, AVPacket* packet){
//    av_packet_rescale_ts(packet, codecContext->time_base, stream->time_base);
    LOGD("writePacket, pts:%lld, st.time_base.num:%d, st.time_base.den:%d", packet->pts, stream->time_base.num, stream->time_base.den);
    packet->stream_index = stream->index;

    if(packet->pts != AV_NOPTS_VALUE){
//        packet->pts = av_rescale_q(packet->pts, stream->time_base, codecContext->time_base);
        packet->pts = av_rescale_q(packet->pts, codecContext->time_base, stream->time_base);
    }
    if(packet->dts != AV_NOPTS_VALUE){
//        packet->dts = av_rescale_q(packet->dts, stream->time_base, codecContext->time_base);
        packet->dts = av_rescale_q(packet->dts, codecContext->time_base, stream->time_base);
    }
//    av_packet_rescale_ts(packet, codecContext->time_base, stream->time_base);
    LOGD("writePacket, st:%d(%d), pts:%lld, size:%d ", stream->index, packet->stream_index, packet->pts, packet->size);
    int ret = 0;
    if(mInterleave){
        ret = av_interleaved_write_frame(formatContext, packet);
        checkError(ret, "av_interleaved_write_frame failed");
    }else{
        ret = av_write_frame(formatContext, packet);
        checkError(ret, "av_write_frame failed");
    }
    if(ret < 0 ){
        LOGE("write_frame failed, size:%d, pts:%lld, dts:%lld, st:%d", packet->size, packet->pts, packet->dts, packet->stream_index);
    }

}