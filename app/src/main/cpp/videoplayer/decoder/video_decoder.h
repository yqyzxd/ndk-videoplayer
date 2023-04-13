//
// Created by wind on 2023/4/4.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_DECODER_H
#define NDK_VIDEOPLAYER_VIDEO_DECODER_H
#include <stdint.h>
extern "C"{
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"
};
#include "decoder_params.h"
#include <list>
#include "../texture_uploader/texture_frame_uploader.h"

#include "../../common/opengl_media/texture/texture_frame.h"

class VideoDecoder {

public:
    VideoDecoder(DecoderParams* decoderParams);
    virtual ~VideoDecoder();
    int openVideo();

    void close();

    int getAudioChannels();

    int getAudioSampleRate();

    void startUploader(UploaderCallback *uploaderCallback);

    virtual float updateTexImage(TextureFrame *textureFrame)=0;

    void signalDecodeThread();

    int getVideoWidth();

    int getVideoHeight();

    float getVideoFps();

    VideoFrame *handleVideoFrame();

    bool validVideo();

    bool validAudio();

    bool isEOF();

    virtual bool isNetwork();

    std::list<MovieFrame*>* decodeFrames(float minDuration, int *decodeVideoErrorState);

    void uploadTexture();//上传解码出来的avFrame

    AudioFrame* handleAudioFrame();
protected:
    AVCodecContext* videoCodecContext;
    AVFrame* videoFrame;
    AVCodecContext* audioCodecContext;
    AVFrame* audioFrame;
    float position;//视频位置
    SwrContext *swrContext;
    void *swrBuffer;
    int swrBufferSize;
    float audioTimeBase;
private:

    UploaderCallback * mUploaderCallback;
    TextureFrameUploader* textureFrameUploader;
    DecoderParams* decoderParams;
    AVFormatContext* avFormatCtx;

    pthread_mutex_t mLock;
    pthread_cond_t mCondition;

    int videoStreamIndex;
    int width;
    int height;
    float fps;
    float videoTimeBase;
    bool isVideoOutputEOF;//视频是否耗尽了

    long long readLatestFrameTimeMillis;

    int openVideoStream();
    int openVideoStream(int streamIndex);
    std::list<int>* collectStreams(AVMediaType type);

    void determineFpsAndTimeBase(AVStream *stream, double defaultTimeBase, float *fps, float *timebase);




    int audioStreamIndex;
    int openAudioStream();
    int openAudioStream(int streamIndex);

    bool audioCodecIsSupported(AVCodecContext *pContext);


    virtual TextureFrameUploader *createTextureFrameUploader()=0;

    void copyFrameData(uint8_t *luma, uint8_t *string, int width, int height, int linesize);

    virtual bool decodeVideoFrame(AVPacket packet, int *decodeVideoErrorState)=0;

    virtual bool decodeAudioFrames(AVPacket *packet, std::list<MovieFrame *> *result, float& decodedDuration,
                           float minDuration, int *decodeVideoErrorState)=0;
};


#endif //NDK_VIDEOPLAYER_VIDEO_DECODER_H
