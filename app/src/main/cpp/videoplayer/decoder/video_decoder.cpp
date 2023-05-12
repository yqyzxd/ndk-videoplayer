//
// Created by wind on 2023/4/4.
//

#include "video_decoder.h"


VideoDecoder::VideoDecoder(DecoderParams *decoderParams) {
    this->decoderParams = decoderParams;
}

VideoDecoder::~VideoDecoder() {}

int VideoDecoder::openVideo() {


    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondition, NULL);

    AVFormatContext *avFormatCtx = avformat_alloc_context();
    AVDictionary *dictionary = nullptr;
    av_dict_set(&dictionary, "timeout", "5000000", 0);
    int ret = avformat_open_input(&avFormatCtx, decoderParams->getUri(), 0, &dictionary);
    av_dict_free(&dictionary);
    if (ret) {
        //todo 打开出错了
        avformat_close_input(&avFormatCtx);
        return ret;
    }
    ret = avformat_find_stream_info(avFormatCtx, nullptr);
    if (ret) {
        //todo 打开出错了
        avformat_close_input(&avFormatCtx);
        return ret;
    }


    int videoErr = openVideoStream();
    int audioErr = openAudioStream();


    if (videoErr < 0 && audioErr < 0) {
        return -1;
    }

    return 0;


}

int VideoDecoder::openVideoStream() {
    int errCode = -1;
    videoStreamIndex = -1;
    std::list<int> *videoStreams = collectStreams(AVMEDIA_TYPE_VIDEO);
    std::list<int>::iterator iter;
    for (iter = videoStreams->begin(); iter != videoStreams->end(); ++iter) {
        int streamIndex = *iter;
        if ((avFormatCtx->streams[streamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC) == 0) {
            errCode = openVideoStream(streamIndex);
            if (errCode >= 0) {
                break;
            }
        }
    }

    return errCode;
}

int VideoDecoder::openAudioStream() {
    int errCode = -1;
    audioStreamIndex = -1;
    std::list<int> *audioStreams = collectStreams(AVMEDIA_TYPE_AUDIO);
    std::list<int>::iterator iter;
    for (iter = audioStreams->begin(); iter != audioStreams->end(); iter++) {
        int streamIndex = *iter;
        errCode = openAudioStream(streamIndex);
        if (errCode >= 0) {
            break;
        }
    }
    return 0;
}

int VideoDecoder::openVideoStream(int streamIndex) {
    AVStream *videoStream = avFormatCtx->streams[streamIndex];
    //获取 编码解码的【参数】
    videoCodecContext = videoStream->codec;
    AVCodec *codec = avcodec_find_decoder(videoCodecContext->codec_id);
    if (!codec) {
        // avformat_close_input(&avFormatCtx);
        return -1;
    }

    int ret = avcodec_open2(videoCodecContext, codec, nullptr);
    if (ret) {
        avcodec_free_context(&videoCodecContext);
        // avformat_close_input(&avFormatCtx);
    }

    // 解码后的视频原始数据包
    videoFrame = av_frame_alloc();
    this->videoStreamIndex = streamIndex;
    if (videoFrame == nullptr) {
        avcodec_free_context(&videoCodecContext);
        return -1;
    }
    determineFpsAndTimeBase(videoStream, 0.04, &fps, &videoTimeBase);

    if (fps > 30.0f || fps < 5.0f) {
        fps = 24.0f;
    }
    if (videoCodecContext->pix_fmt != AV_PIX_FMT_YUV420P &&
        videoCodecContext->pix_fmt != AV_PIX_FMT_YUVJ420P) {
        avcodec_free_context(&videoCodecContext);
        return -1;
    }
    width = videoCodecContext->width;
    height = videoCodecContext->height;
    return 0;
}

int VideoDecoder::openAudioStream(int streamIndex) {
    AVStream *audioStream = avFormatCtx->streams[streamIndex];
    videoCodecContext = audioStream->codec;
    AVCodec *codec = avcodec_find_decoder(videoCodecContext->codec_id);
    if (!codec) {
        avformat_close_input(&avFormatCtx);
        return -1;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        return -1;
    }

    int ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0) {
        avcodec_free_context(&codecContext);
        return -1;
    }
    audioCodecContext = codecContext;

    if (!audioCodecIsSupported(audioCodecContext)) {
        swrContext = swr_alloc_set_opts(nullptr,
                                        av_get_default_channel_layout(audioCodecContext->channels),
                                        AV_SAMPLE_FMT_S16, audioCodecContext->sample_rate,
                                        av_get_default_channel_layout(audioCodecContext->channels),
                                        audioCodecContext->sample_fmt,
                                        audioCodecContext->sample_rate, 0,
                                        nullptr);
        if (!swrContext || swr_init(swrContext)) {
            if (swrContext) {
                swr_free(&swrContext);
            }
            avcodec_free_context(&audioCodecContext);
            return -1;
        }
    }

    audioFrame = av_frame_alloc();
    if (audioFrame == nullptr) {
        if (swrContext) {
            swr_free(&swrContext);
        }
        avcodec_free_context(&audioCodecContext);
        return -1;
    }
    audioStreamIndex = streamIndex;
    determineFpsAndTimeBase(audioStream, 0.025, 0, &audioTimeBase);
    return 0;

}

