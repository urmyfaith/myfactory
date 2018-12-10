#ifndef _TSMUX_H__
#define _TSMUX_H__

#ifdef WIN32
#include <basetsd.h>
typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;
#define __LITTLE_ENDIAN		1
#define __BIG_ENDIAN		2
#define __BYTE_ORDER		1
#else
#include <inttypes.h>
#include <endian.h>
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#include "typedefs.h"
#include "pes.h"
#elif __BYTE_ORDER == __BIG_ENDIAN
#error "Temporarily not support big-endian systems."
#else
#error "Please fix <endian.h>"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* 接口函数，将帧流化。必须指定TsProgramInfo							*/
/************************************************************************/

// 返回流化后TS的长度，出错（如dest空间不足）返回-1。 
int lts_ts_stream(TEsFrame *frame, uint8_t *dest, int maxlen, TsProgramInfo *pi);

#ifdef __cplusplus
}
#endif

#endif //__LITETS_TSSTREAM_H__
