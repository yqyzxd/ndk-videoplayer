//
// Created by wind on 2023/4/4.
//

#include "video_synchronizer.h"


VideoSynchronizer::VideoSynchronizer() {

}
int VideoSynchronizer::init(DecoderParams* decoderParams) {
    decoder= new FFmpegVideoDecoder(decoderParams);
    int ret=decoder->openVideo();
    if (ret<0){
        closeDecoder();
    }


    minBufferedDuration=LOCAL_MIN_BUFFERED_DURATION;
    maxBufferedDuration=LOCAL_MAX_BUFFERED_DURATION;

    //decoder->startUploader();



}

void VideoSynchronizer::closeDecoder() {
    if (decoder != nullptr){
        decoder->close();
        delete decoder;
        decoder= nullptr;
    }
}
