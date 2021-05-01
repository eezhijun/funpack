#ifndef _MAX30100_H_
#define _MAX30100_H_

#include "mxc_config.h"
#include "blood.h"

#define MAX30100_WR_ADDR 0xAE
#define MAX30100_RD_ADDR 0xAF

#define I2C_WRITE_ADDR   0xAE
#define I2C_READ_ADDR    0xAF

/* ÖÐ¶Ï×´Ì¬¼Ä´æÆ÷±êÖ¾Î» */
#define INTERRUPT_STATUS_REG_A_FULL  			(0x01 << 7)
#define INTERRUPT_STATUS_REG_TEMP_RDY  			(0x01 << 6)
#define INTERRUPT_STATUS_REG_HR_RDY  			(0x01 << 5)
#define INTERRUPT_STATUS_REG_SPO2_RDY  			(0x01 << 4)
#define INTERRUPT_STATUS_REG_PWR_RDY  			(0x01 << 0)
/* register addresses */
#define INTERRUPT_STATUS_REG            0x00
#define INTERRUPT_ENABLE_REG            0x01
#define FIFO_WRITE_PTR_REG              0x02
#define OVER_FLOW_COUNTER_REG           0x03
#define FIIO_READ_PTR_REG               0x04
#define FIFO_DATA_REG                   0x05
#define MOED_CONFIG_REG                 0x06
#define SPO2_CONFIG_REG                 0x07
#define LED_CONFIG_REG                  0x09
#define TEMP_INTEGER_REG                0x16
#define TEMP_FRACTION_REG               0x17
#define REV_ID_REG                      0xFE
#define PART_ID_REG                     0xFF

#define SAMPLES_PER_SECOND 			    100	//¼ì²âÆµÂÊ

void max30100_init(void);  
uint8_t max30100_bus_read(uint8_t reg_add);
void max30100_fifo_readBytes(uint8_t register_address, fifo_t *raw_data, uint8_t count);
uint8_t max30100_bus_write(uint8_t reg_add, uint8_t word_data);
#endif

