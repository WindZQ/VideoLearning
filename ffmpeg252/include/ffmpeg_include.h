#pragma once

#ifndef INT64_C 
#define INT64_C(c) (c ## LL) 
#define UINT64_C(c) (c ## ULL) 
#endif


extern  "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#if defined _M_X64
#pragma comment(lib, "x64/avcodec.lib")
#pragma comment(lib, "x64/avformat.lib")
#pragma comment(lib, "x64/avutil.lib")
#pragma comment(lib, "x64/swscale.lib")
#pragma comment(lib, "x64/swresample.lib")
#else
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")
#endif // _M_X64
};
