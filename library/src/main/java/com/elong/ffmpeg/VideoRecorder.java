package com.elong.ffmpeg;

import android.media.Image;

import com.elong.ffmpeg.com.elong.ffmpeg.camera.ICamera;

/**
 * Created by Elong on 2017/12/10.
 */

public class VideoRecorder implements ICamera.IPreviewCallback{

    FFmpegRecorder mFFmpegRecorder;
    ThreadLocal<NativeByteBuffer> mBufferProvider = null;
    volatile boolean mRecording = false;

    public VideoRecorder(FFmpegRecorder fFmpegRecorder, ThreadLocal<NativeByteBuffer> bufferProvider){
        mFFmpegRecorder = fFmpegRecorder;
        mBufferProvider = bufferProvider;
    }

    @Override
    public void onPreviewFrameV1(byte[] data) {
        if (mRecording) {
            NativeByteBuffer buffer = mBufferProvider.get();
            int len = data.length + FFmpegRecorder.TS_LEN;
            buffer.init(BuffersStorage.getFreeByteBuffer(len), len);
            buffer.put(data);
            buffer.native_position(buffer.mAddress, data.length);
            mFFmpegRecorder.recordVideo(buffer);
        }
    }

    @Override
    public void onPreviewFrameV2(Image image) {
        if (mRecording) {
            NativeByteBuffer buffer = mBufferProvider.get();
            Image.Plane[] planes = image.getPlanes();
            int dataLen = 0;
            for(Image.Plane plane : planes){
                dataLen += plane.getBuffer().limit();
            }

            int len = dataLen + FFmpegRecorder.TS_LEN;
            buffer.init(BuffersStorage.getFreeByteBuffer(len), len);

            for(Image.Plane plane : planes){
                buffer.put(plane.getBuffer());
            }
            buffer.native_position(buffer.mAddress, dataLen);
            mFFmpegRecorder.recordVideo(buffer);
        }
    }

    public void startRecord(){
        mRecording = true;
    }

    public void stopRecord(){
        mRecording = false;
    }
}
