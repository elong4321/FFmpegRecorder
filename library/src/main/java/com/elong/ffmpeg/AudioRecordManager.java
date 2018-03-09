package com.elong.ffmpeg;

import android.media.AudioRecord;
import android.util.Log;

/**
 * Created by Elong on 2017/12/6.
 */

public class AudioRecordManager{

    AudioRecord mAudioRecord;

    public AudioRecordManager(){
    }

    public AudioRecord openAudioRecord(int source, int rate, int channelConfig, int format) {
        int audioBufferSize = AudioRecord.getMinBufferSize(rate, channelConfig, format);
        mAudioRecord = new AudioRecord(source, rate, channelConfig, format, audioBufferSize);
        Log.d("openAudioRecord", "bufferSize= " + audioBufferSize);
        return mAudioRecord;
    }

    public void stop(){
        if(mAudioRecord != null) {
            mAudioRecord.release();
            mAudioRecord = null;
        }
    }
}
