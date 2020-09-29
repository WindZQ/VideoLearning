#include <cstdio>
#include "Decode.h"

int main(int argc, char **argv)
{
	char *filepath = "../Titanic.mkv";
	Decode *decode = new Decode();

	decode->decode_init(filepath);
	decode->decode_process();
	decode->decode_uninit();
	getchar();
	return 0;
}