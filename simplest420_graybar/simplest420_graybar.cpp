#include <iostream>
using namespace std;

int simplest_yuv420_graybar(int width, int height, int ymin, int ymax, int barnum, char *url_out)
{
    int barwidth;
    float lum_inc;
    unsigned char lum_temp;
    int uv_width, uv_height;
    FILE *fp = NULL;
    unsigned char *data_y = NULL;
    unsigned char *data_u = NULL;
    unsigned char *data_v = NULL;
    int t = 0, i = 0, j = 0;

    barwidth = width / barnum;
    lum_inc = ((float)(ymax - ymin)) / ((float)(barnum - 1));
    uv_width = width / 2;
    uv_height = height /2;

    data_y = (unsigned char *)malloc(width * height);
    data_u = (unsigned char *)malloc(uv_width * uv_height);
    data_v = (unsigned char *)malloc(uv_width * uv_height);

    if((fp = fopen(url_out, "wb+")) == NULL)
    {
        printf("Error: Cannot create file!");
        return -1;
    }

    //output info
    printf("Y, U, V value from picture's letf to right:\n");
    for(t = 0; t < (width / barwidth); ++t)
    {
        lum_temp = ymin + (char)(t * lum_inc);
        printf("%3d, 128, 128\n", lum_temp);
    }

    //Gen Data
    for(j = 0; j < height; ++j)
    {
        for(i = 0; i < width; ++i)
        {
            t = i / barwidth;
            lum_temp = ymin + (char)(t * lum_inc);
            data_y[j * width + i] = lum_temp;
        }
    }

    for(j = 0; j < uv_height; ++j)
    {
        for(i = 0; i < uv_width; ++i)
        {
            data_u[j *  width + i ] = 128;
        }
    }

    for(j = 0; i < uv_height; ++j)
    {
        for(i = 0; i <uv_width; ++i)
        {
            data_v[j * width + i] = 128;
        }
    }
    fwrite(data_y, width * height, 1, fp);
    fwrite(data_u, uv_width * uv_height, 1, fp);
    fwrite(data_v, uv_width * uv_height, 1, fp);
    fclose(fp);
    free(data_y);
    free(data_u);
    free(data_v);
    return 0;
}

int main()
{
    simplest_yuv420_graybar(640, 360, 0, 255, 10, "graybar_640x360.yuv");
    return 0;
}
