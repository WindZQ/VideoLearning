#include <iostream>
using namespace std;

int simplest_yuv420p_halfy(char *url, int w, int h, int num)
{
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_halfy.yuv", "wb+");
    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    for(int i = 0; i < num; ++i)
    {
        fread(pic, 1, w * h * 3 / 2, fp);
        //halfy
        for(int j = 0; j < w * h; ++j)
        {
            unsigned char temp = pic[j] / 2;
            pic[j] = temp;
        }
        fwrite(pic, 1, w * h * 3 / 2, fp1);
    }
    free(pic);
    fclose(fp);
    fclose(fp1);
    return 0;
}

int main()
{
    simplest_yuv420p_halfy("lena_256x256_yuv420p.yuv", 256, 256, 1);
    return 0;
}