std::list<int> *VideoDecoder::collectStreams(AVMediaType type) {
    std::list<int> *typeStreams = new std::list<int>();
    for (int i = 0; i < avFormatCtx->nb_streams; i++) {
        if (type == avFormatCtx->streams[i]->codec->codec_type) {
            typeStreams->push_back(i);
        }
    }
    return typeStreams;
}

void VideoDecoder::determineFpsAndTimeBase(AVStream *stream, double defaultTimeBase, float *pFps,
                                           float *pTimebase) {
    //时间基：所谓时间基表示的就是每个刻度是多少秒。如果你是把1秒分成90000份，每一个刻度就是1/90000秒，此时的time_base={1，90000}。
    float fps, timebase;
    if (stream->time_base.den && stream->time_base.num) {
        timebase = av_q2d(stream->time_base);
    } else if (stream->codec->time_base.den && stream->codec->time_base.num) {
        timebase = av_q2d(stream->codec->time_base);
    } else {
        timebase = defaultTimeBase;
    }

    if (stream->avg_frame_rate.den && stream->avg_frame_rate.num) {
        fps = av_q2d(stream->avg_frame_rate);
    } else if (stream->r_frame_rate.den && stream->r_frame_rate.num) {
        fps = av_q2d(stream->r_frame_rate);
    } else {
        fps = 1.0 / timebase;
    }
    if (pFps) {
        *pFps = fps;
    }
    if (pTimebase) {
        *pTimebase = timebase;
    }

}

bool VideoDecoder::audioCodecIsSupported(AVCodecContext *audioCodecCtx) {
    if (audioCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16) {
        return true;
    }
    return false;
}

void VideoDecoder::close() {

}

int VideoDecoder::getAudioChannels() {
    if (audioCodecContext != nullptr) {
        return audioCodecContext->channels;
    }
    return -1;
}

int VideoDecoder::getAudioSampleRate() {
    if (audioCodecContext != nullptr) {
        return audioCodecContext->sample_rate;
    }
    return -1;
}

static float update_tex_image_callback(TextureFrame *textureFrame, void *context) {
    VideoDecoder *videoDecoder = (VideoDecoder *) context;
    return videoDecoder->updateTexImage(textureFrame);
}

static void signal_decode_thread_callback(void *context) {
    VideoDecoder *videoDecoder = (VideoDecoder *) context;
    return videoDecoder->signalDecodeThread();
}


void VideoDecoder::startUploader(UploaderCallback *uploaderCallback) {
    this->mUploaderCallback = uploaderCallback;
    textureFrameUploader = createTextureFrameUploader();
    textureFrameUploader->registerUploadTexImageCallback(update_tex_image_callback,
                                                         signal_decode_thread_callback, this);
    textureFrameUploader->setUploaderCallback(uploaderCallback);
    textureFrameUploader->start(width, height);

    pthread_mutex_lock(&mLock);
    pthread_cond_wait(&mCondition, &mLock);
    pthread_mutex_unlock(&mLock);
}


void VideoDecoder::signalDecodeThread() {

}

int VideoDecoder::getVideoHeight() {
    if (videoCodecContext) {
        return videoCodecContext->height;
    }
    return -1;
}

