#include "virtualmem.h"
#include "types.h"

#define TABLE_SIZE 1024
#define PAGE_SIZE  4096  /* kilobytes */

#define PD_INIT_VAL  2  /* write enabled, not present */
#define PD_ADDR_OFFS 22

#define KERNEL_LOC 0x400000
#define KERNEL_INIT_FLAGS 0x43  /* page size, read/write, and present set */
#define PAGE_INIT_FLAGS 0x03    /* read/write and present set */

static uint32_t pd[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));
static uint32_t pt_first[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));

void virtualmem_init()
{
	uint32_t addr;
	int i;

	/* enable PSE for 4 MB pages */
	/* 0x10 - enable 4th bit of cr4 */
	asm volatile("				\n\
		mov		%%cr4, %%eax	\n\
		or		$0x10, %%eax	\n\
		mov		%%eax, %%cr4	\n\
		"
		:
		:
		: "eax"
	);

	/* initialize page directory */
	for(i = 0; i < TABLE_SIZE; i++)
	{
		pd[i] = PD_INIT_VAL;
	}

	/* initialize video memory pages */
	pd[0] = (uint32_t) pt_first | PAGE_INIT_FLAGS;
	for(i = 0, addr = 0; i < TABLE_SIZE; i++, addr += PAGE_SIZE)
	{
		pt_first[i] = addr | PAGE_INIT_FLAGS;
	}


	/* initialize 4 MB kernel page */
	pd[KERNEL_LOC >> PD_ADDR_OFFS] = KERNEL_LOC | KERNEL_INIT_FLAGS;

	/* turn on paging */
	/* 0x80000000 - enable paging bit of cr0 */
	asm volatile("					\n\
		movl	pd, %%eax			\n\
		movl	%%eax, %%cr3		\n\
		movl	%%cr0, %%eax		\n\
		or		$0x80000000, %%eax	\n\
		movl	%%eax, %%cr0		\n\
		"
		:
		:
		: "eax"
	);
}
