//
// Created by 史浩 on 2023/4/6.
//

#ifndef NDK_VIDEOPLAYER_FFMPEG_VIDEO_DECODER_H
#define NDK_VIDEOPLAYER_FFMPEG_VIDEO_DECODER_H


#include "video_decoder.h"

class FFmpegVideoDecoder : public VideoDecoder{
public :
    FFmpegVideoDecoder(DecoderParams *decoderParams);
    ~FFmpegVideoDecoder();
};


#endif //NDK_VIDEOPLAYER_FFMPEG_VIDEO_DECODER_H
