/*
 *
 */

 #ifndef _FS_H
 #define _FS_H

 #include "../types.h"
 #include "../lib.h"

 #define NUM_INODES 63
 #define NUM_DATA_BLOCKS 756 /* arbitrary num of data blocks */
 #define CHARS_PER_BLOCK (4096/8) /* 4kB block/8 bytes per char */
 #define FNAME_LEN 32

 typedef struct dentry {
 	int8_t fname[FNAME_LEN];
 	int32_t ftype;
 	int32_t inode;
 	int32_t pad[6];
 } dentry_t;

 typedef struct bootblock {
 	int32_t dir_entries;
 	int32_t inode_cnt;
 	int32_t data_block_cnt;
 	int32_t pad[13];
 	dentry_t dentry[NUM_INODES];
 } bootblock_t;
 
 typedef struct inode {
 	int32_t length;
 	int32_t data_block[12]; /* number based on lecture slides */
 } inode_t;

 typedef struct data_block {
 	uint8_t data[CHARS_PER_BLOCK];
 } data_block_t;

 /* variables to define the filesystem */
 static bootblock_t *bootblock;
 static inode_t *inodes[NUM_INODES];
 static data_block_t *data_blocks[NUM_DATA_BLOCKS];

 /* Initialize filesystem */
 extern void fs_init();

 /* Reads a dentry by filename, returns pointer to dentry block */
 int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

 /* Reads a dentry by index number, returns pointer to dentry block */
 int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
 
 /* Reads data in dentry starting from offset */
 int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

 /* Filesystem system calls */
 int32_t fs_read (int32_t fd, void* buf, int32_t nbytes);
 int32_t fs_write (int32_t fd, const void* buf, int32_t nbytes);
 int32_t fs_open (const uint8_t* filename);
 int32_t fs_close (int32_t fd);

 static int32_t valid_inode(int32_t inode){
 	if(inode < 0 || inode >= NUM_INODES){
 		return 0;
 	}
 	return 1;
 }


 #endif /* _FS_H */
