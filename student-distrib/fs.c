
#include "fs.h"

static bootblock_t* bootblock;
 
 /* Initialize filesystem */
 void fs_init(module_t *mem_mod){
	bootblock =  (bootblock_t*)mem_mod->mod_start;
 }

/* read_dentry_by_name
 *	  DESCRIPTION: reads the dentry by filename
 *    INPUTS: fname - filename specifying the file to read from
 *			  dentry - pointer to dentry block to fill
 *    OUTPUTS: none
 *    RETURN VALUE: 0 on success, -1 on failure (nonexistant file)
 *    SIDE EFFECTS: fills the second arg (dentry) with file name, 
 * 					file type, and inode number
 */
 int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
 	int32_t i, j;
 	dentry_t* curr_dentry; //iterate through dentries
	curr_dentry = &(bootblock->dentry[0]);

 	for(i = 0; i < bootblock->dir_entries_cnt; i++){	
 		printf("fname: %s\n", curr_dentry -> fname);
 		if(strlen((int8_t*)fname) == strlen((int8_t *) curr_dentry->fname)){
 			if(!strncmp((int8_t *) fname, (int8_t *) curr_dentry->fname, strlen((int8_t*)fname))){
 				for(j = 0; j < strlen((int8_t*)fname); j++){
 					dentry -> fname[j] = curr_dentry -> fname[j];
 				}
 				dentry -> ftype = curr_dentry -> ftype;
 				dentry -> inode = curr_dentry -> inode;
 				return 0;
 			}
 		}
 		curr_dentry++;
 	}
 	return -1;
 }

/* read_dentry_by_index
 *	  DESCRIPTION: reads the dentry by index number
 *    INPUTS: index - index specifying file to read from
 *			  dentry - pointer to dentry block to fill
 *    OUTPUTS: none
 *    RETURN VALUE: 0 on success, -1 on failure (invalid index)
 *    SIDE EFFECTS: fills the second arg (dentry) with file name, 
 * 					file type, and inode number
 */ 
 int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
 	int32_t i;
 	dentry_t* curr_dentry; //iterate through dentries
 	

 	if(index > 0 && index < bootblock->dir_entries_cnt){
 		curr_dentry = &(bootblock->dentry[index]);
 		for(i = 0; i < FNAME_LEN; i++){
 			dentry -> fname[i] = curr_dentry -> fname[i];
 		}
 		dentry -> ftype = curr_dentry -> ftype;
 		dentry -> inode = curr_dentry -> inode;
 		return 0;
 	}
 	return -1;
 }
 
 /* read_data
 *	  DESCRIPTION: reads 'length' bytes starting from position 'offset'
 * 				   in the file with inode number 'inode'.
 *    INPUTS: inode - inode number specifying file to read from
 			  offset - position to start reading from in the file
 			  buf - buffer to be filled by the bytes read from the file
 			  length - number of bytes to read
 *    OUTPUTS: none
 *    RETURN VALUE: number of bytes read and placed in the buffer
 *    SIDE EFFECTS: fills the third arg (buf) with the bytes read from
 *					the file
 */
 int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
 	uint32_t i;
 	int32_t bytes_read;
 	data_block_t* curr_data_block; // data to copy to buf
 	int32_t off_data_block; // data block number in inode
 	int32_t new_offset; // offset once inside correct data block
 	inode_t* curr_inode;
 	bytes_read = 0;

 	if(inode > 0 && inode < bootblock->inode_cnt){
 		// use pointer arithmatic to calculate address of desired inode
 		curr_inode = (inode_t*) ((&bootblock) + (1 + inode)*BLOCK_SIZE);
 		printf("length: %d\n", curr_inode->length);
 		//printf("data blocks: %s\n", (int8_t*)data_blocks[inodes[inode].data_block[0]].data);
 		// calculate correct data block and offset to start copying from
 		new_offset = offset % CHARS_PER_BLOCK;
 		off_data_block = (offset - new_offset) / CHARS_PER_BLOCK;
 		curr_data_block = (data_block_t*) (&(bootblock) + (1 + bootblock->inode_cnt + off_data_block)*BLOCK_SIZE);

 		// copy data to buf
 		for(i = 0; i < length; i++){
 				//check for EOF
 			if(bytes_read + offset < curr_inode->length){
 				buf[i] = curr_data_block->data[new_offset++];
 				bytes_read++;
				// move to next data block
 				if(new_offset >= CHARS_PER_BLOCK){ 
 					curr_data_block++;
 					new_offset = 0;
 				}
 			}

 		}
 	}
 	return bytes_read;
 }

 void fs_tests(){
 	clear();
 	//dentry_t* dentry;
 	//dentry_t* dentry2;
 	uint8_t* buf;
 	//uint8_t name[10] = "frame1.txt";

	/*printf("num dir entries: %d\n", bootblock->dir_entries_cnt);
	printf("num inodes: %d\n", bootblock->inode_cnt);
	printf("num data blocks: %d\n", bootblock->data_block_cnt);

 	printf("read by index: %d\n", read_dentry_by_index(1, dentry));
 	printf("fname: %s\n", dentry -> fname);
 	printf("ftype: %d\n", dentry -> ftype);
 	printf("inode: %d\n", dentry -> inode);*/

 	// WARNING: THIS NEXT LINE GENERATES A COMPILATION ERROR
 	/*printf("read by name: %d\n", read_dentry_by_name("frame1.txt", dentry));
 	printf("fname: %s\n", dentry -> fname);
 	printf("ftype: %d\n", dentry -> ftype);
 	printf("inode: %d\n", dentry -> inode);*/

	printf("read data: %d\n", read_data(13,0,buf, 5));
 	printf("buf: %s\n", buf);
 }

 /* filesystem system calls */
 int32_t fs_read (int32_t fd, void* buf, int32_t nbytes){
 	return -1;
 }

 int32_t fs_write (int32_t fd, const void* buf, int32_t nbytes){ 
 	return -1; 
 }

 int32_t fs_open (const uint8_t* filename){ 
 	return 0; 
 }

 int32_t fs_close (int32_t fd){ 
 	return 0; 
 }
