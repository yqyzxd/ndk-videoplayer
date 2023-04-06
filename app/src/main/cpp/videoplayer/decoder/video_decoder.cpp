//
// Created by wind on 2023/4/4.
//

#include "video_decoder.h"
#include "libavformat/avformat.h"

VideoDecoder::VideoDecoder(DecoderParams *decoderParams) {
    this->decoderParams=decoderParams;
}

int VideoDecoder::openVideo() {

    AVFormatContext* avFormatCtx=avformat_alloc_context();
    AVDictionary* dictionary= nullptr;
    av_dict_set(&dictionary,"timeout","5000000",0);
    int ret=avformat_open_input(&avFormatCtx,decoderParams->getUri(),0,&dictionary);
    av_dict_free(&dictionary);
    if (ret){
        //todo 打开出错了
        avformat_close_input(&avFormatCtx);
        return ret;
    }
    ret=avformat_find_stream_info(avFormatCtx, nullptr);
    if (ret){
        //todo 打开出错了
        avformat_close_input(&avFormatCtx);
        return ret;
    }


    int videoErr=openVideoStream();
    int audioErr=openAudioStream();


    if (videoErr<0 && audioErr<0){
        return -1;
    }

    return 0;



}

int VideoDecoder::openVideoStream() {
    int errCode=-1;
    videoStreamIndex=-1;
    std::list<int>* videoStreams=collectStreams(AVMEDIA_TYPE_VIDEO);
    std::list<int>::iterator iter;
    for (iter = videoStreams->begin(); iter !=videoStreams->end() ; ++iter) {
        int streamIndex=*iter;
        if ((avFormatCtx->streams[streamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC)==0){
            errCode=openVideoStream(streamIndex);
            if (errCode>=0){
                break;
            }
        }
    }

    return errCode;
}
int VideoDecoder::openAudioStream() {
    int errCode=-1;
    audioStreamIndex=-1;
    std::list<int>* audioStreams= collectStreams(AVMEDIA_TYPE_AUDIO);
    std::list<int>::iterator iter;
    for (iter = audioStreams->begin();  iter!=audioStreams->end() ; iter++) {
        int streamIndex=*iter;
        errCode=openAudioStream(streamIndex);
        if (errCode>=0){
            break;
        }
    }
    return 0;
}
int VideoDecoder::openVideoStream(int streamIndex) {
    AVStream* videoStream =avFormatCtx->streams[streamIndex];
    //获取 编码解码的【参数】
    AVCodecParameters *codecpar=videoStream->codecpar;
    AVCodec* codec=avcodec_find_decoder(codecpar->codec_id);
    if (!codec){
       // avformat_close_input(&avFormatCtx);
        return -1;
    }
    AVCodecContext* codecContext =avcodec_alloc_context3(codec);
    if (!codecContext){
       // avformat_close_input(&avFormatCtx);
        return -1;
    }
    int ret=avcodec_parameters_to_context(codecContext,codecpar);
    if (ret<0){
        avcodec_free_context(&codecContext);
      //  avformat_close_input(&avFormatCtx);
        return -1;
    }
    ret=avcodec_open2(codecContext,codec, nullptr);
    if (ret){
        avcodec_free_context(&codecContext);
       // avformat_close_input(&avFormatCtx);
    }
    videoCodecContext=codecContext;
    // 解码后的视频原始数据包
    videoFrame =av_frame_alloc();
    this->videoStreamIndex=streamIndex;
    if (videoFrame== nullptr){
        avcodec_free_context(&codecContext);
        return -1;
    }
    determineFpsAndTimeBase(videoStream,0.04,&fps,&videoTimeBase);

    if (fps>30.0f || fps<5.0f){
        fps=24.0f;
    }
    if (videoCodecContext->pix_fmt!=AV_PIX_FMT_YUV420P && videoCodecContext->pix_fmt!=AV_PIX_FMT_YUVJ420P){
        avcodec_free_context(&codecContext);
        return -1;
    }
    width=codecContext->width;
    height=codecContext->height;
    return 0;
}

int VideoDecoder::openAudioStream(int streamIndex) {
       AVStream * audioStream=avFormatCtx->streams[streamIndex];
    AVCodecParameters *codecpar=audioStream->codecpar;
    AVCodec* codec=avcodec_find_decoder(codecpar->codec_id);
    if (!codec){
        avformat_close_input(&avFormatCtx);
        return -1;
    }
    AVCodecContext* codecContext =avcodec_alloc_context3(codec);
    if (!codecContext){
        return -1;
    }
    int ret=avcodec_parameters_to_context(codecContext,codecpar);
    if (ret<0){
        avcodec_free_context(&codecContext);
        return -1;
    }
   ret= avcodec_open2(codecContext,codec, nullptr);
    if (ret<0){
        avcodec_free_context(&codecContext);
        return -1;
    }
    audioCodecContext=codecContext;

    if (!audioCodecIsSupported(audioCodecContext)){
        swrContext=swr_alloc_set_opts(nullptr, av_get_default_channel_layout(audioCodecContext->channels),AV_SAMPLE_FMT_S16,audioCodecContext->sample_rate,
                           av_get_default_channel_layout(audioCodecContext->channels),audioCodecContext->sample_fmt,audioCodecContext->sample_rate,0,
                           nullptr);
        if (!swrContext || swr_init(swrContext)){
            if (swrContext){
                swr_free(&swrContext);
            }
            avcodec_free_context(&audioCodecContext);
            return -1;
        }
    }

    audioFrame=av_frame_alloc();
    if (audioFrame== nullptr){
        if (swrContext){
            swr_free(&swrContext);
        }
        avcodec_free_context(&audioCodecContext);
        return -1;
    }
    audioStreamIndex=streamIndex;
    determineFpsAndTimeBase(audioStream,0.025,0,&audioTimeBase);
    return 0;

}

std::list<int>* VideoDecoder::collectStreams(AVMediaType type) {
    std::list<int> *typeStreams=new std::list<int>();
    for (int i=0;i<avFormatCtx->nb_streams;i++){
        if (type==avFormatCtx->streams[i]->codecpar->codec_type){
            typeStreams->push_back(i);
        }
    }
    return typeStreams;
}

void VideoDecoder::determineFpsAndTimeBase(AVStream *stream, double defaultTimeBase, float *pFps,
                                           float *pTimebase) {
    //时间基：所谓时间基表示的就是每个刻度是多少秒。如果你是把1秒分成90000份，每一个刻度就是1/90000秒，此时的time_base={1，90000}。
    float  fps,timebase;
    if (stream->time_base.den && stream->time_base.num){
        timebase= av_q2d(stream->time_base);
    } else if (stream->codec->time_base.den &&stream->codec->time_base.num){
        timebase= av_q2d(stream->codec->time_base);
    } else{
        timebase=defaultTimeBase;
    }

    if (stream->avg_frame_rate.den && stream->avg_frame_rate.num){
        fps= av_q2d(stream->avg_frame_rate);
    }else if (stream->r_frame_rate.den && stream->r_frame_rate.num){
        fps= av_q2d(stream->r_frame_rate);
    }else {
        fps=1.0/timebase;
    }
    if (pFps){
        *pFps=fps;
    }
    if (pTimebase){
        *pTimebase=timebase;
    }

}

bool VideoDecoder::audioCodecIsSupported(AVCodecContext *audioCodecCtx) {
    if (audioCodecCtx->sample_fmt==AV_SAMPLE_FMT_S16){
        return true;
    }
    return false;
}

void VideoDecoder::close() {

}


