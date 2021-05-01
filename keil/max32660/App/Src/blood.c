#include <math.h>
#include <stdio.h>
#include <string.h>
#include "blood.h"
#include "max30100.h"
#include "algorithm.h"
#include "gui.h"
#include "user_uart.h"
#include "task.h"

//static void lcd_draw_hrsp(void);
blood_t blood;

/**
 * @description: 心率和血氧参数初始化
 * @param None
 * @return None
 */
void blood_setup(void)
{
	blood.currentPulseDetectorState = PULSE_IDLE;
	blood.dcFilterIR.w = 0;
	blood.dcFilterIR.result = 0;

	blood.dcFilterRed.w = 0;
	blood.dcFilterRed.result = 0;

	blood.lpbFilterIR.v[0] = 0;
	blood.lpbFilterIR.v[1] = 0;
	blood.lpbFilterIR.result = 0;

	blood.meanDiffIR.index = 0;
	blood.meanDiffIR.sum = 0;
	blood.meanDiffIR.count = 0;	
	
	blood.valuesBPM[0] = 0;
	blood.valuesBPMSum = 0;
	blood.valuesBPMCount = 0;
	blood.bpmIndex = 0;


	blood.irACValueSqSum = 0;
	blood.redACValueSqSum = 0;
	blood.samplesRecorded = 0;
	blood.pulsesDetected = 0;
	blood.currentSaO2Value = 0;

	blood.lastBeatThreshold = 0;
}

/**
 * @description: 心率和血氧数据更新 
 * @param None
 * @return None
 */
void blood_data_update(void)
{
	fifo_t raw_data;
	uint16_t temp_num = 0;
	char buf[100] = {0};
	uint8_t index = 0;
	
	temp_num = max30100_bus_read(INTERRUPT_STATUS_REG);
	
	//标志位被使能时 读取FIFO
	if(INTERRUPT_STATUS_REG_A_FULL & temp_num)
	{
		//读取FIFO
		max30100_fifo_readBytes(FIFO_DATA_REG, &raw_data, 1); //read the hr and spo2 data form fifo in reg=0x05
//		index = sprintf(buf, "%d", raw_data.rawIR);
//		print_data((uint8_t*)buf, index);
		
		/* 直流滤波 */
		blood.dcFilterIR = dcRemoval((float)raw_data.rawIR, blood.dcFilterIR.w, ALPHA);
		blood.dcFilterRed = dcRemoval((float)raw_data.rawRed, blood.dcFilterRed.w, ALPHA);
//		index = sprintf(buf, "%.6f", dcFilterIR.result);
//		print_data((uint8_t*)buf, index);
		
		/* 均值中值滤波 */
		float meanDiffResIR = meanDiff(blood.dcFilterIR.result, &blood.meanDiffIR);
		float meanDiffResRed = meanDiff(blood.dcFilterRed.result, &blood.meanDiffRed);
//		index = sprintf(buf, "%.6f", meanDiffResIR);
//		print_data((uint8_t*)buf, index);
		
		/* 巴特沃斯过滤器 */
		lowPassButterworthFilter(meanDiffResIR, &blood.lpbFilterIR);
		lowPassButterworthFilter(meanDiffResRed, &blood.lpbFilterRed);
		
		blood.irACValueSqSum += blood.dcFilterIR.result * blood.dcFilterIR.result;
		blood.redACValueSqSum += blood.dcFilterRed.result * blood.dcFilterRed.result;
//		index = sprintf(buf, "%.6f", blood.lpbFilterIR.result);
//		buf[index] = ',';
//		index += 1;
//		index += sprintf(&buf[index], "%.6f", blood.lpbFilterRed.result);
//		print_data((uint8_t*)buf, index);
		
		blood.samplesRecorded++;
		/* 心跳脉冲检测 */
		if(detectPulse(blood.lpbFilterIR.result))
		{
			blood.updateDisplay = BLOOD_UPDATE_DISPLAY_ENABLE;
			blood.pulsesDetected++;
			float red_log_rms = log(sqrt(blood.redACValueSqSum / blood.samplesRecorded));
			float ir_log_rms = log(sqrt(blood.irACValueSqSum / blood.samplesRecorded) );
			float ratioRMS = 0.0f;
			if(red_log_rms != 0.0f && ir_log_rms != 0.0f)
			{
				ratioRMS = red_log_rms / ir_log_rms;
		    }
			blood.currentSaO2Value = 110.0f - 14.0f * ratioRMS;	// SPo2 value by pulse-rate
//			index = sprintf(buf, "%.6f", blood.currentSaO2Value);
//			print_data((uint8_t*)buf, index);
			if(blood.pulsesDetected % RESET_SPO2_EVERY_N_PULSES == 0)
			{
				blood.irACValueSqSum = 0;
				blood.redACValueSqSum = 0;
				blood.samplesRecorded = 0;
			} 
		} 
		balanceIntesities(blood.dcFilterRed.w, blood.dcFilterIR.w);
	}
}

