#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

typedef enum
{
    NALU_TYPE_SLICE    = 1,
    NALU_TYPE_DPA      = 2,
    NALU_TYPE_DPB      = 3,
    NALU_TYPE_DPC      = 4,
    NALU_TYPE_IDR      = 5,
    NALU_TYPE_SEI      = 6,
    NALU_TYPE_SPS      = 7,
    NALU_TYPE_PPS      = 8,
    NALU_TYPE_AUD      = 9,
    NALU_TYPE_EOSEQ    = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL     = 12
}NaluType;

typedef enum
{
    NALU_PRIORITY_DISPOSABLE  = 0,
    NALU_PRIRITY_LOW          = 1,
    NALU_PRIORITY_HIGH        = 2,
    NALU_PRIORITY_HIGHEST     = 3
}NaluPriority;

typedef struct
{
    int startcodeprfix_len;      //4 for parameter sets and first slice in picture, 3 for everything
    unsigned len;                //length of the Nal unit
    unsigned max_size;           //Nal unit buffer size
    int forbidden_bit;           //should be always false
    int nal_reference_idc;       //NALU_PRIORITY_xxxxx
    int nal_unit_type;           //NALU_TYPE_xxx
    char *buf;                   //contains the first byte followed by the EBSP
}NALU_t;

FILE *h264bitstream = NULL;      // the bit of stream file
int info2 = 0, info3 = 0;

static int FindStartCode2(unsigned char *buf)
{
    //judge 0x000001
    if(buf[0] != 0 || buf[1] != 0 || buf[2] != 1)
        return 0;
    else
        return 1;
}

static int FindStartCode3(unsigned char *buf)
{
    //judge 0x0000001
    if(buf[0] != 0 || buf[1] != 0 || buf[2] != 0 || buf[3] != 1)
        return 0;
    else
        return 1;
}

int GetAnnexbNALU(NALU_t *nalu)
{
    int pos = 0;
    int StartCodeFound = 0;
    unsigned char *Buf;
    if((Buf =(unsigned char*)calloc(nalu->max_size, sizeof(char))) == NULL)
        cout << "GetAnnexbNALU:Can't alloc buf memory!" << endl;

    nalu->startcodeprfix_len = 3;
    if(3 != fread(Buf, 1, 3, h264bitstream))
    {
        free(Buf);
        return 0;
    }
    info2 = FindStartCode2(Buf);
    if(1 != info2)
    {
        if(1 != fread(Buf + 3, 1, 1, h264bitstream))
        {
            free(Buf);
            return 0;
        }
        info3 = FindStartCode3(Buf);
        if(1 != info3)
        {
            free(Buf);
            return -1;
        }
        else
        {
            nalu->startcodeprfix_len = 4;
            pos = 4;
        }
    }
    else
    {
        nalu->startcodeprfix_len = 3;
        pos = 3;
    }
    info2 = 0;
    info3 = 0;
    while(!StartCodeFound)
    {
        if(feof(h264bitstream))
        {
            nalu->len = (pos - 1) - nalu->startcodeprfix_len;
            memcpy(nalu->buf, &Buf[nalu->startcodeprfix_len], nalu->len);
            nalu->forbidden_bit = nalu->buf[0] & 0x80;              //1 bit
            nalu->nal_reference_idc = nalu->buf[0] & 0x60;          //2 bit
            nalu->nal_unit_type = nalu->buf[0] & 0x1f;              //5 bit
            free(Buf);
            return pos - 1;
        }
        Buf[pos++] = fgetc(h264bitstream);
        info3 = FindStartCode3(&Buf[pos - 4]);
        if(1 != info3)
            info2 = FindStartCode2(&Buf[pos - 3]);
        StartCodeFound = (info2 == 1 || info3 == 1);
    }

    //Here, we have found another start code(and read length of 
    //startcode bytes more than we should have.Hence, go back in the file
    int rewind = (info3 == 1) ? -4 : -3;
    
    if(0 != fseek(h264bitstream, rewind, SEEK_CUR))
    {
        free(Buf);
        cout << "GetAnnexbNALU:Can't fseek a bit stream file!" << endl;
    }

    //Here the start code, the complete NALU, and the next start code is in the buf
    //The size of buf is pos, pos + rewind are the number of bytes excluding the next
    //start code, and (pos + rewind)-startcodeprefix_len is the size of the NALU excluding the start code
    nalu->len = (pos + rewind) - nalu->startcodeprfix_len;
    memcpy(nalu->buf, &Buf[nalu->startcodeprfix_len], nalu->len);
    nalu->forbidden_bit = nalu->buf[0] & 0x80;              //1 bit
    nalu->nal_reference_idc = nalu->buf[0] &0x60;           //2 bit 
    nalu->nal_unit_type = nalu->buf[0] & 0x1f;              //5 bit
    free(Buf);

    return pos + rewind;
}

