#ifndef _MYFS_H_
#define _MYFS_H_


#define MYFS_MAGIC                  0x24081996
#define MYFS_DEFAULT_BLOCK_SIZE     4096
#define MYFS_FILENAME_MAXLEN        255

const int MYFS_ROOT_INODE_NUMBER = 1;
const int MYFS_ROOTDIR_DATABLOCK_NUMBER = 2;

struct myfs_dir_record {
	char filename[MYFS_FILENAME_MAXLEN];
	uint32_t inode_no;
};

struct myfs_inode {
	mode_t mode;
	uint32_t inode_no;
	uint32_t data_block_number;

	union {
		uint32_t file_size;
		uint32_t dir_children_count;
	};
};

struct myfs_super_block {
	uint32_t version;
	uint32_t magic;
	uint32_t block_size;
	uint32_t free_blocks;

	struct myfs_inode root_inode;

	char padding[MYFS_DEFAULT_BLOCK_SIZE - (4 * sizeof(uint32_t)) - sizeof(struct myfs_inode)];
};

/*
struct myfs_dir_contents {
	uint32_t children_count;
	struct myfs_dir_record records[];
}; */

#endif /* _MYFS_H_*/