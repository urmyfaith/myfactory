#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <map>
#include <string>
#include <vector>
#include <new>

#include "sdp_demux.h"

struct sdp_demux_header_tag_t
{
	std::string headername;
	std::string headervalue;
};

struct sdp_demux_tag_t
{
    std::vector<sdp_demux_header_tag_t> vecInternalHeaders;

    std::vector<sdp_demux_header> vecHeaders;
};

static int parse_internal(void* inst, char* content, int contentlength)
{
    sdp_demux_tag_t* obj = (sdp_demux_tag_t*)inst;

    std::string parser;
    parser.append(content, contentlength);

    int lastpos = 0;
    obj->vecInternalHeaders.clear();
    while( 1 )
    {
    	int pos = parser.find("\r\n", lastpos);
    	if( pos == std::npos )
    		break;

    	std::string temp = parser.substr(lastpos, pos-lastpos-1);
    	int pos1 = temp.find("=");
    	sdp_demux_header_tag_t header;
    	header.headername = temp.substr(0, pos1-1);
    	header.headervalue = temp.substr(pos1+1, pos-lastpos-1-pos1-1);
    	obj->vecInternalHeaders.push_back(header);
    }

    return 0;
}

static int parse_get_value(void* inst, sdp_demux_header** headers, int *headercount)
{
    sdp_demux_tag_t* obj = (sdp_demux_tag_t*)inst;

    *headers = NULL;
    *headercount = 0;

    if( obj->vecInternalHeaders.empty() )
    	return 0;

    obj->vecHeaders.clear();
    for( std::vector<sdp_demux_header_tag_t>::iterator iter = obj->vecInternalHeaders.begin();
    	iter != ob->vecInternalHeaders.end();
    	++iter )
    {
    	sdp_demux_header header;
    	header.headername = iter->headername;
    	header.headervalue = iter->headervalue;

	    obj->vecHeaders.push_back(header);
    }
    *headers = &obj->vecHeaders[0];
    *headercount = obj->vecHeaders.size();

    return 0;
}
///////////////////////////////////////////////////////
int sdp_demux_alloc(void** inst)
{
    sdp_demux_tag_t* obj = new (std::nothrow) sdp_demux_tag_t;
    if( !obj )
        return -1;

    *inst = obj;
    return 0;
}

int sdp_demux(void* inst, char* content, int contentlength, sdp_demux_header** headers, int *headercount)
{
    sdp_demux_tag_t* obj = (sdp_demux_tag_t*)inst;
    if( !obj || !content || contentlength <= 0 )
        return -1;

    if( parse_internal(inst, content, contentlength ) <= 0 )
    	return -1;

    if( parse_get_value(inst, headers, headercount ) <= 0 )
    	return -1;

    return 0;    
}

int sdp_demux_free(void* inst)
{
    sdp_demux_tag_t* obj = (sdp_demux_tag_t*)inst;
    if( !obj )
        return -1;

    delete obj;
    return 0;    
}