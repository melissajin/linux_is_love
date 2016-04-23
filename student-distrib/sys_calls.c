#include "sys_calls.h"
#include "fs.h"
#include "virtualmem.h"
#include "lib.h"
#include "process.h"
#include "x86_desc.h"
#include "devices/keyboard.h"

#define PROG_VM_START   0x8000000
#define SPACE_4MB       0x400000
#define START_EXE_ADDR  0x08048000

#define ELF_HEADER_LEN  40
#define ELF_MAGIC       0x464c457f
#define ELF_ADDR_OFFS   24

#define WORD_SIZE       4

/* trick to stringify macros */
#define STR2(x)         #x
#define STR(x)          STR2(x)

int32_t halt (uint8_t status) {
    pcb_t * pcb_child_ptr, * pcb_parent_ptr;
    uint32_t esp, ebp;

    pcb(pcb_child_ptr);
    pcb_parent_ptr = pcb_child_ptr -> parent_pcb;

    if(pcb_parent_ptr != NULL) {
        set_pd(pcb_parent_ptr -> pd);
        tss.esp0 = KERNEL_MEM_END - PCB_SIZE * pcb_parent_ptr -> pid - WORD_SIZE;
    } else {
        set_pd(NULL);
        tss.esp0 = KERNEL_MEM_END - WORD_SIZE;
    }
    
    esp = pcb_child_ptr -> esp_parent;
    ebp = pcb_child_ptr -> ebp_parent;

    delete_process(pcb_child_ptr -> pid);

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
    int8_t retval = 0;
    uint8_t command_buf[ARGS_MAX];
    uint8_t args[ARGS_MAX];
    uint8_t buf[ELF_HEADER_LEN];
    dentry_t dentry;
    uint32_t addr;
    uint32_t vm_end;
    pcb_t* pcb;
    fd_t stdin;
    fd_t stdout;
    fd_t fd;
    uint32_t i;
    int32_t pid;
    pcb_t* pcb_start;
    fops_t * term_fops;
    uint32_t * pd;

    parse_arg(command, command_buf, args);

    /* check for valid executable */
    if(-1 == read_dentry_by_name(command_buf, &dentry))
        return -1; 
    if(read_data(dentry.inode, 0, buf, ELF_HEADER_LEN) < ELF_HEADER_LEN)
        return -1;
    if(*((uint32_t *) buf) == ELF_MAGIC){
        pid = add_process();
        if(pid < 0)
            return -1;

        addr = *((uint32_t *) (buf + ELF_ADDR_OFFS));  /* interpret the 4 bytes at buf[24-27] as a uint32_t */
        vm_end = PROG_VM_START + SPACE_4MB - WORD_SIZE;

        /* set up process paging */
        pd = get_process_pd(pid);
        pd_init(pd);
        set_pde(pd, PROG_VM_START, KERNEL_MEM_END + (pid - 1) * SPACE_4MB,
                FLAG_PS | FLAG_U | FLAG_WE | FLAG_P);
        set_pd(pd);
        
        /* starting address of current pcb */
        pcb(pcb_start);

        /* get terminal fops */
        term_fops = get_device_fops(TERM_FTYPE);

        /* setting the pcb in the kernel stack */
        pcb = (pcb_t*) (KERNEL_MEM_END - (pid + 1) * PCB_SIZE);
        stdin.fops = term_fops;
        stdin.inode = NULL;
        stdin.pos = 0;
        stdin.flags = FD_LIVE;
        pcb->files[0] = stdin;

        stdout.fops = term_fops;
        stdout.inode = NULL;
        stdout.pos = 0;
        stdout.flags = FD_LIVE;
        pcb->files[1] = stdout;

        /* initialize the rest of the file descriptor entries */
        for(i = 2; i < FILE_ARRAY_LEN; i++){
            fd.fops = NULL;
            fd.inode = NULL;
            fd.pos = 0;
            fd.flags = 0;
            pcb->files[i] = fd;
        }

        pcb->pid = pid;
        pcb->parent_pcb = curr_terminal_running_process() ? (pcb_t *) pcb_start : NULL;

        pcb -> args_len = strlen((int8_t *) args);
        strcpy((int8_t *) pcb -> args, (int8_t *) args);

        pcb -> pd = pd;

        set_curr_active_process(pid);

        /* saving values in tss to return to process kernel stack */
        tss.esp0 = KERNEL_MEM_END - PCB_SIZE * pid - WORD_SIZE;
        tss.ss0 = KERNEL_DS;

        /* load file in physical memory */
        load(&dentry, (uint8_t*) START_EXE_ADDR);

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
    pcb_t * pcb;

    /* if fd out of bounds or stdout */
    if(fd < 0 || fd == 1 || fd >= FILE_ARRAY_LEN) return -1;

    pcb(pcb);

    if(pcb -> files[fd].flags && FD_LIVE){
        return pcb -> files[fd].fops -> read(fd, buf, nbytes);
    }

    return -1;
}

int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    pcb_t * pcb;

    /* if fd out of bounds or stdin */
    if(fd < 0 || fd == 0 || fd >= FILE_ARRAY_LEN) return -1;

    pcb(pcb);

    if(pcb -> files[fd].flags && FD_LIVE){
        return pcb -> files[fd].fops -> write(fd, buf, nbytes);
    }

    return -1;
}

