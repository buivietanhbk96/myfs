#include "header_myfs.h"
#define FOREACH_BLK_IN_EXT(myi, blk)					\
u32 _ix = 0, b = 0, e = 0;						\
for (_ix = 0, b = myi->i_addrb[0], e = myi->i_addre[0], blk = b-1;	\
_ix < MYFS_INODE_TSIZE;							\
++_ix, b = myi->i_addrb[_ix], e = myi->i_addre[_ix], blk = b-1)		\
	while (++blk < e)

struct myfs_inode *cache_get_inode(void)
{
    struct myfs_inode *di;

	di = kmem_cache_alloc(myfs_inode_cache, GFP_KERNEL);
	printk(KERN_INFO "#: myfs cache_get_inode : di=%p\n", di);

	return di;
}


void cache_put_inode(struct myfs_inode **di)
{
	printk(KERN_INFO "#: myfs cache_put_inode : di=%p\n", *di);
	kmem_cache_free(myfs_inode_cache, *di);
	*di = NULL;
}

void myfs_destroy_inode(struct inode *inode) {
	struct myfs_inode *di = inode->i_private;

	printk(KERN_INFO "#: myfs freeing private data of inode %p (%lu)\n",
		di, inode->i_ino);
	cache_put_inode(&di);
}

void myfs_store_inode(struct super_block *sb, struct myfs_inode *myfsi)
{
	struct buffer_head *bh;
	struct myfs_inode *in_core;
	uint32_t blk = myfsi->i_addrb[0] - 1;

	/* put in-core inode */
	/* Change me: here we just use fact that current allocator is cont.
	 * With smarter allocator the position should be found from itab
	 */
	bh = sb_bread(sb, blk);
	BUG_ON(!bh);

	in_core = (struct myfs_inode *)(bh->b_data);
	memcpy(in_core, myfsi, sizeof(*in_core));

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);
}

/* Here introduce allocation for directory... */
int myfs_add_dir_record(struct super_block *sb, struct inode *dir,
			struct dentry *dentry, struct inode *inode)
{
	struct buffer_head *bh;
	struct myfs_inode *parent, *myfsi;
	struct myfs_dir_entry *dir_rec;
	u32 blk, j;

	parent = dir->i_private;
	myfsi = parent;

	// Find offset, in dir in extends
	FOREACH_BLK_IN_EXT(parent, blk)
    {
		bh = sb_bread(sb, blk);
		BUG_ON(!bh);
		dir_rec = (struct myfs_dir_entry *)(bh->b_data);
		for (j = 0; j < sb->s_blocksize; ++j) {
			/* We found free space */
			if (dir_rec->inode_nr == MYFS_EMPTY_ENTRY) {
				dir_rec->inode_nr = inode->i_ino;
				dir_rec->name_len = strlen(dentry->d_name.name);
				memset(dir_rec->name, 0, 256);
				strcpy(dir_rec->name, dentry->d_name.name);
				mark_buffer_dirty(bh);
				sync_dirty_buffer(bh);
				brelse(bh);
				parent->i_size += sizeof(*dir_rec);
				return 0;
			}
			dir_rec++;
		}
		/* Move to another block */
		bforget(bh);
	}

	printk(KERN_ERR "Unable to put entry to directory");
	return -ENOSPC;
}

int alloc_inode(struct super_block *sb, struct myfs_inode *myfsi)
{
	struct myfs_superblock *dsb;
	u32 i;

	dsb = sb->s_fs_info;
	dsb->s_inode_cnt += 1;
	myfsi->i_ino = dsb->s_inode_cnt;
	myfsi->i_version = 1; //ver 1.0
	myfsi->i_flags = 0;
	myfsi->i_mode = 0;
	myfsi->i_size = 0;

	/* TODO: check if there is any space left on the device */
	/* First block is allocated for in-core inode struct */
	/* Then 4 block for extends: that mean myfsi struct is in i_addrb[0]-1 */
	myfsi->i_addrb[0] = dsb->s_last_blk + 1;
	myfsi->i_addre[0] = dsb->s_last_blk += 4;
	for (i = 1; i < MYFS_INODE_TSIZE; ++i) {
		myfsi->i_addre[i] = 0;
		myfsi->i_addrb[i] = 0;
	}

	myfs_store_inode(sb, myfsi);
	isave_intable(sb, myfsi, (myfsi->i_addrb[0] - 1));
	/* TODO: update inode block bitmap */

	return 0;
}

struct inode *myfs_new_inode(struct inode *dir, struct dentry *dentry,
				umode_t mode)
{
	struct super_block *sb;
	struct myfs_superblock *dsb;
	struct myfs_inode *di;
	struct inode *inode;
	int ret;

	sb = dir->i_sb;
	dsb = sb->s_fs_info;

	di = cache_get_inode();

	/* allocate space myfsy way:
 	 * sb has last block on it just use it
 	 */
	ret = alloc_inode(sb, di);

	if (ret) {
		cache_put_inode(&di);
		printk(KERN_ERR "Unable to allocate disk space for inode");
		return NULL;
	}
	di->i_mode = mode;

	BUG_ON(!S_ISREG(mode) && !S_ISDIR(mode));

	/* Create VFS inode */
	inode = new_inode(sb);

	myfs_fill_inode(sb, inode, di);

	/* Add new inode to parent dir */
	ret = myfs_add_dir_record(sb, dir, dentry, inode);

	return inode;
}

