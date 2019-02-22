#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _POSIX_SOURCE 1

#define FALSE 0
#define TRUE 1

#define FILE_NAME "mmw_output.bin"

#define BUFF_LEN 2048
#define BAUDRATE B9600 //

unsigned char packet_buf[BUFF_LEN]; // frame buffer (no magic 8 bytes)
unsigned int packet_pos = 0; // frame buffer position

unsigned int magic_lost = 1;

unsigned char f_run = TRUE;

int find_magic(unsigned char *buf, unsigned int len)
{
    int n, offset;

    if (len < 8) {
        printf("[%s,%d] len is too small (%d)\n", __FUNCTION__, __LINE__, len);
        return -1;
    }

    for (n=0; n<len-8; n++) {
        if ((buf[n] == 0x02) && (buf[n+1] == 0x01) && (buf[n+2] == 0x04) && (buf[n+3] == 0x03) &&
            (buf[n+4] == 0x06) && (buf[n+5] == 0x05) && (buf[n+6] == 0x08) && (buf[n+7] == 0x07)) {
            offset = n+8;
            printf("[%s,%d] offset( %d )\n", __FUNCTION__, __LINE__, offset);
            //printf(">> %02x %02x %02x %02x %02x %02x %02x %02x\n\n", buf[n], buf[n+1], buf[n+2], buf[n+3], buf[n+4], buf[n+5], buf[n+6], buf[n+7]);

            return offset;
        }
    }

    return -1;
}

void reset_packet(unsigned char *buf, unsigned int len)
{
    memset(buf, 0, len);
    packet_pos = 0;
}

static void parseDetectedObjects(unsigned char *buf, unsigned int len)
{
    unsigned short numDetectedObj, xyzQFormat, rangeIdx, dopplerIdx, peakVal;
    short x, y, z;
    unsigned int i, n=0;

    memcpy(&numDetectedObj, &buf[n], sizeof(unsigned short)); n += sizeof(unsigned short);
    printf(">> numDetectedObj : %d\n", numDetectedObj);
    memcpy(&xyzQFormat, &buf[n], sizeof(unsigned short)); n += sizeof(unsigned short);
    printf(">> xyzQFormat : %d\n", xyzQFormat);

    for (i=0; i<numDetectedObj; i++) {
        printf(">> Object ID : %d\n", i);
        memcpy(&rangeIdx, &buf[n], sizeof(unsigned short)); n += sizeof(unsigned short);
        printf("\t rangeIdx : %d\n", rangeIdx);
        memcpy(&dopplerIdx, &buf[n], sizeof(unsigned short)); n += sizeof(unsigned short);
        printf("\t dopplerIdx : %d\n", dopplerIdx);
        memcpy(&peakVal, &buf[n], sizeof(unsigned short)); n += sizeof(unsigned short);
        printf("\t peakVal : %d\n", peakVal);
        memcpy(&x, &buf[n], sizeof(short)); n += sizeof(short);
        printf("\t x : %8x (%07.3f)\n", x, (float)(x*1.0/(1 << xyzQFormat)));
        memcpy(&y, &buf[n], sizeof(short)); n += sizeof(short);
        printf("\t y : %8x (%07.3f)\n", y, (float)(y*1.0/(1 << xyzQFormat)));
        memcpy(&z, &buf[n], sizeof(short)); n += sizeof(short);
        printf("\t z : %8x (%07.3f)\n", z, (float)(z*1.0/(1 << xyzQFormat)));
    }
    printf("\n");

    return;
}

static void parseRangeProfile(unsigned char *buf, unsigned int len)
{
    unsigned short rangeProfile;
    unsigned int i, n=0;

    for (i=0; i<256; i++) {
        memcpy(&rangeProfile, &buf[n], sizeof(unsigned short)); n += sizeof(unsigned short);
        printf("\t rangeProfile[%3d] : \t%07.3f\n", i, (float)(rangeProfile*1.0*6/8/(1<<8)));
    }
    printf("\n");

    return;
}

