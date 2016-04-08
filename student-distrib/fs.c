
#include "fs.h"

static bootblock_t* bootblock;
 
 /* Initialize filesystem */
 void fs_init(module_t *mem_mod){
	bootblock = (bootblock_t*)mem_mod->mod_start;
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
	int i;
 	dentry_t* curr_dentry; //iterate through dentries
	curr_dentry = &(bootblock->dentry[0]);

 	for(i = 0; i < bootblock->dir_entries_cnt; i++){	
 		if((strlen((int8_t*)fname) == strlen((int8_t *) curr_dentry->fname) || strlen((int8_t*)fname) >= FNAME_LEN)){
			//we !strcmp because strcmp returns 0 on a match
 			if(!strncmp((int8_t *) fname, (int8_t *) curr_dentry->fname, strlen((int8_t*)curr_dentry->fname))){ 
				memcpy(dentry, curr_dentry, BYTES_DENTRY);
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
 	dentry_t* curr_dentry; //iterate through dentries

 	if(index >= 0 && index < bootblock->dir_entries_cnt){
 		curr_dentry = &(bootblock->dentry[index]);
		memcpy(dentry, curr_dentry, BYTES_DENTRY);
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
 	inode_t *curr_inode;
 	bytes_read = 0;
	
	if(inode == DIRECTORY_INODE){
		bytes_read = read_directory(offset, buf, length);
	}
	
 	else if(inode > DIRECTORY_INODE  && inode < bootblock->inode_cnt){

 	// calculate correct data block and offset to start copying from
	new_offset = offset % CHARS_PER_BLOCK;
	off_data_block = offset / CHARS_PER_BLOCK;
	curr_inode = (inode_t*)((uint8_t*) bootblock + ((inode+1) * BLOCK_SIZE));
	curr_data_block = (data_block_t*) ((uint8_t*) bootblock + (1 + bootblock->inode_cnt + curr_inode->data_block[off_data_block])*BLOCK_SIZE);

 		// copy data to buf
 		for(i = 0; i < length; i++){
 			//check for EOF
 			if(bytes_read + offset < curr_inode->length){
 				buf[i] = curr_data_block->data[new_offset];
 				bytes_read++;
 				new_offset++;
				// move to next data block
 				if(new_offset >= CHARS_PER_BLOCK){ 
					off_data_block++;
					curr_data_block = (data_block_t*) ((uint8_t*) bootblock + (1 + bootblock->inode_cnt + curr_inode->data_block[off_data_block])*BLOCK_SIZE);
 					new_offset = 0;
 				}
 			}
 		}
 	}
 	return bytes_read;
 }

 /* read_directory
 *	  DESCRIPTION: copies over 'length' bytes of directory entries into 'buf'
 *    INPUTS: buf - buffer to be filled by the bytes read from the file
 			  length - number of bytes to read
 *    OUTPUTS: none
 *    RETURN VALUE: number of bytes read and placed in the buffer
 *    SIDE EFFECTS: fills the first arg (buf) with the bytes read from
 *					the file
 */
 uint32_t read_directory(uint32_t offset, uint8_t* buf, uint32_t length){
 	uint32_t i,k;
 	uint32_t buf_idx = 0;
	uint32_t buf_off = 0;
	uint32_t ret_val = 0;
 	dentry_t dentry;

 	for(k = 0; k < bootblock->dir_entries_cnt; k++){
 		read_dentry_by_index(k, &dentry);
 		for(i = 0; i < strlen((int8_t*)dentry.fname); i++){
			if(buf_off>= offset && ret_val < length){
 				buf[buf_idx++] = dentry.fname[i]; 
				ret_val++;
			}
			else{
				buf_off++;
			}
 		}
		if(buf_off>= offset && ret_val<length){
 			buf[buf_idx++]= '\n';
		}
 	}
	
	buf[buf_idx] = '\0';
	return ret_val;
}

 /* load
 *	  DESCRIPTION: loads contents of the file 'fname' into memory at address 'addr'
 *    INPUTS: fname - filename specifying the file to read from
 			  addr - address to copy contents of file to
 *    OUTPUTS: none
 *    RETURN VALUE: 0 on success, -1 on failure
 *    SIDE EFFECTS: copies contents of the specified file into the memoy
 *					address.
 */
int32_t load(const uint8_t* fname, uint32_t addr){
	dentry_t dentry;
	uint32_t len;

	if(read_dentry_by_name(fname, &dentry) == 0){
		len = dentry.inode.length
		uint8_t buf[len];
		
		if(read_data(dentry.inode, 0, buf, len) == 0){
			/* check for executable file */
			if(buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F'){
				memcpy(addr, buf, len);
				return 0;
			}
		}
	}
	
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
