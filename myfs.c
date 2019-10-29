/*
 * hello_module.c - The simplest kernel module.
 */
#include<linux/module.h>   /* Need by all modules */
#include<linux/kernel.h>   /* Need for KERN_INFO */
#include<linux/init.h>     /* Need for the macros */
#include<linux/fs.h>       /* Need for filesystem */
#include<linux/time.h>     /* For get current time */
#include<linux/buffer_head.h> 

#include"header_myfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VANH");
MODULE_DESCRIPTION("My first hello filesystem !!!");
MODULE_VERSION("0.1");
struct kmem_cache *dmy_inode_cache = NULL;
struct file_system_type myfs_fs_type = 
{
    .owner = THIS_MODULE,
    .name = "myfs",
    .mount = myfs_mount,
    .kill_sb = myfs_kill_superblock,
    .fs_flags = FS_REQUIRES_DEV,
};


const struct file_operations myfs_dir_operations = 
{
    .owner = THIS_MODULE,
    .iterate_shared = myfs_readdir,
};
const struct inode_operations myfs_inode_ops = 
{
    .lookup = myfs_lookup,
    .create = myfs_create,
	.mkdir = myfs_mkdir,
};
const struct super_operations myfs_sb_ops =
{
	.destroy_inode = myfs_destroy_inode,
	.put_super = myfs_put_super,
};

const struct file_operations myfs_file_ops =
{
	.read_iter = myfs_read,
	.write_iter = myfs_write,
};


static int __init myfs_init(void)
{
    int ret;
    myfs_inode_cache = kmem_cache_create("myfs_inode_cache",
                    sizeof(struct myfs_inode),
                    0,
                    (SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD),
                    NULL);

    if (!myfs_inode_cache)
        return -ENOMEM;
    ret = register_filesystem(&myfs_fs_type);
    if(0 == ret)
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
    if(0 == ret)
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
