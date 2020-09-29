#include <cstdio>

extern "C"
{
#include "libavformat/avformat.h"
};

int main(int argc, char *argv[])
{
	AVOutputFormat *ofmt        = NULL;
	AVFormatContext *ifmt_ctx   = NULL;
	AVFormatContext	*ofmt_ctx   = NULL;
	int             ret         = 0;
	int             i           = 0;
	int             frame_index = 0;
	AVPacket pkt;
	const char *filename_in, *filename_out;

	filename_in  = "cuc_ieschool1.flv";//Input file URL
	filename_out = "cuc_ieschool1.mp4";//Output file URL

	av_register_all();
	//Input
	if ((ret = avformat_open_input(&ifmt_ctx, filename_in, 0, 0)) < 0) 
	{
		printf("Could not open input file.\n");
		goto end;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) 
	{
		printf("Failed to retrieve input stream information.\n");
		goto end;
	}
	av_dump_format(ifmt_ctx, 0, filename_in, 0);
	//Output
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename_out);
	if (!ofmt_ctx) 
	{
		printf("Could not create output context.\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	for (i = 0; i < ifmt_ctx->nb_streams; i++) 
	{
		//Create output AVStream according to input AVStream
		AVStream *stream_in = ifmt_ctx->streams[i];
		AVStream *stream_out = avformat_new_stream(ofmt_ctx, stream_in->codec->codec);
		if (!stream_out) 
		{
			printf("Failed allocating output stream.\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//Copy the settings of AVCodecContext
		if (avcodec_copy_context(stream_out->codec, stream_in->codec) < 0) 
		{
			printf("Failed to copy context from input to output stream codec context.\n");
			goto end;
		}
		stream_out->codec->codec_tag = 0;

		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			stream_out->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	//Output information------------------
	av_dump_format(ofmt_ctx, 0, filename_out, 1);
	//Open output file
	if (!(ofmt->flags & AVFMT_NOFILE)) 
	{
		ret = avio_open(&ofmt_ctx->pb, filename_out, AVIO_FLAG_WRITE);
		if (ret < 0) 
		{
			printf("Could not open output file '%s'", filename_out);
			goto end;
		}
	}
	//Write file header
	if (avformat_write_header(ofmt_ctx, NULL) < 0) 
	{
		printf("Error occurred when opening output file.\n");
		goto end;
	}

	while (1) 
	{
		AVStream *stream_in, *stream_out;
		//Get an AVPacket
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
			break;
		stream_in  = ifmt_ctx->streams[pkt.stream_index];
		stream_out = ofmt_ctx->streams[pkt.stream_index];

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, stream_in->time_base, stream_out->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, stream_in->time_base, stream_out->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, stream_in->time_base, stream_out->time_base);
		pkt.pos = -1;
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) 
		{
			printf("Error muxing packet.\n");
			break;
		}
		printf("Write %8d frames to output file.\n", frame_index);
		av_free_packet(&pkt);
		frame_index++;
	}
	//Write file trailer
	av_write_trailer(ofmt_ctx);
end:
	avformat_close_input(&ifmt_ctx);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	getchar();
	return 0;
}