#include <fstream>
#include "filewritter.h"

struct filewritter_tag_t
{
	std::fstream fs;
};

void *filewritter_alloc(char* filepath, int isbinary)
{
	filewritter_tag_t *inst = new filewritter_tag_t;
	if( !inst )
		return NULL;

	if( isbinary )
		inst->fs.open(filepath, std::ios::binary|std::ios::out);
	else
		inst->fs.open(filepath, std::ios::out);

	return inst;
}

int filewritter_write(void *handle, char* data, int length)
{
	filewritter_tag_t *inst = (filewritter_tag_t*)handle;

	inst->fs.write(data, length);

	return 0;
}

int filewritter_free(void *handle)
{
	filewritter_tag_t *inst = (filewritter_tag_t*)handle;

	if( inst->fs.is_open() )
		inst->fs.close();

	return 0;
}