int VideoDecoder::getVideoWidth() {
    if (videoCodecContext) {
        return videoCodecContext->width;
    }
    return -1;
}

float VideoDecoder::getVideoFps() {
    return fps;
}

VideoFrame *VideoDecoder::handleVideoFrame() {
    if (!videoFrame->data[0]) {
        return nullptr;
    }

    VideoFrame *yuvFrame = new VideoFrame();
    int width = MIN(videoFrame->linesize[0], videoCodecContext->width);
    int height = videoCodecContext->height;
    int lumaLength = width * height;
    uint8_t *luma = new uint8_t[lumaLength];
    copyFrameData(luma, const_cast<uint8_t *>(videoFrame->data[0]), width, height, videoFrame->linesize[0]);
    yuvFrame->luma = luma;

    width = MIN(videoFrame->linesize[1], videoCodecContext->width / 2);
    height = videoCodecContext->height / 2;
    int chromaBLength = width * height;
    uint8_t *chromaB = new uint8_t[chromaBLength];
    copyFrameData(chromaB, const_cast<uint8_t *>(videoFrame->data[1]), width, height, videoFrame->linesize[1]);
    yuvFrame->chromaB = chromaB;

    width = MIN(videoFrame->linesize[2], videoCodecContext->width / 2);
    height = videoCodecContext->height / 2;
    int chromaRLength = width * height;
    uint8_t *chromaR = new uint8_t[chromaRLength];
    copyFrameData(chromaR, const_cast<uint8_t *>(videoFrame->data[2]), width, height, videoFrame->linesize[2]);
    yuvFrame->chromaR = chromaR;


    yuvFrame->width = videoCodecContext->width;
    yuvFrame->height = videoCodecContext->height;
    yuvFrame->position = av_frame_get_best_effort_timestamp(videoFrame) * videoTimeBase;
    const int16_t frameDuration = av_frame_get_pkt_duration(videoFrame);
    if (frameDuration) {
        yuvFrame->duration = frameDuration * videoTimeBase;
        yuvFrame->duration += videoFrame->repeat_pict * videoTimeBase * 0.5;
    } else {
        yuvFrame->duration = 1.0 / fps;
    }

    return yuvFrame;


}

void
VideoDecoder::copyFrameData(uint8_t *dst, uint8_t *src, int width, int height, int linesize) {
    for (int i = 0; i < height; ++i) {
        memcpy(dst, src, width);
        dst += width;
        src += linesize;
    }
}

bool VideoDecoder::validVideo() {
    return videoStreamIndex != -1;
}

bool VideoDecoder::validAudio() {
    return audioStreamIndex != -1;
}

bool VideoDecoder::isEOF() {
    return isVideoOutputEOF;
}

bool VideoDecoder::isNetwork() {
    return false;
}

std::list<MovieFrame *> *VideoDecoder::decodeFrames(float minDuration, int *decodeVideoErrorState) {
    if (avFormatCtx == nullptr) {
        return nullptr;
    }
    if (audioStreamIndex == -1 && videoStreamIndex == -1) {
        return nullptr;
    }
    readLatestFrameTimeMillis = currentTimeMills();
    std::list<MovieFrame *> *result = new std::list<MovieFrame *>();
    AVPacket packet;
    float decodedDuration = 0.0f;
    bool finished = false;
    //todo seek

    int ret = 0;
    char errString[128];

    bool isEof = false;
    while (!finished) {

        ret = av_read_frame(avFormatCtx, &packet);
        if (ret < 0) {
            if (ret != AVERROR_EOF) {
                av_strerror(ret, errString, 128);
            } else {
                isEof = true;
            }
            av_free_packet(&packet);
            break;
        }

        if (packet.stream_index == videoStreamIndex) {
            decodeVideoFrame(packet, decodeVideoErrorState);
        } else if (packet.stream_index == audioStreamIndex) {
            finished = decodeAudioFrames(&packet, result, decodedDuration, minDuration,
                                         decodeVideoErrorState);
        }
    }

    if (isEof) {
        flushVideoFrames(packet,decodeVideoErrorState);
        flushAudioFrames(&packet,result,minDuration,decodeVideoErrorState);
    }

    return result;
}


