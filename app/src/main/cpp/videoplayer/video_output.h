//
// Created by 史浩 on 2023/4/8.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_OUTPUT_H
#define NDK_VIDEOPLAYER_VIDEO_OUTPUT_H


#include <android/native_window.h>
#include "circle_texture_queue.h"
#include "../common/message_queue/message_queue.h"
#include "../common/message_queue/handler.h"
#include "../common/egl_core/egl_core.h"
#include "../common/opengl_media/render/video_gl_surface_render.h"

typedef enum{
    VIDEO_OUTPUT_MESSAGE_CREATE_EGL_CONTEXT,
    VIDEO_OUTPUT_MESSAGE_CREATE_WINDOW_SURFACE,
    VIDEO_OUTPUT_MESSAGE_DESTROY_WINDOW_SURFACE,
    VIDEO_OUTPUT_MESSAGE_DESTROY_EGL_CONTEXT,
    VIDEO_OUTPUT_MESSAGE_RENDER_FRAME
};
class VideoOutputHandler;

typedef int (*getTextureCallback)(FrameTexture** texture, void* ctx, bool forceGetFrame);
class VideoOutput {
private:
    void *ctx;
    getTextureCallback produceDataCallback;
    MessageQueue* queue;
    VideoOutputHandler* handler;
    pthread_t threadId;
    int screenWidth;
    int screenHeight;

    EGLCore* eglCore;
    ANativeWindow* surfaceWindow;
    EGLSurface renderTexSurface;
    VideoGLSurfaceRender* renderer;
    bool surfaceExists;
    bool forceGetFrame;
public:
    bool eglHasDestroyed;

    void initOutput(getTextureCallback, void *ctx);

    void onSurfaceCreated(ANativeWindow *window);

    void onSurfaceChanged(int width, int height);

    void signalFrameAvailable();

    static void* threadStartCallback(void *ctx);

    bool createEGLContext(ANativeWindow *window);

    void createWindowSurface(ANativeWindow *pWindow);

    bool renderVideo();

    void destroyWindowSurface();

    void destroyEGLContext();

    void processMessage();
};


class VideoOutputHandler :public Handler{
private:
    VideoOutput* videoOutput;
    bool initPlayerResourceFlag;
public:
    VideoOutputHandler(VideoOutput* videoOutput,MessageQueue* queue): Handler(queue){
        this->videoOutput=videoOutput;
        initPlayerResourceFlag= false;
    }

    void handleMessage(Message *msg) override{
        int what =msg->getWhat();
        ANativeWindow* obj;
        switch (what) {
            case VIDEO_OUTPUT_MESSAGE_CREATE_EGL_CONTEXT:
                if (videoOutput->eglHasDestroyed){
                    break;
                }
                obj= static_cast<ANativeWindow *>(msg->getObj());
                initPlayerResourceFlag=videoOutput->createEGLContext(obj);
                break;
            case VIDEO_OUTPUT_MESSAGE_RENDER_FRAME:
                if (videoOutput->eglHasDestroyed){
                    break;
                }
                if (initPlayerResourceFlag){
                    videoOutput->renderVideo();
                }
                break;
            case VIDEO_OUTPUT_MESSAGE_CREATE_WINDOW_SURFACE:
                if (videoOutput->eglHasDestroyed){
                    break;
                }
                if(initPlayerResourceFlag){
                    obj = (ANativeWindow*) (msg->getObj());
                    videoOutput->createWindowSurface(obj);
                }
                break;
            case VIDEO_OUTPUT_MESSAGE_DESTROY_WINDOW_SURFACE:
                if(initPlayerResourceFlag){
                    videoOutput->destroyWindowSurface();
                }
                break;
            case VIDEO_OUTPUT_MESSAGE_DESTROY_EGL_CONTEXT:
                videoOutput->destroyEGLContext();
                break;
        }
    }
};

#endif //NDK_VIDEOPLAYER_VIDEO_OUTPUT_H
