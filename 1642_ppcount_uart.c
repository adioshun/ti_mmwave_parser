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

unsigned char packet_buf[BUFF_LEN]; // frame buffer (no magic 8 bytes)
unsigned int packet_pos = 0; // frame buffer position

unsigned int magic_lost = 1;

unsigned char f_run = TRUE;

int find_magic(unsigned char *buf, unsigned int len)
{
    int n, offset;

    if (len < 8) {
        //printf("[%s,%d] len is too small (%d)\n", __FUNCTION__, __LINE__, len);
        return -1;
    }

    for (n=0; n<len-8; n++) {
        if ((buf[n] == 0x02) && (buf[n+1] == 0x01) && (buf[n+2] == 0x04) && (buf[n+3] == 0x03) &&
            (buf[n+4] == 0x06) && (buf[n+5] == 0x05) && (buf[n+6] == 0x08) && (buf[n+7] == 0x07)) {
            offset = n+8;
            //printf("[%s,%d] offset( %d )\n", __FUNCTION__, __LINE__, offset);
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


static void parseTargetList(unsigned char *buf, unsigned int len)
{

    unsigned int tid;
    float posX, posY, velX, velY, accX, accY, EC1, EC2,EC3,EC4,EC5,EC6,EC7,EC8,EC9, g;

    unsigned int i, numDetectedObg, n=0;

    numDetectedObg=(len/68);
    printf("\nNum of Detected Object : %d (%d)\n",numDetectedObg, len );

    for (i=0; i<(numDetectedObg); i++){
        memcpy(&tid, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
        memcpy(&posX, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&posY, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&velX, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&velY, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&accX, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&accY, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC1, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC2, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC3, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC4, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC5, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC6, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC7, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC8, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&EC9, &buf[n], sizeof(float)); n += sizeof(float);
        memcpy(&g, &buf[n], sizeof(float)); n += sizeof(float);
        printf(">> ID : %d [%4.2f, %4.2f]\n", tid,posX, posY);
    }
    return;
}

static void parsePoint(unsigned char *buf, unsigned int len)
{
    return;
}

static void parseTargetIndex(unsigned char *buf, unsigned int len)
{
    return;
}

void parse_packet(unsigned char *buf, unsigned int len)
{
    
    unsigned int version, platform, cpuCycles, Length, frameNum, subframeNumber, chirpMargin, frameMargin, uartSentTime, trackProcessTime, numTLV, checksum;
    unsigned int tlvType, tlvLength;
    unsigned int paddingBytes = 0;
    unsigned int i, n=0;

    //printf("[%s,%d] len( %d )\n", __FUNCTION__, __LINE__, len);
 
   memcpy(&version, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&platform, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&cpuCycles, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&Length, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&frameNum, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&subframeNumber, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&chirpMargin, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&frameMargin, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&uartSentTime, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&trackProcessTime, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
   memcpy(&numTLV, &buf[n], sizeof(unsigned short)); n += sizeof(unsigned short);
   memcpy(&checksum, &buf[n], sizeof(unsigned short)); n += sizeof(unsigned short);

  

    for (i=0; i<numTLV; i++) {
        memcpy(&tlvType, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
        //printf(">>(%d) (%4d) tlvType : %x (%d)\n", i, n, tlvType, tlvType);
        memcpy(&tlvLength, &buf[n], sizeof(unsigned int)); n += sizeof(unsigned int);
        //printf(">>(%d) (%4d) tlvLength : %x (%d)\n", i, n, tlvLength, tlvLength);

        if (tlvType == 6) {  //parsePoint
            //printf("<-[%s,%d] parsePoint( %d )\n", __FUNCTION__, __LINE__, n);
            parsePoint(&buf[n], tlvLength-8);  
            n += tlvLength;
        }
        else if (tlvType == 7) { //parseTargetList  
            //printf("<-[%s,%d] parseTargetList( %d )\n", __FUNCTION__, __LINE__, n);
            parseTargetList(&buf[n], tlvLength-8);
            n += tlvLength;
        }
        else if (tlvType == 8) { //parseTargetIndex
            //printf("<-[%s,%d] parseTargetIndex( %d )\n", __FUNCTION__, __LINE__, n);
            parseTargetIndex(&buf[n], tlvLength-8);
            n += tlvLength;
        }
        else {
            printf("[%s,%d] undefined type( %d ) length( %d )\n", __FUNCTION__, __LINE__, tlvType, tlvLength); //에러 발생 부분
            n += tlvLength;
            return; //에러 처리를 위해 임시 지정
        }
        printf("\n");
    }

    paddingBytes = ((n+8)/32+1)*32-(n+8);       //불확실한 부분
    if (paddingBytes == 32) paddingBytes = 0;   //불확실한 부분
    printf("\n");
    //printf("[%s,%d] Length( %d ), n( %d ), paddingBytes( %d ) ( %d )\n\n", __FUNCTION__, __LINE__, Length, n, paddingBytes, Length-(n+8));

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

    while (f_run == TRUE)
    {
        res = fread((unsigned char *)buf, sizeof(unsigned char), 512, fd);
        offset = find_magic(buf, res);

        if (res < 8) {
            //printf("[%s,%d] res( %d ) -> END !!\n\n", __FUNCTION__, __LINE__, res);
            f_run = TRUE;//FALSE;
        }

        if (magic_lost) {
            if (offset > 0) {
                length = res - offset;
                /*===========*/
                magic_lost = 0;
                /*===========*/
                //printf("[%s,%d] call fill_buffer(%d, %d)\n", __FUNCTION__, __LINE__, offset, length);
                fill_buffer(&buf[offset], length);
            }
            else {
                // do not copy
                //printf("[%s,%d] call reset_packet()\n", __FUNCTION__, __LINE__);
                reset_packet(packet_buf, BUFF_LEN);
            }
        }
        else {
            if (offset > 0) {
                length = res - offset;
                if (offset > 8) {
                    //printf("[%s,%d] call fill_buffer(%d, %d)\n", __FUNCTION__, __LINE__, offset, length);
                    fill_buffer(&buf[0], offset-8);
                }
                // one packet complete
                //printf("[%s,%d] call parse_packet()\n", __FUNCTION__, __LINE__);
                parse_packet(packet_buf, packet_pos);

                //printf("[%s,%d] call reset_packet()\n", __FUNCTION__, __LINE__);
                reset_packet(packet_buf, BUFF_LEN);

                //printf("[%s,%d] call fill_buffer(%d, %d)\n", __FUNCTION__, __LINE__, offset, length);
                fill_buffer(&buf[offset], length);
            }
            else {
                offset = 0;
                length = res;
                // copy to frame buffer
                //printf("[%s,%d] call fill_buffer(%d, %d)\n", __FUNCTION__, __LINE__, offset, length);
                fill_buffer(&buf[offset], length);
            }
        }
    }

    fclose(fd);

    return 0;
}
