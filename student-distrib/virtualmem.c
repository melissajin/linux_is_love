#include "virtualmem.h"
#include "lib.h"
#include "process.h"

/* values for manipulating table entries */
#define PDE_IDX_OFFS 22
#define PTE_IDX_OFFS 12
#define PTE_IDX_MASK 0x3FF
#define PDE_4MB_MASK 0xFFC00000
#define PDE_4KB_MASK 0xFFFFF000

/* initial values */
#define LARGE_INIT_FLAGS (FLAG_P | FLAG_WE | FLAG_PS)

/* memory locations */
#define KERNEL_LOC 0x400000

static uint32_t pd_first[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));
static uint32_t pt_first[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));
static uint32_t pt_user_vidmem[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));

void virtualmem_init()
{
	int i;

	/* initialize first page directory */
	pd_init(pd_first);

	/* initialize page tables */
	for(i = 0; i < TABLE_SIZE; i++)
	{
		pt_first[i] = 0;
		pt_user_vidmem[i] = 0;
	}
	pt_first[VIDEO >> PTE_IDX_OFFS & PTE_IDX_MASK] = VIDEO | FLAG_WE | FLAG_P;
	pt_user_vidmem[PROG_VIDMEM_ADDR >> PTE_IDX_OFFS & PTE_IDX_MASK] = VIDEO | FLAG_WE | FLAG_P | FLAG_U;

	set_pd(pd_first);

	/* enable PSE for 4 MB pages
	   0x10 - enable 4th bit of cr4 */
	/* turn on paging
	   0x80000000 - enable paging bit of cr0 */
	asm volatile("					\n\
		movl	%%cr4, %%eax		\n\
		orl		$0x10, %%eax		\n\
		movl	%%eax, %%cr4		\n\
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
	pd_first[virtual_add >> PDE_IDX_OFFS] = lower_b | LARGE_INIT_FLAGS | FLAG_U;

	/* FLUSHING THE TLB */
	flush_tlb();
}

void set_pde_present(int32_t virtual_add) {
	pd_first[virtual_add >> PDE_IDX_OFFS] |= FLAG_P;
}

void unmap_pde(int32_t virtual_add) {
	pd_first[virtual_add >> PDE_IDX_OFFS] &= ~FLAG_P;
}

/* new functions */

void pd_init(uint32_t * pd) {
	int i;

	/* initialize page directory */
	for(i = 0; i < TABLE_SIZE; i++)
	{
		pd[i] = 0;
	}

	/* initialize video memory pages */
	pd[0] = (uint32_t) pt_first | FLAG_WE | FLAG_P;
	pd[PROG_VIDMEM_ADDR >> PDE_IDX_OFFS] = (uint32_t) pt_user_vidmem | FLAG_WE | FLAG_U;

	/* initialize 4 MB kernel page */
	pd[KERNEL_LOC >> PDE_IDX_OFFS] = KERNEL_LOC | LARGE_INIT_FLAGS;
}

void set_pde(uint32_t * pd, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
	if(flags & FLAG_PS) {
		physical_addr &= PDE_4MB_MASK;
	} else {
		physical_addr &= PDE_4KB_MASK;
	}
	pd[virtual_addr >> PDE_IDX_OFFS] = physical_addr & flags;
}

void set_pde_flags(uint32_t * pd, uint32_t virtual_addr, uint32_t flags) {
	pd[virtual_addr >> PDE_IDX_OFFS] |= flags;
}

void unset_pde_flags(uint32_t * pd, uint32_t virtual_addr, uint32_t flags) {
	pd[virtual_addr >> PDE_IDX_OFFS] &= ~flags;
}

void set_pd(uint32_t * pd) {
	if(pd == NULL) pd = pd_first;

	asm volatile("				\n\
		movl	%0, %%eax		\n\
		movl	%%eax, %%cr3	\n\
		"
		:
		: "rm" (pd)
	);
}
