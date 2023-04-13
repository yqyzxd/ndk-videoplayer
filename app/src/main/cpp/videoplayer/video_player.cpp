//
// Created by wind on 2023/4/4.
//


#include "video_player.h"
#include "circle_texture_queue.h"

VideoPlayer::VideoPlayer() {

}
VideoPlayer::~VideoPlayer() {

}
void VideoPlayer::setDataSource(char *dataSource) {

    path=new char[strlen(dataSource)+1];
    strcpy(path,dataSource);

}


void VideoPlayer::prepare() {
    DecoderParams* params=new DecoderParams(path);
    synchronizer=new VideoSynchronizer();
    int ret =synchronizer->init(params);
    if (ret<0){
        //error
    }
    ret=initAudioOutput();
    if (ret<0){
        //error
    }


}

int VideoPlayer::videoCallbackGetTex(FrameTexture** frameTex, void* ctx, bool forceGetFrame){
    VideoPlayer* player = (VideoPlayer*) ctx;
    return player->getCorrectRenderTexture(frameTex, forceGetFrame);
}
void VideoPlayer::setWindow(ANativeWindow *window) {
    if (videoOutput== nullptr){
        videoOutput=new VideoOutput();
        videoOutput->initOutput(videoCallbackGetTex,this);
    }
    videoOutput->onSurfaceCreated(window);
}

void VideoPlayer::setWindowSize(int width, int height) {
    if (videoOutput){
        videoOutput->onSurfaceChanged(width,height);
    }
}

void VideoPlayer::play() {
    isPlaying=true;
    synchronizer->start();
    audioOutput->start();
}

int VideoPlayer::audioCallbackFillData(byte* outData, size_t bufferSize, void* ctx) {
    VideoPlayer* player = (VideoPlayer*) ctx;
    return player->consumeAudioFrames(outData, bufferSize);
}
int VideoPlayer::initAudioOutput() {
    int channels = synchronizer->getAudioChannels();
    if (channels<0){
        return -1;
    }
    int sampleRate=synchronizer->getAudioSampleRate();
    if (sampleRate<0){
        return -1;
    }
    audioOutput=new AudioOutput();
    SLresult result=audioOutput->initSoundTrack(channels,sampleRate,audioCallbackFillData,this);
    if(result !=SL_RESULT_SUCCESS){
        delete audioOutput;
        audioOutput= nullptr;
        return -1;
    }
    return 0;
}



int VideoPlayer::consumeAudioFrames(byte *outData, size_t bufferSize) {
    int ret =bufferSize;
    if (this->isPlaying&&synchronizer&&!synchronizer->isDestroyed && !synchronizer->isPlayCompleted()){
        ret=synchronizer->fillAudioData(outData,bufferSize);
        signalOutputFrameAvailable();
    }else{
        memset(outData,0,bufferSize);
    }
    return ret;
}

int VideoPlayer::getCorrectRenderTexture(FrameTexture **frameTex, bool forceGetFrame) {
    return 0;
}

void VideoPlayer::signalOutputFrameAvailable() {
    if (NULL != videoOutput){
        videoOutput->signalFrameAvailable();
    }
}

