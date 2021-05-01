#ifndef _ST7789_H_
#define _ST7789_H_		

#include "mxc_config.h"

#define USE_HORIZONTAL 2  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 135
#define LCD_H 240
#else
#define LCD_W 240
#define LCD_H 135
#endif


/*******
		GND
		VCC
		SCL P0_12
		SDA	P0_11
		RES	P0_4  --> P0_11
		DC	P0_3
		CS	P0_2
		BLK	P0_1  --> P0_10
******/

#define ST7789_RST_Set()  GPIO_OutSet(ST7789_RES) 
#define ST7789_RST_Clr()  GPIO_OutClr(ST7789_RES)  //RST

#define ST7789_DC_Set()   GPIO_OutSet(ST7789_DC)
#define ST7789_DC_Clr()   GPIO_OutClr(ST7789_DC)   //DC

#define ST7789_CS_Set()   GPIO_OutSet(ST7789_CS) 
#define ST7789_CS_Clr()   GPIO_OutClr(ST7789_CS)   //CS

#define ST7789_BLK_Set()  GPIO_OutSet(ST7789_BLK)
#define ST7789_BLK_Clr()  GPIO_OutClr(ST7789_BLK)  //BLK

void LCD_Writ_Bus(uint8_t dat);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA(uint16_t dat);
void LCD_WR_REG(uint8_t dat);
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
void lcd_init(void);


#endif  





