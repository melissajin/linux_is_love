#ifndef _PROCESS_H
#define _PROCESS_H

#include "fs.h"
#include "types.h"
#include "devices/keyboard.h"

#define PROG_VIDMEM_ADDR 0x8400000

#define FILE_ARRAY_LEN	8
#define PCB_MASK        0xFFFFE000
#define ARGS_MAX        128

/* fd flags */
#define FD_LIVE            0x1

typedef struct {
    fops_t * fops;
    inode_t * inode;
    uint32_t inode_num;
    uint32_t pos;
    uint32_t flags;
} fd_t;

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp;
} regs_t;

typedef struct pcb {
	fd_t files[FILE_ARRAY_LEN];
    uint8_t args[ARGS_MAX];
    uint32_t args_len;
	int32_t pid;
	struct pcb* parent_pcb;
    regs_t regs;
    uint32_t esp_parent, ebp_parent;
    uint32_t * pd;
} pcb_t;

int add_device(uint32_t fytpe, fops_t * fops);
fops_t * get_device_fops(uint32_t fytpe);
int32_t add_process();
int32_t delete_process(int32_t pid);
uint32_t * get_process_pd(int32_t pid);
int32_t processes();
int32_t get_active_process(uint32_t term_num);
int32_t set_active_process(uint32_t term_num, int32_t pid);

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

#define pcb(x)                          \
do {                                    \
    uint32_t esp;                       \
    get_esp(esp);                       \
    x = (pcb_t *) (esp & PCB_MASK);     \
} while(0)

#endif /* _PROCESS_H */
