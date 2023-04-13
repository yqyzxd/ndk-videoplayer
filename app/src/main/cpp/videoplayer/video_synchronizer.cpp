//
// Created by wind on 2023/4/4.
//


#include "video_synchronizer.h"


void UploaderCallbackImpl::initFromUploaderGLContext(EGLCore *eglCore) {
    if (parent) {
        int videoFrameWidth = parent->getVideoFrameWidth();
        int videoFrameHeight = parent->getVideoFrameHeight();
        parent->OnInitFromUploaderGLContext(eglCore, videoFrameWidth, videoFrameHeight);
    }
}

void
UploaderCallbackImpl::processVideoFrame(GLuint inputTexId, int width, int height, float position) {
    if (parent) {
        parent->processVideoFrame(inputTexId, width, height, position);
    }
}

int UploaderCallbackImpl::processAudioData(short *sample, int size, float position, byte **buffer) {
    if (parent) {
        parent->processAudioData(sample, size, position, buffer);
    }
}

void UploaderCallbackImpl::destroyFromUploaderGLContext() {
    if (parent) {
        parent->onDestroyFromUploaderGLContext();
    }
}


void VideoSynchronizer::OnInitFromUploaderGLContext(EGLCore *eglCore, int videoFrameWidth,
                                                    int videoFrameHeight) {
    if (passThroughRender == nullptr) {
        passThroughRender = new VideoGLSurfaceRender();
        passThroughRender->init(videoFrameWidth, videoFrameHeight);
    }
    initCircleQueue(videoFrameWidth, videoFrameHeight);
    eglCore->doneCurrent();
}

int VideoSynchronizer::processAudioData(short *sample, int size, float position, byte **buffer) {
    int bufferSize = size * 2;
    (*buffer) = new byte[bufferSize];
    convertByteArrayFromShortArray(sample, size, *buffer);
    return bufferSize;

}

void
VideoSynchronizer::processVideoFrame(GLuint inputTexId, int width, int height, float position) {
    renderToVideoQueue(inputTexId, width, height, position);
}

void VideoSynchronizer::onDestroyFromUploaderGLContext() {
    destroyPassThroughRender();
    if (circleFrameTextureQueue) {
        clearVideoFrameQueue();
        circleFrameTextureQueue->abort();
        delete circleFrameTextureQueue;
        circleFrameTextureQueue = nullptr;
    }
}


VideoSynchronizer::VideoSynchronizer() {
    mUploaderCallback.setParent(this);
}

int VideoSynchronizer::init(DecoderParams *decoderParams) {
    decoder = new FFmpegVideoDecoder(decoderParams);
    int ret = decoder->openVideo();
    if (ret < 0) {
        closeDecoder();
    }


    minBufferedDuration = LOCAL_MIN_BUFFERED_DURATION;
    maxBufferedDuration = LOCAL_MAX_BUFFERED_DURATION;


}

void VideoSynchronizer::closeDecoder() {
    if (decoder != nullptr) {
        decoder->close();
        delete decoder;
        decoder = nullptr;
    }
}

int VideoSynchronizer::getAudioChannels() {
    if (decoder != nullptr) {
        return decoder->getAudioChannels();
    }
    return -1;
}

int VideoSynchronizer::getAudioSampleRate() {
    if (decoder != nullptr) {
        return decoder->getAudioSampleRate();
    }
    return -1;
}

void VideoSynchronizer::start() {
    if (decoder != nullptr) {
        decoder->startUploader(&mUploaderCallback);
    }
    circleFrameTextureQueue->setIsFirstFrame(true);
    initDecoderThread();

}

int VideoSynchronizer::getVideoFrameWidth() {
    if (decoder) {
        decoder->getVideoWidth();
    }
    return -1;
}

int VideoSynchronizer::getVideoFrameHeight() {
    if (decoder) {
        decoder->getVideoHeight();
    }
    return -1;
}

