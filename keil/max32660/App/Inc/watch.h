#ifndef _WATCH_H
#define _WATCH_H

#include "mxc_config.h"

#define WATCH_H             30
#define WATCH_W             10
/* */
#define TIME_OF_DAY_SEC     2
#define SUBSECOND_MSEC_0    125
#define SUBSECOND_MSEC_1    250
#define SECS_PER_MIN        60
#define SECS_PER_HR         (60 * SECS_PER_MIN)
#define SECS_PER_DAY        (24 * SECS_PER_HR)

typedef enum 
{
	UPDATE_DISABLE              = 0x0,
	UPDATE_ENABLE               = 0x1,
} real_time_update_t;

typedef enum
{
	UPDATE_SUCCESS = 0x0,
	UPDATE_FAILED,
	UPDATE_INVALID,
} updata_state_t;

typedef enum
{
	JAN = 0x01,
	FEB,
	MAR,
	APR,
	MAY,
	JUN,
	JUL,
	AUG,
	SEP,
	OCT,
	NOV,
	DEC,
} month_enum_t;

typedef enum
{
	SUN = 0x00,
	MON,
	TUES,
	WED,
	THUR,
	FRI,
	SAT,
} wday_enum_t;

typedef struct
{
	uint8_t sec[2];
	uint8_t min[2];
	uint8_t hr[2];
	uint8_t day[2];
	uint8_t mon[2];
	uint8_t year[4];
	uint8_t wday;
	updata_state_t update_state;
} param_time_t;

typedef struct
{
	uint32_t sec;
	uint32_t min;
	uint32_t hr;
	uint32_t day;
	uint32_t mon;
	uint32_t year;
	union
	{
		struct
		{
			real_time_update_t usec          :1;
			real_time_update_t umin          :1;
			real_time_update_t uhr           :1;
			real_time_update_t uday          :1;
			real_time_update_t umon          :1;
			real_time_update_t uyear         :1;
			real_time_update_t uwday         :1;
		} bit;
		uint8_t all;
	} update;
} real_time_t;

extern param_time_t p_time;
extern real_time_t r_time;
extern const gpio_cfg_t TEST[];

void watch_init(void);
void rtc_init(void);
void lcd_display_time(void);
#endif

