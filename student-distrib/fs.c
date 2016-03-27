
#include "fs.h"

static filesys_t *file_system;

static int32_t valid_inode(int32_t inode){
 	if(inode < 0 || inode >= NUM_INODES){
 		return 0;
 	}
 	return 1;
 }

 /* Initialize filesystem */
 void fs_init(module_t *mem_mod){
	file_system =  (filesys_t*)mem_mod->mod_start;
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

 	for(i = 0; i < NUM_INODES; i++){
 		// probably have to change third arg in strncmp to min str length so no seg fault
 		if(!strncmp((int8_t *) fname, (int8_t *) file_system->bootblock.dentry[i].fname, FNAME_LEN)){
 			dentry_t* dentry_ptr = &file_system->bootblock.dentry[i];
 			for(j = 0; j < FNAME_LEN; j++){
 				dentry -> fname[j] = dentry_ptr -> fname[j];
 			}
 			dentry -> ftype = dentry_ptr -> ftype;
 			dentry -> inode = dentry_ptr -> inode;
 			for(j = 0; j < 6; j++){
 				dentry -> pad[j] = dentry_ptr -> pad[j];
 			}
 			return 0;
 		}
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

 	if(valid_inode(index)){
 		dentry_t* dentry_ptr = &file_system->bootblock.dentry[index];
 		for(i = 0; i < FNAME_LEN; i++){
 			dentry -> fname[i] = dentry_ptr -> fname[i];
 		}
 		dentry -> ftype = dentry_ptr -> ftype;
 		dentry -> inode = dentry_ptr -> inode;
 		for(i = 0; i < 6; i++){
 			dentry -> pad[i] = dentry_ptr -> pad[i];
 		}
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
 	int32_t curr_data_block; // data to copy to buf
 	int32_t off_data_block; // data block number in inode
 	int32_t new_offset; // offset once inside correct data block
 	bytes_read = 0;

 	if(valid_inode(inode)){
 		if(!(offset >= file_system->inodes[inode].length)){

 			/* calculate correct data block and offset to start copying from */
 			new_offset = offset % CHARS_PER_BLOCK;
 			off_data_block = (offset - new_offset) / CHARS_PER_BLOCK;
 			curr_data_block = file_system->inodes[inode].data_block[off_data_block];

 			/* copy data to buf */
 			for(i = 0; i < length; i++){
 				buf[i] = file_system->data_blocks[curr_data_block].data[new_offset++];
 				bytes_read++;
				/* move to next data block */
 				if(new_offset >= CHARS_PER_BLOCK){ 
 					curr_data_block = file_system->inodes[inode].data_block[off_data_block++];
 					new_offset = 0;
 				}
 			}
 		}
 		
 	}
 	return bytes_read;
 }

 /* filesystem system calls */

 int32_t fs_halt (uint8_t status){
 	return -1;
 }

 int32_t fs_execute (const uint8_t* command){
 	return -1;
 }

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

 int32_t fs_getargs (uint8_t* buf, int32_t nbytes){
 	return -1;
 }

 int32_t fs_vidmap (uint8_t** screen_start){
 	return -1;
 }

 int32_t fs_set_handler (int32_t signum, void* handler_address){
 	return -1;
 }

 int32_t fs_sigreturn (void){
 	return -1;
 }
