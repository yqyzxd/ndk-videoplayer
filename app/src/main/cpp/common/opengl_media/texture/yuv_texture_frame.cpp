#include "yuv_texture_frame.h"

#define LOG_TAG "YUVTextureFrame"

YUVTextureFrame::YUVTextureFrame() {
//	writeFlag = true;
}

YUVTextureFrame::~YUVTextureFrame() {

}

bool YUVTextureFrame::createTexture() {
	LOGI("enter YUVTextureFrame::createTexture");
	textures[0] = 0;
	textures[1] = 0;
	textures[2] = 0;
	int ret = initTexture();
	if (ret < 0) {
		LOGI("init texture failed...");
		this->dealloc();
		return false;
	}
	return true;
}

void YUVTextureFrame::setVideoFrame(VideoFrame *yuvFrame){
	this->frame = yuvFrame;
//	if (writeFlag) {
//		LOGI("after glReadPixels... ");
//		FILE* textureFrameFile = fopen("/mnt/sdcard/a_songstudio/texture.yuv", "wb+");
//		if (NULL != textureFrameFile) {
//			int width = yuvFrame->width;
//			int height = yuvFrame->height;
//			fwrite(yuvFrame->luma, sizeof(byte), width*height, textureFrameFile);
//			fwrite(yuvFrame->chromaB, sizeof(byte), width*height / 4, textureFrameFile);
//			fwrite(yuvFrame->chromaR, sizeof(byte), width*height / 4, textureFrameFile);
//			fclose(textureFrameFile);
//			LOGI("write textureFrameFile success ... ");
//		}
//		writeFlag = false;
//	}
}

void YUVTextureFrame::updateTexImage() {
//	LOGI("YUVTextureFrame::updateTexImage");
	if (frame) {
//		LOGI("start upload texture");
		int frameWidth = frame->width;
		int frameHeight = frame->height;
		if(frameWidth % 16 != 0){
		//控制的是所读取数据的对齐方式，默认4字节对齐，即一行的图像数据字节数必须是4的整数倍，即读取数据时，
		// 读取4个字节用来渲染一行，之后读取4字节数据用来渲染第二行。对RGB 3字节像素而言，若一行10个像素，即30个字节，
		// 在4字节对齐模式下，OpenGL会读取32个字节的数据，若不加注意，会导致glTextImage中致函数的读取越界，从而全面崩溃。
			//参数1表示按1字节进行对齐
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
		uint8_t *pixels[3] = { frame->luma, frame->chromaB, frame->chromaR };
		int widths[3] = { frameWidth, frameWidth >> 1, frameWidth >> 1 };
		int heights[3] = { frameHeight, frameHeight >> 1, frameHeight >> 1 };
		for (int i = 0; i < 3; ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			if (checkGlError("glBindTexture")) {
				return;
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widths[i], heights[i], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels[i]);
		}
	}
}

bool YUVTextureFrame::bindTexture(GLint* uniformSamplers) {
	for (int i = 0; i < 3; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		if (checkGlError("glBindTexture")) {
			return false;
		}
		glUniform1i(uniformSamplers[i], i);
	}
	return true;
}

void YUVTextureFrame::dealloc() {
	LOGI("enter YUVTextureFrame::dealloc");
	if (textures[0]) {
		glDeleteTextures(3, textures);
	}
}

int YUVTextureFrame::initTexture() {
	glGenTextures(3, textures);
	for (int i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		if (checkGlError("glBindTexture")) {
			return -1;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (checkGlError("glTexParameteri")) {
			return -1;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if (checkGlError("glTexParameteri")) {
			return -1;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		if (checkGlError("glTexParameteri")) {
			return -1;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (checkGlError("glTexParameteri")) {
			return -1;
		}
	}
	return 1;
}
