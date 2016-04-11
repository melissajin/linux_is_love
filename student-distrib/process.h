
#ifndef _PROCESS_H
#define _PROCESS_H

#include "fs.h"

#define FILE_ARRAY_LEN	8
#define MAX_PROCESSES	6

int32_t process_num = 0;
int32_t procs[MAX_PROCESSES] = {0,0,0,0,0,0};

typedef struct pcb {
	fd_t files[FILE_ARRAY_LEN];
	int32_t pid;
	pcb_t* parent_pcb;
} pcb_t;

typedef struct {
    fops_t * fops;
    inode_t * inode;
    uint32_t pos
    uint32_t flags;
} fd_t;

#endif /* _PROCESS_H */