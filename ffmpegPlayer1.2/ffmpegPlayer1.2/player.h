#include <iostream>

#define SFM_REFRESH_EVEN (SDL_USEREVENT + 1)
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "SDL.h"
}

using namespace std;

class Player
{
public:
	Player();
	int init(char *fileName);
	int codec();
	void destory();
private:
	//ffmpeg 
	unsigned int i;
	int videoIndex;
	AVFormatContext *pFormatCtx;
	char *fileName;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame;
	AVFrame *pFrameYUV;
	AVPacket *packet;
	struct SwsContext *img_convert_ctx;

	//SDL
	int ret, got_picture;
	int screen_w, screen_h;
	SDL_Surface *screen;
	SDL_Overlay *bmp;
	SDL_Rect rect;
	SDL_Thread *video_tid;
	SDL_Event event;
};