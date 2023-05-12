//
// Created by 史浩 on 2023/4/8.
//

#include "video_output.h"
#define LOG_TAG "VideoOutput"
void *VideoOutput::threadStartCallback(void *ctx) {
    VideoOutput* videoOutput= static_cast<VideoOutput *>(ctx);
    videoOutput->processMessage();
    return 0;
}

void VideoOutput::initOutput(getTextureCallback produceDataCallback, void *ctx) {
    this->produceDataCallback=produceDataCallback;
    this->ctx=ctx;
    queue = new MessageQueue("video output message queue");
    handler = new VideoOutputHandler(this, queue);


    pthread_create(&threadId, 0, threadStartCallback, this);
}

void VideoOutput::onSurfaceCreated(ANativeWindow *window) {
    handler->postMessage(new Message(VIDEO_OUTPUT_MESSAGE_CREATE_EGL_CONTEXT, window));
}

void VideoOutput::onSurfaceChanged(int width, int height) {
    this->screenWidth = width;
    this->screenHeight = height;
}

void VideoOutput::signalFrameAvailable() {
    if(surfaceExists){
        if (handler)
            handler->postMessage(new Message(VIDEO_OUTPUT_MESSAGE_RENDER_FRAME));
    }
}

bool VideoOutput::createEGLContext(ANativeWindow *window) {
    eglCore=new EGLCore();
    bool ret=eglCore->initWithSharedContext();
    if (!ret){
        LOGE("create egl context failed");
        return false;
    }
    createWindowSurface(window);
    eglCore->doneCurrent();
    return false;
}

void VideoOutput::createWindowSurface(ANativeWindow *pWindow) {
    this->surfaceWindow=pWindow;
    renderTexSurface=eglCore->createWindowSurface(pWindow);
    if (renderTexSurface!= nullptr){
        eglCore->makeCurrent(renderTexSurface);
        renderer=new VideoGLSurfaceRender();
        //todo 需要确保已经调用了onSurfaceChanged
        bool isGLViewInitialized =  renderer->init(screenWidth,screenHeight);
        if (isGLViewInitialized){
            surfaceExists = true;
            forceGetFrame = true;
        }
    }
}

bool VideoOutput::renderVideo() {
    FrameTexture* texture= nullptr;
    produceDataCallback(&texture,ctx,forceGetFrame);
    if (texture!= nullptr && renderer!= nullptr){
        eglCore->makeCurrent(renderTexSurface);
        renderer->renderToViewWithAutoFill(texture->texId, screenWidth, screenHeight, texture->width, texture->height);
        if (!eglCore->swapBuffers(renderTexSurface)) {
            LOGE("eglSwapBuffers(renderTexSurface) returned error %d", eglGetError());
        }
    }
    if(forceGetFrame){
        forceGetFrame = false;
    }
    return true;
}

void VideoOutput::destroyWindowSurface() {
    if (renderTexSurface!=EGL_NO_SURFACE){
        if (renderer){
            renderer->dealloc();
            delete renderer;
            renderer= nullptr;
        }
        if(eglCore){
            eglCore->releaseSurface(renderTexSurface);
        }

        renderTexSurface = EGL_NO_SURFACE;
        surfaceExists = false;
        if(NULL != surfaceWindow){
            LOGI("VideoOutput Releasing surfaceWindow");
            ANativeWindow_release(surfaceWindow);
            surfaceWindow = NULL;
        }
    }
}

void VideoOutput::destroyEGLContext() {
    LOGI("enter VideoOutput::destroyEGLContext");
    if (EGL_NO_SURFACE != renderTexSurface){
        eglCore->makeCurrent(renderTexSurface);
    }
    this->destroyWindowSurface();

    if (NULL != eglCore){
        eglCore->release();
        delete eglCore;
        eglCore = NULL;
    }

    eglHasDestroyed = true;

    LOGI("leave VideoOutput::destroyEGLContext");
}

void VideoOutput::processMessage() {
    bool renderingEnabled = true;
    while (renderingEnabled) {
        Message* msg = NULL;
        if(queue->dequeueMessage(&msg, true) > 0){
//			LOGI("msg what is %d", msg->getWhat());
            if(MESSAGE_QUEUE_LOOP_QUIT_FLAG == msg->execute()){
                renderingEnabled = false;
            }
            delete msg;
        }
    }
}
