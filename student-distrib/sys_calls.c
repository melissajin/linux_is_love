
#include "sys_calls.h"
#include "fs.h"
#include "virtualmem.h"
#include "lib.h"

#define PROG_VM_START 0x8000000
#define SPACE_4MB     0x400000

int32_t halt (uint8_t status){ return -1; }
int32_t execute (const uint8_t* command) {
    uint8_t* args[1024];
    uint8_t buf[40];
    dentry_t dentry;
    uint32_t addr;
    uint32_t vm_end;
    parse_arg(command, (uint8_t**)args);

    /* check for valid executable */
    if(-1 == read_dentry_by_name(args[0], &dentry))
		return -1; 
	if(40 > read_data(dentry.inode, 0, buf, 40))
		return -1;
	if(buf[0] == 0x7f && buf[1] == 0x45 && buf[2] == 0x4c && buf[3] == 0x46){

		addr = ((uint32_t)buf[26] << 24) | ((uint32_t)buf[25] << 16) | ((uint32_t)buf[24] << 8)| ((uint32_t)buf[23]);
		vm_end = PROG_VM_START + SPACE_4MB;
		//map_large_page(PROG_VM_START, PROG_VM_START + /*process number*/ SPACE_4MB);
		
		/* load file in physical memory */
		load(dentry.inode, (uint8_t*) addr);
		
		asm volatile("\n\
			"
			:
			:"d"(addr), "b"(vm_end)
		);
	}
	return -1;
}

int32_t read (int32_t fd, void* buf, int32_t nbytes){ return -1; }
int32_t write (int32_t fd, const void* buf, int32_t nbytes){ return -1;}
int32_t open (const uint8_t* filename){ return -1; }
int32_t close (int32_t fd){ return -1; }
int32_t getargs (uint8_t* buf, int32_t nbytes){ return -1; }
int32_t vidmap (uint8_t** screen_start){ return -1; }
int32_t set_handler (int32_t signum, void* handler_address){ return -1; }
int32_t sigreturn (void){ return -1; }

void parse_arg(const uint8_t* command, uint8_t** args){
	uint8_t buf[1026];
    uint8_t* scan;
    uint32_t n_arg;

    if (1023 < strlen((int8_t*)command))
    	return ;
    strcpy((int8_t*)buf, (int8_t*)command);
    for (scan = buf; '\0' != *scan && ' ' != *scan && '\n' != *scan; scan++);
    	args[0] = (uint8_t*)buf;
    n_arg = 1;
    if ('\0' != *scan) {
    	*scan++ = '\0';
        /* parse arguments */
    	while (1) {
    		while (' ' == *scan) scan++;
    		if ('\0' == *scan || '\n' == *scan) {
    			*scan = '\0';
    			break;
    		}
    		args[n_arg++] = (uint8_t*)scan;
    		while ('\0' != *scan && ' ' != *scan && '\n' != *scan) scan++;
    		if ('\0' != *scan)
    			*scan++ = '\0';
    	}
    }
    args[n_arg] = NULL;
	return;
}
