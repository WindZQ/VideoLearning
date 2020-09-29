#include <cstdio>
//#define __STDC_CONSTANT_MACROS
extern "C"
{
#include "libavcodec/avcodec.h"
};

#define TEST_H264 0
#define TEST_HEVC 1


int main(int argc, char **argv)
{
	FILE *fp_in = NULL, *fp_out = NULL;
	AVCodec *pCodec = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodecParserContext *pCodecParserCtx = NULL;
	AVFrame *pFrame = NULL;
	AVPacket packet;
	const int in_buffer_size = 4096;
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = {0};
	uint8_t *cur_ptr = NULL;
	int ret, got_picture;
	int cur_size;
	int first_time = 1;
#if TEST_H264
	enum AVCodecID codec_id =  AV_CODEC_ID_H264;
	char *filepath_in = "bigbuckbunny_480x272.h264";
#elif TEST_HEVC
	AVCodecID codec_id = AV_CODEC_ID_HEVC;
	char *filepath_in = "bigbuckbunny_480x272.hevc";
#elif
	AVCodecID codec_id = AV_CODEC_ID_MPEG2VIDEO;
	char *filepath_in = "bbigbuckbunny_480x272.m2v";
#endif
	char *filepath_out = "bigbuckbunny_480x272.yuv";

	avcodec_register_all();

	pCodec = avcodec_find_decoder(codec_id);
	if(!pCodec)
	{
		printf("codec not find!\n");
		return -1;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if(!pCodecCtx)
	{
		printf("Could not allocate video codec context\n");
		return -1;
	}

	pCodecParserCtx = av_parser_init(codec_id);
	if(!pCodecParserCtx)
	{
		printf("Could not allocate video parser context\n");
		return -1;
	}

	if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("Could not open codec\n");
		return -1;
	}

	fp_in = fopen(filepath_in, "rb");
	if(!fp_in)
	{
		printf("Could not open input stream!\n");
		return -1;
	}

	fp_out = fopen(filepath_out, "wb");
	if(!fp_out)
	{
		printf("Could not write YUV file!\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	av_init_packet(&packet);

	while(1)
	{
		cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
		if(cur_size == 0)
			break;
		cur_ptr = in_buffer;
		while(cur_size > 0)
		{
			int len = av_parser_parse2(pCodecParserCtx, pCodecCtx, &packet.data,
				      &packet.size, cur_ptr, cur_size, AV_NOPTS_VALUE, 
					  AV_NOPTS_VALUE, AV_NOPTS_VALUE);
			cur_ptr += len;
			cur_size -= len;
			if(packet.size == 0)
				continue;
			printf("[Packet]size: %6d\t", packet.size);
			switch(pCodecParserCtx->pict_type)
			{
			case AV_PICTURE_TYPE_P: printf("Type:P\t");break;
			case AV_PICTURE_TYPE_B: printf("Type:B\t");break;
			case AV_PICTURE_TYPE_I: printf("Type:I\t");break;
			default: printf("Type:Other\t");break;
			}
			printf("Number: %6d\n", pCodecParserCtx->output_picture_number);

			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
			if(ret < 0)
			{
				printf("Decodec failed!\n");
				return -1;
			}
			if(got_picture)
			{
				if(first_time)
				{
					printf("\nCodec full Name: %s\n", pCodecCtx->codec->long_name);
					printf("Width:%d\nHeight:%d\n\n", pCodecCtx->width, pCodecCtx->height);
					first_time = 0;
				}

				for(int i = 0; i < pFrame->height; ++i)
					fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
				for(int i = 0; i < pFrame->height / 2; ++i)
					fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
				for(int i = 0; i < pFrame->height / 2; ++i)
					fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
				printf("Successed to Decode 1 frame!\n");
			}
		}
	}
	packet.data = NULL;
	packet.size = 0;
	while(1)
	{
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
		if(ret < 0)
		{
			printf("Decodec failed!\n");
			return ret;
		}
		if(!got_picture)
			break;
		else
		{
			for(int i = 0; i < pFrame->height; ++i)
				fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
			for(int i = 0; i < pFrame->height / 2; ++i)
				fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
			for(int i = 0; i < pFrame->height / 2; ++i)
				fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
			printf("Successed to Decode 1 frame!\n");
		}
	}
	fclose(fp_in);
	fclose(fp_out);
	av_parser_close(pCodecParserCtx);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	getchar();
	return 0;
}