//
// Created by wind on 2023/4/4.
//

#ifndef NDK_VIDEOPLAYER_DECODER_PARAMS_H
#define NDK_VIDEOPLAYER_DECODER_PARAMS_H
#include <cstring>

class DecoderParams {

public:
    DecoderParams(char *path);
    ~DecoderParams();

    char* getUri(){
        return uri;
    }

private:
    char *uri;


};


#endif //NDK_VIDEOPLAYER_DECODER_PARAMS_H
