#include "process.h"
#include "lib.h"

#define MAX_DEVICES 6
#define MAX_PROCESSES	6

typedef struct {
	uint8_t * name;
	fops_t * fops;
} dev_t;

static int32_t procs[MAX_PROCESSES] = {0};

static dev_t devices[MAX_DEVICES];
static int num_devices = 0;

/* add_device
 *	  DESCRIPTION: 
 *    INPUTS: 
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS: 
 */
int add_device(uint8_t * name, fops_t * fops) {
	if(num_devices == MAX_DEVICES) return -1;

	devices[num_devices].name = name;
	devices[num_devices].fops = fops;
	num_devices++;
	return 0;
}

/* get_device_fops
 *	  DESCRIPTION: 
 *    INPUTS: 
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS: 
 */
fops_t * get_device_fops(const uint8_t * req_name) {
	int i;
	uint8_t * dev_name;
	for(i = 0; i < num_devices; i++) {
		dev_name = devices[i].name;
		if(strlen((int8_t *) req_name) != 0 && strncmp((int8_t *) req_name, (int8_t *) dev_name,
				strlen((int8_t *) req_name)) == 0) {
			return devices[i].fops;
		}
	}
	return NULL;
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
	return 0;
}
