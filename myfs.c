/*
 * hello_module.c - The simplest kernel module.
 */
#include<linux/module.h>   /* Need by all modules */
#include<linux/kernel.h>   /* Need for KERN_INFO */
#include<linux/init.h>     /* Need for the macros */
#include<linux/fs.h>       /* Need for filesystem */
#include<linux/time.h>     /* For get current time */
#include<linux/buffer_head.h> 

#include"myfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VANH");
MODULE_DESCRIPTION("My first hello filesystem !!!");
MODULE_VERSION("0.1");


static int myfs_iterate(struct file *filp, struct dir_context *ctx)
{
    /* ls will list nothing as of now */
    return 0;
}
const struct file_operations myfs_dir_operations = 
{
    .owner = THIS_MODULE,
    .iterate = myfs_iterate,
};

struct dentry *myfs_lookup(struct inode *parent_inode,
                struct dentry *child_dentry, unsigned int flags)
{
    /* The lookup function is used for dentry association.
     * As of now, we don't deal with dentries in myfs
     * So we keep this simple for now and revisit later */
    return NULL;
}
static struct inode_operations myfs_inode_ops = 
{
    .lookup = myfs_lookup,
};

struct inode *myfs_get_inode(struct super_block *sb,
        const struct inode *dir, umode_t mode,
        dev_t dev)
{
    struct inode *inode = new_inode(sb);
    if(inode)
    {
        inode->i_ino = get_next_ino();
        inode_init_owner(inode, dir, mode);

        inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
        switch (mode & S_IFMT)
        {
            case S_IFDIR:
                inc_nlink(inode);
                break;
            case S_IFREG:
            case S_IFLNK:
            default:
                printk(KERN_ERR "myfs can create meaningfull inode for only root dir at the moment\n");
                return NULL;
                break;
        }
    }
    return inode;
}
int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *inode;
    struct buffer_head *bh;
    struct myfs_super_block *sb_disk;

    bh = (struct buffer_head *)sb_bread(sb,0);
    sb_disk = (struct myfs_super_block *)bh->b_data;
    printk(KERN_INFO "The magic number obtained in disk is: [%d]\n",
            sb_disk->magic);
	if (unlikely(sb_disk->magic != MYFS_MAGIC)) {
		printk(KERN_ERR
		       "The filesystem that you try to mount is not of type myfs. Magicnumber mismatch.");
		return -EPERM;
	}

	if (unlikely(sb_disk->block_size != MYFS_DEFAULT_BLOCK_SIZE)) {
		printk(KERN_ERR
		       "myfs seem to be formatted using a non-standard block size.");
		return -EPERM;
	}

	printk(KERN_INFO
	       "myfs filesystem of version [%d] formatted with a block size of [%d] detected in the device.\n",
	       sb_disk->version, sb_disk->block_size);
    /* A magic number that uniquely identifies our filesystem type */
    sb->s_magic = MYFS_MAGIC; /* today =)) */

    inode = myfs_get_inode(sb, NULL, S_IFDIR, 0);
    inode->i_op = &myfs_inode_ops;
    inode->i_fop = &myfs_dir_operations;
    sb->s_root = d_make_root(inode);
    if(!sb->s_root)
    {
        return -ENOMEM;
    }
    return 0;
}

static struct dentry *myfs_mount(struct file_system_type *fs_type,
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

static void myfs_kill_superblock(struct super_block *s)
{
    printk(KERN_INFO "myfs superblock is destroyed. Unmount succesful.\n");
    /* do meaningful operations here */
    return;
}

struct file_system_type myfs_fs_type = 
{
    .owner = THIS_MODULE,
    .name = "myfs",
    .mount = myfs_mount,
    .kill_sb = myfs_kill_superblock,
};

static int __init myfs_init(void)
{
    int ret;
    ret = register_filesystem(&myfs_fs_type);
    if(likely(0 == ret))
    {
        printk(KERN_INFO "Sucessfully registered myfs\n");
    }
    else
    {
        printk(KERN_ERR "Failed to register myfs. Error: [%d]\n",ret);
    }
    return ret;
}

static void __exit myfs_exit(void)
{
    int ret;
    ret = unregister_filesystem(&myfs_fs_type);
    if(likely(0 == ret))
    {
        printk(KERN_INFO "Sucessfully unregistered myfs.\n");
    }
    else 
    {
        printk(KERN_ERR "Failed to unregister myfs. Error code:[%d]",ret);
    }
}

module_init(myfs_init);
module_exit(myfs_exit);
