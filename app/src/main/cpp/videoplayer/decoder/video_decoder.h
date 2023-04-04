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

#include <list>
class VideoDecoder {

public:
    VideoDecoder(DecoderParams* decoderParams);
    ~VideoDecoder();
    int openVideo();
private:
    DecoderParams* decoderParams;
    AVFormatContext* avFormatCtx;

    int openVideoStream();
    int openVideoStream(int streamIndex);
    std::list<int>* collectStreams(AVMediaType type);
};


#endif //NDK_VIDEOPLAYER_VIDEO_DECODER_H
