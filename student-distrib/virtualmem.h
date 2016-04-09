#ifndef _VIRTUALMEM_H_
#define _VIRTUALMEM_H_

#include "types.h"


//initializes the paging for virtual mem
void virtualmem_init();

/* maps a 4MB page in physical memory */
void map_large_page(int32_t virtual_add, int32_t lower_b);

#endif
