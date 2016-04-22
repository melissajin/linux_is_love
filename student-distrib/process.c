#include "process.h"
#include "lib.h"
#include "virtualmem.h"

#define MAX_DEVICES		6
#define MAX_PROCESSES	6

static uint32_t proc_count = 0;
static uint8_t procs[MAX_PROCESSES] = {0};
static uint32_t pd[MAX_PROCESSES][TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));

static fops_t * devices[MAX_DEVICES];

/* add_device
 *	  DESCRIPTION: 
 *    INPUTS: 
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS: 
 */
int add_device(uint32_t ftype, fops_t * fops) {
	if(ftype < 0 || ftype >= MAX_DEVICES) return -1;

	devices[ftype] = fops;
	return 0;
}

/* get_device_fops
 *	  DESCRIPTION: 
 *    INPUTS: 
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS: 
 */
fops_t * get_device_fops(uint32_t ftype) {
	if(ftype < 0 || ftype >= MAX_DEVICES) return NULL;
	
	return devices[ftype];
}

/* add_process
 *	  DESCRIPTION: adds a process in a PCB
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: process number or -1 on failure
 *    SIDE EFFECTS: changes procs bitmap to show added process
 */
int32_t add_process(){
	uint32_t i;
	for(i = 0; i < MAX_PROCESSES; i++){
		if(procs[i] == 0){
			procs[i] = 1;
			proc_count++;
			return i + 1;
		}
	}
	return -1;
}

/* delete_process
 *	  DESCRIPTION: deletes a process in the PCB corresponding the the fd
 *    INPUTS: fd
 *    OUTPUTS: none
 *    RETURN VALUE: 0 on success, -1 on failure
 *    SIDE EFFECTS: changes the procs bitmap to show deleted process
 */
int32_t delete_process(int32_t pid){
	pid--;
	if(pid < 0 || pid >= MAX_PROCESSES){
		return -1;
	}
	procs[pid] = 0;
	proc_count--;
	return 0;
}

uint32_t * get_process_pd(int32_t pid) {
	pid--;
	if(pid < 0 || pid >= MAX_PROCESSES) {
		return NULL;
	}
	return pd[pid];
}

int32_t are_processes() {
	return proc_count > 0;
}
