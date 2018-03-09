package com.elong.ffmpeg.com.elong.ffmpeg.camera;

import android.content.Context;
import android.os.Build;
import android.view.SurfaceHolder;

import java.io.IOException;

/**
 * Created by Elong on 2017/12/14.
 */

public class CameraManager{

    private ICamera mCamera;
    Context mContext;
    private int mCameraId = 0;

    public CameraManager(Context context){
        mContext = context;

        mCamera = new CameraV1();
        //TODO: 根据版本选择
        /*由于CameraV2在360手机上当加入ImageReader.getSurface()时，会导致除开头几帧外出现onCaptureFailed(0):dropped this frame only due to an error
         in the framework.暂时未能解决，暂时先注释掉 */
//        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
//            mCamera = new CameraV2(mContext);
//        }else {
//            mCamera = new CameraV1();
//        }
    }

    public ICamera openDriver(SurfaceHolder holder){

        mCamera.open(mCameraId);

        try {
            mCamera.setPreviewDisplay(holder);
        } catch (IOException e) {
            e.printStackTrace();
            mCamera.close();
            mCamera = null;
        }

//        if(mCamera != null){
//            mCamera.startPreview();
//        }

        return mCamera;
    }

    public void closeDriver(){
        if(mCamera != null){
            mCamera.close();
            mCamera = null;
        }
    }

    public ICamera getCamera() {
        return mCamera;
    }
}
