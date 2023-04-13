//
// Created by 史浩 on 2023/4/8.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_OUTPUT_H
#define NDK_VIDEOPLAYER_VIDEO_OUTPUT_H


#include <android/native_window.h>
#include "circle_texture_queue.h"
typedef int (*getTextureCallback)(FrameTexture** texture, void* ctx, bool forceGetFrame);
class VideoOutput {

public:
    void initOutput(getTextureCallback, void *ctx);

    void onSurfaceCreated(ANativeWindow *window);

    void onSurfaceChanged(int width, int height);

    void signalFrameAvailable();
};


#endif //NDK_VIDEOPLAYER_VIDEO_OUTPUT_H
