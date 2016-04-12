
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
#define ESP_MASK        0xFFFFE000
#define START_EXE_ADDR  0x08048000

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
        vm_end = PROG_VM_START + SPACE_4MB - 4;
		map_large_page(PROG_VM_START, KERNEL_MEM_END + pid*SPACE_4MB);
		
        /* get current stack pointer and put in */
        get_esp(esp);
        
        /* starting address of current pcb */
        pcb_start = esp & ESP_MASK;

        /* get terminal fops */
        term_fops = get_device_fops("term");

        /* setting the pcb in the kernel stack */
        pcb = (pcb_t*) (KERNEL_MEM_END - (pid + 1) * PCB_SIZE);
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
        tss.esp0 = KERNEL_MEM_END - 4;
        tss.ss0 = KERNEL_DS;

		/* load file in physical memory */
		load(&dentry, (uint8_t*) START_EXE_ADDR);
		
		asm volatile("\n\
			xorl    %%ecx, %%ecx \n\
            movw    $0x23, %%cx   \n\
            movw    %%cx, %%ds \n\
            movw    %%cx, %%es \n\
            movw    %%cx, %%fs \n\
            movw    %%cx, %%gs \n\
            pushl   $0x2B  \n\
            pushl   %1 \n\
            pushf   \n\
            orl     $0x200, (%%esp)  \n\
            pushl   $0x23 \n\
            pushl   %0 \n\
            iret \n\
            halt_ret_label: \n\
            "
			:
			: "r" (addr), "r" (vm_end)
            : "cc", "memory"
		);

        return 0;
	}
	return -1;
}

int32_t read (int32_t fd, void* buf, int32_t nbytes){
    uint32_t esp;
    pcb_t * pcb;

	if(fd < 0 || fd == 1 || fd > 7) return -1;

    get_esp(esp);
    pcb = (pcb_t *) (esp & ESP_MASK);

    return pcb -> files[fd].fops -> read(fd, buf, nbytes);
}

int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    uint32_t esp;
    pcb_t * pcb;

    if(fd < 0 || fd == 0 || fd > 7) return -1;

    get_esp(esp);
    pcb = (pcb_t *) (esp & ESP_MASK);

    return pcb -> files[fd].fops -> write(fd, buf, nbytes);
}

int32_t open (const uint8_t* filename){
	int i, fd = -1;
    uint32_t esp;
	pcb_t* pcb_ptr;
	dentry_t temp_dentry;
	fd_t* fd_ptr = NULL;
    fops_t * fops;

    get_esp(esp);
    pcb_ptr = (pcb_t*)(esp & ESP_MASK);

    /* find available file descriptor entry */
    for(i = 2; i < FILE_ARRAY_LEN ; i++){
        if(pcb_ptr->files[i].flags == DEAD){
            fd = i;
            break;
        }
    }

    /* if no available fd directory */
    if(fd == -1) return -1;

    fd_ptr = &(pcb_ptr -> files[i]);

	/* get driver fops */
    fops = get_device_fops((char *) filename);
    if(fops != NULL) {
        fd_ptr -> fops = fops;
        fd_ptr -> inode = NULL;
        fd_ptr -> flags = LIVE;
        return fd;
    }

	if(read_dentry_by_name(filename, &temp_dentry) == -1){
		return -1;
	}

    fd_ptr -> fops = get_device_fops("fs");
    fd_ptr -> inode = get_inode_ptr(temp_dentry.inode);
    fd_ptr -> flags = LIVE;
    fd_ptr -> pos = 0;
	
 	return fd;
}

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
