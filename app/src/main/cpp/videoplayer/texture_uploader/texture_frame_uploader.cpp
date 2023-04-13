//
// Created by 史浩 on 2023/4/8.
//

#include <unistd.h>
#include "texture_frame_uploader.h"
#define LOG_TAG "TextureFrameUploader"
void *TextureFrameUploader::threadStartCallback(void *myself) {
    TextureFrameUploader* uploader= static_cast<TextureFrameUploader *>(myself);
    uploader->rendLoop();

    return 0;
}

TextureFrameUploader::TextureFrameUploader() {
    pthread_mutex_init(&mLock, nullptr);
    pthread_cond_init(&mCond,nullptr);
}
TextureFrameUploader::~TextureFrameUploader() {
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCond);
}

void TextureFrameUploader::registerUploadTexImageCallback(
        float (*update_tex_image_callback)(TextureFrame *, void *),
        void (*signal_decode_thread_callback)(void *), void *ctx) {

    this->updateTexImageCallback=update_tex_image_callback;
    this->signalDecodeThreadCallback=signal_decode_thread_callback;
    this->updateTexImageCtx=ctx;
}

void TextureFrameUploader::setUploaderCallback(UploaderCallback *callback) {
    mUploaderCallback=callback;
}

void TextureFrameUploader::start(int width, int height) {
    videoHeight=height;
    videoWidth=width;

    memcpy(vertexCoords, DECODER_COPIER_GL_VERTEX_COORDS, sizeof(GLfloat) * OPENGL_VERTEX_COORDNATE_CNT);
    memcpy(textureCoords, DECODER_COPIER_GL_TEXTURE_COORDS_NO_ROTATION, sizeof(GLfloat) * OPENGL_VERTEX_COORDNATE_CNT);

    _msg=MSG_WINDOW_SET;
    pthread_create(&_threadId,0,threadStartCallback,this);
}

void TextureFrameUploader::rendLoop() {
    bool renderingEnabled=true;
    while (renderingEnabled){
        pthread_mutex_lock(&mLock);
        switch (_msg) {
            case MSG_WINDOW_SET:
                initialize();
                break;
            case MSG_RENDER_LOOP_EXIT:
                renderingEnabled= false;
                destroy();
                break;
            default:
                break;
        }
        _msg=MSG_NONE;
        if (eglCore){
            this->signalDecodeThread();
            pthread_cond_wait(&mCond,&mLock);
            eglCore->makeCurrent(copyTexSurface);
            this->drawFrame();
        }
        pthread_mutex_unlock(&mLock);
    }
}

void TextureFrameUploader::initialize() {
    eglCore=new EGLCore();
    eglCore->initWithSharedContext();
    copyTexSurface=eglCore->createOffscreenSurface(videoWidth,videoHeight);
    eglCore->makeCurrent(copyTexSurface);
    glGenFramebuffers(1,&mFbo);
    glGenTextures(1,&outputTexId);
    glBindTexture(GL_TEXTURE_2D,outputTexId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,videoWidth,videoHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
    glBindTexture(GL_TEXTURE_2D,0);

    if (mUploaderCallback){
        mUploaderCallback->initFromUploaderGLContext(eglCore);
    }
    eglCore->makeCurrent(copyTexSurface);

}

void TextureFrameUploader::destroy() {
    if (mUploaderCallback){
        mUploaderCallback->destroyFromUploaderGLContext();
    }
    eglCore->makeCurrent(copyTexSurface);
    if (outputTexId!=-1){
        glDeleteTextures(1,&outputTexId);
    }
    if (mFbo){
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glDeleteFramebuffers(1,&mFbo);
    }
    eglCore->releaseSurface(copyTexSurface);
    eglCore->release();
    delete eglCore;
    eglCore= nullptr;
}

void TextureFrameUploader::signalDecodeThread() {
    signalDecodeThreadCallback(updateTexImageCtx);
}

void TextureFrameUploader::drawFrame() {
    float position=this->updateTexImage();
    glBindFramebuffer(GL_FRAMEBUFFER,mFbo);
    textureFrameCopier->renderWithCoords(textureFrame,outputTexId, vertexCoords, textureCoords);
    if (mUploaderCallback)
        mUploaderCallback->processVideoFrame(outputTexId, videoWidth, videoHeight, position);
    else
        LOGE("TextureFrameUploader::mUploaderCallback is NULL");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float TextureFrameUploader::updateTexImage() {
    return updateTexImageCallback(textureFrame,updateTexImageCtx);
}

void TextureFrameUploader::signalFrameAvailable() {
    while(_msg == MSG_WINDOW_SET || NULL == eglCore){
        usleep(100 * 1000);
    }
    pthread_mutex_lock(&mLock);
    pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mLock);
}
