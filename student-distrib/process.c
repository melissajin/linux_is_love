#include "process.h"
#include "lib.h"
#include "virtualmem.h"
#include "x86_desc.h"

#define MAX_DEVICES		6

static uint32_t proc_count = 0;
static uint8_t procs[MAX_PROCESSES] = {0};
static uint32_t pd[MAX_PROCESSES][TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));
static int32_t active_processes[MAX_TERMINALS] = {-1, -1, -1};

static fops_t * devices[MAX_DEVICES];

/* add_device
 *	  DESCRIPTION: Registers a device by adding it to the 'devices' array of fops_t*.
 *				   Once a device/file is registered, it can be used by a user program.
 *    INPUTS: ftype - specify the type of device/file to register.
 *			  fops - pointer to an fops corresponding to the ftype.
 *    OUTPUTS: fills in the devices array at the 'ftype' index with the correct fops pointer
 *    RETURN VALUE: 0 on success, -1 on failure.
 */
int add_device(uint32_t ftype, fops_t * fops) {
	if(ftype < 0 || ftype >= MAX_DEVICES) return -1;

	devices[ftype] = fops;
	return 0;
}

/* get_device_fops
 *	  DESCRIPTION: Obtains an 'fops' pointer based on the ftype.
 *    INPUTS: ftype - specify the type of device/file to get the fops of.
 *    OUTPUTS: none
 *    RETURN VALUE: 'fops' pointer corresponding to the ftype passed in.
 */
fops_t * get_device_fops(uint32_t ftype) {
	if(ftype < 0 || ftype >= MAX_DEVICES) return NULL;
	
	return devices[ftype];
}

/* add_process
 *	  DESCRIPTION: adds a process to the 'procs' bitmap array.
 *    INPUTS: none
 *    OUTPUTS: adds the pid (1-6) of the new process to the 'procs' array.
 *    RETURN VALUE: process id number or -1 on failure.
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
 *	  DESCRIPTION: deletes a process if the 'procs' array.
 *    INPUTS: pid - the process id number of the process to be deleted.
 *    OUTPUTS: delets the specified process from the procs array.
 *    RETURN VALUE: 0 on success, -1 on failure
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

/* get_processes_pd
 *	  DESCRIPTION: gets a pointer to the page directory corresponding
 *				   to the pid of the specified process. 
 *    INPUTS: pid - process id the the page directory to get.
 *    OUTPUTS: none
 *    RETURN VALUE: pointer to the page directory.
 */
uint32_t * get_process_pd(int32_t pid) {
	pid--;
	if(pid < 0 || pid >= MAX_PROCESSES) {
		return NULL;
	}
	return pd[pid];
}

/* processes
 *	  DESCRIPTION: returns the total number of processes running.
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: proc_count - total number of processes running
 */
int32_t processes() {
	return proc_count;
}

/* free_procs
 *	  DESCRIPTION: indicates if theres enough space in memory to 
 *				   add a new process
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: 1 if total processes is less than the max amount
 *					of processes allowed (6). 0 if another process
 *					cannot be added.
 */
int32_t free_procs() {
	return proc_count < MAX_PROCESSES;
}

/* get_active_processes
 *	  DESCRIPTION: gets the pid of the active process running on
 *				   terminal 'term_num'
 *    INPUTS: term-num - terminal number to get the active process on.
 *    OUTPUTS: none
 *    RETURN VALUE: returns the pid of the acctive process in the 
 *					terminal specified.
 */
int32_t get_active_process(uint32_t term_num){
	if(term_num < 0 || term_num >= MAX_TERMINALS) return -1;

	return active_processes[term_num];
}

/* set_active_process
 *	  DESCRIPTION: sets the active_processes array at index
 *				   'term_num' with the process id.
 *    INPUTS: term_num - index in the active_process array
 *			  pid - process id indicating the active process
 *    RETURN VALUE: 0 on success, -1 on failure.
 */
int32_t set_active_process(uint32_t term_num, int32_t pid){
	if(term_num < 0 || term_num >= MAX_TERMINALS) return -1;

	active_processes[term_num] = pid;
	return 0;
}
