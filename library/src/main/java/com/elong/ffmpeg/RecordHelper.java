package com.elong.ffmpeg;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Point;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.Size;
import android.view.SurfaceHolder;

import com.elong.ffmpeg.com.elong.ffmpeg.camera.CameraManager;
import com.elong.ffmpeg.com.elong.ffmpeg.camera.CameraV1;
import com.elong.ffmpeg.com.elong.ffmpeg.camera.CameraV2;
import com.elong.ffmpeg.com.elong.ffmpeg.camera.ICamera;

import java.io.File;

/**
 * Created by Elong on 2017/12/8.
 */

public class RecordHelper implements SurfaceHolder.Callback{
    private static final String TAG = "RecordHelper";

    CameraManager mCameraManager;
    AudioRecordManager mAudioRecordManager;
    boolean mSurfaceCreated = false;
    SurfaceHolder mSurfaceHolder;

    FFmpegRecorder mFFmpegRecorder;
    VideoRecorder mVideoRecorder;
    AudioRecorder mAudioRecorder;
    boolean mToStart = false;

    Context mContext;

    int mVideoFormat = ImageFormat.NV21;
    int mFrameRate = 25;
    int mSrcWidth;
    int mSrcHeight;
    int mDstWidth = 1024;
    int mDstHeight = 1024;

    int mSource = MediaRecorder.AudioSource.MIC;
    int mSampleFormat = AudioFormat.ENCODING_PCM_16BIT;
    int mChannelConfig = AudioFormat.CHANNEL_IN_MONO;
    int mChannelCount = 1;
    int mSampleRate = 44100;
    int mAudioBufferSize = 0;

    String mFileFormat = "mp4";
    String mFilePath;

    //TODO：释放
    ThreadLocal<NativeByteBuffer> mBufferProvider = new ThreadLocal<NativeByteBuffer>() {
        @Override
        protected NativeByteBuffer initialValue() {
            return new NativeByteBuffer();
        }
    };

    public RecordHelper(Context context, SurfaceHolder holder){
        mContext = context;
        mSurfaceHolder = holder;
        mCameraManager = new CameraManager(context);
        mAudioRecordManager = new AudioRecordManager();
        holder.addCallback(this);
    }

    public void startRecord(){
        if(!mToStart) {
            mToStart = true;
            prepare();
            try {
                mFFmpegRecorder.start(new Runnable() {
                    @Override
                    public void run() {
                        mVideoRecorder.startRecord();
                        mAudioRecorder.startRecord();
                    }
                });
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    public void prepare(){
        File dir = mContext.getExternalFilesDir("video");
        if (!dir.exists()) {
            dir.mkdirs();
        }
        mFilePath = dir.getAbsolutePath() + "/" + System.currentTimeMillis() + "." + mFileFormat;

        AudioRecord audioRecord = mAudioRecordManager.openAudioRecord(mSource, mSampleRate, mChannelConfig, mSampleFormat);

        ICamera camera = mCameraManager.getCamera();
        Size size = camera.getPreviewSize();
        mDstWidth = mSrcWidth = size.getWidth();
        mDstHeight = mSrcHeight = size.getHeight();

        if(mFFmpegRecorder == null){
                    mFFmpegRecorder = new FFmpegRecorder.Builder()
                .setFormatParams(mFileFormat, mFilePath)
                .setVideoParams(mVideoFormat, mFrameRate, mSrcWidth, mSrcHeight, mDstWidth, mDstHeight)
                //TODO: getBufferSizeInFrames 低版本下需要使用另外的api
                .setAudioParams(mSampleFormat, audioRecord.getBufferSizeInFrames(), mChannelCount, mChannelConfig, mSampleRate)
                .build();
        }
        mAudioBufferSize = mFFmpegRecorder.getAudioBufferSize();

        if(mVideoRecorder == null){
            mVideoRecorder = new VideoRecorder(mFFmpegRecorder, mBufferProvider);
        }
        if(mAudioRecorder == null){
            mAudioRecorder = new AudioRecorder(mFFmpegRecorder, audioRecord, mBufferProvider, mAudioBufferSize);
        }

        camera.setPreviewCallback(mVideoRecorder);
    }

    public void onActivityResume(){
        initCamera();
    }

    public void onActivityPause(){
        stopRecord();
        mCameraManager.closeDriver();
    }

    public void onActivityDestroy(){
        release();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "surfaceCreated");
        mSurfaceHolder = holder;
        if(!mSurfaceCreated){
            mSurfaceCreated = true;
            initCamera();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mSurfaceHolder = holder;
        Log.d(TAG, "surfaceChanged");
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "surfaceDestroyed");
        mSurfaceHolder = null;
        mSurfaceCreated = false;
    }

    private void initCamera(){
        Log.d("initCamera", "mSurfaceCreated=" + mSurfaceCreated + " " + mSurfaceHolder);
        if(mSurfaceCreated) {
            ICamera camera = mCameraManager.getCamera();
            mVideoFormat = selectImageFormatForCamera(camera);
            DisplayMetrics metrics = mContext.getResources().getDisplayMetrics();
            camera.setParams(mVideoFormat, metrics.widthPixels, metrics.heightPixels);
            mCameraManager.openDriver(mSurfaceHolder);
            camera.startPreview();
        }
    }

    private int selectImageFormatForCamera(ICamera camera){
        int format = ImageFormat.NV21;
        if(camera instanceof CameraV1){
            format = ImageFormat.NV21;
        }else if(camera instanceof CameraV2){
            format = ImageFormat.YUV_420_888;
        }
        return format;
    }

    public void stopRecord(){
        if(mVideoRecorder != null) {
            mVideoRecorder.stopRecord();
            mVideoRecorder = null;
        }
        if(mAudioRecorder != null) {
            try {
                mAudioRecorder.stopRecord();
                mAudioRecorder = null;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        mAudioRecordManager.stop();

        if(mFFmpegRecorder != null){
            mFFmpegRecorder.stop();
        }

        mToStart = false;
    }

    public void release(){
        mCameraManager.closeDriver();
        mFFmpegRecorder.release();
    }
}
