cmake_minimum_required(VERSION 3.22.1)
project("videoplayer")
set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg) # ffmpeg的路径
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${FFMPEG}/libs/${CMAKE_ANDROID_ARCH_ABI}") # ffmpeg库指定

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
        videoplayer/decoder/ffmpeg_video_decoder.cpp
        videoplayer/audio_output.cpp
        videoplayer/video_output.cpp
        videoplayer/circle_texture_queue.cpp
        videoplayer/texture_uploader/texture_frame_uploader.cpp
        videoplayer/texture_uploader/yuv_texture_frame_uploader.cpp

        common/opensl_media/opensl_es_context.cpp
        common/egl_core/egl_core.cpp
        common/egl_core/egl_share_context.cpp
        common/message_queue/handler.cpp
        common/message_queue/message_queue.cpp
        common/opengl_media/render/video_gl_surface_render.cpp
        common/opengl_media/texture/texture_frame.cpp
        common/opengl_media/texture/yuv_texture_frame.cpp
        common/opengl_media/texture_copier/texture_frame_copier.cpp
        common/opengl_media/texture_copier/yuv_texture_frame_copier.cpp
        common/opengl_media/movie_frame.cpp
        common/matrix.cpp
        )



target_link_libraries( # Specifies the target library.
        videoplayer

        # Links the target library to the log library
        # included in the NDK.
        android
        GLESv2

        z
        EGL
        log
        # 忽略顺序的方式，导入
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        OpenSLES
        )