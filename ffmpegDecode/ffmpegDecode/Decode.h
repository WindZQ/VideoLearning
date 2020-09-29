#ifndef _DECODE_H_
#define _DECODE_H_
#include <cstdio>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

class Decode
{
public:
	Decode(void);
	~Decode(void);

	int decode_init(char *filepath);
	void decode_process();
	void decode_uninit();

private:
	int                videoindex;
	AVFormatContext    *pFormatCtx;
	AVCodecContext     *pCodecCtx;
	AVCodec            *pCodec;
	AVPacket           *packet;
	AVFrame            *pFrame;
	AVFrame            *pFrameYUV;
	int                got_picture;
	int                ret;
	unsigned char      * out_buffer;
	struct SwsContext  *img_convert_ctx;    
	char *             m_filepath;
};
#endif