/**
 * @description: 心率和血氧数据更新显示
 * @param None
 * @return None
 */
void lcd_display_update(void)
{
	uint8_t str[50];
	
	if(blood.updateDisplay)
	{
		blood.updateDisplay = BLOOD_UPDATE_DISPLAY_DISABLE;
		/* 心率信息显示 */
		sprintf((char*)str, "%.2f  ", blood.currentBPM);
		//	sprintf((char*)str, "%3d  ", 98);
		LCD_ShowString(70, 70, str, YELLOW, BLACK, 24, 0);
		/* 血氧饱和度显示 */
		blood.currentSaO2Value = (blood.currentSaO2Value > 99.99f) ? 99.99f : blood.currentSaO2Value;
		sprintf((char*)str, "%2.2f%%  ", blood.currentSaO2Value);
		LCD_ShowString(70, 100, str, BLUE, BLACK, 24, 0);		
	}

}

/* 直流滤波 */
dcFilter_t dcRemoval(float x, float prev_w, float alpha)
{
	dcFilter_t filtered;
	
	filtered.w = x + alpha * prev_w;
	filtered.result = filtered.w - prev_w;
	
	return filtered;
}

/* 均值中值滤波 */
float meanDiff(float M, meanDiffFilter_t* filterValues)
{
	float avg = 0;
	filterValues->sum -= filterValues->values[filterValues->index];
	filterValues->values[filterValues->index] = M;
	filterValues->sum += filterValues->values[filterValues->index];

	filterValues->index++;
	filterValues->index = filterValues->index % MEAN_FILTER_SIZE;

	if(filterValues->count < MEAN_FILTER_SIZE)
	{
	    filterValues->count++;	
	}

	avg = filterValues->sum / filterValues->count;
	return avg - M;
}

/* 巴特沃斯过滤器 */
void lowPassButterworthFilter(float x, butterworthFilter_t* filterResult)
{
	filterResult->v[0] = filterResult->v[1];

	//Fs = 100Hz and Fc = 10Hz
	filterResult->v[1] = (2.452372752527856026e-1f * x) + (0.50952544949442879485f * filterResult->v[0]);

    //Fs = 100Hz and Fc = 4Hz
    //filterResult->v[1] = (1.367287359973195227e-1 * x) + (0.72654252800536101020 * filterResult->v[0]); //Very precise butterworth filter 

    filterResult->result = filterResult->v[0] + filterResult->v[1];
}

