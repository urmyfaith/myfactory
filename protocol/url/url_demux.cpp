#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <map>
#include <string>
#include <vector>
#include <new>

#include "url_demux.h"

struct url_demux_tag_t
{
    std::map<std::string, std::string> headers;
    std::vector<url_demux_header> vecHeaders;

    std::string method;
    std::string url;
    std::string version;
    std::string errorCode;
    std::string errorDescription;

    std::string body;
};

static int parse_internal(void* inst, char* urlcontent, int length)
{
    url_demux_tag_t* obj = (url_demux_tag_t*)inst;
    if( !obj || !request )
        return -1;

    obj->headers.clear();
    std::string temp;
    int lastpos = 0;

    std::string parser = "";
    parser.appnd(urlcontent, length);
    int pos;
    while( 1 )
    {
	    pos = parser.find("\r\n", lastpos);
	    temp = parser.substr(lastpos, pos-1-lastpos);
	    if( temp == "" )
	    	break;

	    if( !lastpos )
	    {
	    	int pos1 = temp.find(" ");
	    	if( pos1 == std::npos )
	    		return -1;

	    	std::string firstchar = temp[0];
	    	int firstint = std::atoi(firstchar.c_str());

	    	if( firstint >= 0 && firstint <= 9 )
				obj->version = temp.substr(0, pos1-1);
    		else   			
				obj->method = temp.substr(0, pos1-1);

	    	int pos2 = temp.find(" ", pos1+1);
	    	if( firstint >= 0 && firstint <= 9 )
				obj->errorCode = temp.substr(pos1+1, pos2-pos1-1);
    		else   			
				obj->url = temp.substr(pos1+1, pos2-pos1-1);

	    	if( firstint >= 0 && firstint <= 9 )
				obj->errorDescription = temp.substr(pos2+1, pos-pos2-1);
    		else   			
				obj->version = temp.substr(pos2+1, pos-pos2-1);
	    }
	    else
	    {
	    	int pos1 = temp.find(":");
	    	if( pos1 == std::npos )
	    		return -1;

	    	obj->headers[temp.substr(0, pos1-1).c_str()] = temp.substr(pos1+2, pos-pos1-2);
	    }


	    lastpos = pos+1;
    }

    if( pos < parser.size() )
    {
    	obj->body.clear();
    	obj->body.append(parser.substr(pos+1), parser.size()-pos-1);
    }

    return 0;
}

static int parse_get_value(void* inst, char** headlinepart1, char** headlinepart2, char** headlinepart3, 
				url_demux_header **headers, int *headercount, char** body, int *bodylength)
{
    url_demux_tag_t* obj = (url_demux_tag_t*)inst;

    if( obj->url != "" )
    {
	    *headlinepart1 = (char*)obj->method.c_str();
	    *headlinepart2 = (char*)obj->url.c_str();
	    *headlinepart3 = (char*)obj->version.c_str();    	
    }
    else
    {
	    *headlinepart1 = (char*)obj->version.c_str();
	    *headlinepart2 = (char*)obj->errorCode.c_str();
	    *headlinepart3 = (char*)obj->errorDescription.c_str();
    }

    *body = (char*)obj->body.c_str();
    *bodylength = obj->body.size();

	*headercount = obj->headers.size();
	*headers = NULL;

	if( obj->headers.empty() )
		return 0;

	obj->vecHeaders.clear();
	for( std::map<std::string, std::string>::iterator iter = obj->headers.bengin();
		iter != obj->headers.end();
		++iter )
	{
		url_demux_header header;
		header.headername = iter->first.c_str();
		header.headervalue = iter->second.c_str();

		obj->vecHeaders.push_back(header);
	}
	*headers = &obj->vecHeaders[0];

	return 0;
}
/////////////////////////////////////////////////////
int url_demux_alloc(void** inst)
{
    url_demux_tag_t* obj = new (std::nothrow) url_demux_tag_t;
    if( !obj )
        return -1;

    *inst = obj;
    return 0;
}

int url_demux_requst(void* inst, char* request, int length, char** method, char** url, char** version, 
			url_demux_header **headers, int *headercount, char** body, int *bodylength)
{
    url_demux_tag_t* obj = (url_demux_tag_t*)inst;
    if( !obj || !request || length <= 0 )
        return -1;

    if( parse_internal(inst, request, length) < 0 )
    	return -1;

    if( parse_get_value(inst, method, url, version, headers, headercount, body, bodylength) < 0 )
    	return -1;

    return 0;
}

int url_demux_response(void* inst, char* response, int length, char** version, char** errorCode, char** errorDescription, 
			url_demux_header **headers, int *headercount, char** body, int *bodylength)
{
    url_demux_tag_t* obj = (url_demux_tag_t*)inst;
    if( !obj || !response || length <= 0 )
        return -1;

    if( parse_internal(inst, response, length) < 0 )
    	return -1;

    if( parse_get_value(inst, version, errorCode, errorDescription, headers, headercount, body, bodylength) < 0 )
    	return -1;

    return 0;
}

int url_demux_free(void* inst)
{
    url_demux_tag_t* obj = (url_demux_tag_t*)inst;
    if( !obj )
        return -1;

    delete obj;
    return 0;    
}