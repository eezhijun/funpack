#include "task.h"
#include "rtc.h"
#include "watch.h"
#include "max30100.h"
#include "gui.h"
#include "blood.h"

task_status_t task_status;
task_cnt_t task_cnt;

/* */
void task_process(void)
{


	/* 心率血氧数据更新 */
	blood_data_update();
	/* 心率血氧数据更新显示 */
	lcd_display_update();

	/* 启动显示时间任务 */
	if (task_status.body.bit.display_task == ENABLE)
	{
		GPIO_OutSet(TEST);
		lcd_display_time();
		task_status.body.bit.display_task = DISABLE;
		GPIO_OutClr(TEST);
	}
}


