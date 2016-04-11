#ifndef _PROCESS_H
#define _PROCESS_H

#include "fs.h"

#define FILE_ARRAY_LEN	8
#define MAX_PROCESSES	6

int32_t process_num = 0;
int32_t procs[MAX_PROCESSES] = {0,0,0,0,0,0};

typedef struct {
    fops_t * fops;
    inode_t * inode;
    uint32_t pos;
    uint32_t flags;
} fd_t;

typedef struct pcb {
	fd_t files[FILE_ARRAY_LEN];
	int32_t pid;
	struct pcb* parent_pcb;
} pcb_t;

int add_device(char * name, fops_t * fops);
fops_t * get_device_fops(char * req_name);

#endif /* _PROCESS_H */
