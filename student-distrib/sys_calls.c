
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
#define START_EXE_ADDR  0x08048000

#define ELF_HEADER_LEN  40
#define ELF_MAGIC       0x464c457f
#define ELF_ADDR_OFFS   24

#define TERM_NAME       "term"
#define WORD_SIZE       4

/* trick to stringify macros */
#define STR2(x)         #x
#define STR(x)          STR2(x)

int32_t halt (uint8_t status) {
    pcb_t * pcb_child_ptr, * pcb_parent_ptr;
    uint32_t esp, ebp;

    get_esp(esp);
    pcb_child_ptr = (pcb_t *) (esp & PCB_MASK);
    pcb_parent_ptr = pcb_child_ptr -> parent_pcb;

    unmap_large_page(PROG_VM_START);
    if(pcb_parent_ptr != NULL) {
        map_large_page(PROG_VM_START, KERNEL_MEM_END + pcb_parent_ptr -> pid * SPACE_4MB);
        tss.esp0 = KERNEL_MEM_END - PCB_SIZE * pcb_parent_ptr -> pid - WORD_SIZE;
    } else {
        tss.esp0 = KERNEL_MEM_END;
    }
    
    esp = pcb_child_ptr -> esp_parent;
    ebp = pcb_child_ptr -> ebp_parent;

    delete_process(pcb_child_ptr -> pid);
    proc_count--;

    asm volatile("              \n\
        movl    %1, %%esp       \n\
        movl    %2, %%ebp       \n\
        movb    %0, %%bl        \n\
        jmp     halt_ret_label  \n\
        "
        :
        : "r" (status), "r" (esp), "r" (ebp)
        : "cc", "memory"
    );
    return 0;
}

int32_t execute (const uint8_t* command) {
    int8_t retval;
    uint8_t command_buf[ARGS_MAX];
    uint8_t* args[ARGS_MAX];
    uint8_t buf[ELF_HEADER_LEN];
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

    memcpy(command_buf, command, strlen((int8_t *) command));
    parse_arg(command, (uint8_t**)args);

    /* check for valid executable */
    if(-1 == read_dentry_by_name(args[0], &dentry))
		return -1; 
	if(read_data(dentry.inode, 0, buf, ELF_HEADER_LEN) < ELF_HEADER_LEN)
		return -1;
	if(*((uint32_t *) buf) == ELF_MAGIC){
        addr = *((uint32_t *) (buf + ELF_ADDR_OFFS));  /* interpret the 4 bytes at buf[24-27] as a uint32_t */
        vm_end = PROG_VM_START + SPACE_4MB - WORD_SIZE;
		map_large_page(PROG_VM_START, KERNEL_MEM_END + pid * SPACE_4MB);
		
        /* get current stack pointer and put in */
        get_esp(esp);
        
        /* starting address of current pcb */
        pcb_start = esp & PCB_MASK;

        /* get terminal fops */
        term_fops = get_device_fops((uint8_t *) TERM_NAME);

        /* setting the pcb in the kernel stack */
        pcb = (pcb_t*) (KERNEL_MEM_END - (pid + 1) * PCB_SIZE);
        stdin.fops = term_fops;
        stdin.inode = NULL;
        stdin.pos = 0;
        stdin.flags = LIVE;
        pcb->files[0] = stdin;

        stdout.fops = term_fops;
        stdout.inode = NULL;
        stdout.pos = 0;
        stdout.flags = LIVE;
        pcb->files[1] = stdout;

        /* initialize the rest of the file descriptor entries */
        for(i = 2; i < FILE_ARRAY_LEN; i++){
            fd.fops = NULL;
            fd.inode = NULL;
            fd.pos = 0;
            fd.flags = DEAD;
            pcb->files[i] = fd;
        }

        pcb->pid = pid;
        pcb->parent_pcb = proc_count ? (pcb_t *) pcb_start : NULL;

        pcb -> args_len = strlen((int8_t *) command_buf);
        memcpy(pcb -> args, command_buf, pcb -> args_len);

        /* saving values in tss to return to process kernel stack */
        tss.esp0 = KERNEL_MEM_END - PCB_SIZE * pid - WORD_SIZE;
        tss.ss0 = KERNEL_DS;

		/* load file in physical memory */
		load(&dentry, (uint8_t*) START_EXE_ADDR);

        /* increment # of processes */
        proc_count++;

        get_ebp(pcb -> ebp_parent);
        get_esp(pcb -> esp_parent);
		
		asm volatile("                        \n\
			xorl    %%ecx, %%ecx              \n\
            movw    $"STR(USER_CS)", %%cx     \n\
            movw    %%cx, %%ds                \n\
            movw    %%cx, %%es                \n\
            movw    %%cx, %%fs                \n\
            movw    %%cx, %%gs                \n\
            pushl   $"STR(USER_DS)"           \n\
            pushl   %2                        \n\
            pushf                             \n\
            orl     $0x200, (%%esp)           \n\
            pushl   $"STR(USER_CS)"           \n\
            pushl   %1                        \n\
            iret                              \n\
            halt_ret_label:                   \n\
            movb    %%bl, %0                  \n\
            "
			: "=rm" (retval)
			: "r" (addr), "r" (vm_end)
            : "cc", "memory"
		);

        return retval;
	}
	return -1;
}