int myfs_add_ondir(struct inode *inode, struct inode *dir, struct dentry *dentry,
			umode_t mode)
{
	inode_init_owner(inode, dir, mode);
	d_add(dentry, inode);

	return 0;
}

int myfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	return myfs_create_inode(dir, dentry, mode);
}

int myfs_create_inode(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct inode *inode;

	/* allocate space
	 * create incore inode
	 * create VFS inode
	 * finally ad inode to parent dir
	 */
	inode = myfs_new_inode(dir, dentry, mode);

	if (!inode)
		return -ENOSPC;

	/* add new inode to parent dir */
	return myfs_add_ondir(inode, dir, dentry, mode);
}

int myfs_mkdir(struct inode *dir, struct dentry *dentry,
			umode_t mode)
{
	int ret = 0;

	ret = myfs_create_inode(dir, dentry,  S_IFDIR | mode);

	if (ret) {
		printk(KERN_ERR "Unable to allocate dir");
		return -ENOSPC;
	}

	dir->i_op = &myfs_inode_ops;
	dir->i_fop = &myfs_dir_ops;

	return 0;
}

void myfs_put_inode(struct inode *inode)
{
	struct myfs_inode *ip = inode->i_private;

	cache_put_inode(&ip);
}

int isave_intable(struct super_block *sb, struct myfs_inode *myfsi, u32 i_block)
{
	struct buffer_head *bh;
	struct myfs_inode *itab;
	u32 blk = 0;
	u32 *ptr;

	/* get inode table 'file' */
	bh = sb_bread(sb, MYFS_INODE_TABLE_OFFSET);
	itab = (struct myfs_inode*)(bh->b_data);
	/* right now we just allocated one itable extend for files */
	blk = itab->i_addrb[0];
	bforget(bh);

	bh = sb_bread(sb, blk);
	/* Get block of ino inode*/
	ptr = (u32 *)(bh->b_data);
	/* inodes starts from index 1: -2 offset */
	*(ptr + myfsi->i_ino - 2) = i_block;

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	return 0;
}

struct myfs_inode *myfs_iget(struct super_block *sb, ino_t ino)
{
	struct buffer_head *bh;
	struct myfs_inode *ip;
	struct myfs_inode *minode;
	struct myfs_inode *itab;
	u32 blk = 0;
	u32 *ptr;

	/* get inode table 'file' */
	bh = sb_bread(sb, MYFS_INODE_TABLE_OFFSET);
	itab = (struct myfs_inode*)(bh->b_data);
	/* right now we just allocated one itable extend for files */
	blk = itab->i_addrb[0];
	bforget(bh);

	bh = sb_bread(sb, blk);
	/* Get block of ino inode*/
	ptr = (u32 *)(bh->b_data);
	/* inodes starts from index 1: -2 offset */
	blk = *(ptr + ino - 2);
	bforget(bh);

	bh = sb_bread(sb, blk);
	ip = (struct myfs_inode*)bh->b_data;
	if (ip->i_ino == MYFS_EMPTY_ENTRY)
		return NULL;
	minode = cache_get_inode();
	memcpy(minode, ip, sizeof(*ip));
	bforget(bh);

	return minode;
}

void myfs_fill_inode(struct super_block *sb, struct inode *des, struct myfs_inode *src)
{
	des->i_mode = src->i_mode;
	des->i_flags = src->i_flags;
	des->i_sb = sb;
	des->i_atime = des->i_ctime = des->i_mtime = current_time(des);
	des->i_ino = src->i_ino;
	des->i_private = src;
	des->i_op = &myfs_inode_ops;

	if (S_ISDIR(des->i_mode))
		des->i_fop = &myfs_dir_ops;
	else if (S_ISREG(des->i_mode))
		des->i_fop = &myfs_file_ops;
	else {
		des->i_fop = NULL;
	}

	WARN_ON(!des->i_fop);
}
struct dentry *myfs_lookup(struct inode *dir,
                              struct dentry *child_dentry,
                              unsigned int flags)
{
	struct myfs_inode *mparent = dir->i_private;
	struct myfs_inode *mchild;
	struct super_block *sb = dir->i_sb;
	struct buffer_head *bh;
	struct myfs_dir_entry *dir_rec;
	struct inode *ichild;
	u32 j = 0, i = 0;

	/* Here we should use cache instead but myfs is doing stuff in dummy way.. */
	for (i = 0; i < MYFS_INODE_TSIZE; ++i) {
		u32 b = mparent->i_addrb[i] , e = mparent->i_addre[i];
		u32 blk = b;
		while (blk < e) {

			bh = sb_bread(sb, blk);
			BUG_ON(!bh);
			dir_rec = (struct myfs_dir_entry *)(bh->b_data);

			for (j = 0; j < sb->s_blocksize; ++j) {
				if (dir_rec->inode_nr == MYFS_EMPTY_ENTRY) {
					break;
				}

				if (0 == strcmp(dir_rec->name, child_dentry->d_name.name)) {
					mchild = myfs_iget(sb, dir_rec->inode_nr);
					ichild = new_inode(sb);
					if (!mchild) {
						return NULL;
					}
					myfs_fill_inode(sb, ichild, mchild);
					inode_init_owner(ichild, dir, mchild->i_mode);
					d_add(child_dentry, ichild);

				}
				dir_rec++;
			}

			/* Move to another block */
			blk++;
			bforget(bh);
		}
	}
	return NULL;
}