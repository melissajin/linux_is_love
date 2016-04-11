
#include "sys_calls.h"
#include "fs.h"
#include "virtualmem.h"
#include "lib.h"
#include "process.h"
#include "x86_desc.h"

#define PROG_VM_START   0x8000000
#define KERNEL_MEM_END  0x800000
#define SPACE_4MB       0x400000
#define PCB_SIZE        0x2000
#define LIVE            0x1
#define DEAD            0x0
#define ESP_MASK        0xFFFFF000

int32_t halt (uint8_t status){ return -1; }
int32_t execute (const uint8_t* command) {
    uint8_t* args[1024];
    uint8_t buf[40];
    dentry_t dentry;
    uint32_t addr;
    uint32_t esp;
    uint32_t vm_end;
    pcb_t* pcb;
    fd_t stdin;
    fd_t stdout;
    fd_t fd;
    uint32_t i;
    int32_t pid;
    uint32_t pcb_start;
    fops_t * term_fops;

    pid = add_process();
    if(pid < 0)
        return -1;

    parse_arg(command, (uint8_t**)args);

    printf("%s\n", args[0]);
    /* check for valid executable */
    if(-1 == read_dentry_by_name(args[0], &dentry))
		return -1; 
	if(40 > read_data(dentry.inode, 0, buf, 40))
		return -1;
	if(buf[0] == 0x7f && buf[1] == 0x45 && buf[2] == 0x4c && buf[3] == 0x46){
        printf("executable\n");
		addr = ((uint32_t)buf[27] << 24) | ((uint32_t)buf[26] << 16) | ((uint32_t)buf[25] << 8)| ((uint32_t)buf[24]);
		printf("addr: %x\n", addr);
        vm_end = PROG_VM_START + SPACE_4MB;
		map_large_page(PROG_VM_START, KERNEL_MEM_END + pid*SPACE_4MB);
		
        /* get current stack pointer and put in */
        asm volatile (
            "movl %%esp, %0"
            :"=rm" (esp)
        );
        
        /* starting address of current pcb */
        pcb_start = esp & ESP_MASK; 

        /* get terminal fops */
        term_fops = get_device_fops("term");

        /* setting the pcb in the kernel stack */
        pcb = (pcb_t*) KERNEL_MEM_END - (pid + 1) * PCB_SIZE;
        stdin.fops = term_fops;
        stdin.inode = NULL;
        stdin.pos = 0;
        stdin.flags = LIVE;
        pcb->files[0] = stdin;

        stdout.fops = term_fops;
        stdout.inode = NULL;
        stdout.pos = 1;
        stdout.flags = LIVE;
        pcb->files[1] = stdout;

        for(i = 2; i < 8; i++){
            fd.fops = NULL;
            fd.inode = NULL;
            fd.pos = i;
            fd.flags = DEAD;
            pcb->files[i] = fd;
        }

        pcb->pid = pid;
        pcb->parent_pcb = (pcb_t *) pcb_start;
        /* saving values in tss to return to parent process */
        tss.esp0 = esp;
        tss.ss0 = pcb_start;

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
