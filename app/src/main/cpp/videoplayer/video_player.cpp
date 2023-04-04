//
// Created by wind on 2023/4/4.
//

#include <cstring>
#include "video_player.h"

void VideoPlayer::setDataSource(char *dataSource) {

    path=new char[strlen(dataSource)+1];
    strcpy(path,dataSource);

}


void VideoPlayer::prepare() {
    DecoderParams* params=new DecoderParams(path);
    synchronizer=new VideoSynchronizer();
    synchronizer->init(params);
}

void VideoPlayer::play() {

}
