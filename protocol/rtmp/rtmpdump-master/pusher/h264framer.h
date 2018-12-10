#ifndef __H264FRAMER__H__
#define __H264FRAMER__H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    
#include <fstream>

typedef struct {
    int FPS;
    int Width;
    int Height;
    std::fstream fs;

    char databuffer[2 * 1000 * 1000];
    int offset;
    char buffer[100 * 1024];
}H264FileDesc_t;

int H264Framer_Init(H264FileDesc_t &desc);

int H264Framer_ReadData(H264FileDesc_t &desc);

int H264Framer_GetFrame(H264FileDesc_t &desc, char *buffer, int buflength);

#endif