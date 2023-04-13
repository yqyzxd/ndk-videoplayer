//
// Created by wind on 2023/4/4.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_SYNCHRONIZER_H
#define NDK_VIDEOPLAYER_VIDEO_SYNCHRONIZER_H

#define LOCAL_MIN_BUFFERED_DURATION 0.5
#define LOCAL_MAX_BUFFERED_DURATION 0.8
#define LOCAL_AV_SYNC_MAX_TIME_DIFF 0.05

#include "decoder/decoder_params.h"
#include "decoder/ffmpeg_video_decoder.h"
#include "texture_uploader/texture_frame_uploader.h"
#include "../common/opengl_media/render/video_gl_surface_render.h"
#include "../common/opengl_media/movie_frame.h"
#include <queue>
#include <unistd.h>
using namespace std;
class VideoSynchronizer;

class UploaderCallbackImpl :public UploaderCallback{
public:
    void setParent(VideoSynchronizer *parent){
        this->parent=parent;
    }
public:
    void processVideoFrame(GLuint inputTexId, int width, int height, float position);

    int processAudioData(short *sample, int size, float position, byte** buffer);

    //void onSeekCallback(float seek_seconds);

    void initFromUploaderGLContext(EGLCore* eglCore);

    void destroyFromUploaderGLContext();

protected:
    VideoSynchronizer *parent;

};

class VideoSynchronizer {
public:
    VideoSynchronizer();
    ~VideoSynchronizer();
    int init(DecoderParams* decoderParams);

    int getAudioChannels();

    int getAudioSampleRate();

    void start();


    virtual void OnInitFromUploaderGLContext(EGLCore* eglCore,
                                             int videoFrameWidth, int videoFrameHeight);
    virtual void onDestroyFromUploaderGLContext();
    virtual void processVideoFrame(GLuint inputTexId, int width, int height, float position);
    virtual int processAudioData(short *sample, int size, float position, byte** buffer);
    UploaderCallbackImpl mUploaderCallback;
    bool isDestroyed;
    bool isCompleted;

    int getVideoFrameWidth();
    int getVideoFrameHeight();

    bool isPlayCompleted();

    int fillAudioData(byte *outData, size_t bufferSize);

private:
    VideoDecoder* decoder;
    int decodeVideoErrorState;
    float minBufferedDuration;
    float maxBufferedDuration;
    float syncMaxTimeDiff;

    bool buffered;//当前缓冲区是否有数据
    float bufferedDuration;//当前缓冲区时长
    bool isDecodingFrames;
    bool isOnDecoding;
    pthread_t videoDecoderThread;//解码线程
    pthread_mutex_t videoDecoderLock;
    pthread_cond_t videoDecoderCondition;
    bool isInitializedDecodeThread;//是否已经启动解码线程
    /** 这里是为了将audioFrame中的数据，缓冲到播放音频的buffer中，有可能需要积攒几个frame，所以记录位置以及保存当前frame **/
    AudioFrame* currentAudioFrame;
    int currentAudioFramePos;
    float moviePosition;
    bool pauseDecodeThreadFlag;//是否暂停解码线程

    VideoGLSurfaceRender* passThroughRender;
    CircleFrameTextureQueue* circleFrameTextureQueue;
    std::queue<AudioFrame*>* audioFrameQueue;
    pthread_mutex_t audioFrameQueueMutex;


    static void* startDecoderThread(void* ptr);

    void closeDecoder();

    void initCircleQueue(int width, int height);

    void destroyPassThroughRender();

    void clearVideoFrameQueue();

    void renderToVideoQueue(GLuint texId, int width, int height, float position);

    void frameAvailable();

    void signalDecodeThread();

    float getVideoFPS();

    bool checkPlayState();

    void initDecoderThread();

    void decode();

    void decodeFrames();

    bool canDecode();

    void processDecodingFrame(bool* good, float duration);
};


#endif //NDK_VIDEOPLAYER_VIDEO_SYNCHRONIZER_H