void VideoSynchronizer::initCircleQueue(int width, int height) {
    float fps = decoder->getVideoFps();
    if (fps > 30.0f) {
        fps = 30.0f;
    }
    int queueSize = (maxBufferedDuration + 1.0) * fps;
    circleFrameTextureQueue = new CircleFrameTextureQueue("decode frame texture queue");
    circleFrameTextureQueue->init(width, height, queueSize);
    audioFrameQueue = new std::queue<AudioFrame *>();
    pthread_mutex_init(&audioFrameQueueMutex, 0);
}

void VideoSynchronizer::destroyPassThroughRender() {
    if (passThroughRender) {
        passThroughRender->dealloc();
        delete passThroughRender;
        passThroughRender = nullptr;
    }
}

void VideoSynchronizer::clearVideoFrameQueue() {
    if (circleFrameTextureQueue) {
        circleFrameTextureQueue->clear();
    }
}

void VideoSynchronizer::renderToVideoQueue(GLuint texId, int width, int height, float position) {
    if (!passThroughRender) {
        return;
    }
    if (!circleFrameTextureQueue) {
        return;
    }
    bool isFirstFrame = circleFrameTextureQueue->getIsFirstFrame();
    FrameTexture *frameTexture = circleFrameTextureQueue->lockPushCursorFrameTexture();
    if (frameTexture) {
        frameTexture->position = position;
        passThroughRender->renderToTexture(texId, frameTexture->texId);
        circleFrameTextureQueue->unLockPushCursorFrameTexture();

        frameAvailable();
        if (isFirstFrame) {
            FrameTexture *firstFrameTexture = circleFrameTextureQueue->getFirstFrameFrameTexture();
            if (firstFrameTexture) {
                //cpy input texId to target texId
                passThroughRender->renderToTexture(texId, firstFrameTexture->texId);
            }
        }
    }
}

void VideoSynchronizer::frameAvailable() {

}

bool VideoSynchronizer::isPlayCompleted() {
    return isCompleted;
}

int VideoSynchronizer::fillAudioData(byte *outData, size_t bufferSize) {
    signalDecodeThread();//激活解码线程
    checkPlayState();
    if (buffered) {
        memset(outData, 0, bufferSize);
        return bufferSize;//缓存中，返回空白数据
    }

    int needBufferSize = bufferSize;
    while (bufferSize > 0) {

        if (currentAudioFrame == nullptr) {
            pthread_mutex_lock(&audioFrameQueueMutex);
            int count = audioFrameQueue->size();
            if (count > 0) {
                AudioFrame *frame = audioFrameQueue->front();
                bufferedDuration = frame->duration;
                audioFrameQueue->pop();
                moviePosition = frame->position;
                currentAudioFrame = new AudioFrame;
                currentAudioFramePos = 0;
                int frameSize = frame->size;
                currentAudioFrame->samples = new byte[frameSize];
                memcpy(currentAudioFrame->samples, frame->samples, frameSize);
                currentAudioFrame->size = frameSize;
                delete frame;
            }
            pthread_mutex_unlock(&audioFrameQueueMutex);
        }
        if (currentAudioFrame != nullptr) {
            byte *bytes = currentAudioFrame->samples + currentAudioFramePos;
            int bytesLeft = currentAudioFrame->size - currentAudioFramePos;
            int bytesCopy = MIN(bufferSize, bytesLeft);
            memcpy(outData, bytes, bytesCopy);
            bufferSize -= bytesCopy;
            outData += bytesCopy;
            if (bytesCopy < bytesLeft) {
                currentAudioFramePos += bytesCopy;
            } else {
                delete currentAudioFrame;
                currentAudioFrame = nullptr;
            }

        } else {
            memcpy(outData, 0, bufferSize);
            bufferSize = 0;
        }
    }
    return needBufferSize - bufferSize;
}

