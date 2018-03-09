package com.elong.ffmpeg.com.elong.ffmpeg.camera;

import android.content.Context;
import android.graphics.Rect;
import android.hardware.camera2.*;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * Created by Elong on 2017/12/10.
 */

public class CameraV2 implements ICamera, ImageReader.OnImageAvailableListener{
    public static final String TAG = "CameraV2";
    android.hardware.camera2.CameraManager mCameraManager;

    private SurfaceHolder mSurfaceHolder;
    private CameraDevice mCameraDevice;
    private CameraCaptureSession mSession;
    private ImageReader mImageReader;
    private Handler mBackgroundHandler;
    private int mMaxImages = 4;
    private int mPreviewWidth, mPreviewHeight;
    IPreviewCallback mPreviewCallback;
    boolean mSurfaceCreated = false;
    boolean mCameraOpened = false;
    String mCameraId;
    int mImageFormat;

    HandlerThread mThread;

    CameraDevice.StateCallback mDeviceStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            mCameraDevice = camera;
            mCameraOpened = true;
            Log.d("Camera2","onOpen " + mCameraOpened + " " + mSurfaceCreated);
            startPreviewActually();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            Log.d("Camera2","onDisconnect");
            close();

        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            Log.d("Camera2","onError " + error);
            close();
        }
    };

    CameraCaptureSession.StateCallback mSessionCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            mSession = session;
            CaptureRequest.Builder builder = null;
            try {
                builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                // Auto focus should be continuous for camera preview.
                builder.set(CaptureRequest.CONTROL_AF_MODE,
                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                // Flash is automatically enabled when necessary.
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
            if(builder != null) {
                builder.addTarget(mSurfaceHolder.getSurface());
                builder.addTarget(mImageReader.getSurface());
                CaptureRequest request = builder.build();
                try {
                    session.setRepeatingRequest(request, new CameraCaptureSession.CaptureCallback() {
                        @Override
                        public void onCaptureStarted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, long timestamp, long frameNumber) {
                            super.onCaptureStarted(session, request, timestamp, frameNumber);
                            Log.d(TAG, "onCaptureStarted");
                        }

                        @Override
                        public void onCaptureProgressed(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull CaptureResult partialResult) {
                            super.onCaptureProgressed(session, request, partialResult);
                            Log.d(TAG, "onCaptureProgressed");
                        }

                        @Override
                        public void onCaptureCompleted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull TotalCaptureResult result) {
                            super.onCaptureCompleted(session, request, result);
                            Log.d(TAG, "onCaptureCompleted");
                        }

                        @Override
                        public void onCaptureBufferLost(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull Surface target, long frameNumber) {
                            super.onCaptureBufferLost(session, request, target, frameNumber);
                            Log.d(TAG, "onCaptureBufferLost");
                        }

                        @Override
                        public void onCaptureSequenceAborted(@NonNull CameraCaptureSession session, int sequenceId) {
                            super.onCaptureSequenceAborted(session, sequenceId);
                            Log.d(TAG, "onCaptureSequenceAborted");
                        }

                        @Override
                        public void onCaptureSequenceCompleted(@NonNull CameraCaptureSession session, int sequenceId, long frameNumber) {
                            super.onCaptureSequenceCompleted(session, sequenceId, frameNumber);
                            Log.d(TAG, "onCaptureSequenceCompleted");
                        }

                        @Override
                        public void onCaptureFailed(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull CaptureFailure failure) {
                            super.onCaptureFailed(session, request, failure);
                            Log.d(TAG, "onCaptureFailed " + failure.getReason());
                        }
                    }, mBackgroundHandler);
                } catch (CameraAccessException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {
            session.close();
            mSession = null;
        }

    };

    public CameraV2(Context context){
        mCameraManager = (android.hardware.camera2.CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
    }

    @Override
    public void open(int cameraId) {
        mThread = new HandlerThread("Camera2Thread");
        mThread.start();
        mBackgroundHandler = new Handler(mThread.getLooper());

        CameraCharacteristics characteristics = null;
        String[] cameraIdList = null;
        try {
            cameraIdList = mCameraManager.getCameraIdList();
            for (String id : cameraIdList){
                characteristics = mCameraManager.getCameraCharacteristics(id);
                if (characteristics.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_BACK) {
                    mCameraId = id;
                    break;
                }
            }

            if(characteristics != null){
                //获取StreamConfigurationMap，它是管理摄像头支持的所有输出格式和尺寸
                StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);

                //根据TextureView的尺寸设置预览尺寸
                Size size = choosePreviewSize(map.getOutputSizes(mImageFormat), mPreviewWidth, mPreviewHeight);
                if(size != null) {
                    mPreviewWidth = size.getWidth();
                    mPreviewHeight = size.getHeight();
                }
            }

            Log.d("Camera2", "before openCamera");
            mCameraManager.openCamera(mCameraId, mDeviceStateCallback, mBackgroundHandler);
            Log.d("Camera2", "after openCamera");
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        mImageReader = ImageReader.newInstance(mPreviewWidth, mPreviewHeight, mImageFormat, mMaxImages);
        mImageReader.setOnImageAvailableListener(this, mBackgroundHandler);
    }

    @Override
    public void close() {
        if(mThread != null){
            mThread.quitSafely();
            mThread = null;
        }

        if(mImageReader != null){
            mImageReader.close();
            mImageReader = null;
        }

        if(mSession != null){
            mSession.close();
            mSession = null;
        }

        if(mCameraDevice != null) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
    }

    @Override
    public void setParams(int format, int previewWidth, int previewHeight) {
        mImageFormat = format;
        mPreviewWidth = previewWidth;
        mPreviewHeight = previewHeight;

    }

    private void startPreviewActually(){
        Log.d("Camera2", "startPreviewActually " + mSurfaceCreated + " " + mCameraOpened);
        if(mSurfaceCreated && mCameraOpened) {
            mSurfaceHolder.setFixedSize(mPreviewWidth, mPreviewHeight);
            try {
                mCameraDevice.createCaptureSession(Arrays.asList(mSurfaceHolder.getSurface(), mImageReader.getSurface()), mSessionCallback, mBackgroundHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) throws IOException {
        mSurfaceHolder = surfaceHolder;
        Rect rect = surfaceHolder.getSurfaceFrame();
        mPreviewWidth = rect.width();
        mPreviewHeight = rect.height();
    }

    @Override
    public void setPreviewCallback(IPreviewCallback callback) {
        mPreviewCallback = callback;
    }

    @Override
    public void startPreview() {
        mSurfaceCreated = true;
        startPreviewActually();
    }

    @Override
    public int getOrientation() {
        return 0;
    }

    @Override
    public Size getPreviewSize() {
        return new Size(mPreviewWidth, mPreviewHeight);
    }

    private Size choosePreviewSize(Size[] choices, int width, int height) {
        if(choices == null){
            return null;
        }
        List<Size> candidate = new ArrayList<>();
        for(Size size : choices) {
            if(size.getHeight()*width==size.getWidth()*height && size.getWidth()>=width) {
                candidate.add(size);
            }
        }

        if(candidate.size() > 0) {
            return Collections.min(candidate, new Comparator<Size>() {
                @Override
                public int compare(Size lhs, Size rhs) {
                    return Long.signum(lhs.getWidth()-rhs.getWidth());
                }
            });
        }

        return choices[0];
    }

    @Override
    public void onImageAvailable(ImageReader reader) {
        Log.d("Camera2", "onImageAvailable " + reader + mPreviewCallback);
        if(mPreviewCallback != null){
            Image image = reader.acquireLatestImage();
            mBackgroundHandler.post(new ImageCallback(mPreviewCallback, image));
        }
    }

    private static class ImageCallback implements Runnable{
        private IPreviewCallback mCallback;
        Image mImage;
        public ImageCallback(IPreviewCallback callback, Image image){
            mCallback = callback;
            mImage = image;
        }
        @Override
        public void run() {
            mCallback.onPreviewFrameV2(mImage);
            mImage.close();
        }
    }
}
