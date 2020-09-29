#include <iostream>

extern "C"
{
#include "SDL.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

using namespace std;

class Player
{
public:
	Player();
	int init(char *filename);
	void codec();
	void destory();

private:
	//ffmpeg 
	AVPacket *packet;
	int videoIndex;
	AVCodecContext *pCodecCtx;
	AVFormatContext *pFormatCtx;
	AVCodec *pCodec;
	AVFrame *pFrame, *pFrameYUV;
	unsigned char *outBuffer;
	struct SwsContext *img_convert_ctx;
	int ret, got_picture;

	//SDL2.0
	SDL_Window *screen;
	SDL_Renderer *sdlRenderer;
	SDL_Texture *sdlTexture;
	SDL_Rect sdlRect;
	int screen_w, screen_h;

};