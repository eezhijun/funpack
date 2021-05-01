#ifndef _TASK_H_
#define _TASK_H_

#include "mxc_config.h"

typedef enum
{
	DISABLE = 0,
	ENABLE = !DISABLE,
} FunctionalState;

typedef struct
{
	union
	{
		struct 
		{
			__IO uint8_t display_task           :1;
			__IO uint8_t data_process_task      :1;
		} bit;
		uint8_t all;
	} body;
} task_status_t;

typedef struct
{
	volatile uint32_t cnt1us;
	volatile uint32_t cnt1ms;
	volatile uint32_t cnt10ms;
	volatile uint32_t cnt100ms;
	volatile uint32_t millis;
} task_cnt_t;

extern task_status_t task_status;
extern task_cnt_t task_cnt;

void task_process(void);

#endif


