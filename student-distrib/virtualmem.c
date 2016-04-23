#include "virtualmem.h"
#include "lib.h"
#include "process.h"
#include "devices/keyboard.h"

/* values for manipulating table entries */
#define PDE_IDX_OFFS 22
#define PTE_IDX_OFFS 12
#define PTE_IDX_MASK 0x3FF
#define PDE_4MB_MASK 0xFFC00000
#define PDE_4KB_MASK 0xFFFFF000
#define FLAGS_MASK	 0xFFF

/* initial values */
#define LARGE_INIT_FLAGS (FLAG_P | FLAG_WE | FLAG_PS)

/* memory locations */
#define KERNEL_LOC 0x400000

static uint32_t pd_first[TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));

/* 0 - vidmem; 1 - term 0 back; 2 - term 1 back; 3 - term 2 back */
static uint32_t pt_vidmem[MAX_TERMINALS][TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));
static uint32_t pt_user_vidmem[MAX_TERMINALS][TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));

void virtualmem_init()
{
	int i, j;

	/* initialize first page directory */
	pd_init(pd_first);

	/* initialize page tables */
	for(i = 0; i < MAX_TERMINALS; i++) {
		for(j = 0; j < TABLE_SIZE; j++) {
			pt_vidmem[i][j] = 0;
			pt_user_vidmem[i][j] = 0;
		}
	}

	for(i = 0; i < MAX_TERMINALS; i++) {
		pt_vidmem[i][VIDEO >> PTE_IDX_OFFS & PTE_IDX_MASK] = (VIDEO + i * PAGE_SIZE) | FLAG_WE | FLAG_P;
		for(j = 1; j <= MAX_TERMINALS; j++) {
			pt_vidmem[i][(VIDEO + j * PAGE_SIZE) >> PTE_IDX_OFFS & PTE_IDX_MASK] =
					(VIDEO + j * PAGE_SIZE) | FLAG_WE | FLAG_P;
		}

		pt_user_vidmem[i][PROG_VIDMEM_ADDR >> PTE_IDX_OFFS & PTE_IDX_MASK] = (VIDEO + i * PAGE_SIZE) | FLAG_WE | FLAG_P | FLAG_U;
	}

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

void pd_init(uint32_t * pd) {
	int i;

	/* initialize page directory */
	for(i = 0; i < TABLE_SIZE; i++)
	{
		pd[i] = 0;
	}

	/* initialize video memory pages */
	pd[0] = (uint32_t) pt_vidmem[0] | FLAG_WE | FLAG_P;
	pd[PROG_VIDMEM_ADDR >> PDE_IDX_OFFS] = (uint32_t) pt_user_vidmem[0] | FLAG_WE | FLAG_U;

	/* initialize 4 MB kernel page */
	set_pde(pd, KERNEL_LOC, KERNEL_LOC, LARGE_INIT_FLAGS);
}

void set_pde(uint32_t * pd, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
	if(flags & FLAG_PS) {
		physical_addr &= PDE_4MB_MASK;
	} else {
		physical_addr &= PDE_4KB_MASK;
	}
	pd[virtual_addr >> PDE_IDX_OFFS] = physical_addr | flags;
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

void set_vidmem_tables(uint32_t * pd, uint32_t num) {
	uint32_t flags;

	pd[0] = (uint32_t) pt_vidmem[num] | FLAG_WE | FLAG_P;

	/* preserve flags for if user has called vidmap */
	flags = pd[PROG_VIDMEM_ADDR >> PDE_IDX_OFFS] & FLAGS_MASK;
	pd[PROG_VIDMEM_ADDR >> PDE_IDX_OFFS] = (uint32_t) pt_user_vidmem[num] | flags;
}
