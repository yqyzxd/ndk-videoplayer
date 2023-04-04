cmake_minimum_required(VERSION 3.22.1)
project("videoplayer")

set(PATH_TO_PRE_BUILT ffmpeg/libs/${ANDROID_ABI})
include_directories(ffmpeg/include)

add_library( # Sets the name of the library.
        videoplayer

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).

        com_wind_ndk_videoplayer_VideoPlayer.cpp
        videoplayer/video_player.cpp
        videoplayer/video_synchronizer.cpp
        videoplayer/decoder/decoder_params.cpp
        videoplayer/decoder/video_decoder.cpp
        )




target_link_libraries( # Specifies the target library.
        videoplayer

        # Links the target library to the log library
        # included in the NDK.
        android
        GLESv2
        EGL
        log
        # 忽略顺序的方式，导入
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        )