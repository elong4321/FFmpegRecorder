package com.elong.ffmpeg.com.elong.ffmpeg.camera;

import android.graphics.Point;
import android.hardware.Camera;
import android.media.Image;
import android.util.Size;
import android.view.SurfaceHolder;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by Elong on 2017/12/10.
 */

public interface ICamera {

    void open(int cameraId);
    void close();
    void setParams(int format, int previewWidth, int previewHeight);
    void setPreviewDisplay(SurfaceHolder surfaceHolder) throws IOException;
    void setPreviewCallback(IPreviewCallback callback);
    void startPreview();
    int getOrientation();
    Size getPreviewSize();

    interface IPreviewCallback{
        void onPreviewFrameV1(byte[] data);
        void onPreviewFrameV2(Image image);
    }
}
