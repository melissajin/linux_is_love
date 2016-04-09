#include "virtualmem.h"
#include "lib.h"

#define TABLE_SIZE 1024
#define PAGE_SIZE  4096  /* kilobytes */
#define PROCESS_SIZE 0x400000

/* values for manipulating table entries */
#define PDE_IDX_OFFS 22
#define PTE_IDX_OFFS 12
#define PTE_IDX_MASK 0x3FF
#define PDE_4MB_MASK 0xFFC00000

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

/* initial values */
#define LARGE_INIT_FLAGS (FLAG_P | FLAG_WE | FLAG_PS)
#define PDE_INIT_FLAGS FLAG_WE
#define PTE_INIT_FLAGS FLAG_WE

/* memory locations */
#define KERNEL_LOC 0x400000
#define PROCESS_LOC 0x800000

static uint32_t pd[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));
static uint32_t pt_first[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));

void virtualmem_init()
{
	int i;

	/* initialize page directory */
	for(i = 0; i < TABLE_SIZE; i++)
	{
		pd[i] = PDE_INIT_FLAGS;
	}

	/* initialize video memory pages */
	pd[0] = (uint32_t) pt_first | PDE_INIT_FLAGS | FLAG_P;
	for(i = 0; i < TABLE_SIZE; i++)
	{
		pt_first[i] = PTE_INIT_FLAGS;
	}
	pt_first[VIDEO >> PTE_IDX_OFFS & PTE_IDX_MASK] = VIDEO | PTE_INIT_FLAGS | FLAG_P;


	/* initialize 4 MB kernel page */
	pd[KERNEL_LOC >> PDE_IDX_OFFS] = KERNEL_LOC | LARGE_INIT_FLAGS;

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

/* map_large_page
 * Description: maps a 4MB page to physical memory
 * Inputs: lower_b - the 4Mb section of physical memory that were trying to map to
	   virtual_add - the virtual address which gets us to the PDE to map
 * Outputs: none
 * Return: None
 */
void map_large_page(int32_t virtual_add, int32_t lower_b){
	lower_b &= PDE_4MB_MASK;
	pd[virtual_add >> PDE_IDX_OFFS] = lower_b | LARGE_INIT_FLAGS;
	return;
}
