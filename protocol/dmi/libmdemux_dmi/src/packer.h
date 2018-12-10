#ifndef __PACKER_H__
#define __PACKER_H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string>    
#include "utils.h"

std::string GetRequestLine(DMICInfo_t &dmic, const char* method, const char* streamid, int version);

std::string GetHeader(const char* headername, const char* headervalue);
std::string GetHeader(const char* headername, float headervalue);

int DMIDRequest_Marshal(int version, char* payload, int payloadsize, char *buffer, int bufsize);

#endif