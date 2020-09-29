#include "player.h"

Player::Player() : videoIndex(-1),
    screen_w(0),
	screen_h(0)
{

}

int thread_exit = 0;
int sfp_refresh_even(void *opaque)
{
	SDL_Event event;
	while (thread_exit == 0)
	{
		event.type = SFM_REFRESH_EVEN;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	return 0;
}

int Player::init(char *fileName)
{
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, fileName, NULL, NULL) != 0)
	{
		cout << "Can't open input stream!" << endl;
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		cout << "Can't find stream info!" << endl;
		return -1;
	}
	for (i = 0; i < pFormatCtx->nb_streams; ++i)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoIndex = i;
			break;
		}
		if (-1 == videoIndex)
		{
			cout << "Can't find video stream!" << endl;
		}
	}
	pCodecCtx = pFormatCtx->streams[i]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (NULL == pCodec)
	{
		cout << "Can't find decoder!" << endl;
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		cout << "Can't open codec!" << endl;
		return -1;
	}

	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();


	if (SDL_Init(SDL_INIT_AUDIO || SDL_INIT_VIDEO || SDL_INIT_TIMER))
	{
		cout << "Can't SDL initialize SDL -" << SDL_GetError() << endl;
		return -1;
	}

	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	screen = SDL_SetVideoMode(screen_w, screen_h, 0, 0);
	if (!screen)
	{
		cout << "SDL: Can't set video mode!" << endl;
		return -1;
	}
	return 0;
}

int Player::codec()
{
	bmp = SDL_CreateYUVOverlay(screen_w, screen_h, SDL_YV12_OVERLAY, screen);

	rect.x = 0;
	rect.y = 0;
	rect.w = screen_w;
	rect.h = screen_h;

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	cout << "----------------File Infomation-------------------------" << endl;
	av_dump_format(pFormatCtx, 0, fileName, 0);
	cout << "--------------------------------------------------------" << endl;

	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL);
	video_tid = SDL_CreateThread(sfp_refresh_even, NULL);

	SDL_WM_SetCaption("simplest ffmpeg player", NULL);

	for (;;)
	{
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVEN)
		{
			if (av_read_frame(pFormatCtx, packet) >= 0)
			{
				if (packet->stream_index == videoIndex)
				{
					//Decoder
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if (ret < 0)
					{
						cout << "Decoder error!" << endl;
						return -1;
					}
					if (got_picture)
					{
						SDL_LockYUVOverlay(bmp);
						pFrameYUV->data[0] = bmp->pixels[0];
						pFrameYUV->data[1] = bmp->pixels[2];
						pFrameYUV->data[2] = bmp->pixels[1];
						pFrameYUV->linesize[0] = bmp->pitches[0];
						pFrameYUV->linesize[1] = bmp->pitches[2];
						pFrameYUV->linesize[2] = bmp->pitches[1];
						sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data,
							pFrame->linesize, 0, pCodecCtx->height,
							pFrameYUV->data, pFrameYUV->linesize);
						SDL_UnlockYUVOverlay(bmp);

						SDL_DisplayYUVOverlay(bmp, &rect);
					}
				}
				av_free_packet(packet);
			}
			else
			{
				thread_exit = 1;
				break;
			}
		}
	}
	return 0;
}

void Player::destory()
{
	SDL_Quit();
	av_free(pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
}