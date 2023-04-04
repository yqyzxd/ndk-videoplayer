package com.wind.ndk.videoplayer

import android.view.Surface

/**
 * Copyright (C), 2015-2022, 杭州迈优文化创意有限公司
 * FileName: VideoPlayer
 * Author: wind
 * Date: 2023/4/4 10:21
 * Description: 描述该类的作用
 * Path: 路径
 * History:
 *  <author> <time> <version> <desc>
 *
 */
class VideoPlayer {
    companion object{
        init {
            System.loadLibrary("videoplayer")
        }
    }
    private var mHandle:Long = nativeInit()

    fun setDataSource(dataSource:String){
        nativeSetDataSource(mHandle,dataSource)
    }
    fun prepare(){
        nativePrepare(mHandle)
    }

    fun play(){
        nativePlay(mHandle)
    }

    fun surfaceCreated(surface: Surface){
        nativeSurfaceCreated(mHandle,surface)
    }
    fun surfaceChanged(width: Int, height: Int){
        nativeSurfaceChanged(mHandle,width,height)
    }
    fun surfaceDestroyed(){
        nativeSurfaceDestroyed(mHandle)
    }
    private external fun nativePrepare(handle:Long)
    private external fun nativeInit(): Long
    private external fun nativeSetDataSource(handle:Long,dataSource: String)

    external fun nativePlay(handle:Long)
    external fun nativeSurfaceCreated(handle:Long,surface: Surface)
    external fun nativeSurfaceChanged(handle:Long,width: Int, height: Int)
    external fun nativeSurfaceDestroyed(handle:Long)

}