void VideoSynchronizer::signalDecodeThread() {
    if (NULL == decoder || isDestroyed) {
        //LOGI("NULL == decoder || isDestroyed == true");
        return;
    }

    //如果没有剩余的帧了或者当前缓存的长度大于我们的最小缓冲区长度的时候，就再一次开始解码
    bool isBufferedDurationDecreasedToMin = bufferedDuration <= minBufferedDuration ||
                                            (circleFrameTextureQueue->getValidSize() <=
                                             minBufferedDuration * getVideoFPS());

    if (!isDestroyed || ((!isDecodingFrames) && isBufferedDurationDecreasedToMin)) {
        pthread_mutex_lock(&videoDecoderLock);
        pthread_cond_signal(&videoDecoderCondition);
        pthread_mutex_unlock(&videoDecoderLock);
    }
}

float VideoSynchronizer::getVideoFPS() {
    if (decoder) {
        return decoder->getVideoFps();
    }
    return -1;
}

bool VideoSynchronizer::checkPlayState() {
    if (decoder == nullptr || circleFrameTextureQueue == nullptr || audioFrameQueue == nullptr) {
        return false;
    }

    if (decodeVideoErrorState == 1) {
        decodeVideoErrorState = 0;
        //todo 回调java层 videoDecodeException
    }

    int leftVideoFrames = decoder->validVideo() ? circleFrameTextureQueue->getValidSize() : 0;
    int leftAudioFrames = decoder->validAudio() ? audioFrameQueue->size() : 0;
    if (leftVideoFrames == 1 || leftAudioFrames == 0) {//播放完了
        //需要暂缓播放
        buffered = true;
        //todo 回调java层 showLoadingDialog()

        if (decoder->isEOF()) {
            //由于OpenSLES 暂停之后有一些数据 还在buffer里面，暂停200ms让他播放完毕
            usleep(0.2 * 1000000);
            isCompleted = true;
            //todo 回调java层 通知播放结束 onCompletion();
//			LOGI("onCompletion...");
            return true;
        }
    } else {
        bool isBufferedDurationIncreasedToMin =
                leftVideoFrames >= int(minBufferedDuration * getVideoFPS()) &&
                (bufferedDuration >= minBufferedDuration);
        if (isBufferedDurationIncreasedToMin || decoder->isEOF()) {
            buffered = false;
            //todo 回调java层 hideLoadingDialog();
        }
    }
    return false;
}

void *VideoSynchronizer::startDecoderThread(void *ptr) {
    VideoSynchronizer* synchronizer=(VideoSynchronizer*)ptr;
    while (synchronizer->isOnDecoding){
        synchronizer->decode();
    }

    return 0;
}
void VideoSynchronizer::initDecoderThread() {
    if(isDestroyed){
        return;
    }
    isDecodingFrames=false;
    pthread_mutex_init(&videoDecoderLock, nullptr);
    pthread_cond_init(&videoDecoderCondition, nullptr);
    isInitializedDecodeThread=true;
    pthread_create(&videoDecoderThread, nullptr,startDecoderThread,this);
}

void VideoSynchronizer::decode() {
    pthread_mutex_lock(&videoDecoderLock);
    pthread_cond_wait(&videoDecoderCondition,&videoDecoderLock);
    pthread_mutex_unlock(&videoDecoderLock);

    isDecodingFrames= true;
    decodeFrames();
    isDecodingFrames=false;
}

void VideoSynchronizer::decodeFrames() {
    bool good=true;
    float duration=decoder->isNetwork()?0.0f:0.1f;
    while (good){
        good=false;
        if(canDecode()){
            processDecodingFrame(&good,duration);
        }
    }

}

bool VideoSynchronizer::canDecode() {
    return !pauseDecodeThreadFlag && !isDestroyed && decoder && (decoder->validVideo() || decoder->validAudio()) && !decoder->isEOF();
}

void VideoSynchronizer::processDecodingFrame(bool* good, float duration) {
    std::list<MovieFrame> frames=decoder->decodeFrames(duration,&decodeVideoErrorState);

}
