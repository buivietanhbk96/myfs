#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/parser.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>

#include "header_myfs.h"

static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *root_inode;
    struct buffer_head *bh;
    struct myfs_superblock *m_sb;
    struct myfs_inode *root_myfsinode, *rbuf;
    int ret = 0;

    bh = sb_bread(sb,MYFS_SUPER_OFFSET);
    m_sb = (struct myfs_superblock *)bh->b_data;
    printk(KERN_INFO "The magic number obtained in disk is: [%d]\n",
            m_sb->s_magic);
	printk(KERN_INFO
	       "myfs filesystem of version [%d] formatted with a block size of [%d] detected in the device.\n",
	       m_sb->s_version, m_sb->s_blocksize);
    /* A magic number that uniquely identifies our filesystem type */
    sb->s_magic = m_sb->s_magic;
	sb->s_blocksize = m_sb->s_blocksize;
	sb->s_op = &myfs_sb_ops;
	sb->s_fs_info = m_sb;
	bforget(bh);

    bh = sb_bread(sb, MYFS_ROOT_INODE_OFFSET);
    rbuf = (struct myfs_inode *)bh->b_data;
    root_myfsinode = cache_get_inode();
    memcpy(root_myfsinode, rbuf, sizeof(*rbuf));
    root_inode = new_inode(sb);

    
    /* Fill inode with myfs info */
	root_inode->i_mode = root_myfsinode->i_mode;

	root_inode->i_flags = root_myfsinode->i_flags;
	root_inode->i_ino = root_myfsinode->i_ino;
	root_inode->i_sb = sb;
	root_inode->i_atime = current_time(root_inode);
	root_inode->i_ctime = current_time(root_inode);
	root_inode->i_mtime = current_time(root_inode);
	root_inode->i_ino = MYFS_ROOT_INO;
    root_inode->i_op = &myfs_inode_ops;
    root_inode->i_fop = &myfs_dir_ops;
	root_inode->i_private = root_myfsinode;

    sb->s_root = d_make_root(root_inode);
    if(!sb->s_root)
    {
        ret = -ENOMEM;
        goto release;
    }
release:
	brelse(bh);
	return ret;
}
struct dentry *myfs_mount(struct file_system_type *fs_type,
                      int flags, const char *dev_name,
                      void *data)
{
    struct dentry *ret;
    ret = mount_bdev(fs_type, flags, dev_name, data, myfs_fill_super);
    if(unlikely(IS_ERR(ret)))
        printk(KERN_ERR "Error mounting myfs");
    else
        printk(KERN_INFO "myfs is sucessfully mounted on [%s]\n",dev_name);
    return ret;
}

void myfs_kill_superblock(struct super_block *sb)
{
    printk(KERN_INFO "myfs superblock is destroyed. Unmount succesful.\n");
    /* do meaningful operations here */
    kill_block_super(sb);
    return;
}
void myfs_put_super(struct super_block *sb) {
	return;
}