void VideoDecoder::uploadTexture() {
    pthread_mutex_lock(&mLock);
    textureFrameUploader->signalFrameAvailable();
    pthread_cond_wait(&mCondition, &mLock);
    pthread_mutex_unlock(&mLock);
}

AudioFrame *VideoDecoder::handleAudioFrame() {

    if (!audioFrame->data[0]) {
        return nullptr;
    }
    int numChannels = audioCodecContext->channels;
    int numFrames = 0;
    void *audioData;
    if (swrContext) {
        const int ratio = 2;
        const int bufSize = av_samples_get_buffer_size(nullptr, numChannels,
                                                       ratio * audioFrame->nb_samples,
                                                       AV_SAMPLE_FMT_S16, 1);
        if (!swrBuffer || swrBufferSize < bufSize) {
            swrBufferSize = bufSize;
            /**
			 * 指针名=（数据类型*）realloc（要改变内存大小的指针名，新的大小）。
			 * 新的大小一定要大于原来的大小，不然的话会导致数据丢失！
			 * 不考虑数据内容，新的大小可大可小
			 * 头文件
			 * #include <stdlib.h> 有些编译器需要#include <malloc.h>，在TC2.0中可以使用alloc.h头文件
			 * 功能
			 * 先判断当前的指针是否有足够的连续空间，如果有，扩大mem_address指向的地址，并且将mem_address返回，如果空间不够，先按照newsize指定的大小分配空间，将原有数据从头到尾拷贝到新分配的内存区域，而后释放原来mem_address所指内存区域（注意：原来指针是自动释放，不需要使用free），同时返回新分配的内存区域的首地址。即重新分配存储器块的地址。
			 * 返回值
			 * 如果重新分配成功则返回指向被分配内存的指针，否则返回空指针NULL。
			 * 注意
			 * 这里原始内存中的数据还是保持不变的。当内存不再使用时，应使用free()函数将内存块释放。
			 */
            swrBuffer = realloc(swrBuffer, swrBufferSize);
        }
        byte *outbuf[2] = {(byte *) swrBuffer, nullptr};

        numFrames = swr_convert(swrContext, outbuf, audioFrame->nb_samples * ratio,
                                (audioFrame->data), audioFrame->nb_samples);
        if (numFrames < 0) {
            return nullptr;
        }
        audioData = swrBuffer;

    } else {
        if (audioCodecContext->sample_fmt != AV_SAMPLE_FMT_S16) {
            return nullptr;
        }
        audioData = (uint8_t *)audioFrame->data[0];
        numFrames = audioFrame->nb_samples;
    }

    int numElements = numFrames * numChannels;
    float position = av_frame_get_best_effort_timestamp(audioFrame) * audioTimeBase;
    byte *buffer = nullptr;
    int actualSize = -1;
    if (mUploaderCallback) {
        //short类型转byte类型
        actualSize=  mUploaderCallback->processAudioData((short *) (audioData), numElements, position, &buffer);
    }
    if (actualSize<0){
        return nullptr;
    }
    AudioFrame* frame=new AudioFrame();
    frame->position=position;
    frame->samples=buffer;
    frame->size=actualSize;
    frame->duration= av_frame_get_pkt_duration(audioFrame)*audioTimeBase;
    if (frame->duration==0){
        frame->duration= frame->size / (sizeof(float) * numChannels * 2 * audioCodecContext->sample_rate);
    }
    return frame;
}

void
VideoDecoder::flushAudioFrames(AVPacket *packet, std::list<MovieFrame *> *result, float minDuration,
                               int *decodeVideoErrorState) {

    if(audioCodecContext->codec->capabilities&CODEC_CAP_DELAY){
        float decodeDuration=0.0f;
        while(true){
            packet->data=0;
            packet->size=0;
            av_init_packet(packet);
            int gotFrame=0;
            int len = avcodec_decode_audio4(audioCodecContext,audioFrame,&gotFrame,packet);
            if(len<0){
                *decodeVideoErrorState=1;
                break;
            }
            if (gotFrame){
                AudioFrame* frame=handleAudioFrame();
                if (frame!= nullptr){
                    result->push_back(frame);
                    position=frame->position;
                    decodeDuration+=frame->duration;
                    if (decodeDuration>minDuration){
                        break;
                    }

                }
            }else{
                isAudioOutputEOF=true;
                break;
            }
        }
    }
}



