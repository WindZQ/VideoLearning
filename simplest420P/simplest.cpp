#include <iostream>
using namespace std;

int simplest_yuv420p_split(char *url, int w, int h, int num)
{
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_420_y.y", "wb+");
    FILE *fp2 = fopen("output_420_u.y", "wb+");
    FILE *fp3 = fopen("output_420_v.y", "wb+");

    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    for(int i = 0; i < num; ++i)
    {
        fread(pic, 1, w * h * 3 / 2, fp);
        //Y
        fwrite(pic, 1, w * h, fp1);
        //U
        fwrite(pic + w * h, 1, w * h / 4, fp2);
        //V
        fwrite(pic + 5 * w * h / 4, 1, w * h / 4, fp3);
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
    simplest_yuv420p_split("lena_256x256_yuv420p.yuv", 256, 256, 1);
    return 0;
}
