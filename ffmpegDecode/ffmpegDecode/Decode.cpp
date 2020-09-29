#include "Decode.h"
FILE *fp_yuv = NULL;
Decode::Decode(void) 
	: pCodecCtx(NULL)
	, pFormatCtx(NULL)
	, packet(NULL)
	, pCodec(NULL)
	, pFrame(NULL)
	, pFrameYUV(NULL)
	, videoindex(-1)
	, ret(0)
	, img_convert_ctx(NULL)
	, got_picture(0)
	, out_buffer(NULL)
{
	fp_yuv = fopen("outfile.yuv", "wb+");
}


Decode::~Decode(void)
{

}


int Decode::decode_init(char *filepath)
{
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx, filepath, NULL, NULL) < 0)
	{
		printf("Couldn't open input stream!\n");
		return -1;
	}

	if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("Couldn't find stream infomation!\n");
		return -1;
	}

	for(int i = 0; i < pFormatCtx->nb_streams; ++i)
	{
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}

	if(videoindex == -1)
	{
		printf("Didn't find video stream!\n");
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(NULL == pCodec)
	{
		printf("Codec not find!\n");
		return -1;
	}

	if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("Couldn't open codec!\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	m_filepath = filepath;
}

void Decode::decode_process()
{
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
		pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P,
		pCodecCtx->width, pCodecCtx->height, 1);

	printf("--------------------File Information--------------------\n");
	av_dump_format(pFormatCtx, 0, m_filepath, 0);
	printf("--------------------------------------------------------\n");


	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	while(av_read_frame(pFormatCtx, packet) >= 0)
	{
		if(packet->stream_index == videoindex)
		{
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0)
			{
				printf("decode error!\n");
				return ;
			}

			if(got_picture)
			{
				sws_scale(img_convert_ctx, (const uint8_t *const *)pFrame->data, pFrame->linesize, 0, 
					pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

				int y_size = pCodecCtx->width * pCodecCtx->height;
				fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);
				fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);
				fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);
			}
			printf("Succeed Decode 1 Frame!\n");
		}
		av_free_packet(packet);
	}

	while(1)
	{
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		if(ret < 0)
		{
			printf("Decode error!\n");
			return ;
		}

		if(!got_picture)
			break;
		sws_scale(img_convert_ctx, (const uint8_t * const *)pFrame->data, pFrame->linesize, 0,
			pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
		int y_size = pCodecCtx->width * pCodecCtx->height;

		fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);
		fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);
		fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);
		printf("Flush Decode: succeed Decode 1 Frame!\n");
		av_free_packet(packet);
	}
}

void Decode::decode_uninit()
{
	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrame);
	av_frame_free(&pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
}