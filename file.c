#include <linux/uio.h>
#include "header_myfs.h"
ssize_t myfs_get_loffset(struct myfs_inode *mi, loff_t off)
{
	ssize_t ret = MYFS_EMPTY_ENTRY;
	loff_t add = 0;
	u32 i = 0;

	if (off > MYFS_DEFAULT_BSIZE)
		add += MYFS_DEFAULT_BSIZE % off;

	for (i = 0; i < MYFS_INODE_TSIZE; ++i) {
		if (mi->i_addrb[i] + off > mi->i_addre[i]) {
			off -= (mi->i_addre[i] - mi->i_addrb[i]);
		} else {
			ret = mi->i_addrb[i] + off;
			break;
		}
	}

	BUG_ON(ret == 0xdeadbeef);

	return ret;
}

ssize_t myfs_read(struct kiocb *iocb, struct iov_iter *to)
{
	struct super_block *sb;
	struct inode *inode;
	struct myfs_inode *minode;
	struct buffer_head *bh;
	char *buffer;
	void *buf = to->iov->iov_base;
	int nbytes;
	size_t count = iov_iter_count(to);
	loff_t off = iocb->ki_pos;
	loff_t end = off + count;
	size_t blk = 0;


	inode = iocb->ki_filp->f_path.dentry->d_inode;
	sb = inode->i_sb;
	minode = inode->i_private;
	if (off) {
		return 0;
	}

	/* calculate datablock number here */
	blk = myfs_get_loffset(minode, off);
	bh = sb_bread(sb, blk);
	if (!bh) {
        	printk(KERN_ERR "Failed to read data block %lu\n", blk);
		return 0;
	}

	buffer = (char *)bh->b_data + (off % MYFS_DEFAULT_BSIZE);
	nbytes = min((size_t)(minode->i_size - off), count);

	if (copy_to_user(buf, buffer, nbytes)) {
		brelse(bh);
		printk(KERN_ERR
			"Error copying file content to userspace buffer\n");
		return -EFAULT;
	}

	brelse(bh);
	iocb->ki_pos += nbytes;

	return nbytes;
}

ssize_t myfs_alloc_if_necessary(struct myfs_superblock *sb, struct myfs_inode *di,
				loff_t off, size_t cnt)
{
	// Mock it until using bitmap
	return 0;
}

ssize_t myfs_write(struct kiocb *iocb, struct iov_iter *from)
{
	struct super_block *sb;
	struct inode *inode;
	struct myfs_inode *minode;
	struct buffer_head *bh;
	struct myfs_superblock *dsb;
	void *buf = from->iov->iov_base; 
	loff_t off = iocb->ki_pos;
	size_t count = iov_iter_count(from);
	size_t blk = 0;	
	size_t boff = 0;
	char *buffer;
	int ret;

	inode = iocb->ki_filp->f_path.dentry->d_inode;
	sb = inode->i_sb;
	minode = inode->i_private;
	dsb = sb->s_fs_info;
	
	ret = generic_write_checks(iocb, from);
	if (ret <= 0) {
	    printk(KERN_INFO "DummyFS: generic_write_checks Failed: %d", ret); 
	    return ret;
	}
	
	/* calculate datablock to write alloc if necessary */
	blk = myfs_alloc_if_necessary(dsb, minode, off, count);
	/* dummy files are contigous so offset can be easly calculated */
	boff = myfs_get_loffset(minode, off);
	bh = sb_bread(sb, boff);
	if (!bh) {
	    printk(KERN_ERR "Failed to read data block %lu\n", blk);
	    return 0;
	}
	
	buffer = (char *)bh->b_data + (off % MYFS_DEFAULT_BSIZE);
	if (copy_from_user(buffer, buf, count)) {
	    brelse(bh);
	    printk(KERN_ERR
	           "Error copying file content from userspace buffer "
	           "to kernel space\n");
	    return -EFAULT;
	}
	
	iocb->ki_pos += count; 
	
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);
	
	minode->i_size = max((size_t)(minode->i_size), count);

	myfs_store_inode(sb, minode);
	
	return count;
}