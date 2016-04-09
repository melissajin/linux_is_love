#ifndef _PROCESS_H
#define _PROCESS_H

#include "fs.h"

#define FILE_ARRAY_LEN 8

typedef struct {
    fops_t * fops;
    inode_t * inode;
    uint32_t pos;
    uint32_t flags;
} fd_t;

typedef struct pcb {
	fd_t files[FILE_ARRAY_LEN];
	int32_t pid;
	int32_t parent_pid;
} pcb_t;

int add_device(char * name, fops_t * fops);
fops_t * get_device_fops(char * req_name);

#endif /* _PROCESS_H */
