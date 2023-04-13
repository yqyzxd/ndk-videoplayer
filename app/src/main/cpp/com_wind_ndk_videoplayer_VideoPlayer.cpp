//
// Created by wind on 2023/4/4.
//
#include "com_wind_ndk_videoplayer_VideoPlayer.h"
#include "videoplayer/video_player.h"
#include <android/native_window_jni.h>

JNIEXPORT void JNICALL Java_com_wind_ndk_videoplayer_VideoPlayer_nativePrepare
        (JNIEnv *, jobject, jlong){
    VideoPlayer* player=new VideoPlayer;
    player->prepare();
}


JNIEXPORT jlong JNICALL Java_com_wind_ndk_videoplayer_VideoPlayer_nativeInit
        (JNIEnv *, jobject){
    VideoPlayer* player=new VideoPlayer;
    return reinterpret_cast<jlong>(player);

}
JNIEXPORT void JNICALL Java_com_wind_ndk_videoplayer_VideoPlayer_nativePlay
        (JNIEnv *, jobject, jlong handle){
    VideoPlayer* player = reinterpret_cast<VideoPlayer *>(handle);
    player->play();
}

JNIEXPORT void JNICALL Java_com_wind_ndk_videoplayer_VideoPlayer_nativeSetDataSource
        (JNIEnv * env, jobject, jlong handle, jstring jpath){
    const char* cpath=env->GetStringUTFChars(jpath,0);
    VideoPlayer* player = reinterpret_cast<VideoPlayer *>(handle);
    player->setDataSource(const_cast<char *>(cpath));
    env->ReleaseStringUTFChars(jpath,cpath);
}

JNIEXPORT void JNICALL Java_com_wind_ndk_videoplayer_VideoPlayer_nativeSurfaceCreated
        (JNIEnv *env, jobject videoplayerobj, jlong handle, jobject surface){
    VideoPlayer* player = reinterpret_cast<VideoPlayer *>(handle);
    ANativeWindow* window=ANativeWindow_fromSurface(env,surface);
    player->setWindow(window);
}


JNIEXPORT void JNICALL Java_com_wind_ndk_videoplayer_VideoPlayer_nativeSurfaceChanged
        (JNIEnv *, jobject, jlong handle, jint w, jint h){
    VideoPlayer* player = reinterpret_cast<VideoPlayer *>(handle);
    player->setWindowSize(w,h);
}


JNIEXPORT void JNICALL Java_com_wind_ndk_videoplayer_VideoPlayer_nativeSurfaceDestroyed
        (JNIEnv *, jobject, jlong){

}


