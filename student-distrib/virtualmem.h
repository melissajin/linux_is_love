#ifndef _VIRTUALMEM_H_
#define _VIRTUALMEM_H_

//initializes the paging for virtual mem
void virtualmem_init();

/* maps a 4MB page in physical memory */
int32_t map_page(int32_t process_num);

#endif
