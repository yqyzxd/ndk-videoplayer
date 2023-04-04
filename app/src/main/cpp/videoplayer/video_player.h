//
// Created by wind on 2023/4/4.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_PLAYER_H
#define NDK_VIDEOPLAYER_VIDEO_PLAYER_H


#include <android/native_window.h>
#include "video_synchronizer.h"
#include "decoder/decoder_params.h"

class VideoPlayer {

private:
    char* path;
    VideoSynchronizer* synchronizer;
public:
    void setDataSource(char *path);

    void prepare();

    void play();

    void setWindow(ANativeWindow *window);
};


#endif //NDK_VIDEOPLAYER_VIDEO_PLAYER_H
