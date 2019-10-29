#ifndef _MYFS_H_
#define _MYFS_H_


/* FS SIZE/OFFSET CONST */
#define MYFS_INODE_TSIZE		3
#define MYFS_SUPER_OFFSET		0
#define MYFS_DEFAULT_BSIZE	4096
#define MYFS_OLT_OFFSET		(MYFS_SUPER_OFFSET + 1)
#define MYFS_INODE_TABLE_OFFSET	(MYFS_OLT_OFFSET + 1)
#define MYFS_INODE_BITMAP_OFFSET	(MYFS_INODE_TABLE_OFFSET + MYFS_INODE_TABLE_SIZE + 1)
#define MYFS_ROOT_INODE_OFFSET	(MYFS_INODE_BITMAP_OFFSET + 1)
#define MYFS_ROOT_IN_EXT_OFF	(MYFS_ROOT_INODE_OFFSET + 1)
#define MYFS_LF_INODE_OFFSET	(MYFS_ROOT_IN_EXT_OFF + MYFS_DEF_ALLOC)
/* Default place where FS will start using after mkfs (all above are used for mkfs) */
#define MYFS_FS_SPACE_START	(MYFS_LF_INODE_OFFSET + MYFS_DEF_ALLOC)

/* FS constants */
#define MYFS_MAGIC_NR		0xf00dbeef
#define MYFS_INODE_SIZE		512
#define MYFS_INODE_NUMBER_TABLE	128
#define MYFS_INODE_TABLE_SIZE	(MYFS_INODE_NUMBER_TABLE * MYFS_INODE_SIZE)/MYFS_DEFAULT_BSIZE
#define MYFS_EMPTY_ENTRY		0xdeeddeed

#define MYFS_FILENAME_MAXLEN 255
#define MYFS_DEF_ALLOC		4	/* By default alloc N blocks per extend */
/*
 * Special inode numbers 
 */
#define MYFS_BAD_INO		1 /* Bad blocks inode */
#define MYFS_ROOT_INO		2 /* Root inode nr */
#define MYFS_LAF_INO		3 /* Lost and Found inode nr */

/**
 * The on-disk superblock
 */
struct myfs_superblock {
	uint32_t	s_magic;	/* magic number */
	uint32_t	s_version;	/* fs version */
	uint32_t	s_blocksize;	/* fs block size */
	uint32_t	s_block_olt;	/* Object location table block */
	uint32_t	s_inode_cnt;	/* number of inodes in inode table */
	uint32_t	s_last_blk;	/* just move forward with allocation */
};

/**
 * Object Location Table
 */
struct myfs_olt {
	uint32_t	inode_table;		/* inode_table block location */
	uint32_t	inode_cnt;		/* number of inodes */
	uint32_t	inode_bitmap;		/* inode bitmap block */
};

/**
 * The on Disk inode
 */
struct myfs_inode {
	uint8_t		i_version;	/* inode version */
	uint8_t		i_flags;	/* inode flags: TYPE */
	uint32_t	i_mode;		/* File mode */
	uint32_t	i_ino;		/* inode number */
	uint16_t	i_uid;		/* owner's user id */
	uint16_t	i_hrd_lnk;	/* number of hard links */
	uint32_t 	i_ctime;	/* Creation time */
	uint32_t	i_mtime;	/* Modification time*/
	uint32_t	i_size;		/* Number of bytes in file */
	/* address begin - end block, range exclusive: addres end (last block) does not belogs to extend! */
	uint32_t	i_addrb[MYFS_INODE_TSIZE];	/* Start block of extend ranges */
	uint32_t	i_addre[MYFS_INODE_TSIZE];	/* End block of extend ranges */
};

struct myfs_dir_entry {
	uint32_t inode_nr;		/* inode number */
	uint32_t name_len;		/* Name length */
	char name[256];			/* File name, up to MYFS_NAME_LEN */
};

#endif /* _MYFS_H_*/