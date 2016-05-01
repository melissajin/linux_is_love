#ifndef _SYS_CALLS_H
#define _SYS_CALLS_H

#include "types.h"

#define KERNEL_MEM_END 	 	0x800000
#define KERNEL_STACK_SIZE   0x2000

/* 10 system calls */

//terminates the execution of a file
int32_t halt (uint8_t status);

//executes the command inputted from the command line
int32_t execute (const uint8_t* command);

//reads nbytes from the file/evice  described by the file descriptor at the index fd into buf
int32_t read (int32_t fd, void* buf, int32_t nbytes);

//writes nbytes to the file/device described by the file descriptor at the index fd into buf
int32_t write (int32_t fd, const void* buf, int32_t nbytes);

//opens the file with given filename and puts it into PCB
int32_t open (const uint8_t* filename);

//closes the file described by the given file descriptor and sets the file descriptor to be used again
int32_t close (int32_t fd);

//gets the arguments 
int32_t getargs (uint8_t* buf, int32_t nbytes);

//maps the video memory for the user program
int32_t vidmap (uint8_t** screen_start);

//2 unimplemented signal functions for extra credit. To be completed
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

#endif /* _SYS_CALLS_H */
