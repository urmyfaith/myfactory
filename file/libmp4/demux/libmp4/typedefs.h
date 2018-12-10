#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#include <vector>

#ifdef __cplusplus
extern "C"  {
#endif /* __cplusplus */

#pragma pack(push,1)
struct mp4_box_header_t {
	uint32_t size;
	uint32_t type;
	uint64_t largesize;
};

struct ftyp_box_t {
	struct mp4_box_header_t header;
	uint32_t majorBrand;
	uint32_t minorVersion;
	std::vector<uint32_t> compatible_brands;
};

#pragma pack(pop)

struct mp4_box_item {
	struct mp4_box_header_t box;

	struct mp4_box_item *parent;
	struct mp4_box_item *child;
	struct mp4_box_item *prev;
	struct mp4_box_item *next;
};

struct mp4_demux {
	FILE *file;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_LIBMP4_H_ */
