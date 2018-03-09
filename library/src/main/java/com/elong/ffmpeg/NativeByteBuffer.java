package com.elong.ffmpeg;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Created by Elong on 2017/12/21.
 */

public class NativeByteBuffer {

    protected int mAddress = 0;
    public ByteBuffer mBuffer;

    public void init(int address, int size){
        if(address != 0){
            mAddress = address;
            mBuffer = native_getJavaByteBuffer(address);
            mBuffer.position(native_position(address));
            mBuffer.limit(native_limit(address));
            mBuffer.order(ByteOrder.LITTLE_ENDIAN);
        }
    }

    public void flip(){
        native_flip(mAddress);
        mBuffer.flip();
    }

    public void put(ByteBuffer data){
        mBuffer.put(data);
    }

    public void put(byte[] data){
        mBuffer.put(data);
    }

    public void putLong(long data) { mBuffer.putLong(data); }

    public int getAddress(){
        return mAddress;
    }

    public int position(){
        return mBuffer.position();
    }

    public void position(int position){
        mBuffer.position(position);
    }

    public int capacity(){
        return mBuffer.capacity();
    }

    public int limit(){
        return mBuffer.limit();
    }

    public void limit(int limit){
        mBuffer.limit(limit);
    }

    public void rewind(){
        mBuffer.rewind();
    }

    public ByteBuffer getBuffer(){
        return mBuffer;
    }

    public static native ByteBuffer native_getJavaByteBuffer(int address);
    public static native void native_flip(int address);
    public static native int native_limit(int address);
    public static native int native_position(int address);
    public static native void native_position(int address, int position);
}
