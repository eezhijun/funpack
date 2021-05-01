/*
 * @Author: your name
 * @Date: 2021-03-01 21:55:49
 * @LastEditTime: 2021-03-25 22:02:31
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \maxim\App\Src\watch.c
 */
#include <stdio.h>
#include <string.h>
#include "watch.h"
#include "math.h"
#include "rtc.h"
#include "gui.h"
#include "st7789.h"
#include "task.h"
#include "user_uart.h"
/***** Definitions *****/



#define USE_SYSTEM_CLK                  1
#define SYSTICK_PERIOD_SYS_CLK          (SystemCoreClock / 1000)   // 1ms with 96MHz system clock   1/(96MHz/96000Hz) = 0.001s = 1ms
#define SYSTICK_PERIOD_EXT_CLK          3277    //100ms with 32768Hz external RTC clock
#define MSEC_TO_RSSA(x)                 (0 - ((x * 256) / 1000)) /* Converts a time in milleseconds to the equivalent RSSA register value. */

static void display_init_time(void);
static void display_real_time(void);

uint32_t ss_interval = SUBSECOND_MSEC_0;
param_time_t p_time;
real_time_t r_time;


const gpio_cfg_t TEST[] = {
    {PORT_0, PIN_2, GPIO_FUNC_OUT, GPIO_PAD_NONE},
};

/**
 * @description: 时间显示初始化  
 * @param None
 * @return None
 */
void watch_init(void)
{
	uint32_t sysTicks;
	
	rtc_init();
	uart_init();
	GPIO_Config(TEST);
	GPIO_OutClr(TEST);
	
	p_time.sec[0] = 0;
	p_time.sec[1] = 0;
	p_time.min[0] = 0;
	p_time.min[1] = 0;
	p_time.hr[0] = 0;
	p_time.hr[1] = 0;
	p_time.day[0] = 0;
	p_time.day[1] = 0;
	p_time.mon[0] = 0;
	p_time.mon[1] = 0;
	p_time.year[0] = 0;
	p_time.year[1] = 0;
	p_time.year[2] = 0;
	p_time.year[3] = 0;
	p_time.update_state = UPDATE_INVALID;
	
	/* year */
	LCD_ShowIntNum(WATCH_W, WATCH_H - 25, p_time.year[3], 1, WHITE, BLACK, 24);
	LCD_ShowIntNum(WATCH_W + 15, WATCH_H - 25, p_time.year[2], 1, WHITE, BLACK, 24);
	LCD_ShowIntNum(WATCH_W + 30, WATCH_H - 25, p_time.year[1], 1, WHITE, BLACK, 24);
	LCD_ShowIntNum(WATCH_W + 45, WATCH_H - 25, p_time.year[0], 1, WHITE, BLACK, 24);
	LCD_ShowChar(WATCH_W + 60, WATCH_H - 20, '/', WHITE, BLACK, 16, 0);
	/* mon */
	LCD_ShowIntNum(WATCH_W + 75, WATCH_H - 25, p_time.mon[1], 1, WHITE, BLACK, 24);
	LCD_ShowIntNum(WATCH_W + 90, WATCH_H - 25, p_time.mon[0], 1, WHITE, BLACK, 24);
	LCD_ShowChar(WATCH_W + 105, WATCH_H - 20, '/', WHITE, BLACK, 16, 0);
	/* day */
	LCD_ShowIntNum(WATCH_W + 120, WATCH_H - 25, p_time.day[1], 1, WHITE, BLACK, 24);
	LCD_ShowIntNum(WATCH_W + 135, WATCH_H - 25, p_time.day[0], 1, WHITE, BLACK, 24);
	LCD_ShowChinese(WATCH_W + 150, WATCH_H - 25, (uint8_t*)"星期天", WHITE, BLACK, 24, 0);
	/* hr */
	LCD_ShowIntNum(WATCH_W, WATCH_H, p_time.hr[1], 1, WHITE, BLACK, 32);
	LCD_ShowIntNum(WATCH_W + 20, WATCH_H, p_time.hr[0], 1, WHITE, BLACK, 32);
	LCD_ShowChar(WATCH_W + 40, WATCH_H - 2, ':', WHITE, BLACK, 32, 0);
	/* min */
	LCD_ShowIntNum(WATCH_W + 60, WATCH_H, p_time.min[1], 1, WHITE, BLACK, 32);
	LCD_ShowIntNum(WATCH_W + 80, WATCH_H, p_time.min[0], 1, WHITE, BLACK, 32);
	LCD_ShowChar(WATCH_W + 100, WATCH_H - 2, ':', WHITE, BLACK, 32, 0);
	/* sec */
	LCD_ShowIntNum(WATCH_W + 120, WATCH_H, p_time.sec[1], 1, WHITE, BLACK, 32);
	LCD_ShowIntNum(WATCH_W + 140, WATCH_H, p_time.sec[0], 1, WHITE, BLACK, 32);
	
	/* 绘制框图 */
	LCD_DrawRectangle(0, 0, LCD_W - 1, LCD_H - 1, WHITE);
	LCD_DrawLine(0, 65, LCD_W - 1, 65, WHITE);
	
	/* HR和SPO2 */
	LCD_ShowString(WATCH_W, 70, (uint8_t*)"HR  :", YELLOW, BLACK, 24, 0);
	LCD_ShowString(WATCH_W, 100, (uint8_t*)"SpO2:", BLUE, BLACK, 24, 0);
	
	/* sys定时器初始化 */
	if (USE_SYSTEM_CLK) {
        sysTicks = SYSTICK_PERIOD_SYS_CLK;
    } else {
        sysTicks = SYSTICK_PERIOD_EXT_CLK;
    }
    SYS_SysTick_Config(sysTicks, USE_SYSTEM_CLK, MXC_TMR0);
}

