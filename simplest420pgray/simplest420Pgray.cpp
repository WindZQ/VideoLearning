#include <iostream>
#include <cstring>
using namespace std;

int simplest_yuv420p_gray(char *url, int w, int h, int num)
{
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_420P_gray.yuv", "wb+");
    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    for(int i = 0; i < num; ++i)
    {
        fread(pic, 1, w * h * 3 / 2, fp);
        //gray
        memset(pic + w * h, 128, w * h / 2);
        fwrite(pic, 1, w * h * 3 / 2, fp1);
    }
    free(pic);
    fclose(fp);
    fclose(fp1);
    return 0;
}

int main()
{
    simplest_yuv420p_gray("lena_256x256_yuv420p.yuv", 256, 256, 1);
    return 0;
}
