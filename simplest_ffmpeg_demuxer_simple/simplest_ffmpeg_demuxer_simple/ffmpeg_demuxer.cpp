#include <cstdio>

extern "C"
{
#include "libavformat/avformat.h"
};

#define USE_H264BFS 1

int main(int argc, char **argv)
{
	AVFormatContext  *imft_ctx       = NULL;
	const char       *filename_in    = "../cuc_ieschool.mp4";
	const char       *filename_out_a = "../cuc_ieschool.mp3";
	const char       *filenmae_out_v = "../cuc_ieschool.h264";
	int              ret;
	int              videoindex      = -1;
	int              audioindex      = -1;
	AVPacket         pkt;
	av_register_all();

	if((ret = avformat_open_input(&imft_ctx, filename_in, 0, 0)) < 0)
	{
		printf("Coudn't open input file!\n");
		return -1;
	}

	if((ret = avformat_find_stream_info(imft_ctx, 0)) < 0)
	{
		printf("Failed to retrieve input stream information\n");
		return -1;
	}

	for(int i = 0; i < imft_ctx->nb_streams; ++i)
	{
		if(imft_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			videoindex = i;
		else if(imft_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			audioindex = i;
	}

	//Dump Format
	printf("\n==============Input Video=====================\n");
	av_dump_format(imft_ctx, 0, filename_in, 0);
	printf("\n==============================================\n");

	FILE *fp_audio = fopen(filename_out_a, "wb+");
	FILE *fp_video = fopen(filenmae_out_v, "wb+");

	/*
	 *FIX: H.264 in some container format (FLV, MP4, MKV etc.) need 
	 *"h264_mp4toannexb" bitstream filter (BSF)
	 *Add SPS,PPS in front of IDR frame
	 *Add start code ("0,0,0,1") in front of NALU
	 *H.264 in some container (MPEG2TS) don't need this BSF.
	 */
#if USE_H264BFS
	AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif

	while(av_read_frame(imft_ctx, &pkt) >= 0)
	{
		if(pkt.stream_index == videoindex)
		{
#if USE_H264BFS
			av_bitstream_filter_filter(h264bsfc, imft_ctx->streams[videoindex]->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
			printf("Write Video Packet. size: %d\t pts:%lld\n", pkt.size, pkt.pts);
			fwrite(pkt.data, 1, pkt.size, fp_video);
		}
		else if(pkt.stream_index == audioindex)
		{
		/*
		 * AAC in some container format (FLV, MP4, MKV etc.) need to add 7 Bytes
		 * ADTS Header in front of AVPacket data manually.
		 *  Other Audio Codec (MP3...) works well.
		 */
			printf("Write Audio Packet. size: %d\t pts:%lld\n", pkt.size, pkt.pts);
			fwrite(pkt.data, 1, pkt.size, fp_audio);
		}
		av_free_packet(&pkt);
	}
#if USE_H264BFS
	av_bitstream_filter_close(h264bsfc);
#endif

	fclose(fp_video);
	fclose(fp_audio);
	avformat_close_input(&imft_ctx);

	if(ret < 0 && ret != AVERROR_EOF)
	{
		printf("Error Occurred!\n");
		return -1;
	}
	getchar();
	return 0;
}