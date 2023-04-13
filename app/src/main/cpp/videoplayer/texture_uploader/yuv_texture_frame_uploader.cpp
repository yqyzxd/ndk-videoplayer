//
// Created by 史浩 on 2023/4/8.
//

#include "yuv_texture_frame_uploader.h"



YUVTextureFrameUploader::YUVTextureFrameUploader() {

}

YUVTextureFrameUploader::~YUVTextureFrameUploader() {

}

void YUVTextureFrameUploader::initialize() {
    TextureFrameUploader::initialize();

    textureFrame=new YUVTextureFrame();
    textureFrame->createTexture();
    textureFrameCopier=new YUVTextureFrameCopier();
    textureFrameCopier->init();
}