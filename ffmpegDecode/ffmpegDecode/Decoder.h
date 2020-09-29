#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

extern "C"
{
#include "libavcodec/avcodec.h"
}

using namespace std;

class Decodec
{
public:
	Decodec();
	int init(char *fileNameIn, AVCodecID codec_id, char *fileNameOut);
	void process();
	void destroy();

private:
	AVCodec              *pCodec;
	AVCodecContext       *pCodecCtx;
	AVCodecParserContext *pCodecParserCtx;
	FILE                 *fp_in;
	FILE                 *fp_out;
	AVFrame              *pFrame;
	AVPacket             packet;
	int                  first_time;
};