//button.

#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f0xx_hal.h"


typedef uint64_t tick_type;

#define TICKS_PER_SECOND 10000 //timer 3 running at 10,000Hz
#define LONG_CLICK_DURATION 5*TICKS_PER_SECOND //5 seconds 
#define DOUBLE_CLICK_PERIOD TICKS_PER_SECOND // double click must occur within 1/2 sec
#define KEY_BOUNCE_PERIOD 0.1*TICKS_PER_SECOND
#define NUM_BUTTONS 1

//function prototypes
void buttonTask(void);
void buttonInit(void);
int oneBtnTask(int);
#endif
