package com.elong.ffmpeg;

/**
 * Created by Elong on 2017/12/22.
 */

public class BuffersStorage {

    public static int getFreeByteBuffer(int size){
        return native_getFreeBuffer(size);
    }
    public static int reuse(int address){
        return  native_reuse(address);
    }
    public static native int native_getFreeBuffer(int size);
    public static native int native_reuse(int address);
}