int32_t read (int32_t fd, void* buf, int32_t nbytes){
    uint32_t esp;
    pcb_t * pcb;

    /* if fd out of bounds or stdout */
	if(fd < 0 || fd == 1 || fd >= FILE_ARRAY_LEN) return -1;

    get_esp(esp);
    pcb = (pcb_t *) (esp & PCB_MASK);

    return pcb -> files[fd].fops -> read(fd, buf, nbytes);
}

int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    uint32_t esp;
    pcb_t * pcb;

    /* if fd out of bounds or stdin */
    if(fd < 0 || fd == 0 || fd >= FILE_ARRAY_LEN) return -1;

    get_esp(esp);
    pcb = (pcb_t *) (esp & PCB_MASK);

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
    pcb_ptr = (pcb_t*)(esp & PCB_MASK);

    /* find available file descriptor entry */
    /* start after stdin (0) and stdout (1) */
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
    fops = get_device_fops(filename);
    if(fops != NULL) {
        fd_ptr -> fops = fops;
        fd_ptr -> inode = NULL;
        fd_ptr -> inode_num = 0;
        fd_ptr -> flags = LIVE;

        fd_ptr -> fops -> open(filename);
        
        return fd;
    }

	if(read_dentry_by_name(filename, &temp_dentry) == -1){
		return -1;
	}

    fd_ptr -> fops = get_device_fops((uint8_t *) FS_DEV_NAME);
    fd_ptr -> inode = get_inode_ptr(temp_dentry.inode);
    fd_ptr -> inode_num = temp_dentry.inode;
    fd_ptr -> flags = LIVE;
    fd_ptr -> pos = 0;
	
    fd_ptr -> fops -> open(filename);

 	return fd;
}

int32_t close (int32_t fd){ 
    uint32_t esp;
    pcb_t* pcb_ptr;
    fd_t file_desc;

    if(fd < 2 || fd > 7) return -1; 

    get_esp(esp);
    pcb_ptr = (pcb_t*)(esp & PCB_MASK);
    file_desc = pcb_ptr -> files[fd];
    
    file_desc.fops = NULL;
    file_desc.inode = NULL;
    file_desc.pos = 0;
    file_desc.flags = DEAD;

    return 0;
}

int32_t getargs (uint8_t* buf, int32_t nbytes){
    uint32_t esp;
    pcb_t * pcb_ptr;

    get_esp(esp);
    pcb_ptr = (pcb_t * ) (esp & PCB_MASK);

    if(nbytes < (pcb_ptr -> args_len + 1)) return -1;

    memcpy(buf, pcb_ptr -> args, pcb_ptr -> args_len);
    buf[pcb_ptr -> args_len + 1] = '\0';

    return 0;
}

int32_t vidmap (uint8_t** screen_start){ return -1; }

int32_t set_handler (int32_t signum, void* handler_address){ return -1; }
int32_t sigreturn (void){ return -1; }

/* taken from given syscall material for now */
void parse_arg(const uint8_t* command, uint8_t** args){
	uint8_t buf[ARGS_MAX + 2];
    uint8_t* scan;
    uint32_t n_arg;

    if (ARGS_MAX - 1 < strlen((int8_t*)command))
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
