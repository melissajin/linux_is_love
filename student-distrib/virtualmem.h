#ifndef _VIRTUALMEM_H_
#define _VIRTUALMEM_H_

#include "types.h"

#define TABLE_SIZE 1024
#define PAGE_SIZE  4096  /* kilobytes */

/* flags */
#define FLAG_P  0x1    /* present */
#define FLAG_WE 0x2    /* write enabled */
#define FLAG_U  0x4    /* user access */
#define FLAG_WT 0x8    /* write-through caching */
#define FLAG_CD 0x10   /* cache disable */
#define FLAG_A  0x20   /* accessed */
#define FLAG_D  0x40   /* dirty */
#define FLAG_PS 0x80   /* page size (4 MB) */
#define FLAG_G  0x100  /* global */

/* flush the tlb without changing it */
#define flush_tlb()						\
{										\
	asm volatile("					\n\
		movl	%%cr3, %%eax		\n\
		movl	%%eax, %%cr3		\n\
		"								\
		:								\
		:								\
		: "eax"							\
	);									\
}

/* initializes the paging for virtual mem */
void virtualmem_init();

/* generalized paging functions */
/* initialize a page directory to default values */
void pd_init(uint32_t * pd, int32_t term_num);
/* set a page directory entry */
void set_pde(uint32_t * pd, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
/* turn on flags of a page directory entry */
void set_pde_flags(uint32_t * pd, uint32_t virtual_addr, uint32_t flags);
/* turn off flags of a page directory entry */
void unset_pde_flags(uint32_t * pd, uint32_t virtual_addr, uint32_t flags);
/* set the PDPR to a page directory */
void set_pd(uint32_t * pd);

#endif
