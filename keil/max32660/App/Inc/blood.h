#ifndef _BLOOD_H_
#define _BLOOD_H_

#include "mxc_config.h"

/* Adjust RED LED current balancing*/
#define MAGIC_ACCEPTABLE_INTENSITY_DIFF         65000
#define RED_LED_CURRENT_ADJUSTMENT_MS           500

/* SaO2 parameters */
#define RESET_SPO2_EVERY_N_PULSES     4

/* Filter parameters */
#define ALPHA 0.95  //dc filter alpha value
#define MEAN_FILTER_SIZE        15  

/* Pulse detection parameters */
#define PULSE_MIN_THRESHOLD         100 //300 is good for finger, but for wrist you need like 20, and there is shitloads of noise
#define PULSE_MAX_THRESHOLD         2000
#define PULSE_GO_DOWN_THRESHOLD     1
#define PULSE_BPM_SAMPLE_SIZE       10 //Moving average size

typedef enum
{
	BLOOD_UPDATE_DISPLAY_DISABLE,
	BLOOD_UPDATE_DISPLAY_ENABLE,
} BloodUpdateDisplay;

typedef enum
{
	MAX30100_LED_CURRENT_0MA              = 0x00,
    MAX30100_LED_CURRENT_4_4MA            = 0x01,
    MAX30100_LED_CURRENT_7_6MA            = 0x02,
    MAX30100_LED_CURRENT_11MA             = 0x03,
    MAX30100_LED_CURRENT_14_2MA           = 0x04,
    MAX30100_LED_CURRENT_17_4MA           = 0x05,
    MAX30100_LED_CURRENT_20_8MA           = 0x06,
    MAX30100_LED_CURRENT_24MA             = 0x07,
    MAX30100_LED_CURRENT_27_1MA           = 0x08,
    MAX30100_LED_CURRENT_30_6MA           = 0x09,
    MAX30100_LED_CURRENT_33_8MA           = 0x0A,
    MAX30100_LED_CURRENT_37MA             = 0x0B,
    MAX30100_LED_CURRENT_40_2MA           = 0x0C,
    MAX30100_LED_CURRENT_43_6MA           = 0x0D,
    MAX30100_LED_CURRENT_46_8MA           = 0x0E,
    MAX30100_LED_CURRENT_50MA             = 0x0F
} LEDCurrent;

typedef enum
{
   PULSE_IDLE,
   PULSE_TRACE_UP,
   PULSE_TRACE_DOWN
}PulseStateMachine;

typedef struct 
{
  uint8_t pulseDetected;
  float heartBPM;

  float irCardiogram;

  float irDcValue;
  float redDcValue;

  float SaO2;

  uint32_t lastBeatThreshold;

  float dcFilteredIR;
  float dcFilteredRed;
} pulseoxymeter_t;

typedef struct 
{
	uint16_t rawIR;
	uint16_t rawRed;
} fifo_t;

/* 直流滤波器 */
typedef struct  
{
	float w;
	float result;
} dcFilter_t;

/* 均值中值滤波器 */
typedef struct  
{
	float values[MEAN_FILTER_SIZE];
	uint8_t index;
	float sum;
	uint8_t count;
} meanDiffFilter_t;

/* 巴特沃斯滤波器 */
typedef struct 
{
	float v[2];
	float result;
} butterworthFilter_t;

typedef struct 
{
	uint8_t redLEDCurrent;
    volatile uint32_t lastREDLedCurrentCheck;
	
    uint8_t currentPulseDetectorState;
    float currentBPM;
    float valuesBPM[PULSE_BPM_SAMPLE_SIZE];
    float valuesBPMSum;
    uint8_t valuesBPMCount;
    uint8_t bpmIndex;
    uint32_t lastBeatThreshold;
    
    fifo_t prevFifo;
    
    dcFilter_t dcFilterIR;          
    dcFilter_t dcFilterRed;
    butterworthFilter_t lpbFilterIR;
	butterworthFilter_t lpbFilterRed;
    meanDiffFilter_t meanDiffIR;
	meanDiffFilter_t meanDiffRed;

    float irACValueSqSum;
    float redACValueSqSum;
    uint16_t samplesRecorded;
    uint16_t pulsesDetected;
    float currentSaO2Value;	

    LEDCurrent IrLedCurrent;
	BloodUpdateDisplay updateDisplay;
} blood_t;

typedef enum
{
	BLD_NORMAL,		//正常
	BLD_ERROR,		//侦测错误	
}BloodState;//血液状态

typedef struct
{
	uint16_t 		heart;		//心率数据
	float 			SpO2;			//血氧数据
	BloodState	state;		//状态
	uint8_t   	update;		//信息更新标志位
	uint8_t   	display;	//数据更新标志位
}BloodData;

void blood_data_update(void);
void blood_data_translate(void);
void blood_setup(void);
void lcd_display_update(void);
dcFilter_t dcRemoval(float x, float prev_w, float alpha);
float meanDiff(float M, meanDiffFilter_t* filterValues);
void lowPassButterworthFilter(float x, butterworthFilter_t* filterResult);
uint8_t detectPulse(float sensor_value);
void balanceIntesities(float redLedDC, float IRLedDC);
#endif

