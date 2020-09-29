#include <iostream>
#include "Decoder.h"
#define TEST_H264 0
#define TEST_HEVC 1
using namespace std;

int main(int argc, char *argv[])
{
#if TEST_HEVC
	enum AVCodecID codec_id = AV_CODEC_ID_HEVC;
	char *filepath_in = "bigbuckbunny_480x272.hevc";
#elif TEST_H264
	AVCodecID codec_id = AV_CODEC_ID_H264;
	char *filepath_in = "bigbuckbunny_480x272.h264";
#else
	AVCodecID codec_id = AV_CODEC_ID_MPEG2VIDEO;
	char *filepath_in = "bigbuckbunny_480x272.m2v";
#endif
	char *filepath_out = "bigbuckbunny_480x272.yuv";
	Decodec decodec;
	decodec.init(filepath_in, codec_id, filepath_out);
	decodec.process();
	decodec.destroy();
	return 0;
}