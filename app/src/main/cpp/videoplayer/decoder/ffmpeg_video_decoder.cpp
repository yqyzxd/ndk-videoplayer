//
// Created by 史浩 on 2023/4/6.
//

#include "ffmpeg_video_decoder.h"


FFmpegVideoDecoder::FFmpegVideoDecoder(DecoderParams *decoderParams): VideoDecoder(decoderParams) {

}

FFmpegVideoDecoder::~FFmpegVideoDecoder()  {}


TextureFrameUploader *FFmpegVideoDecoder::createTextureFrameUploader() {
    TextureFrameUploader* uploader=new YUVTextureFrameUploader();
    return uploader;
}

float FFmpegVideoDecoder::updateTexImage(TextureFrame *textureFrame) {
    float position=-1;
    VideoFrame* yuvFrame=handleVideoFrame();
    if (yuvFrame){
        YUVTextureFrame* yuvTextureFrame= dynamic_cast<YUVTextureFrame *>(textureFrame);
        yuvTextureFrame->setVideoFrame(yuvFrame);
        position=yuvFrame->position;
        delete yuvFrame;
    }
    return position;
}

bool FFmpegVideoDecoder::decodeVideoFrame(AVPacket packet, int *decodeVideoErrorState) {
    int pkgSize=packet.size;

    int gotFrame=0;
    while (pkgSize>0){
        int len =avcodec_decode_video2(videoCodecContext,videoFrame,&gotFrame,&packet);
        if (len<0){
            *decodeVideoErrorState=1;
            break;
        }
        if (gotFrame){
            if (videoFrame->interlaced_frame){
                avpicture_deinterlace((AVPicture*)videoFrame,(AVPicture*)videoFrame,videoCodecContext->pix_fmt,videoCodecContext->width,videoCodecContext->height);
            }

            uploadTexture();
        }
        if (len==0){
            break;
        }
        pkgSize-=len;
    }
    return (bool)gotFrame;
}

bool FFmpegVideoDecoder::decodeAudioFrames(AVPacket *packet, std::list<MovieFrame *> *result,
                                           float& decodedDuration, float minDuration,
                                           int *decodeVideoErrorState) {
    bool finished=false;
    int pktSize=packet->size;
    while (pktSize>0){
        int gotFrame=0;
        int len=avcodec_decode_audio4(audioCodecContext,audioFrame,&gotFrame,packet);
        if (len<0){
            *decodeVideoErrorState=1;
            break;
        }
        if (gotFrame){
            AudioFrame* frame=handleAudioFrame();
            if (frame!= nullptr){
                result->push_back(frame);
                position=frame->position;
                decodedDuration+=frame->duration;
                if (decodedDuration>minDuration){
                    finished= true;
                }
            }
        }
        if (len==0){
            break;
        }
        pktSize-=len;

    }
    return finished;
}




