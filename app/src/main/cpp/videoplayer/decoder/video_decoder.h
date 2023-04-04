//
// Created by wind on 2023/4/4.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_DECODER_H
#define NDK_VIDEOPLAYER_VIDEO_DECODER_H


#include "decoder_params.h"

class VideoDecoder {

public:
    VideoDecoder(DecoderParams* decoderParams);
    ~VideoDecoder();
    int openVideo();
private:
    DecoderParams* decoderParams;


};


#endif //NDK_VIDEOPLAYER_VIDEO_DECODER_H
