#include <stdio.h>
#include <string.h>
#include "st7789.h"
#include "spi.h"
#include "mxc_delay.h"

#define SPI_BAUD_RATE         2000000  /* 1.14寸屏幕支持最大SPI速率 */
/*******
		GND
		VCC
		SCL P0_6
		SDA	P0_5
		DC	P0_12
		CS	P0_7 
		RES	
		BLK	
******/
const gpio_cfg_t ST7789_DC[] = {
    {PORT_0, PIN_12, GPIO_FUNC_OUT, GPIO_PAD_NONE},
};
//const gpio_cfg_t ST7789_BLK[] = {
//    {PORT_0, PIN_10, GPIO_FUNC_OUT, GPIO_PAD_NONE},
//};

static spi_req_t master_req;

/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(uint8_t dat) 
{
    master_req.tx_data = &dat;	
	if (SPI_MasterTrans(SPI0A, &master_req) != E_NO_ERROR) 
	{
		printf("Transfer error on SPI0\n");
		while (1);
	} 	
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(uint8_t dat)
{
	ST7789_DC_Set();//写数据
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat)
{
	ST7789_DC_Set();//写数据
	LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(uint8_t dat)
{
	ST7789_DC_Clr();//写命令
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	if(USE_HORIZONTAL==0)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+52);
		LCD_WR_DATA(x2+52);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+40);
		LCD_WR_DATA(y2+40);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+53);
		LCD_WR_DATA(x2+53);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+40);
		LCD_WR_DATA(y2+40);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+40);
		LCD_WR_DATA(x2+40);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+53);
		LCD_WR_DATA(y2+53);
		LCD_WR_REG(0x2c);//储存器写
	}
	else
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+40);
		LCD_WR_DATA(x2+40);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+52);
		LCD_WR_DATA(y2+52);
		LCD_WR_REG(0x2c);//储存器写
	}
}


/******************************************************************************
      函数说明：LCD初始化函数
      入口数据：无
      返回值：  无
******************************************************************************/
void lcd_init(void)
{
	uint16_t tx_data[8];
	
	/* gpio init */
//  GPIO_Config(ST7789_RES);
	GPIO_Config(ST7789_DC);
//	GPIO_Config(ST7789_BLK);
//	GPIO_OutSet(ST7789_RES);
	GPIO_OutSet(ST7789_DC);
//	GPIO_OutSet(ST7789_BLK);
	
	/* spi init */
    if(SPI_Init(SPI0A , 0, SPI_BAUD_RATE) != E_NO_ERROR) {
        printf("Error configuring SPI\n");
        while(1) {}
    }
	/* 三线模式 */
	SPI17Y_ThreeWire(MXC_SPI17Y);
	/* 进行一次非传输初始化SPI */
	memset(tx_data, 0x0, 8);
	master_req.ssel = 0;
	master_req.deass = 1;
    master_req.ssel_pol = SPI_POL_LOW;
    master_req.tx_data = tx_data;
    master_req.rx_data = NULL;  // 三线
	master_req.width = SPI17Y_WIDTH_1;
    master_req.len = 1;
    master_req.bits = 8;
    master_req.rx_num = 0;
    master_req.tx_num = 0;
    master_req.callback = NULL;
	if (SPI_MasterTrans(SPI0A, &master_req) != E_NO_ERROR) 
	{
		printf("Transfer error on SPI0\n");
		while (1);
	}
	mxc_delay(MXC_DELAY_MSEC(200));
	
	/* lcd init */
	LCD_WR_REG(0x36);  // MADCTL (Display Rotation)
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);      
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);  
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);

	LCD_WR_REG(0x3A);  	// Set color mode
	LCD_WR_DATA8(0x55);
	LCD_WR_REG(0xB2);	// Porch control
	LCD_WR_DATA8(0x0C); 
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33); 
	
	/* Internal LCD Voltage generator settings */
	LCD_WR_REG(0xB7); 	// Gate Control
	LCD_WR_DATA8(0x35); // Default value 
	LCD_WR_REG(0xBB);   // VCOM setting
	LCD_WR_DATA8(0x19); // 0.725v (default 0.75v for 0x20)
	LCD_WR_REG(0xC0);   // LCMCTRL
	LCD_WR_DATA8(0x2C); // Default value
	LCD_WR_REG(0xC2);   // VDV and VRH command Enable
	LCD_WR_DATA8(0x01); // Default value
	LCD_WR_REG(0xC3);   // VRH set
	LCD_WR_DATA8(0x12); // +-4.45v (defalut +-4.1v for 0x0B)  
	LCD_WR_REG(0xC4);   // VDV set
	LCD_WR_DATA8(0x20); // Default value 
	LCD_WR_REG(0xC6);   // Frame rate control in normal mode
	LCD_WR_DATA8(0x0F); // Default value (60HZ)   
	LCD_WR_REG(0xD0);   // Power control
	LCD_WR_DATA8(0xA4); // Default value
	LCD_WR_DATA8(0xA1); // Default value

	/* Division line */
	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0x21);  // Inversion ON
	LCD_WR_REG(0x11);  // Out of sleep mode
	LCD_WR_REG(0x13);  // Normal Display on
	LCD_WR_REG(0x29);  // Main screen turned on

} 

