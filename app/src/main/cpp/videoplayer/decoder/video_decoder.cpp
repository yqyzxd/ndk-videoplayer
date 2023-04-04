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





}

int VideoDecoder::openVideoStream() {
    int errCode=-1;
    int  videoStreamIndex=-1;
    std::list<int>* videoStreams=collectStreams(AVMEDIA_TYPE_VIDEO);
    std::list<int>::iterator iter;
    for (iter = videoStreams->begin(); iter !=videoStreams->end() ; ++iter) {
        int streamIndex=*iter;
        if ((avFormatCtx->streams[streamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC)==0){
            errCode=openVideoStream(streamIndex);
        }

    }

    return errCode;
}

int VideoDecoder::openVideoStream(int streamIndex) {
    AVStream* videoStream =avFormatCtx->streams[streamIndex];
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