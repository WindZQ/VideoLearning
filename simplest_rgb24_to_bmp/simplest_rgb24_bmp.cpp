#include <iostream>

using namespace std;

typedef struct
{
    long imageSize;
    long blank;
    long startPosition;
}BmpHead;

typedef struct
{
    long length;
    long width;
    long height;
    unsigned short colorPlane;
    unsigned short bitColor;
    long zipFormat;
    long realSize;
    long xPels;
    long yPels;
    long colorUse;
    long colorImportant;
}InfoHead;

int simplest_rgb24_to_bmp(const char *rgb24path, int width, int height, const char *bmppath)
{
    int i = 0, j = 0;
    BmpHead m_BMPHeader = {0};
    InfoHead m_BMPInfoHeader = {0};
    char bfType[2] = {'B', 'M'};
    int header_size = sizeof(bfType) + sizeof(BmpHead) + sizeof(InfoHead);
    unsigned char *rgb24_buffer = NULL;
    FILE *fp_rgb24 = NULL, *fp_bmp = NULL;
    if((fp_rgb24 = fopen(rgb24path, "rb+")) == NULL)
    {
        printf("Error: Cannot open input RGB24 file.\n");
        return -1;
    }

    if((fp_bmp = fopen(bmppath, "wb+")) == NULL)
    {
        printf("Error: Cannot open output BMP file.\n");
        return -1;
    }

    rgb24_buffer = (unsigned char *)malloc(width * height * 3);
    fread(rgb24_buffer, 1, width * height * 3, fp_rgb24);

    m_BMPHeader.imageSize = 3 * width * height + header_size;
    m_BMPHeader.startPosition = header_size;

    m_BMPInfoHeader.length = sizeof(InfoHead);
    m_BMPInfoHeader.width = width;

    //BMP storage pixel data in opposite direction of Y-axis(from bottom to top)
    m_BMPInfoHeader.height = -height;
    m_BMPInfoHeader.colorPlane = 1;
    m_BMPInfoHeader.bitColor = 24;
    m_BMPInfoHeader.realSize = 3 * height * width;

    fwrite(bfType, 1, sizeof(bfType), fp_bmp);
    fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
    fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);

    //BMP save R1[G1]B1,R2[G2]B2 as B1[G1]R1,B2[R2]R2
    //It saves pixel data in little Endian
    //So we cahge 'R' and 'B'

    for(j = 0; j < height; ++j)
    {
        for(i = 0; i < width; ++i)
        {
            char temp = rgb24_buffer[(j * width + i) * 3 + 2];
            rgb24_buffer[(j * width + i) * 3 + 2] = rgb24_buffer[(j * width + i) * 3 + 0];
            rgb24_buffer[(j * width + i) * 3 + 0] = temp;
        }
    }
    fwrite(rgb24_buffer, 3 * width *height, 1, fp_bmp);
    fclose(fp_rgb24);
    fclose(fp_bmp);
    free(rgb24_buffer);
    printf("Finish generate %s!\n", bmppath);
    return 0;
}

int main()
{
    simplest_rgb24_to_bmp("lena_256x256_rgb24.rgb", 256, 256, "output_lena.bmp");
    return 0;
}
