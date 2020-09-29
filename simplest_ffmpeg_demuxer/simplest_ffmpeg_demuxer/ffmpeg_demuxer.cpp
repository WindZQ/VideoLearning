#include <cstdio>
#define USE_H264BSF 1

extern "C"
{
#include "libavformat/avformat.h"
};

int main(int argc, char **argv)
{
	AVFormatContext  *ifmt_ctx   = NULL;
	AVFormatContext  *ofmt_ctx_a = NULL;
	AVFormatContext  *ofmt_ctx_v = NULL;
	AVOutputFormat   *ofmt_a     = NULL;
	AVOutputFormat   *ofmt_v     = NULL;
	int              audioindex  = -1;
	int              videoindex  = -1;
	AVPacket         pkt;
	int              frame_index = 0;
	const char *filename_in      = "../cuc_ieschool.mp4";
	const char *filename_out_v   = "../cuc_ieschool.h264";
	const char *filename_out_a   = "../cuc_ieschool.aac";
	int        ret               = 0;

	av_register_all();
	if((ret = avformat_open_input(&ifmt_ctx, filename_in, NULL, 0)) < 0)
	{
		printf("Could not open file!\n");
		goto end;
	}

	if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
	{
		printf("Failed to retrieve input stream information!\n");
		goto end;
	}

	avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, filename_out_v);
	if(!ofmt_ctx_v)
	{
		printf("Could not create output context!\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_v = ofmt_ctx_v->oformat;

	avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, filename_out_a);
	if(!ofmt_ctx_a)
	{
		printf("Could not create output context!\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_a = ofmt_ctx_a->oformat;

	for(int i = 0; i < ifmt_ctx->nb_streams; ++i)
	{
		//Create output AVStream according to input AVStream
		AVFormatContext   *ofmt_ctx   = NULL;
		AVStream          *stream_in  = ifmt_ctx->streams[i];
		AVStream          *stream_out = NULL;

		if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			stream_out = avformat_new_stream(ofmt_ctx_v, stream_in->codec->codec);
			ofmt_ctx = ofmt_ctx_v;
		}
		else if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioindex = i;
			stream_out = avformat_new_stream(ofmt_ctx_a, stream_in->codec->codec);
			ofmt_ctx = ofmt_ctx_a;
		}
		else 
			break;

		if(!stream_out)
		{
			printf("Failed allocating output stream!\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		//Copy the settings of AVCodecContext
		if(avcodec_copy_context(stream_out->codec, stream_in->codec) < 0)
		{
			printf("Failed to copy context from input to output stream codec context!\n");
			goto end;
		}
		stream_out->codec->codec_tag = 0;
		if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			stream_out->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	//Dump Format------------------
	printf("\n==============Input Video=============\n");
	av_dump_format(ifmt_ctx, 0, filename_in, 0);
	printf("\n==============Output Video============\n");
	av_dump_format(ofmt_ctx_v, 0, filename_out_v, 1);
	printf("\n==============Output Audio============\n");
	av_dump_format(ofmt_ctx_a, 0, filename_out_a, 1);
	printf("\n======================================\n");
	//Open output file
	if(!(ofmt_v->flags & AVFMT_NOFILE))
	{
		if(avio_open(&ofmt_ctx_v->pb, filename_out_v, AVIO_FLAG_WRITE) < 0)
		{
			printf("Could not open output file '%s'", filename_out_v);
			goto end;
		}
	}

	if(!(ofmt_a->flags & AVFMT_NOFILE))
	{
		if(avio_open(&ofmt_ctx_a->pb, filename_out_a, AVIO_FLAG_WRITE) < 0)
		{
			printf("Could not open output file '%s'", filename_out_a);
			goto end;
		}
	}

	//Write file header
	if(avformat_write_header(ofmt_ctx_v, NULL) < 0)
	{
		printf("Error occurred when opening video output file!\n");
		goto end;
	}
	if(avformat_write_header(ofmt_ctx_a, NULL) < 0)
	{
		printf("Error occurred when opening audio output file!\n");
		goto end;
	}

#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb"); 
#endif

	while(1)
	{
		AVFormatContext *ofmt_ctx   = NULL;
		AVStream        *stream_in  = NULL; 
		AVStream	    *stream_out = NULL;
		//Get an AVPacket
		if (av_read_frame(ifmt_ctx, &pkt) < 0)
			break;
		stream_in = ifmt_ctx->streams[pkt.stream_index];

		if(pkt.stream_index == videoindex)
		{
			stream_out = ofmt_ctx_v->streams[0];
			ofmt_ctx = ofmt_ctx_v;
			printf("Write Video Packet, size: %d\t pts: %lld\n", pkt.size, pkt.pts);
#if USE_H264BSF
			av_bitstream_filter_filter(h264bsfc, stream_in->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
		}
		else if(pkt.stream_index == audioindex)
		{
			stream_out = ofmt_ctx_a->streams[0];
			ofmt_ctx = ofmt_ctx_a;
			printf("Write Audio Packet, size: %d\t pts: %lld\n", pkt.size, pkt.pts);
		}
		else 
			continue;

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, stream_in->time_base, stream_out->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, stream_in->time_base, stream_out->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, stream_in->time_base, stream_out->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;

		//Write
		if(av_interleaved_write_frame(ofmt_ctx, &pkt) < 0)
		{
			printf("Error muxing packet!\n");
			break;
		}
		printf("Write %8d frames to output file!\n",frame_index);
		av_free_packet(&pkt);
		frame_index++;
	}

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);  
#endif
	//Write file trailer
	av_write_trailer(ofmt_ctx_a);
	av_write_trailer(ofmt_ctx_v);

end:
	avformat_close_input(&ifmt_ctx);
	/* close output */
	if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_a->pb);

	if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_v->pb);

	avformat_free_context(ofmt_ctx_a);
	avformat_free_context(ofmt_ctx_v);


	if (ret < 0 && ret != AVERROR_EOF) 
	{
		printf("Error occurred.\n");
		return -1;
	}

	getchar();
	return 0;
}