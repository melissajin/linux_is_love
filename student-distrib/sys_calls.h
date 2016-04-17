#ifndef _SYS_CALLS_H
#define _SYS_CALLS_H

#include "types.h"

/* 10 system calls */
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

/* helper function to parse args for execute */
void parse_arg(const uint8_t* command, uint8_t* command_buf, uint8_t * arg_buf);

#endif /* _SYS_CALLS_H */
