#include "virtualmem.h"
#include "types.h"

#define TABLE_SIZE 1024
#define PAGE_SIZE  4096  /* kilobytes */

#define PD_INIT_VAL 2  /* write enabled, not present */
#define PD_IDX_OFFS 22

#define KERNEL_LOC 0x400000
#define KERNEL_INIT_FLAGS 0x83  /* page size, write enabled, and present set */
#define PAGE_INIT_FLAGS 0x2    /* write enabled, not present */

#define VIDEO 0xB8000

static uint32_t pd[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));
static uint32_t pt_first[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));

void virtualmem_init()
{
	uint32_t addr;
	int i;

	/* initialize page directory */
	for(i = 0; i < TABLE_SIZE; i++)
	{
		pd[i] = PD_INIT_VAL;
	}

	/* initialize video memory pages */
	pd[0] = (uint32_t) pt_first | PAGE_INIT_FLAGS;
	for(i = 0, addr = 0; i < TABLE_SIZE; i++, addr += PAGE_SIZE)
	{
		pt_first[i] = 0 | PAGE_INIT_FLAGS;
	}
	pt_first[VIDEO >> 12 & 0x03FF] = VIDEO | PAGE_INIT_FLAGS | 1;


	/* initialize 4 MB kernel page */
	pd[KERNEL_LOC >> PD_IDX_OFFS] = KERNEL_LOC | KERNEL_INIT_FLAGS;

	/* enable PSE for 4 MB pages
	   0x10 - enable 4th bit of cr4 */
	/* turn on paging
	   0x80000000 - enable paging bit of cr0 */
	asm volatile("					\n\
		movl	%%cr4, %%eax		\n\
		orl		$0x10, %%eax		\n\
		movl	%%eax, %%cr4		\n\
		movl	$pd, %%eax			\n\
		movl	%%eax, %%cr3		\n\
		movl	%%cr0, %%eax		\n\
		orl		$0x80000000, %%eax	\n\
		movl	%%eax, %%cr0		\n\
		"
		:
		:
		: "eax"
	);
}
