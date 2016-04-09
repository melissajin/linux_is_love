#include "process.h"
#include "lib.h"

#define MAX_DEVICES 6

typedef struct {
	char * name;
	fops_t * fops;
} dev_t;

static dev_t devices[MAX_DEVICES];
static int num_devices = 0;

int add_device(char * name, fops_t * fops) {
	if(num_devices == MAX_DEVICES) return -1;

	devices[num_devices].name = name;
	devices[num_devices].fops = fops;
	num_devices++;
	return 0;
}

fops_t * get_device_fops(char * req_name) {
	int i;
	char * dev_name;
	for(i = 0; i < num_devices; i++) {
		dev_name = devices[i].name;
		if(strncmp(req_name, dev_name, strlen(req_name))) {
			return devices[i].fops;
		}
	}
	return NULL;
}
