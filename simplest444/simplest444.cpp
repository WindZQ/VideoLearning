#include <iostream>
using namespace std;

int simplest_yuv444_split(char *url, int w, int h, int num)
{
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_yuv444_y.y", "wb+");
    FILE *fp2 = fopen("output_yuv444_u.y", "wb+");
    FILE *fp3 = fopen("output_yuv444_v.y", "wb+");

    unsigned char *pic = (unsigned char *)malloc(w * h * 3);

    for(int i = 0; i < num; ++i)
    {
        fread(pic, 1, w * h * 3, fp);
        fwrite(pic, 1, w * h, fp1);
        fwrite(pic + w * h, 1, w * h, fp2);
        fwrite(pic + 2 * w * h, 1, w * h, fp3);
    }
    free(pic);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    return 0;
}

int main()
{
    simplest_yuv444_split("lena_256x256_yuv444p.yuv", 256, 256, 1);
    return 0;
}