/*Analysis H.264 bitstream
*Analysis H.264 bitstream
*param url  Location of input H.264 bit file
*/
int simplest_h264_parser(char *url)
{
    NALU_t *n;
    int bufferSize = 100000;
    int dataOffset = 0;
    int nalNum = 0;
    int dataLength = 0;
    char typeStr[20] = {0};
    char idcStr[20] = {0};
    FILE *myOut = stdout;
    h264bitstream = fopen(url, "rb+");
    if(h264bitstream == NULL)
    {
        cout << "Open file failed!" << endl;
        return 0;
    }

    n = (NALU_t *) calloc(1, sizeof(NALU_t));
    if(NULL == n)
    {
        cout << "Alloc NALU failed!" << endl;
        return 0;
    }

    n->max_size = bufferSize;
    n->buf = (char *)calloc(bufferSize, sizeof(char));
    if(NULL == n->buf)
    {
        free(n);
        return 0;
    }
    
    cout << "-----+-------- NALU Table ------+---------+" << endl;
    cout << " NUM |    POS  |    IDC |  TYPE |   LEN   |"  << endl;
    cout << "-----+---------+--------+-------+---------+"  << endl;

    while(!feof(h264bitstream))
    {
        dataLength = GetAnnexbNALU(n);
        switch(n->nal_unit_type)
        {
        case NALU_TYPE_SLICE:
            sprintf(typeStr, "SLICE");
            break;
        case NALU_TYPE_DPA:
            sprintf(typeStr, "DPA");
            break;
        case NALU_TYPE_DPB:
            sprintf(typeStr, "DPB");
            break;
        case NALU_TYPE_DPC:
            sprintf(typeStr, "DPC");
            break;
        case NALU_TYPE_IDR:
            sprintf(typeStr, "IDR");
            break;
        case NALU_TYPE_SEI:
            sprintf(typeStr, "SEI");
            break;
        case NALU_TYPE_SPS:
            sprintf(typeStr, "SPS");
            break;
        case NALU_TYPE_PPS:
            sprintf(typeStr, "PPS");
            break;
        case NALU_TYPE_AUD:
            sprintf(typeStr, "AUD");
            break;
        case NALU_TYPE_EOSEQ:
            sprintf(typeStr, "EOSEQ");
            break;
        case NALU_TYPE_EOSTREAM:
            sprintf(typeStr, "EOSTREAM");
            break;
        case NALU_TYPE_FILL:
            sprintf(typeStr, "FILL");
            break;
        }
        switch(n->nal_reference_idc >> 5)
        {
        case NALU_PRIORITY_DISPOSABLE:
            sprintf(idcStr, "DISPOSABLE");
            break;
        case NALU_PRIRITY_LOW:
            sprintf(idcStr, "LOW");
            break;
        case NALU_PRIORITY_HIGH:
            sprintf(idcStr, "HIGH");
            break;
        case NALU_PRIORITY_HIGHEST:
            sprintf(idcStr, "HIGHEST");
            break;
        }
        fprintf(myOut, "%5d| %8d| %7s| %8s|\n", nalNum, dataOffset, idcStr, typeStr, n->len);
        dataOffset = dataOffset + dataLength;
        nalNum++;
    }
    
    //free
    if(n)
    {
        if(n->buf)
        {
            free(n->buf);
            n->buf = NULL;
        }
        free(n);
        n = NULL;
    }
    return 0;
}

int main()
{
    simplest_h264_parser("sintel.h264");
    return 0;
}
