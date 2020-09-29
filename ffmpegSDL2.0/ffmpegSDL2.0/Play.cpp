#include "Play.h"

Player::Player() : videoIndex(-1),
	screen_w(0),
	screen_h(0)
{

}

int Player::init(char *fileName)
{
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, fileName, NULL, NULL) != 0)
		cout << "Can't open input stream!" << endl;
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		cout << "Can't find stream information! " << endl;

	for (int i = 0; i < pFormatCtx->nb_streams; ++i)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoIndex = i;
			break;
		}
		if (videoIndex == -1)
		{
			cout << "Can't find video stream!" << endl;
		}
	}

	pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		cout << "Codec not find!" << endl;
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		cout << "Can't open the Codec!" << endl;
		return -1;
	}

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO))
	{
		cout << "Can't initalize SDL: -" << SDL_GetError();
		return -1;
	}

	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;

	screen = SDL_CreateWindow("the simplest ffmpeg player2.0", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_OPENGL);
	if (!screen)
	{
		cout << "Can't create window -" << SDL_GetError() << endl;
		return -1;
	}

	cout << "-------------------File Information---------------------" << endl;
	av_dump_format(pFormatCtx, 0, fileName, 0);
	cout << "--------------------------------------------------------" << endl;

	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	return 0;
}

void Player::codec()
{
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	outBuffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, outBuffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == videoIndex)
		{
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0)
				cout << "Decodec Error" << endl;

			if (got_picture)
			{
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

				SDL_UpdateYUVTexture(sdlTexture, &sdlRect, pFrameYUV->data[0],
					pFrameYUV->linesize[0], pFrameYUV->data[1], pFrameYUV->linesize[1],
					pFrameYUV->data[2], pFrameYUV->linesize[2]);
			}

			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);

			SDL_Delay(40);
		}
	}
	av_free_packet(packet);


	//flush decoder
	//FIX:Flush Frames remained in Codec 
	while (1)
	{
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		if (ret < 0)
			break;
		if (!got_picture)
			break;

		sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize,
			0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

		SDL_UpdateTexture(sdlTexture, &sdlRect, pFrameYUV->data[0], pFrameYUV->linesize[0]);
		SDL_RenderClear(sdlRenderer);
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
		SDL_RenderPresent(sdlRenderer);

		SDL_Delay(40);
	}
	sws_freeContext(img_convert_ctx);
}

void Player::destory()
{
	SDL_Quit();
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
}