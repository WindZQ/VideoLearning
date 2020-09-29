#include  "Decoder.h"

Decodec::Decodec() : pCodecCtx(NULL),
	pCodecParserCtx(NULL),
	first_time(1)
{

}

int Decodec::init(char *fileNameIn, AVCodecID codec_id, char *fileNameOut)
{
	avcodec_register_all();
	pCodec = avcodec_find_decoder(codec_id);

	if (!pCodec)
	{
		cout << "Could't found the Codec!" << endl;
		return -1;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx)
	{
		cout << "Could't allocate video codec context!" << endl;
		return -1;
	}

	pCodecParserCtx = av_parser_init(codec_id);
	if (!pCodecParserCtx)
	{
		cout << "Could't allocate video parser context!" << endl;
		return -1;
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		cout << "Could not open codec!" << endl;
		return -1;
	}

	fp_in = fopen(fileNameIn, "rb+");
	if (!fp_in)
	{
		cout << "Could not open input stream!" << endl;
		return -1;
	}

	fp_out = fopen(fileNameOut, "wb+");
	if (!fp_out)
	{
		cout << "Could not open output YUV file!" << endl;
		return -1;
	}
	return 0;
}

void Decodec::process()
{
	pFrame = av_frame_alloc();
	av_init_packet(&packet);

	int   cur_size, y_size;
	int   ret, got_picture;
	const int  in_buffer_size = 4096;
	uint8_t    in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };
	uint8_t*   cur_ptr = NULL;

	while (1)
	{
		cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
		if (cur_size == 0)
			break;
		cur_ptr = in_buffer;

		while (cur_size > 0)
		{
			int len = av_parser_parse2(pCodecParserCtx, pCodecCtx,
				&packet.data, &packet.size, cur_ptr, cur_size,
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
			cur_ptr += len;
			cur_size -= len;

			if (packet.size == 0)
				continue;

			printf("[Packet]Size:%6d\t", packet.size);
			switch (pCodecParserCtx->pict_type)
			{
			case AV_PICTURE_TYPE_I:
				cout << "Type:I" << endl;
				break;
			case AV_PICTURE_TYPE_P:
				cout << "Type:P" << endl;
				break;
			case AV_PICTURE_TYPE_B:
				cout << "Type:B" << endl;
				break;
			default:
				cout << "Type:Other" << endl;
				break;
			}

			printf("Number:%4d\n", pCodecParserCtx->output_picture_number);

			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);

			if (ret < 0)
				cout << "Decode Error" << endl;
			if (got_picture)
			{
				if (first_time)
				{
					printf("\nCodec Full Name:%s\n", pCodecCtx->codec->long_name);
					printf("width:%d\nheight:%d\n\n", pCodecCtx->width, pCodecCtx->height);
					first_time = 0;
				}

				//YUV
				for (int i = 0; i < pFrame->height; ++i)
					fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
				for (int i = 0; i < pFrame->height / 2; ++i)
					fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
				for (int i = 0; i < pFrame->height / 2; ++i)
					fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
				cout << "Succeed to decodec 1 frame" << endl;
			}
		}
	}

	packet.data = NULL;
	packet.size = 0;

	while (1)
	{
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
		if (ret < 0)
			cout << "" << endl;
		if (!got_picture)
			break;
		else
		{
			//YUV
			for (int i = 0; i < pFrame->height; ++i)
				fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
			for (int i = 0; i < pFrame->height / 2; ++i)
				fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
			for (int i = 0; i < pFrame->height / 2; ++i)
				fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
		}
	}

}

void Decodec::destroy()
{
	fclose(fp_in);
	fclose(fp_out);
	av_parser_close(pCodecParserCtx);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
}