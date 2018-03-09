package com.elong.ffmpeg;

import android.media.AudioRecord;
import android.util.Log;

/**
 * Created by Elong on 2017/12/6.
 */

public class AudioRecorder implements Runnable{
    private FFmpegRecorder mFFmpegRecorder;
    private AudioRecord mAudioRecord;
    private ThreadLocal<NativeByteBuffer> mBufferProvider = null;
    private int mBufferSize = 0;
    private volatile boolean mRecording = false;
    private Thread mThread;

    public AudioRecorder(FFmpegRecorder fFmpegRecorder, AudioRecord audioRecord, ThreadLocal<NativeByteBuffer> bufferProvider, int bufferSize){
        mFFmpegRecorder = fFmpegRecorder;
        mAudioRecord = audioRecord;
        mBufferProvider = bufferProvider;
        mBufferSize = bufferSize;
    }

    @Override
    public void run() {
        int read = 0;
        int offset = 0;
        NativeByteBuffer buffer = mBufferProvider.get();
        int bufferLen = mBufferSize + FFmpegRecorder.TS_LEN;
        mAudioRecord.startRecording();
        while (mRecording) {
            buffer.init(BuffersStorage.getFreeByteBuffer(bufferLen), bufferLen);
            offset = 0;
            while (offset < mBufferSize) {
                Log.e("mAudioRecord", "offset=" + offset + "left=" + (mBufferSize - offset));
                read = mAudioRecord.read(buffer.getBuffer(), mBufferSize - offset);
                Log.e("mAudioRecord", "read=" + read);
                if (read > 0) {
                    offset += read;
                    //因为mAudioRecord.read()不会移动position，所以需要自己移动
                    buffer.position(offset);
                    if (offset >= mBufferSize) {
                        //当java端写满了，才一次一次性移动native position，因为此间native端不需要使用
                        NativeByteBuffer.native_position(buffer.mAddress, offset);
//                                buffer.position(bufferLen);
//                                buffer.getBuffer().flip();
                        //native会进行flip
                        mFFmpegRecorder.recordAudio(buffer);
                        break;
                    }
                } else {
                    break;
                }
            }
        }
        if(mAudioRecord != null){
            mAudioRecord.stop();
            mAudioRecord = null;
        }
        Log.d("AudioRecorder", "run end");
    }

    public void startRecord(){
        mRecording = true;
        mThread = new Thread(this);
        mThread.start();
    }

    public void stopRecord() throws InterruptedException {
        mRecording = false;
        if(mThread != null) {
            mThread.join();
            mThread = null;
        }
        Log.d("AudioRecorder", "stopRecord");
    }
}
