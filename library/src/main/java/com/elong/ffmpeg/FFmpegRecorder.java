package com.elong.ffmpeg;

import android.util.Log;

import java.util.concurrent.CountDownLatch;

/**
 * Created by Elong on 2017/12/20.
 */

public class FFmpegRecorder {
    public static final int TS_LEN = 8; //时间戳长度，在native层会加入时间戳

    private int mPtr = 0;
    private int mAudioBufferSize = 2048;

    Params mParams;

    static{
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("ffmpeg");
    }

    public FFmpegRecorder(Params params){
        mPtr = native_new();
        mParams = params;
        setFormatParams(params.mFileType, params.mFilePath);
        setVideoParams(params.mVideoFormat, params.mFrameRate, params.mSrcWidth, params.mSrcHeight, params.mDstWidth, params.mDstHeight);
        setAudioParams(params.mAudioFormat, params.mBufferSize, params.mChannelCount, params.mChannelLayout, params.mSampleRate);
    }

    public void setFormatParams(String type, String filePath){
        native_setFormatParams(mPtr, type, filePath);
    }

    public void setVideoParams(int fotmat, int frameRate, int srcWidth, int srcHeight, int dstWidth, int dstHeight){
        native_setVideoParams(mPtr, fotmat, frameRate, srcWidth, srcHeight, dstWidth, dstHeight);
    }

    public void setAudioParams(int format, int bufferSize, int channelCount, int channelLayout, int sampleRate){
        mAudioBufferSize =native_setAudioParams(mPtr, format, bufferSize, channelCount, channelLayout, sampleRate);
        Log.e("native_setAudioParams", "size= " + mAudioBufferSize);
    }

    public void start(Runnable callback) throws InterruptedException {
        CountDownLatch startLatch = new CountDownLatch(1);
        native_start(mPtr, startLatch);
        startLatch.await();
        callback.run();
    }

    public void recordVideo(NativeByteBuffer buffer){
        native_recordVideo(mPtr, buffer.getAddress());
    }

    public void recordAudio(NativeByteBuffer buffer){
        native_recordAudio(mPtr, buffer.getAddress());
    }

    public void stop(){
        native_stop(mPtr);
    }

    public void release(){
        native_release(mPtr);
        mPtr = 0;
    }

    public int getAudioBufferSize() {
        return mAudioBufferSize;
    }

    private native void native_setFormatParams(int ptr, String type, String filePath);
    private native void native_setVideoParams(int ptr, int fotmat, int frameRate, int srcWidth, int srcHeight, int dstWidth, int dstHeight);
    private native int native_setAudioParams(int ptr, int format, int bufferSize, int channelCount, int channelLayout, int sampleRate);
    private native int native_new();
    private native void native_start(int ptr, CountDownLatch latch);
    private native void native_stop(int ptr);
    private native void native_release(int ptr);
    private native void native_recordVideo(int ptr, int buff);
    private native void native_recordAudio(int ptr, int buff);

    public static class Params{
        public String mFileType;
        public String mFilePath;
        public int mVideoFormat;
        public int mFrameRate;
        public int mSrcWidth;
        public int mSrcHeight;
        public int mDstWidth;
        public int mDstHeight;
        public int mBufferSize;
        public int mAudioFormat;
        public int mChannelCount;
        public int mChannelLayout;
        public int mSampleRate;
    }

    public static class Builder{
        private Params mParams = new Params();

        public Builder setFormatParams(String type, String filePath){
            mParams.mFileType = type;
            mParams.mFilePath = filePath;
            return this;
        }

        public Builder setVideoParams(int format, int frameRate, int srcWidth, int srcHeight, int dstWidth, int dstHeight){
            mParams.mVideoFormat = format;
            mParams.mFrameRate = frameRate;
            mParams.mSrcWidth = srcWidth;
            mParams.mSrcHeight = srcHeight;
            mParams.mDstWidth = dstWidth;
            mParams.mDstHeight = dstHeight;
            return this;
        }

        public Builder setAudioParams(int format, int bufferSize, int channelCount, int channelLayout, int sampleRate){
            mParams.mAudioFormat = format;
            mParams.mBufferSize = bufferSize;
            mParams.mChannelCount = channelCount;
            mParams.mChannelLayout = channelLayout;
            mParams.mSampleRate = sampleRate;
            return this;
        }

        public FFmpegRecorder build(){
            FFmpegRecorder recorder = new FFmpegRecorder(mParams);
            return recorder;
        }
    }
}