/* 心率和血氧检测 */
uint8_t detectPulse(float sensor_value)
{
	char buf[100] = {0};
	uint8_t index = 0;
	static float prev_sensor_value = 0;
	static uint8_t values_went_down = 0;
	static uint32_t currentBeat = 0;
	static uint32_t lastBeat = 0;
	
	if(sensor_value > PULSE_MAX_THRESHOLD)
    {
      blood.currentPulseDetectorState = PULSE_IDLE;
      prev_sensor_value = 0;
      lastBeat = 0;
      currentBeat = 0;
      values_went_down = 0;
      blood.lastBeatThreshold = 0;
      return 0;
    }
	
	switch(blood.currentPulseDetectorState)
    {
	case PULSE_IDLE:
		if(sensor_value >= PULSE_MIN_THRESHOLD) 
		{
			blood.currentPulseDetectorState = PULSE_TRACE_UP;
			values_went_down = 0;
		}
	break;

    case PULSE_TRACE_UP:
		if(sensor_value > prev_sensor_value)
		{
			currentBeat = task_cnt.millis;
			blood.lastBeatThreshold = sensor_value;
		}
		else
		{
//			index = sprintf(buf, "%.6f", sensor_value);
//			buf[index] = ',';
//		    index += 1;
//		    index += sprintf(&buf[index], "%.6f", prev_sensor_value);
//			print_data((uint8_t*)buf, index);
			
		    uint32_t beatDuration = currentBeat - lastBeat;
			lastBeat = currentBeat;

			float rawBPM = 0;
			if(beatDuration > 0)
			{
				rawBPM = 60000.0f / (float)beatDuration;	
			}
//			index = sprintf(buf, "%d", rawBPM);
//			print_data((uint8_t*)buf, index);
			
			blood.valuesBPM[blood.bpmIndex] = rawBPM;
			blood.valuesBPMSum = 0;
			for(int i = 0; i < PULSE_BPM_SAMPLE_SIZE; i++)
			{
			   blood.valuesBPMSum += blood.valuesBPM[i];
			}

			blood.bpmIndex++;
			blood.bpmIndex = blood.bpmIndex % PULSE_BPM_SAMPLE_SIZE;

			if(blood.valuesBPMCount < PULSE_BPM_SAMPLE_SIZE)
			{
				blood.valuesBPMCount++;	
			}
			index = sprintf(buf, "%0.6f", blood.valuesBPMSum);
			buf[index] = ',';
		    index += 1;
		    index += sprintf(&buf[index], "%d", blood.valuesBPMCount);

			blood.currentBPM = blood.valuesBPMSum / blood.valuesBPMCount;
			buf[index] = ',';
			index += 1;
		    index += sprintf(&buf[index], "%0.6f", blood.currentBPM);
			print_data((uint8_t*)buf, index);
			blood.currentPulseDetectorState = PULSE_TRACE_DOWN;
			return 1;
		}
	 break;
     case PULSE_TRACE_DOWN:
		 if(sensor_value < prev_sensor_value)
		 {
			values_went_down++;
		 }
		 if(sensor_value < PULSE_MIN_THRESHOLD)
		 {
			blood.currentPulseDetectorState = PULSE_IDLE;
		 }
	 break;
   }
   prev_sensor_value = sensor_value;
   return 0;
	
}

/* */
void balanceIntesities(float redLedDC, float IRLedDC)
{
	if(task_cnt.millis - blood.lastREDLedCurrentCheck >= RED_LED_CURRENT_ADJUSTMENT_MS)
	{
		if( IRLedDC - redLedDC > MAGIC_ACCEPTABLE_INTENSITY_DIFF && blood.redLEDCurrent < MAX30100_LED_CURRENT_50MA) 
		{
		  blood.redLEDCurrent++;
		  max30100_bus_write(LED_CONFIG_REG, (blood.redLEDCurrent << 4) | blood.IrLedCurrent);
		} 
	}
	else if(redLedDC - IRLedDC > MAGIC_ACCEPTABLE_INTENSITY_DIFF && blood.redLEDCurrent > 0) 
    {
      blood.redLEDCurrent--;
      max30100_bus_write(LED_CONFIG_REG, (blood.redLEDCurrent << 4) | blood.IrLedCurrent);
    }
	blood.lastREDLedCurrentCheck = task_cnt.millis;
}
