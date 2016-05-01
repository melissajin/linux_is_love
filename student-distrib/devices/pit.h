#ifndef PIT_H
#define PIT_H
	
#include "../types.h"
#include "../process.h"

//initialize the programmable interrupt timer
void pit_init();

//the interrupt handler for the PIT and also the scheduler
void pit_handler_main();

//Sets the rate to the argument rate (in hertz)
void pit_set_rate(uint16_t rate);

//schedules a process for execution
void schedule_for_execution(uint8_t * command);

//gets the current count on the PIT
uint16_t pit_get_count();

#endif