int32_t open (const uint8_t* filename){
    int i, fd = -1;
    pcb_t* pcb;
    dentry_t dentry;
    fd_t* fd_ptr = NULL;
    fops_t * fops;

    pcb(pcb);

    /* find available file descriptor entry */
    /* start after stdin (0) and stdout (1) */
    for(i = 2; i < FILE_ARRAY_LEN ; i++){
        if(!(pcb->files[i].flags && FD_LIVE)){
            fd = i;
            break;
        }
    }

    /* if no available fd directory or valid filename*/
    if(fd == -1 || read_dentry_by_name(filename, &dentry) == -1) 
        return -1;

    fd_ptr = &(pcb -> files[i]);
    fops = get_device_fops(dentry.ftype);

    /* RTC or Directory */
    if(dentry.ftype == RTC_FTYPE || dentry.ftype == DIR_FTYPE){
        fd_ptr -> inode = NULL;
        fd_ptr -> inode_num = dentry.inode;
    }

    /* Regular File */
    else if(dentry.ftype == FILE_FTYPE){
        fd_ptr -> inode = get_inode_ptr(dentry.inode);
        fd_ptr -> inode_num = dentry.inode;
    }
    
    fd_ptr -> fops = fops;
    fd_ptr -> flags = FD_LIVE;
    fd_ptr -> fops -> open(filename);

    return fd;
}

int32_t close (int32_t fd){
    pcb_t* pcb;
    fd_t * file_desc;

    /* prevent closing stdin or stdout */
    if(fd < 2 || fd >= FILE_ARRAY_LEN) return -1; 

    pcb(pcb);
    file_desc = &(pcb -> files[fd]);

    if(file_desc -> flags && FD_LIVE){

        file_desc -> fops -> close(fd);

        file_desc -> fops = NULL;
        file_desc -> inode = NULL;
        file_desc -> pos = 0;
        file_desc -> flags = 0;
        return 0;
    }

    return -1;
}

int32_t getargs (uint8_t* buf, int32_t nbytes){
    pcb_t * pcb;

    pcb(pcb);

    if(nbytes < (pcb -> args_len + 1)) return -1;

    memcpy(buf, pcb -> args, pcb -> args_len);
    buf[pcb -> args_len] = '\0';

    return 0;
}

int32_t vidmap (uint8_t** screen_start){
    pcb_t * pcb;

    if(((int32_t) screen_start < PROG_VM_START) || ((int32_t) screen_start >= PROG_VM_START + SPACE_4MB))
        return -1;

    *screen_start = (uint8_t *) PROG_VIDMEM_ADDR;

    pcb(pcb);

    // set_pde_present(PROG_VIDMEM_ADDR);
    set_pde_flags(pcb -> pd, PROG_VIDMEM_ADDR, FLAG_P);

    return PROG_VIDMEM_ADDR;
}

int32_t set_handler (int32_t signum, void* handler_address){ return -1; }
int32_t sigreturn (void){ return -1; }

/* taken from given syscall material for now */
void parse_arg(const uint8_t* command, uint8_t* command_buf, uint8_t * arg_buf){
    uint32_t i;

    /* increment scan to the end of the first token */
    for (i = 0; '\0' != command[i] && ' ' != command[i] && '\n' != command[i]; i++);
    
    memcpy(command_buf, command, i);
    command_buf[i] = '\0';
    
    for(i = i; command[i] == ' '; i++);
    strcpy((int8_t *) arg_buf, (int8_t *) command + i);
}
