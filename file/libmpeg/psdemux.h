#ifndef __PSDEMUX_H__
#define __PSDEMUX_H__

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
#include "pes.h"
#include "typedefs.h"
#elif __BYTE_ORDER == __BIG_ENDIAN
#error "Temporarily not support big-endian systems."
#else
#error "Please fix <endian.h>"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// PS码流解复用，成功返回已处理长度，失败返回-1
// ps_buf是传入的PS流的缓冲
int lts_ps_demux(TDemux *handle, uint8_t *ps_buf, int len);

#ifdef __cplusplus
}
#endif

#endif //__LITETS_TSSTREAM_H__
