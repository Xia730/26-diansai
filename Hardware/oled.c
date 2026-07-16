#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"
#include "ti/driverlib/dl_i2c.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

uint8_t OLED_GRAM[144][8];

/*------------------------------------------------
作用    : 发送一个字节
参数    : dat:  字节
参数    : mode: 0表示命令，1表示数据
返回值  : 无
------------------------------------------------*/
void OLED_WR_Byte(uint8_t dat,uint8_t mode)
{
    uint16_t i;

    uint8_t Send_Buff[5] = {0};

    uint8_t SendData_Count = 2;

    if(mode)
        Send_Buff[0] = 0x40;
    else
        Send_Buff[0] = 0x00;

    Send_Buff[1] = dat;

    DL_I2C_fillControllerTXFIFO(I2C_OLED_INST, Send_Buff, SendData_Count);

    while (!(DL_I2C_getControllerStatus(I2C_OLED_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(I2C_OLED_INST, 0x3C, DL_I2C_CONTROLLER_DIRECTION_TX, SendData_Count);

}

/*------------------------------------------------
作用    : 画点
参数    : x: x轴起点坐标0~127
参数    : y: y轴起点坐标0~63
参数    : t: 1填充, 0清空
返回值  : 无
------------------------------------------------*/
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t i,m,n;
	i=y/8;
	m=y%8;
	n=1<<m;
	if(t){OLED_GRAM[x][i]|=n;}
	else
	{
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
		OLED_GRAM[x][i]|=n;
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
	}
}

/*------------------------------------------------
作用    : 在指定位置显示一个字符，包括部分字符
参数    : x:     x轴起点坐标0~127
参数    : y:     y轴起点坐标0~63
参数    : size1: 字体大小，8 或 12 或 16 或 24
参数    : chr:   字符串起始地址
参数    : mode:  0反色显示，1正常显示
返回值  : 无
------------------------------------------------*/
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1,uint8_t mode)
{
	uint8_t i,m,temp,size2,chr1;
	uint8_t x0=x,y0=y;
	if(size1==8)size2=6;
	else size2=(size1/8+((size1%8)?1:0))*(size1/2);  //得到字体一个字符对应点阵集所占的字节数
	chr1=chr-' ';  //计算偏移后的值
	for(i=0;i<size2;i++)
	{
		if(size1==8)
		{temp=asc2_0806[chr1][i];} //调用0806字体
		else if(size1==12)
        {temp=asc2_1206[chr1][i];} //调用1206字体
		else if(size1==16)
        {temp=asc2_1608[chr1][i];} //调用1608字体
		else if(size1==24)
        {temp=asc2_2412[chr1][i];} //调用2412字体
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((size1!=8)&&((x-x0)==size1/2))
		{x=x0;y0=y0+8;}
		y=y0;
	}
}

/*------------------------------------------------
作用    : 显示字符串
参数    : x:     x轴起点坐标0~127
参数    : y:     y轴起点坐标0~63
参数    : size1: 字体大小，8 或 12 或 16 或 24
参数    : chr:   字符串起始地址
参数    : mode:  0反色显示，1正常显示
返回值  : 无
------------------------------------------------*/
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1,uint8_t mode)
{
	while((*chr>=' ')&&(*chr<='~'))//判断是不是非法字符!
	{
		OLED_ShowChar(x,y,*chr,size1,mode);
		if(size1==8)x+=6;
		else x+=size1/2;
		chr++;
    }
}

/*------------------------------------------------
作用    : OLED初始化
参数    : 无
返回值  : 无
------------------------------------------------*/
void oled_init(void)
{
	delay_ms(320);

	OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
	OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
	OLED_WR_Byte(0xCF,OLED_CMD);// Set SEG Output Current Brightness
	OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);//-not offset
	OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
	OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
	OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
	OLED_WR_Byte(0x12,OLED_CMD);
	OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
	OLED_WR_Byte(0x30,OLED_CMD);//Set VCOM Deselect Level
	OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);//
	OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
	OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
	oled_clear();
	OLED_WR_Byte(0xAF,OLED_CMD);
}

/*------------------------------------------------
作用    : 清屏
参数    : 无
返回值  : 无
------------------------------------------------*/
void oled_clear(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
	    for(n=0;n<128;n++)
        {
            OLED_GRAM[n][i]=0;//清除所有数据
        }
	}
	oled_refresh();//更新显示
}

/*------------------------------------------------
作用    : 更新显存
参数    : 无
返回值  : 无
------------------------------------------------*/
void oled_refresh(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
		OLED_WR_Byte(0xb0+i,OLED_CMD); //设置行起始地址
		OLED_WR_Byte(0x00,OLED_CMD);   //设置低列起始地址
		OLED_WR_Byte(0x10,OLED_CMD);   //设置高列起始地址

        OLED_WR_Byte(0x40,OLED_DATA);
		for(n=0;n<128;n++)
		{
			OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
		}
    }
}

/*------------------------------------------------
作用    : OLED使用printf函数打印格式化字符串
说明    : 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数 oled_refresh();
参数    : Line:   指定格式化字符串左上角的纵坐标，范围：0~7
参数    : format: 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
参数    : ...:    格式化字符串参数列表
返回值  : 无
------------------------------------------------*/
void oled_printf(uint8_t Line, char *format, ...)
{
	char String[30];						//定义字符数组
	va_list arg;							//定义可变参数列表数据类型的变量arg
	va_start(arg, format);					//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);							//结束变量arg

	/* OLED显示字符数组（字符串） */
	OLED_ShowString(0, (Line % 8) * 8, (uint8_t *) String, 8, 1);
}
