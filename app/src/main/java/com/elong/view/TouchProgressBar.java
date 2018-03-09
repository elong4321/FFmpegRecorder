package com.elong.view;

import android.animation.Animator;
import android.animation.TimeInterpolator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.animation.LinearInterpolator;
import android.widget.ProgressBar;

/**
 * Created by Elong on 2017/12/3.
 */

public class TouchProgressBar extends ProgressBar implements ValueAnimator.AnimatorUpdateListener, Animator.AnimatorListener {

    ValueAnimator mValueAnimator;
    long mDuration;
    CountDownCallback mCallback;

    public TouchProgressBar(Context context) {
        super(context);
    }

    public TouchProgressBar(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public TouchProgressBar(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public TouchProgressBar(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getAction()){
            case MotionEvent.ACTION_DOWN:
                start();
                break;
            case MotionEvent.ACTION_UP:
                stop();
                break;
        }
        return true;
    }

    @Override
    public boolean performClick() {
        return super.performClick();
    }

    void start(){
        stop();
        setProgress(0);
        mValueAnimator = ValueAnimator.ofInt(getProgress(), getMax());
        mValueAnimator.setDuration(mDuration);
        mValueAnimator.addUpdateListener(this);
        mValueAnimator.addListener(this);
        mValueAnimator.setRepeatCount(1);
        mValueAnimator.start();
    }

    void stop(){
        if(mValueAnimator != null){
            mValueAnimator.end();
            mValueAnimator = null;
        }
    }


    @Override
    public void onAnimationStart(Animator animation) {
        if(mCallback != null){
            mCallback.onCountDownStart();
        }
    }

    @Override
    public void onAnimationEnd(Animator animation) {
        if(mCallback != null) {
            mCallback.onCountDownEnd();
        }
    }

    @Override
    public void onAnimationCancel(Animator animation) {

    }

    @Override
    public void onAnimationRepeat(Animator animation) {

    }

    @Override
    public void onAnimationUpdate(ValueAnimator animation) {
        Log.d("onAnimationUpdate", "value="+animation.getAnimatedValue());
        setProgress((Integer) animation.getAnimatedValue());
    }

    public interface CountDownCallback{
        void onCountDownStart();
        void onCountDownEnd();
    }

    public void setCallback(CountDownCallback callback) {
        mCallback = callback;
    }

    public void setDuration(long duration) {
        mDuration = duration;
    }
}
