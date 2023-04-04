//
// Created by wind on 2023/4/4.
//


#include "decoder_params.h"
DecoderParams::DecoderParams(char *path) {
    uri=new char[strlen(path)+1];
    strcpy(uri,path);

}