/**
 * @description: RTC初始化
 * @param None
 * @return None
 */
void rtc_init(void)
{
	sys_cfg_rtc_t sys_cfg;
	
	NVIC_EnableIRQ(RTC_IRQn);
	sys_cfg.tmr = MXC_TMR0;
    if (RTC_Init(MXC_RTC, 0, 0, &sys_cfg) != E_NO_ERROR) {
        printf("Failed RTC_Setup().\n");
		while(1);
    }
	
	if (RTC_SetTimeofdayAlarm(MXC_RTC, TIME_OF_DAY_SEC) != E_NO_ERROR) {
        printf("Failed RTC_SetTimeofdayAlarm().\n");
        while(1);
    }

    if (RTC_EnableTimeofdayInterrupt(MXC_RTC) != E_NO_ERROR) {
        printf("Failed RTC_EnableTimeofdayInterrupt().\n");
        while(1);
    }

    if (RTC_SetSubsecondAlarm(MXC_RTC,  (uint32_t)MSEC_TO_RSSA(SUBSECOND_MSEC_0)) != E_NO_ERROR) {
        printf("Failed RTC_SetSubsecondAlarm().\n");
        while(1);
    }

    if (RTC_EnableSubsecondInterrupt(MXC_RTC) != E_NO_ERROR) {
        printf("Failed RTC_EnableSubsecondInterrupt().\n");
        while(1);
    }

    if (RTC_EnableRTCE(MXC_RTC) != E_NO_ERROR) {
        printf("Failed RTC_EnableRTCE().\n");
        while(1);
    }
}


/**
 * @description: 时间显示
 * @param None
 * @return None
 */
void lcd_display_time(void)
{
	switch(p_time.update_state)
	{
		case UPDATE_INVALID:
			/* 已校准时间 50ms*/
			display_init_time();
			break;
		case UPDATE_SUCCESS:
			/* 初始化时间 200ms*/
			display_real_time();
			break;
		case UPDATE_FAILED:
			break;
		default:
			break;
	}
}

/**
 * @description: 显示已校准时间
 * @param None
 * @return None
 */
