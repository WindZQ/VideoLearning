#include "Play.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	char *fileName = argv[1];
	Player player;
	player.init(fileName);
	player.codec();
	player.destory();
	return 0;
}