static void parseStats(unsigned char *buf, unsigned int len)
{
    unsigned int interProcess, transmitOut, frameMargin, chirpMargin, activeCPULoad, interCPULoad;
    unsigned int n=0;

    memcpy(&interProcess, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf("\t>> interProcess : %x (%d)\n", interProcess, interProcess);
    memcpy(&transmitOut, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf("\t>> transmitOut : %x (%d)\n", transmitOut, transmitOut);
    memcpy(&frameMargin, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf("\t>> frameMargin : %x (%d)\n", frameMargin, frameMargin);
    memcpy(&chirpMargin, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf("\t>> chirpMargin : %x (%d)\n", chirpMargin, chirpMargin);
    memcpy(&activeCPULoad, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf("\t>> activeCPULoad : %x (%d)\n", activeCPULoad, activeCPULoad);
    memcpy(&interCPULoad, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf("\t>> interCPULoad : %x (%d)\n", interCPULoad, interCPULoad);

    printf("\n");

    return;
}

void parse_packet(unsigned char *buf, unsigned int len)
{
    unsigned int Version, Length, Platform, frameNum, cpuCycles, numObj, numTLV, subFrameNum;
    unsigned int tlvType, tlvLength;
    unsigned int paddingBytes = 0;
    unsigned int i, n=0;

    printf("[%s,%d] len( %d )\n", __FUNCTION__, __LINE__, len);
    //printf(">> %02x %02x %02x %02x %02x %02x %02x %02x\n\n", packet_buf[0], packet_buf[1], packet_buf[2], packet_buf[3], packet_buf[4], packet_buf[5], packet_buf[6], packet_buf[7]);

    memcpy(&Version, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf(">> (%4d) Version : %x\n", n, Version);
    memcpy(&Length, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf(">> (%4d) Length : %x (%d)\n", n, Length, Length);
    memcpy(&Platform, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf(">> (%4d) Platform : %x\n", n, Platform);
    memcpy(&frameNum, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf(">> (%4d) frameNum : %x (%d)\n", n, frameNum, frameNum);
    memcpy(&cpuCycles, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf(">> (%4d) cpuCycles : %x\n", n, cpuCycles); // time in cpu cycles when message created
    memcpy(&numObj, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf(">> (%4d) numObj : %x (%d)\n", n, numObj, numObj);
    memcpy(&numTLV, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf(">> (%4d) numTLV : %x (%d)\n", n, numTLV, numTLV);
    memcpy(&subFrameNum, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
    printf(">> (%4d) subFrameNum : %x (%d)\n", n, subFrameNum, subFrameNum);
    printf("\n");

    for (i=0; i<numTLV; i++) {
        memcpy(&tlvType, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
        printf(">>(%d) (%4d) tlvType : %x (%d)\n", i, n, tlvType, tlvType);
        memcpy(&tlvLength, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
        printf(">>(%d) (%4d) tlvLength : %x (%d)\n", i, n, tlvLength, tlvLength);

        if (tlvType == 1) {
            printf("[%s,%d] parseDetectedObjects( %d )\n", __FUNCTION__, __LINE__, n);
            parseDetectedObjects(&buf[n], tlvLength);
            n += tlvLength;
        }
        else if (tlvType == 2) {
            printf("[%s,%d] parseRangeProfile( %d )\n", __FUNCTION__, __LINE__, n);
            parseRangeProfile(&buf[n], tlvLength);
            n += tlvLength;
        }
        else if (tlvType == 6) {
            printf("[%s,%d] parseStats( %d )\n", __FUNCTION__, __LINE__, n);
            parseStats(&buf[n], tlvLength);
            n += tlvLength;
        }
        else {
            printf("[%s,%d] undefined type( %d ) length( %d )\n", __FUNCTION__, __LINE__, tlvType, tlvLength);
            n += tlvLength;
        }
        printf("\n");
    }

    paddingBytes = ((n+8)/32+1)*32-(n+8);
    if (paddingBytes == 32) paddingBytes = 0;

    printf("[%s,%d] Length( %d ), n( %d ), paddingBytes( %d ) ( %d )\n\n", __FUNCTION__, __LINE__, Length, n, paddingBytes, Length-(n+8));

    return;
}

static void fill_buffer(unsigned char *buf, unsigned int len)
{
    memcpy(&packet_buf[packet_pos], buf, len);
    packet_pos += len;

    if (packet_pos > BUFF_LEN) {
        printf("[%s,%d] packet_pos( %d ) -> END !!\n\n", __FUNCTION__, __LINE__, packet_pos);
        // end
        f_run = FALSE;
    }
}

int main(int argc, char *argv[])
{
    int res, n;
    int offset, length;
    unsigned char buf[512];
    FILE* fd;

    if (argc < 2) {
        printf("Usage: %s FILE_NAME\n", argv[0]);
        return 0;
    }

    fd = fopen(/*FILE_NAME*/argv[1], "r");
    if (fd == NULL) {
        printf("file( %s ) open error\n", /*FILE_NAME*/argv[1]);
        exit(-1);
    }

    printf("[%s,%d] call reset_packet()\n", __FUNCTION__, __LINE__);
    reset_packet(packet_buf, BUFF_LEN);
    printf("Reset_packet done \n");
    while (f_run == TRUE)
    {

        res = fread((unsigned char *)buf, sizeof(unsigned char), 512, fd);

        
        offset = find_magic(buf, res);
        printf("Read : %s\n", buf);
        if (res < 8) {
            printf("[%s,%d] res( %d ) -> END !!\n\n", __FUNCTION__, __LINE__, res);
            f_run = FALSE;
        }

        if (magic_lost) {
            if (offset > 0) {
                length = res - offset;
                /*===========*/
                magic_lost = 0;
                /*===========*/
                printf("[%s,%d] call fill_buffer(%d, %d)\n", __FUNCTION__, __LINE__, offset, length);
                fill_buffer(&buf[offset], length);
            }
            else {
                // do not copy
                printf("[%s,%d] call reset_packet()\n", __FUNCTION__, __LINE__);
                reset_packet(packet_buf, BUFF_LEN);
            }
        }
        else {
            if (offset > 0) {
                length = res - offset;
                if (offset > 8) {
                    printf("[%s,%d] call fill_buffer(%d, %d)\n", __FUNCTION__, __LINE__, offset, length);
                    fill_buffer(&buf[0], offset-8);
                }
                // one packet complete
                printf("[%s,%d] call parse_packet()\n", __FUNCTION__, __LINE__);
                parse_packet(packet_buf, packet_pos);

                printf("[%s,%d] call reset_packet()\n", __FUNCTION__, __LINE__);
                reset_packet(packet_buf, BUFF_LEN);

                printf("[%s,%d] call fill_buffer(%d, %d)\n", __FUNCTION__, __LINE__, offset, length);
                fill_buffer(&buf[offset], length);
            }
            else {
                offset = 0;
                length = res;
                // copy to frame buffer
                printf("[%s,%d] call fill_buffer(%d, %d)\n", __FUNCTION__, __LINE__, offset, length);
                fill_buffer(&buf[offset], length);
            }
        }
    }

    fclose(fd);

    return 0;
}
