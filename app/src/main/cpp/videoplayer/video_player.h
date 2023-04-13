//
// Created by wind on 2023/4/4.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_PLAYER_H
#define NDK_VIDEOPLAYER_VIDEO_PLAYER_H

#include <cstring>
#include <android/native_window.h>
#include "video_synchronizer.h"
#include "decoder/decoder_params.h"
#include "audio_output.h"
#include "../common/CommonTools.h"
#include "video_output.h"
#include "circle_texture_queue.h"

class VideoPlayer {

private:
    bool isPlaying;
    char* path;
    VideoSynchronizer* synchronizer;
    AudioOutput* audioOutput;
    VideoOutput* videoOutput;
public:
    VideoPlayer();
    ~VideoPlayer();

    void setDataSource(char *path);

    void prepare();

    void play();

    void setWindow(ANativeWindow *window);

    int initAudioOutput();


    static int audioCallbackFillData(byte *outData, size_t bufferSize, void *ctx);
    int consumeAudioFrames(byte *outData, size_t bufferSize);

    void setWindowSize(int width, int height);

    static int videoCallbackGetTex(FrameTexture **frameTex, void *ctx, bool forceGetFrame);
    int getCorrectRenderTexture(FrameTexture **frameTex, bool forceGetFrame);

    void signalOutputFrameAvailable();
};


#endif //NDK_VIDEOPLAYER_VIDEO_PLAYER_H
