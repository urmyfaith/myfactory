#ifndef __GLOGHELPER_H__
#define __GLOGHELPER_H__

#include <glog/logging.h>
#include <glog/raw_logging.h>

class gloghelper
{
public:
    //GLOG配置：
    gloghelper(char* program);
    //GLOG内存清理：
    ~gloghelper();

    int Log(char *str);
};

#endif