static void display_real_time(void)
{
	uint32_t ge, shi, bai, qian, tmp;
	uint8_t enum_tmp;
	
    r_time.sec = RTC_GetSecond();
	r_time.update.bit.usec = UPDATE_ENABLE;
	
	/* sec */
	tmp = r_time.sec + p_time.sec[1] * 10 + p_time.sec[0];
	if((tmp % 60) == 0)
	{
		r_time.min = tmp / 60;
		r_time.update.bit.umin = UPDATE_ENABLE;
	}
	tmp %= 60;
	ge = tmp % 10;
	shi = tmp / 10;
	if(r_time.update.bit.usec)
	{
		r_time.update.bit.usec = UPDATE_DISABLE;
		LCD_ShowIntNum(WATCH_W + 120, WATCH_H, shi, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W + 140, WATCH_H, ge, 1, WHITE, BLACK, 32);
	}
	/* min */
	tmp = r_time.min + p_time.min[1] * 10 + p_time.min[0];
	if((tmp % 60) == 0)
	{
		r_time.hr = tmp / 60;
		r_time.update.bit.uhr = UPDATE_ENABLE;
	}
	tmp %= 60;
	ge = tmp % 10;
	shi = tmp / 10;
	if(r_time.update.bit.umin)
	{
		r_time.update.bit.umin = UPDATE_DISABLE;
		LCD_ShowIntNum(WATCH_W + 60, WATCH_H, shi, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W + 80, WATCH_H, ge, 1, WHITE, BLACK, 32);
		LCD_ShowChar(WATCH_W + 100, WATCH_H - 2, ':', WHITE, BLACK, 32, 0);	
	}
	/* hr */
	tmp = r_time.hr + p_time.hr[1] * 10 + p_time.hr[0];
	if((tmp % 24) == 0)
	{
		r_time.day = tmp / 24;
		r_time.update.bit.uday = UPDATE_ENABLE;
	}
	tmp %= 24;
	ge = tmp % 10;
	shi = tmp / 10;
	if(r_time.update.bit.uhr)
	{
		r_time.update.bit.uhr = UPDATE_DISABLE;
		LCD_ShowIntNum(WATCH_W, WATCH_H, shi, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W + 20, WATCH_H, ge, 1, WHITE, BLACK, 32);
		LCD_ShowChar(WATCH_W + 40, WATCH_H - 2, ':', WHITE, BLACK, 32, 0);	
	}
	/* day */
	enum_tmp = (month_enum_t)(p_time.mon[1] * 10 + p_time.mon[0]);
	tmp = r_time.day + p_time.day[1]*10 + p_time.day[0];
	if(enum_tmp == JAN || enum_tmp == MAR || enum_tmp == MAY || enum_tmp == JUL || enum_tmp == AUG ||
			enum_tmp == OCT || enum_tmp == DEC)
	{
		if((tmp % 31) == 0)
		{
			r_time.mon = tmp / 31;
			r_time.update.bit.umon = UPDATE_ENABLE;
		}
		tmp %= 31;
	}
	else if(enum_tmp == APR || enum_tmp == JUN || enum_tmp == SEP || enum_tmp ==NOV)
	{
		if((tmp % 30 == 0))
		{
			r_time.mon = tmp / 30;
			r_time.update.bit.umon = UPDATE_ENABLE;
		}
		tmp %= 30;
	}
	else
	{
		if((tmp % 28 == 0))
		{
			r_time.mon = tmp / 28;
			r_time.update.bit.umon = UPDATE_ENABLE;
		}
		tmp %= 28;
	}
	ge = tmp % 10;
	shi = tmp / 10;
	if(r_time.update.bit.uday)
	{
		r_time.update.bit.uday = UPDATE_DISABLE;
		LCD_ShowIntNum(WATCH_W + 120, WATCH_H - 25, shi, 1, WHITE, BLACK, 24);
		LCD_ShowIntNum(WATCH_W + 135, WATCH_H - 25, ge, 1, WHITE, BLACK, 24);
	}
	/* mon */
	tmp = r_time.mon + p_time.mon[1]*10 + p_time.mon[0];
	if((tmp % 12) == 0)
	{
		r_time.year = tmp / 12;
		r_time.update.bit.uyear = UPDATE_ENABLE;
	}
	tmp %= 12;
	ge = tmp % 10;
	shi = tmp / 10;
	if(r_time.update.bit.umon)
	{
		r_time.update.bit.umon = UPDATE_DISABLE;
		LCD_ShowIntNum(WATCH_W + 75, WATCH_H - 25, shi, 1, WHITE, BLACK, 24);
		LCD_ShowIntNum(WATCH_W + 90, WATCH_H - 25, ge, 1, WHITE, BLACK, 24);
		LCD_ShowChar(WATCH_W + 105, WATCH_H - 20, '/', WHITE, BLACK, 16, 0);
	}

	/* year */
	tmp = r_time.year + p_time.year[3] * 1000 + p_time.year[2] * 100 + p_time.year[1] * 10 + p_time.year[0];
	ge = tmp % 10;
	shi = tmp / 10 % 10;
	bai = tmp / 100 % 10;
	qian = tmp / 1000 % 10;
	if(r_time.update.bit.uyear)
	{
		r_time.update.bit.uyear = UPDATE_DISABLE;
		LCD_ShowIntNum(WATCH_W, WATCH_H - 25, qian, 1, WHITE, BLACK, 24);
		LCD_ShowIntNum(WATCH_W + 15, WATCH_H - 25, bai, 1, WHITE, BLACK, 24);
		LCD_ShowIntNum(WATCH_W + 30, WATCH_H - 25, shi, 1, WHITE, BLACK, 24);
		LCD_ShowIntNum(WATCH_W + 45, WATCH_H - 25, ge, 1, WHITE, BLACK, 24);
		LCD_ShowChar(WATCH_W + 60, WATCH_H - 20, '/', WHITE, BLACK, 16, 0);	
	}
	/* wday */
	enum_tmp = (wday_enum_t)p_time.wday;
	if(r_time.update.bit.uwday)
	{
		r_time.update.bit.uwday = UPDATE_DISABLE;
		switch(enum_tmp)
		{
			case SUN:
				LCD_ShowChinese(WATCH_W + 150, WATCH_H - 25, (uint8_t*)"星期天", WHITE, BLACK, 24, 0);
				break;
			case MON:
				LCD_ShowChinese(WATCH_W + 150, WATCH_H - 25, (uint8_t*)"星期一", WHITE, BLACK, 24, 0);
				break;
			case TUES:
				LCD_ShowChinese(WATCH_W + 150, WATCH_H - 25, (uint8_t*)"星期二", WHITE, BLACK, 24, 0);
				break;
			case WED:
				LCD_ShowChinese(WATCH_W + 150, WATCH_H - 25, (uint8_t*)"星期三", WHITE, BLACK, 24, 0);
				break;
			case THUR:
				LCD_ShowChinese(WATCH_W + 150, WATCH_H - 25, (uint8_t*)"星期四", WHITE, BLACK, 24, 0);
				break;
			case FRI:
				LCD_ShowChinese(WATCH_W + 150, WATCH_H - 25, (uint8_t*)"星期五", WHITE, BLACK, 24, 0);
				break;
			case SAT:
				LCD_ShowChinese(WATCH_W + 150, WATCH_H - 25, (uint8_t*)"星期六", WHITE, BLACK, 24, 0);
				break;
			default:
				break;
		}
	}	
}

