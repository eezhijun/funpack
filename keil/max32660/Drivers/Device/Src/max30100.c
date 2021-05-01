#include <stdio.h>
#include <string.h>
#include "max30100.h"
#include "i2c.h"
#include "mxc_delay.h"

#define I2C_MASTER	    MXC_I2C0
#define I2C_MASTER_IDX	0
#define I2C_MAX_LEN     500
#define MAX_BRIGHTNESS 255

i2c_req_t req;

/*******
		GND
		VCC
		SCL P0_8
		SDA	P0_9
		INT 
******/
const gpio_cfg_t MAX30100_INT[] = {
    {PORT_0, PIN_10, GPIO_FUNC_IN, GPIO_PAD_PULL_UP},
};

void max30100_init(void)
{
	int error;
    const sys_cfg_i2c_t sys_i2c_cfg = NULL;  

	/* gpio init */
	GPIO_Config(MAX30100_INT);
	
	//Setup the I2CM
    I2C_Shutdown(I2C_MASTER);
    if((error = I2C_Init(I2C_MASTER, I2C_STD_MODE, &sys_i2c_cfg)) != E_NO_ERROR) {
        printf("Error initializing I2C%d.  (Error code = %d)\n", I2C_MASTER_IDX, error);
    }
	mxc_delay(100);
	
	/* 配置SPO2 和 HR模式 */
	max30100_bus_write(MOED_CONFIG_REG, 0x0B);
	/* 使能所有中断 */
	max30100_bus_write(INTERRUPT_ENABLE_REG, 0xF0); 
	/* Red_PA:27.1mA IR_PA:50mA */
	max30100_bus_write(LED_CONFIG_REG, 0x8F);

#if (SAMPLES_PER_SECOND == 50)
	max30100_bus_write(SPO2_CONFIG_REG, 0x43);
#elif (SAMPLES_PER_SECOND == 100)	/* 采样率100，脉宽1600us，ADC 16bit */
	max30100_bus_write(SPO2_CONFIG_REG, 0x47); 
#elif (SAMPLES_PER_SECOND == 200)
	max30100_bus_write(SPO2_CONFIG_REG, 0x4F); 
#elif (SAMPLES_PER_SECOND == 400)
	max30100_bus_write(SPO2_CONFIG_REG, 0x53); 
#endif
	
	max30100_bus_write(FIFO_WRITE_PTR_REG, 0x00);       //set FIFO write Pointer reg = 0x00 for clear it
	max30100_bus_write(OVER_FLOW_COUNTER_REG, 0x00);	//set Over Flow Counter  reg = 0x00 for clear it
	max30100_bus_write(FIIO_READ_PTR_REG, 0x0F);	    //set FIFO Read Pointer  reg = 0x0f for
							                
}

uint8_t max30100_bus_write(uint8_t reg_add, uint8_t word_data)
{
	int error;
	uint8_t txdata[2];

	txdata[0] = reg_add;
	txdata[1] = word_data;
	
	if((I2C_MasterWrite(MXC_I2C0, MAX30100_WR_ADDR, txdata, 2, 0)) != 2) {
		printf("Error writing %d\n", error);
		while(1);
    }
	
	return 0;
}

uint8_t max30100_bus_read(uint8_t reg_add)
{
	int error;
	uint8_t data;
	
	if((error = I2C_MasterWrite(MXC_I2C0, MAX30100_WR_ADDR, &reg_add, 1, 1)) != 1) {
		printf("Error writing %d\n", error);
		while(1);
    }	
	if((error = I2C_MasterRead(MXC_I2C0, MAX30100_RD_ADDR, &data, 1, 0)) != 1) {
        printf("Error reading%d\n", error);
        while(1);
    }
	
	return data;
}

void max30100_fifo_readBytes(uint8_t register_address, fifo_t *raw_data, uint8_t count)
{
	int error;
	uint8_t data[4] = {0};
	
	if((error = I2C_MasterWrite(MXC_I2C0, MAX30100_WR_ADDR, &register_address, 1, 1)) != 1) {
		printf("Error writing %d\n", error);
		while(1);
    }
	if((error = I2C_MasterRead(MXC_I2C0, MAX30100_RD_ADDR, data, 4, 0)) != 4) {
        printf("Error reading%d\n", error);
        while(1);
    }
	
	raw_data->rawIR = (((uint16_t)data[0] << 8) | data[1]);
	raw_data->rawRed = (((uint16_t)data[2] << 8) | data[3]);
}

