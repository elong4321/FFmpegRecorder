package com.elong.ffmpeg;

import android.Manifest;
import android.app.Activity;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AlertDialog;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import com.elong.view.TouchProgressBar;

import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.OnNeverAskAgain;
import permissions.dispatcher.OnPermissionDenied;
import permissions.dispatcher.OnShowRationale;
import permissions.dispatcher.PermissionRequest;
import permissions.dispatcher.RuntimePermissions;

/**
 * Created by Elong on 2017/12/3.
 */

@RuntimePermissions
public class FFmpegRecordActivity extends Activity implements TouchProgressBar.CountDownCallback {
    private RecordHelper mHelper;
    private long mVideoLen = 10000; //10秒视频
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Window window = getWindow();
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        window.addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(com.elong.ffmpeg.R.layout.layout_record);
        SurfaceView previewView = findViewById(com.elong.ffmpeg.R.id.sv_preview);
        TouchProgressBar recordIv = findViewById(com.elong.ffmpeg.R.id.tpv_record);
        recordIv.setDuration(mVideoLen);
        recordIv.setCallback(this);
        recordIv.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()){
                    case MotionEvent.ACTION_CANCEL:
                    case MotionEvent.ACTION_UP:
                        stopRecord();
                        break;
                    default:
                        break;
                }
                return false;
            }
        });

        mHelper = new RecordHelper(this, previewView.getHolder());


    }

    @NeedsPermission({Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO})
    protected void startRecord(){
        mHelper.startRecord();
    }

    protected void stopRecord(){
        mHelper.stopRecord();
    }

    @OnShowRationale({Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO})
    protected void showRationaleForCamera(final PermissionRequest request) {
        new AlertDialog.Builder(this)
                .setMessage(R.string.permission_camera_rationale)
                .setPositiveButton(R.string.button_allow, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        request.proceed();
                    }
                })
                .setNegativeButton(R.string.button_deny, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        request.cancel();
                    }
                })
                .show();
    }

    @OnPermissionDenied({Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO})
    protected void showDeniedPermissions() {
        Toast.makeText(this, R.string.permission_denied, Toast.LENGTH_SHORT).show();
        finish();
    }

    @OnNeverAskAgain({Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO})
    protected void showNeverAsk() {
        Toast.makeText(this, R.string.permission_neverask, Toast.LENGTH_SHORT).show();
        finish();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mHelper.onActivityResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mHelper.onActivityPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mHelper.onActivityDestroy();
    }

    @Override
    public void onCountDownStart() {
        startRecord();
    }

    @Override
    public void onCountDownEnd() {
        stopRecord();
    }
}
