package com.elong.ffmpeg.com.elong.ffmpeg.camera;

import android.graphics.Point;
import android.hardware.Camera;
import android.util.Log;
import android.util.Size;
import android.view.SurfaceHolder;

import java.io.IOException;

/**
 * Created by Elong on 2017/12/10.
 */

public class CameraV1 implements ICamera {
    public final String TAG = "CameraV1";

    private Camera mCamera;
    private int mOrientation;
    int mImageFormat;
    private int mPreviewWidth, mPreviewHeight;

    @Override
    public void open(int cameraId) {
        int numCameras = Camera.getNumberOfCameras();
        if (numCameras == 0) {
            Log.w(TAG, "No cameras!");
            return;
        }

        boolean explicitRequest = cameraId >= 0;

        Camera.CameraInfo selectedCameraInfo = null;
        int index;
        if (explicitRequest) {
            index = cameraId;
            selectedCameraInfo = new Camera.CameraInfo();
            Camera.getCameraInfo(index, selectedCameraInfo);
        } else {
            index = 0;
            while (index < numCameras) {
                Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
                Camera.getCameraInfo(index, cameraInfo);
                int  reportedFacing = cameraInfo.facing;
                if (reportedFacing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                    selectedCameraInfo = cameraInfo;
                    break;
                }
                index++;
            }
        }

        if (index < numCameras) {
            Log.i(TAG, "Opening mCamera #" + index);
            mCamera = Camera.open(index);
        } else {
            if (explicitRequest) {
                Log.w(TAG, "Requested mCamera does not exist: " + cameraId);
                mCamera = null;
            } else {
                mCamera = Camera.open(0);
                selectedCameraInfo = new Camera.CameraInfo();
                Camera.getCameraInfo(0, selectedCameraInfo);
            }
        }

        if(mCamera != null){
            mOrientation = selectedCameraInfo.orientation;
            if(mCamera != null) {
                Camera.Size size = mCamera.getParameters().getPreferredPreviewSizeForVideo();
                Camera.Parameters parameters = mCamera.getParameters();
                parameters.setPreviewFormat(mImageFormat);
//        parameters.setPreviewFpsRange(mFrameRate, mFrameRate);
                parameters.setPreviewSize(size.width, size.height);
                mCamera.setParameters(parameters);
            }
        }
    }

    @Override
    public void close() {
        if(mCamera != null){
            mCamera.setPreviewCallback(null);
            try {
                mCamera.setPreviewDisplay(null);
            } catch (IOException e) {
                e.printStackTrace();
            }
            mCamera.stopPreview();
            mCamera.release();
        }
    }

    @Override
    public void setParams(int format, int previewWidth, int previewHeight) {
        mImageFormat = format;
        mPreviewWidth = previewWidth;
        mPreviewHeight = previewHeight;

    }

    @Override
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) throws IOException {
        if(mCamera != null){
            mCamera.setPreviewDisplay(surfaceHolder);
        }
    }

    @Override
    public void setPreviewCallback(final IPreviewCallback callback) {
        if(mCamera != null){
            mCamera.setPreviewCallback(new Camera.PreviewCallback() {
                @Override
                public void onPreviewFrame(byte[] data, Camera camera) {
                    callback.onPreviewFrameV1(data);
                }
            });
        }

    }

    @Override
    public void startPreview() {
        if(mCamera != null){
            mCamera.startPreview();
        }
    }

    @Override
    public int getOrientation() {
        return 0;
    }

    @Override
    public Size getPreviewSize() {
        Camera.Size size = mCamera.getParameters().getPreviewSize();
        return new Size(size.width, size.height);
    }
}
