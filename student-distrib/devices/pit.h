#ifndef PIT_H
#define PIT_H
	
#include "../types.h"
#include "../process.h"

void pit_init();
void pit_handler_main();

void pit_set_rate(uint16_t rate);

void schedule_for_execution(uint8_t * command);

uint16_t pit_get_count();

#endif
