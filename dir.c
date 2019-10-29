#include "header_myfs.h"
int myfs_readdir(struct file *filp, struct dir_context *ctx)
{
	loff_t pos = ctx->pos;
	struct inode *inode = file_inode(filp);
	struct super_block *sb = inode->i_sb;
	struct buffer_head *bh;
	struct myfs_inode *minode;
	struct myfs_dir_entry *dir_rec;
	u32 j = 0, i = 0;

	minode = inode->i_private;

	if (pos)
		return 0;
	WARN_ON(!S_ISDIR(minode->i_mode));

	/* For each extends from file */
	for (i = 0; i < MYFS_INODE_TSIZE; ++i) {
		u32 b = minode->i_addrb[i] , e = minode->i_addre[i];
		u32 blk = b;
		while (blk < e) {

			bh = sb_bread(sb, blk);
			BUG_ON(!bh);
			dir_rec = (struct myfs_dir_entry *)(bh->b_data);

			for (j = 0; j < sb->s_blocksize; j += sizeof(*dir_rec)) {
				/* We mark empty/free inodes */
				if (dir_rec->inode_nr == 0xdeeddeed) {
					break;
				}
				dir_emit(ctx, dir_rec->name, dir_rec->name_len,
					dir_rec->inode_nr, DT_UNKNOWN);
				filp->f_pos += sizeof(*dir_rec);
				ctx->pos += sizeof(*dir_rec);
				dir_rec++;
			}

			/* Move to another block */
			blk++;
			bforget(bh);
		}
	}

	return 0;
}