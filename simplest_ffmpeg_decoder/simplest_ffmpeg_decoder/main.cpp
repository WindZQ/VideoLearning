#include <iostream>
#include "decoder.h"

using namespace std;

int main(int argc, char *argv[])
{
	char *fileName = "Titanic.mkv";
	Decodec decodec;
	decodec.init(fileName);
	decodec.decodec();
	decodec.destroy();
	return 0;
}