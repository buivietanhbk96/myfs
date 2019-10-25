/*
 * hello_module.c - The simplest kernel module.
 */
#include<linux/module.h>   /* Need by all modules */
#include<linux/kernel.h>   /* Need for KERN_INFO */
#include<linux/init.h>     /* Need for the macros */
#include<linux/fs.h>       /* Need for filesystem */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VANH");
MODULE_DESCRIPTION("My first hello module!!!");
MODULE_VERSION("0.1");

struct inode *myfs_get_inode(struct super_block *sb,
        const struct inode *dir, umode_t mode,
        dev_t dev)
{
    struct inode *inode = new_inode(sb);
    if(inode)
    {
        inode->i_ino = get_next_ino();
        inode_init_owner(inode, dir, mode);

        //inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
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

    /* A magic number that uniquely identifies our filesystem type */
    sb->s_magic = 0x25102019; /* today =)) */

    inode = myfs_get_inode(sb, NULL, S_IFDIR, 0);
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
