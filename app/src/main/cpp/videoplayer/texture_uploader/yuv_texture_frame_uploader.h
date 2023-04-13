//
// Created by 史浩 on 2023/4/8.
//

#ifndef NDK_VIDEOPLAYER_YUV_TEXTURE_FRAME_UPLOADER_H
#define NDK_VIDEOPLAYER_YUV_TEXTURE_FRAME_UPLOADER_H


#include "texture_frame_uploader.h"

class YUVTextureFrameUploader : public TextureFrameUploader{

public:
    YUVTextureFrameUploader();
    virtual ~YUVTextureFrameUploader();


    virtual void initialize();
};


#endif //NDK_VIDEOPLAYER_YUV_TEXTURE_FRAME_UPLOADER_H
