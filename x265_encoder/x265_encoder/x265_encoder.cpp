#include <cstdio>
#include <cstdlib>
#pragma comment(lib, "libx265.lib")

#if defined ( __cplusplus)
extern "C"
{
#include "x265.h"
};
#else
#include "x265.h"
#endif


int main(int argc, char **arv)
{
	int i, j;
	FILE     *fp_src     = NULL;
	FILE     *fp_dst     = NULL;
	int      y_size      = 0;
	int      buffer_size = 0;
	char     *buffer     = NULL;
	int      ret         = 0;
	x265_nal *pNals      = NULL;
	uint32_t iNal        = 0;

	x265_param   *pParam  = NULL;
	x265_encoder *pHandle = NULL;
	x265_picture *pPic_in = NULL;

	//Encode 50 frame
	//if set 0, encode all frame
	int frame_num = 50;
	int csp       = X265_CSP_I420;
	int width     = 640;
	int height    = 360;

	fp_src = fopen("../cuc_ieschool_640x360_yuv420p.yuv", "rb");
	fp_dst = fopen("../cuc_ieschool.h265", "wb");
	//Check
	if(NULL == fp_src || NULL == fp_dst)
		return -1;

	pParam = x265_param_alloc();
	x265_param_default(pParam);
	pParam->bRepeatHeaders = 1; // Wirte sps, pps berfore keyframe
	pParam->internalCsp    = csp;
	pParam->sourceWidth    = width;
	pParam->sourceWidth    = height;
	pParam->fpsNum         = 25;
	pParam->fpsDenom       = 1;

	//Init
	pHandle = x265_encoder_open(pParam);
	if(NULL == pHandle)
	{
		printf("x265_encoder_open err\n");
		return -1;
	}

	y_size  = pParam->sourceWidth * pParam->sourceHeight;
	pPic_in = x265_picture_alloc();
	x265_picture_init(pParam, pPic_in);

	switch (csp)
	{
	case X265_CSP_I444:
		{
			buffer = (char *)malloc(y_size * 3);
			pPic_in->planes[0] = buffer;
			pPic_in->planes[1] = buffer + y_size;
			pPic_in->planes[2] = buffer + y_size * 2;
			pPic_in->stride[0] = width;
			pPic_in->stride[1] = width;
			pPic_in->stride[2] = width;
			break;
		}
	case X265_CSP_I420:
		{
			buffer = (char *)malloc(y_size * 3 / 2);
			pPic_in->planes[0] = buffer;
			pPic_in->planes[1] = buffer + y_size;
			pPic_in->planes[2] = buffer + y_size * 5 / 4;
			pPic_in->stride[0] = width;
			pPic_in->stride[1] = width / 2;
			pPic_in->stride[2] = width / 2;
			break;
		}
	default:
		printf("Colorspace Not Support.\n");
		return -1;
	}


	//detect frame number
	if(0 == frame_num)
	{
		fseek(fp_src, 0, SEEK_END);
		switch (csp)
		{
		case X265_CSP_I444: 
			frame_num = ftell(fp_src) / (y_size * 3);
			break;
		case X265_CSP_I420:
			frame_num= ftell(fp_src) / (y_size * 3 / 2);
			break;
		default:
			printf("Colorspace Not Support!\n");
			return -1;
		}
		fseek(fp_src, 0, SEEK_SET);
	}

	//loop to encode
	for(i = 0; i < frame_num; ++i)
	{
		switch (csp)
		{
		case X265_CSP_I444:
			{
				fread(pPic_in->planes[0], 1, y_size, fp_src);
				fread(pPic_in->planes[1], 1, y_size, fp_src);
				fread(pPic_in->planes[2], 1, y_size, fp_src);
				break;
			}
		case X265_CSP_I420:
			{
				fread(pPic_in->planes[0], 1, y_size, fp_src);
				fread(pPic_in->planes[1], 1, y_size / 4, fp_src);
				fread(pPic_in->planes[2], 1, y_size / 4, fp_src);
				break;
			}
		default:
			printf("Colorspace Not Support!\n");
			return -1;
		}
		ret = x265_encoder_encode(pHandle, &pNals, &iNal, pPic_in, NULL);	
		printf("Succeed encode %5d frames\n",i);

		for(j = 0; j < iNal; j++)
			fwrite(pNals[j].payload, 1, pNals[j].sizeBytes, fp_dst);
	}

	//Flush Decoder
	while(1)
	{
		ret = x265_encoder_encode(pHandle, &pNals, &iNal, NULL, NULL);
		if(0 == ret)
			break;
		printf("Flush 1 frame.\n");

		for(j = 0; j < iNal; j++)
			fwrite(pNals[j].payload, 1, pNals[j].sizeBytes, fp_dst);
	}

	x265_encoder_close(pHandle);
	x265_picture_free(pPic_in);
	x265_param_free(pParam);
	free(buffer);
	fclose(fp_src);
	fclose(fp_dst);

	return 0;
}