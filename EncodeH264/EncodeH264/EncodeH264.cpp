#include <cstdio>

extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
	int ret = 0, got_frame = 0;
	AVPacket enc_pkt;
	if(!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;
	while(1)
	{
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame)
		{
			ret = 0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",enc_pkt.size);
		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
}

int main(int argc, char **argv)
{
	AVFormatContext *pFormatCtx = NULL;
	AVOutputFormat  *fmt        = NULL;
	AVStream        *video_ts   = NULL;
	AVCodecContext  *pCodecCtx  = NULL;
	AVCodec         *pCodec     = NULL;
	AVFrame         *pFrame     = NULL;
	AVPacket        packet;
	uint8_t         *picture_buf = NULL;
	int             framenum1    = 0;       
	FILE            *fp_in = fopen("../bigbuckbunny_480x272.yuv", "rb");
	int              got_picture = 0;
	const char *outfile = "test.h264";
	int in_width = 480, in_height = 272;
	int picture_size = 0;
	int y_size = 0;
	int framenum = 100;
	av_register_all();

	//method 1
	pFormatCtx = avformat_alloc_context();
	fmt = av_guess_format(NULL, outfile,NULL);
	pFormatCtx->oformat = fmt;

	//method 2
	//avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, outfile);
	//pFormatCtx->oformat = fmt;

	if(avio_open(&pFormatCtx->pb, outfile, AVIO_FLAG_READ_WRITE)  < 0)
	{
		printf("Failed open outfile!\n");
		return -1;
	}

	video_ts = avformat_new_stream(pFormatCtx, 0);
	if(NULL == video_ts)
	{
		printf("Couldn't find the stream!\n");
		return -1;
	}

	//Param must set
	pCodecCtx = video_ts->codec;

	//
	pCodecCtx->codec_id = fmt->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecCtx->bit_rate = 400000;
	pCodecCtx->coded_height = in_height;
	pCodecCtx->coded_width  = in_width;
	pCodecCtx->gop_size = 250;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;

	pCodecCtx->qmin = 10;
	pCodecCtx->qmax = 51;

	pCodecCtx->max_b_frames = 3;

	AVDictionary *param = 0;
	//H264
	if(pCodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		av_dict_set(&param, "preset", "slow", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);
	}

	//H265
	if(pCodecCtx->codec_id == AV_CODEC_ID_H265)
	{
		av_dict_set(&param, "preset", "ultrafast", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);
	}

	//Show some information
	av_dump_format(pFormatCtx, 0, outfile, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if(!pCodec)
	{
		printf("encoder not found!\n");
		return -1;
	}

	if(avcodec_open2(pCodecCtx, pCodec, &param) < 0)
	{
		printf("Failed open encoder!\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	avformat_write_header(pFormatCtx, NULL);
	picture_buf = (uint8_t *)av_malloc(picture_size);
	avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);


	av_new_packet(&packet, picture_size);

	y_size = pCodecCtx->width * pCodecCtx->height;

	for(int i  = 0; i < framenum; ++i)
	{
		if(fread(picture_buf, 1, y_size * 3 / 2, fp_in) <= 0)
		{
			printf("Failed read raw file!\n");
			return -1;
		}
		else if(feof(fp_in))
			break;

		pFrame->data[0] = picture_buf;
		pFrame->data[1] = picture_buf + y_size;
		pFrame->data[2] = picture_buf + y_size * 5 / 4;

		pFrame->pts = i * (video_ts->time_base.den) / ((video_ts->time_base.num) * 25);

		int ret = avcodec_encode_video2(pCodecCtx, &packet, pFrame, &got_picture);
		if(ret < 0)
		{
			printf("");
			return -1;
		}

		if(got_picture == 1)
		{
			printf("Succeed to encode frame: %5d\tsize:%5d\n", framenum1, packet.size);
			framenum1++;
			packet.stream_index = video_ts->index;
			ret = av_write_frame(pFormatCtx, &packet);
			av_free_packet(&packet);
		}

	}

	//
	int ret = flush_encoder(pFormatCtx, 0);
	if(ret < 0)
	{
		printf("Flushing encoder failed\n");
		return -1;
	}

	av_write_trailer(pFormatCtx);

	if (video_ts)
	{
		avcodec_close(video_ts->codec);
		av_free(pFrame);
		av_free(picture_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(fp_in);

	getchar();

	return 0;
}