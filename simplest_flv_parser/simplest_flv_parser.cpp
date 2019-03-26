#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;
#pragma pack(1)

#define TAG_TYPE_SCRIPT   18
#define TAG_TYPE_VIDEO     9
#define TAG_TYPE_AUDIO     8

typedef unsigned char uchar;
typedef unsigned int uint;

typedef struct
{
    uchar signature[3];
    uchar version;
    uchar flags;
    uint  dataoffset;
}FLV_HEADER;

typedef struct
{
    uchar tagType;
    uchar dataSize[3];
    uchar timestamp[3];
    uint  reserved;
}TAG_HEADER;

//
uint reserseBytes(uchar *p, char c)
{
    int r = 0;
    for(int i = 0; i < c; ++i)
        r |= *(p + i) << (((c - 1) * 8) - 8 * i);
    return r;
}

//
int simplestFlvParser(char *url)
{
    //
    int outputAudio = 1;
    int outputVedio = 1;

    //
    uint previoustagsize, previoustagsize_z = 0;
    char tagTypeStr[20] = {0};
    TAG_HEADER tagHeader;
    FLV_HEADER flv;
    FILE *ifh = NULL, *iah = NULL, *ivh = NULL;
    FILE *out = stdout;
    uint ts = 0, ts_new = 0;
    ifh = fopen(url, "rb+");
    if(NULL == ifh)
    {
        cout << "Can't open the file!" << endl;
        return -1;
    }

    fread((char *)&flv, 1, sizeof(FLV_HEADER), ifh);
    //
    fprintf(out, "==============FLV Header================\n");
    fprintf(out, "Signature:   0x %c %c %c \n", flv.signature[0], flv.signature[1], flv.signature[2]);
    fprintf(out, "Version:     0x %x \n", flv.version);
    fprintf(out, "Flags:       0x %x \n", flv.flags);
    fprintf(out, "HeaderSize:  0x %x \n", reserseBytes((uchar *)&flv.dataoffset, sizeof(flv.dataoffset)));
    fprintf(out, "========================================\n");

    //
    fseek(ifh, reserseBytes((uchar *)&flv.dataoffset, sizeof(flv.dataoffset)), SEEK_SET);

    //
    do
    {
        previoustagsize = _getw(ifh);

        fread((void *)&tagHeader, sizeof(TAG_HEADER), 1, ifh);
        int tagHeaderDataSize = tagHeader.dataSize[0] * 65536 + tagHeader.dataSize[1] * 256 + tagHeader.dataSize[2];
        int tagHeaderTimeStamp = tagHeader.timestamp[0] * 65536 + tagHeader.timestamp[1] * 256 + tagHeader.timestamp[2];
        switch(tagHeader.tagType)
        {
        case TAG_TYPE_AUDIO:
            sprintf(tagTypeStr, "AUDIO");
            break;
        case TAG_TYPE_VIDEO:
            sprintf(tagTypeStr, "VIDEO");
            break;
        case TAG_TYPE_SCRIPT:
            sprintf(tagTypeStr, "SCRIPT");
            break;
        default:
            sprintf(tagTypeStr, "UNKNOWN");
            break;
        }
        fprintf(out, "[%6s] %6d %6d |", tagTypeStr, tagHeaderDataSize, tagHeaderTimeStamp);
        if(feof(ifh))
            break;

        switch(tagHeader.tagType)
        {
        case TAG_TYPE_AUDIO:
        {
            char audioTagStr[100] = {0};
            strcat(audioTagStr, "|");
            char tagDataFirstByte;
            tagDataFirstByte = fgetc(ifh);
            int x = tagDataFirstByte & 0xF0;
            x = x >> 4;
            switch(x)
            {
            case 0:
                strcat(audioTagStr, "Linear PCM, platform endian");
                break;
            case 1:
                strcat(audioTagStr, "ADPCM");
                break;
            case 2:
                strcat(audioTagStr, "MP3");
                break;
            case 3:
                strcat(audioTagStr, "Linear PCM, little endian");
                break;
            case 4:
                strcat(audioTagStr, "Nellymoser 16-kHZ mono");
                break;
            case 5:
                strcat(audioTagStr, "Nellymoser 8-kHZ mono");
                break;
            case 6:
                strcat(audioTagStr, "Nellymoser");
                break;
            case 7:
                strcat(audioTagStr, "G.711 A-law logarthmic PCM");
                break;
            case 8:
                strcat(audioTagStr, "G.711 mu-law logarthmic PCM");
                break;
            case 9:
                strcat(audioTagStr, "reserved");
                break;
            case 10:
                strcat(audioTagStr, "AAC");
                break;
            case 11:
                strcat(audioTagStr, "Speex");
                break;
            case 14:
                strcat(audioTagStr, "MP3 8-kHZ");
                break;
            case 15:
                strcat(audioTagStr, "Device-specific sound");
                break;
            default:
                strcat(audioTagStr, "UNKNOWN");
                break;
            }
            strcat(audioTagStr, "|");
            x = tagDataFirstByte & 0x0C;
            x = x >> 2;
            switch(x)
            {
            case 0:
                strcat(audioTagStr, "5.5-kHZ");
                break;
            case 1:
                strcat(audioTagStr, "1-kHZ");
                break;
            case 2:
                strcat(audioTagStr, "22-kHZ");
                break;
            case 3:
                strcat(audioTagStr, "44-kHZ");
                break;
            default:
                strcat(audioTagStr, "UNKNOWN");
                break;
            }
            strcat(audioTagStr, "|");
            x = tagDataFirstByte & 0x02;
            x = x >> 1;
            switch(x)
            {
            case 0:
                strcat(audioTagStr, "8-Bit");
                break;
            case 1:
                strcat(audioTagStr, "16-Bit");
                break;
            default:
                strcat(audioTagStr, "UNKNOWN");
                break;
            }
            strcat(audioTagStr, "|");
            x = tagDataFirstByte & 0x01;
            switch(x)
            {
            case 0:
                strcat(audioTagStr, "Mono");
                break;
            case 1:
                strcat(audioTagStr, "Stereo");
                break;
            default:
                strcat(audioTagStr, "UNKNOWN");
                break;
            }
            fprintf(out, "%s", audioTagStr);

            //
            if(outputAudio != 0 && iah == NULL)
            {
                iah = fopen("output.mp3", "wb+");
            }

            //
            int data_size = reserseBytes((uchar *)&tagHeader.dataSize, sizeof(tagHeader.dataSize)) - 1;
            if(outputAudio != 0)
            {
                for(int i = 0; i < data_size; ++i)
                    fputc(fgetc(ifh), iah);
            }
            else
            {
                for(int i = 0; i < data_size; ++i)
                    fgetc(ifh);
            }
            break;
        }
        case TAG_TYPE_VIDEO:
        {
            char videoTagStr[100] = {0};
            strcat(videoTagStr, "|");
            char tagDataFirstByte = fgetc(ifh);
            int x = tagDataFirstByte & 0xF0;
            x = x >> 4;
            switch(x)
            {
            case 1:
                strcat(videoTagStr, "key frame");
                break;
            case 2:
                strcat(videoTagStr, "inter frame");
                break;
            case 3:
                strcat(videoTagStr, "disposable inter frame");
                break;
            case 4:
                strcat(videoTagStr, "generated key frame");
                break;
            case 5:
                strcat(videoTagStr, "video info/command frame");
                break;
            default:
                strcat(videoTagStr, "UNKNOWN");
                break;
            }
            strcat(videoTagStr, "|");
            x = tagDataFirstByte & 0x0F;
            switch(x)
            {
            case 1:
                strcat(videoTagStr, "JPEG (currently uesd)");
                break;
            case 2:
                strcat(videoTagStr, "Sorenson H.263");
                break;
            case 3:
                strcat(videoTagStr, "Screen video");
                break;
            case 4:
                strcat(videoTagStr, "On2 VP6");
                break;
            case 5:
                strcat(videoTagStr, "On2 VP6 with alpha channel");
                break;
            case 6:
                strcat(videoTagStr, "Screen video version 2");
                break;
            case 7:
                strcat(videoTagStr, "AVC");
                break;
            default:
                strcat(videoTagStr, "UNKNOWN");
                break;
            }
            fprintf(out, "%s", videoTagStr);

            fseek(ifh, -1, SEEK_CUR);
            //
            if(outputVedio != 0 && ivh == NULL)
            {
                ivh = fopen("output.flv", "wb+");
                fwrite((char *)&flv, 1, sizeof(flv), ivh);
                fwrite((char *)&previoustagsize_z, 1, sizeof(previoustagsize_z), ivh);
            }

#if 0
            //
            //
            ts = reserseBytes((uchar *)&tagHeader.timestamp, sizeof(tagHeader.timestamp));
            ts = ts * 2;
            //
            ts_new = reserseBytes((uchar *)&ts, sizeof(ts));
            memcpy(&tagHeader.timestamp, ((char *)&ts_new) + 1, sizeof(tagHeader.timestamp));
#endif
            //
            int data_size = reserseBytes((uchar *)&tagHeader.dataSize, sizeof(tagHeader.dataSize)) + 4;
            if(outputVedio != 0)
            {
                //
                fwrite((char *)&tagHeader, 1, sizeof(tagHeader), ivh);
                //
                for(int i = 0; i < data_size; ++i)
                    fputc(fgetc(ifh), ivh);
            }
            else
            {
                for(int i = 0; i < data_size; ++i)
                    fgetc(ifh);
            }
            fseek(ifh, -4, SEEK_CUR);
            break;
        }
        default:
            //
            fseek(ifh, reserseBytes((uchar *)&tagHeader.dataSize, sizeof(tagHeader.dataSize)), SEEK_CUR);
            break;
        }
        fprintf(out, "\n");
    }while(!feof(ifh));
    _fcloseall();
    return 0;
}

int main()
{
    simplestFlvParser("cuc_ieschool.flv");
    return 0;
}
