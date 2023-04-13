//
// Created by 史浩 on 2023/4/8.
//

#ifndef NDK_VIDEOPLAYER_TEXTURE_FRAME_UPLOADER_H
#define NDK_VIDEOPLAYER_TEXTURE_FRAME_UPLOADER_H

#include "../circle_texture_queue.h"
#include "../../common/egl_core/egl_core.h"
#include "../../common/CommonTools.h"
#include "../../common/opengl_media/texture/texture_frame.h"
#include "../../common/opengl_media/texture/yuv_texture_frame.h"
#include "../../common/opengl_media/texture_copier/texture_frame_copier.h"
#include "../../common/opengl_media/texture_copier/yuv_texture_frame_copier.h"


#define OPENGL_VERTEX_COORDNATE_CNT			8

static GLfloat DECODER_COPIER_GL_VERTEX_COORDS[8] = {
        -1.0f, -1.0f,	// 0 top left
        1.0f, -1.0f,	// 1 bottom left
        -1.0f, 1.0f,  // 2 bottom right
        1.0f, 1.0f,	// 3 top right
};

static GLfloat DECODER_COPIER_GL_TEXTURE_COORDS_NO_ROTATION[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
};

static GLfloat DECODER_COPIER_GL_TEXTURE_COORDS_ROTATED_90[8] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
};

static GLfloat DECODER_COPIER_GL_TEXTURE_COORDS_ROTATED_180[8] = {
        1.0, 1.0,
        0.0, 1.0,
        1.0, 0.0,
        0.0, 0.0,
};
static GLfloat DECODER_COPIER_GL_TEXTURE_COORDS_ROTATED_270[8] = {
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
};

class UploaderCallback{
public:
    virtual void processVideoFrame(GLuint inputTexId,int width,int height,float position)=0;
    virtual int processAudioData(short* sample,int size,float position,byte** buffer)=0;
   // virtual void onSeekCallback(float seek_seconds) = 0;

    virtual void initFromUploaderGLContext(EGLCore* eglCore) = 0;

    virtual void destroyFromUploaderGLContext() = 0;
};

class TextureFrameUploader {


public:
    TextureFrameUploader();
    virtual ~TextureFrameUploader();
    typedef float (*update_tex_image_callback)(TextureFrame* textureFrame, void *context);
    typedef void (*signal_decode_thread_callback)(void *context);
    void registerUploadTexImageCallback(update_tex_image_callback, signal_decode_thread_callback,
                                   void *ctx);

    void setUploaderCallback(UploaderCallback *callback);

    void start(int width, int height);
    void destroy();

    void signalFrameAvailable();

protected:
    EGLCore* eglCore;
    EGLSurface copyTexSurface;

    GLfloat* vertexCoords;
    GLfloat* textureCoords;

    GLuint mFbo;
    GLuint outputTexId;
    int videoWidth;
    int videoHeight;
    update_tex_image_callback updateTexImageCallback;
    signal_decode_thread_callback signalDecodeThreadCallback;
    void* updateTexImageCtx;
    UploaderCallback* mUploaderCallback=0;

    TextureFrame* textureFrame=0;
    TextureFrameCopier* textureFrameCopier=0;

    enum RenderThreadMessage {
        MSG_NONE = 0, MSG_WINDOW_SET, MSG_RENDER_LOOP_EXIT
    };
    enum RenderThreadMessage _msg;


    pthread_t _threadId;
    pthread_mutex_t mLock;
    pthread_cond_t mCond;
    static void* threadStartCallback(void *myself);
    void rendLoop();

    virtual void initialize();

    void signalDecodeThread();

    void drawFrame();

    float updateTexImage();
};


#endif //NDK_VIDEOPLAYER_TEXTURE_FRAME_UPLOADER_H