/**
 * @description: 初始化时间显示 
 * @param None
 * @return None
 */
static void display_init_time(void)
{	
	uint32_t day, hr, min, sec;
	uint32_t ge, shi;
    double subsec;

    subsec = RTC_GetSubSecond() / 256.0;
    sec = RTC_GetSecond();

    day = sec / SECS_PER_DAY;
    sec -= day * SECS_PER_DAY;

    hr = sec / SECS_PER_HR;
    sec -= hr * SECS_PER_HR;

    min = sec / SECS_PER_MIN;
    sec -= min * SECS_PER_MIN;

    subsec += sec;
	
	/* sec */
	if(sec < 10)
	{
		LCD_ShowIntNum(WATCH_W + 140, WATCH_H, sec, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W + 120, WATCH_H, 0, 1, WHITE, BLACK, 32);
	}
	else
	{
		ge = sec % 10;
		shi = sec / 10;
		LCD_ShowIntNum(WATCH_W + 140, WATCH_H, ge, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W + 120, WATCH_H, shi, 1, WHITE, BLACK, 32);
	}
	/* min */
	if(min < 10)
	{
		LCD_ShowIntNum(WATCH_W + 80, WATCH_H, min, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W + 60, WATCH_H, 0, 1, WHITE, BLACK, 32);
	}
	else
	{
		ge = min % 10;
		shi = min / 10;
		LCD_ShowIntNum(WATCH_W + 80, WATCH_H, ge, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W + 60, WATCH_H, shi, 1, WHITE, BLACK, 32);
	}
	/* hour */
	if(hr < 10)
	{
		LCD_ShowIntNum(WATCH_W + 20, WATCH_H, hr, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W, WATCH_H, 0, 1, WHITE, BLACK, 32);
	}
	else
	{
		ge = hr % 10;
		shi = hr / 10;
		LCD_ShowIntNum(WATCH_W + 20, WATCH_H, ge, 1, WHITE, BLACK, 32);
		LCD_ShowIntNum(WATCH_W, WATCH_H, shi, 1, WHITE, BLACK, 32);
	}
	LCD_ShowChar(WATCH_W + 40, WATCH_H - 2, ':', WHITE, BLACK, 32, 0);
	LCD_ShowChar(WATCH_W + 100, WATCH_H -2, ':', WHITE, BLACK, 32, 0);
	
}

