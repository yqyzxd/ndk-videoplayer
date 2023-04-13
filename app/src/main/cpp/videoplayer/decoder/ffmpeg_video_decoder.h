//
// Created by 史浩 on 2023/4/6.
//

#ifndef NDK_VIDEOPLAYER_FFMPEG_VIDEO_DECODER_H
#define NDK_VIDEOPLAYER_FFMPEG_VIDEO_DECODER_H


#include "video_decoder.h"
#include "../texture_uploader/yuv_texture_frame_uploader.h"
class FFmpegVideoDecoder : public VideoDecoder{
public :
    FFmpegVideoDecoder(DecoderParams *decoderParams);
    ~FFmpegVideoDecoder();

    virtual TextureFrameUploader *createTextureFrameUploader();
    virtual float updateTexImage(TextureFrame *textureFrame);
    virtual bool decodeVideoFrame(AVPacket packet, int *decodeVideoErrorState);
    virtual bool decodeAudioFrames(AVPacket *packet, std::list<MovieFrame *> *result, float& decodedDuration,
                                   float minDuration, int *decodeVideoErrorState);


};


#endif //NDK_VIDEOPLAYER_FFMPEG_VIDEO_DECODER_H
