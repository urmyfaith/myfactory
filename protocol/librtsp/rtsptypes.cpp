#include "rtsptypes.h"

static const char *statusCode10x[] = 
{
    "Continue", // 100
    "Switching Protocols", // 101
    "Processing", // 102
};

static const char *statusCode20x[] = 
{
    "OK", // 200
    "Created", // 201
    "Accepted", // 202
    "Non-Authoritative Information", // 203
    "No Content", // 204
    "Reset Content", // 205
    "Partial Content", // 206
    "Multi-Status", // 207
};

static const char *statusCode30x[] = 
{
    "Multiple Choices", // 300
    "Moved Permanently", // 301
    "Move temporarily", // 302
    "See Other", // 303
    "Not Modified", // 304
    "Use Proxy", // 305
    "Switch Proxy", // 306
    "Temporary Redirect", // 307
};

static const char *statusCode40x[] = 
{
    "Bad Request", // 400
    "Unauthorized", // 401
    "Payment Required", // 402
    "Forbidden", // 403
    "Not Found", // 404
    "Method Not Allowed", // 405
    "Not Acceptable", // 406
};

static const char *statusCode45x[] = 
{
    "Parameter Not Understood", // 451
    "Conference Not Found", // 452
    "Not Enough Bandwidth", // 453
    "Session Not Found", // 454
    "Method Not Valid in This State", // 455
    "Header Field Not Valid for Resource", // 456
    "Invalid Range", // 457
    "Parameter Is Read-Only", // 458
    "Aggregate Operation Not Allowed", // 459
    "Only Aggregate Operation Allowed", // 460
    "Unsupported Transport", // 461
    "Destination Unreachable", // 462
};

static const char *statusCode50x[] = 
{
    "Internal Server Error", // 500
    "Not Implemented", // 501
    "Bad Gateway", // 502
    "Service Unavailable", // 503
    "Gateway Timeout", // 504
    "HTTP Version Not Supported", // 505
    "Variant Also Negotiates", // 506
    "Insufficient Storage", // 507
    "Bandwidth Limit Exceeded", // 508
};

int rtsp_get_error_info(int code, const char** errorinfo)
{


    if(451 <= code && code < 451+sizeof(statusCode45x)/sizeof(statusCode45x[0]))
        *errorinfo = statusCode45x[code-451];

    switch(code)
    {
    case 505:
        *errorinfo = "RTSP Version Not Supported";
        break;
    case 551:
        *errorinfo = "Option not supported";
        break;
    default:
        return -1;
    }

    return 0;
}
