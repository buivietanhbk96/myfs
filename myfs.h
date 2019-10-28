#ifndef _MYFS_H_
#define _MYFS_H_
const int MYFS_MAGIC = 0x24081996;
const int MYFS_DEFAULT_BLOCK_SIZE = 4 * 1024;

struct myfs_super_block {
	uint32_t version;
	uint32_t magic;
	uint32_t block_size;
	uint32_t free_blocks;

	char padding[ (4 * 1024) - (4 * sizeof(unsigned int))];
};





#endif /* _MYFS_H_*/