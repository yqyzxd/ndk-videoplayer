//
// Created by wind on 2023/4/4.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_DECODER_H
#define NDK_VIDEOPLAYER_VIDEO_DECODER_H

extern "C"{
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
};
#include "decoder_params.h"
#include "libswresample/swresample.h"

#include <list>
class VideoDecoder {

public:
    VideoDecoder(DecoderParams* decoderParams);
    ~VideoDecoder();
    int openVideo();

    void close();

private:
    DecoderParams* decoderParams;
    AVFormatContext* avFormatCtx;
    AVCodecContext* videoCodecContext;
    AVFrame* videoFrame;
    int videoStreamIndex;
    int width;
    int height;
    float fps;
    float videoTimeBase;
    int openVideoStream();
    int openVideoStream(int streamIndex);
    std::list<int>* collectStreams(AVMediaType type);

    void determineFpsAndTimeBase(AVStream *stream, double defaultTimeBase, float *fps, float *timebase);

    AVCodecContext* audioCodecContext;
    SwrContext *swrContext;
    AVFrame* audioFrame;
    float audioTimeBase;
    int audioStreamIndex;
    int openAudioStream();
    int openAudioStream(int streamIndex);

    bool audioCodecIsSupported(AVCodecContext *pContext);
};


#endif //NDK_VIDEOPLAYER_VIDEO_DECODER_H
