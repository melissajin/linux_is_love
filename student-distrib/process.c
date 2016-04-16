#include "process.h"
#include "lib.h"

#define MAX_DEVICES 6
#define MAX_PROCESSES	6

typedef struct {
	uint8_t * name;
	fops_t * fops;
} dev_t;

static int32_t procs[MAX_PROCESSES + 1] = {0};

static dev_t devices[MAX_DEVICES];
static int num_devices = 0;

int add_device(uint8_t * name, fops_t * fops) {
	if(num_devices == MAX_DEVICES) return -1;

	devices[num_devices].name = name;
	devices[num_devices].fops = fops;
	num_devices++;
	return 0;
}

fops_t * get_device_fops(const uint8_t * req_name) {
	int i;
	uint8_t * dev_name;
	for(i = 0; i < num_devices; i++) {
		dev_name = devices[i].name;
		if(strncmp((int8_t *) req_name, (int8_t *) dev_name,
				strlen((int8_t *) req_name)) == 0) {
			return devices[i].fops;
		}
	}
	return NULL;
}

int32_t add_process(){
	uint32_t i;
	for(i = 1; i < MAX_PROCESSES + 1; i++){
		if(procs[i] == 0){
			procs[i] = 1;
			return i;
		}
	}
	return -1;
}

int32_t delete_process(int32_t pid){
	if(pid < 0 || pid >= MAX_PROCESSES){
		return -1;
	}
	procs[pid] = 0;
	return 0;
}
