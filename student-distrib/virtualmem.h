#ifndef _VIRTUALMEM_H_
#define _VIRTUALMEM_H_

#include "types.h"
//initializes the paging for virtual mem
void virtualmem_init();
void map_large_page(int32_t virtual_add, int32_t lower_b);
void set_pde_present(int32_t virtual_add);
void unmap_pde(int32_t virtual_add);
#endif
