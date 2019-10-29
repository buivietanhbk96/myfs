#ifndef _KERN_MYFS_H
#define _KERN_MYFS_H

#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>

#include "myfs.h"

extern const struct super_operations myfs_sb_ops;
extern const struct inode_operations myfs_inode_ops;
extern const struct file_operations myfs_dir_ops;
extern const struct file_operations myfs_file_ops;

extern struct kmem_cache *myfs_inode_cache;

struct dentry *myfs_mount(struct file_system_type *ft, int f, const char *dev, void *d);
int myfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);
struct dentry *myfs_lookup(struct inode *dir, struct dentry *child_dentry, unsigned int flags);

/* file.c */
ssize_t myfs_read(struct kiocb *iocb, struct iov_iter *to);
ssize_t myfs_write(struct kiocb *iocb, struct iov_iter *from);

/* dir.c */
int myfs_readdir(struct file *filp, struct dir_context *ctx);

/* inode.c */
int isave_intable(struct super_block *sb, struct myfs_inode *myfsi, u32 i_block);
void myfs_destroy_inode(struct inode *inode);
void myfs_fill_inode(struct super_block *sb, struct inode *des, struct myfs_inode *src);
int myfs_create_inode(struct inode *dir, struct dentry *dentry, umode_t mode);
int myfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
void myfs_store_inode(struct super_block *sb, struct myfs_inode *myfsi);

/* inode cache */
struct myfs_inode *cache_get_inode(void);
void cache_put_inode(struct myfs_inode **di);

/* super.c */
void myfs_put_super(struct super_block *sb);
void myfs_kill_superblock(struct super_block *sb);

#endif /* _KERN_MYFS_H */ 
