#ifndef _PROCESS_H
#define _PROCESS_H

#include "fs.h"
#include "types.h"

#define FILE_ARRAY_LEN	8

typedef struct {
    fops_t * fops;
    inode_t * inode;
    uint32_t pos;
    uint32_t flags;
} fd_t;

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp;
} regs_t;

typedef struct pcb {
	fd_t files[FILE_ARRAY_LEN];
	int32_t pid;
	struct pcb* parent_pcb;
    regs_t regs;
    uint32_t esp_parent, ebp_parent;
} pcb_t;

uint32_t proc_count;

int add_device(uint8_t * name, fops_t * fops);
fops_t * get_device_fops(const uint8_t * req_name);
int32_t add_process();
int32_t delete_process(int32_t pid);

#define get_esp(x)          \
do {                        \
    asm volatile (          \
        "movl %%esp, %0"    \
        : "=rm" (x)         \
    );                      \
} while(0)

#define get_ebp(x)          \
do {                        \
    asm volatile (          \
        "movl %%ebp, %0"    \
        : "=rm" (x)         \
    );                      \
} while(0)

#endif /* _PROCESS_H */
