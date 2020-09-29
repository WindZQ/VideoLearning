#include <iostream>
#include "player.h"
using namespace std;

int main(int argc, char *argv[])
{
	char *fileName = "bigbuckbunny_480x272.h265";
	Player test;
	test.init(fileName);
	test.codec();
	test.destory();
	return 0;
}

