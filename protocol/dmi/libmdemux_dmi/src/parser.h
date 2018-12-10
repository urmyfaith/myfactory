#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string>    
#include <vector>

int ParseStatusCode(char *response);

int ParseHeader(char* response, char *name, std::string &value);

int DMIDHeader_UnMarshal(DMIDHeader_t* header, char *buffer);

#endif