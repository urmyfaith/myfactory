#include <fstream>
#include "filereader.h"

struct filereader_tag_t
{
	std::fstream fs;
};

void *filereader_alloc(char* filepath, int isbinary)
{
	filereader_tag_t *inst = new filereader_tag_t;
	if( !inst )
		return NULL;

	if( isbinary )
		inst->fs.open(filepath, std::ios::binary|std::ios::in);
	else
		inst->fs.open(filepath, std::ios::in);

	return inst;
}

int filereader_write(void *handle, char* data, int length)
{
	filewritter_tag_t *inst = (filewritter_tag_t*)handle;

	inst->fs.read(data, length);
	return desc->fs.gcount();
}

int filewritter_free(void *handle)
{
	filewritter_tag_t *inst = (filewritter_tag_t*)handle;

	if( inst->fs.is_open() )
		inst->fs.close();

	return 0;
}