/**
 * @description: 
 * @param None
 * @return None
 */
/* sys定时器 */
void SysTick_Handler(void)
{

	/* 1ms */
	task_cnt.millis++;
	task_cnt.cnt1ms++;
	/* 10ms */
	if (task_cnt.cnt1ms >= 10)
	{
		task_cnt.cnt1ms = 0;
		task_cnt.cnt10ms++;
 		
	}
	/* 100ms */
	if(task_cnt.cnt10ms >= 10)
	{
		task_status.body.bit.display_task = ENABLE;
		task_cnt.cnt10ms = 0;
	}
}

/**
 * @description: RTC中断回调函数
 * @param None
 * @return None
 */
/***** Functions *****/
void RTC_IRQHandler(void)
{
    int time;
    int flags = RTC_GetFlags();

    /* Check sub-second alarm flag. */
    if (flags & MXC_F_RTC_CTRL_ALSF) {
//        LED_Toggle(LED_ALARM);
        RTC_ClearFlags(MXC_F_RTC_CTRL_ALSF);
    }

    /* Check time-of-day alarm flag. */
    if (flags & MXC_F_RTC_CTRL_ALDF) {
        RTC_ClearFlags(MXC_F_RTC_CTRL_ALDF);

        /* Set a new alarm 10 seconds from current time. */
        time = RTC_GetSecond();
        if (RTC_SetTimeofdayAlarm(MXC_RTC, time + TIME_OF_DAY_SEC) != E_NO_ERROR) {
            /* Handle Error */
        }

        // Toggle the sub-second alarm interval.
        if (ss_interval == SUBSECOND_MSEC_0) {
            ss_interval = SUBSECOND_MSEC_1;
        } else {
            ss_interval = SUBSECOND_MSEC_0;
        }

        if (RTC_SetSubsecondAlarm(MXC_RTC, MSEC_TO_RSSA(ss_interval)) != E_NO_ERROR) {
            /* Handle Error */
        }
    }

    return;
}
