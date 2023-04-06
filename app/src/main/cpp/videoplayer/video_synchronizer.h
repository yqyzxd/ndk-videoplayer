//
// Created by wind on 2023/4/4.
//

#ifndef NDK_VIDEOPLAYER_VIDEO_SYNCHRONIZER_H
#define NDK_VIDEOPLAYER_VIDEO_SYNCHRONIZER_H

#define LOCAL_MIN_BUFFERED_DURATION 0.5
#define LOCAL_MAX_BUFFERED_DURATION 0.8
#define LOCAL_AV_SYNC_MAX_TIME_DIFF 0.05

#include "decoder/decoder_params.h"
#include "decoder/video_decoder.h"
#include "decoder/ffmpeg_video_decoder.h"
class VideoSynchronizer {
public:
    VideoSynchronizer();
    ~VideoSynchronizer();
    int init(DecoderParams* decoderParams);

private:
    VideoDecoder* decoder;
    float minBufferedDuration;
    float maxBufferedDuration;
    float syncMaxTimeDiff;

    void closeDecoder();
};


#endif //NDK_VIDEOPLAYER_VIDEO_SYNCHRONIZER_H
