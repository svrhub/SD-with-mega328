/*
 * LCD_96_64.h
 *
 * Created: 2015/10/25 12:36:51
 *  Author: stone
 */ 


#ifndef LCD_96_64_H_
#define LCD_96_64_H_
#define uchar unsigned char
#define SET_LCD_RST	PORTC|=(1<<PC5)		//复位
#define CLR_LCD_RST	PORTC&=~(1<<PC5)
#define SET_LCD_CS	PORTC|=(1<<PC3)		//片选
#define CLR_LCD_CS	PORTC&=~(1<<PC3)
#define SET_LCD_DC	PORTC|=(1<<PC2)		//数据/命令
#define CLR_LCD_DC	PORTC&=~(1<<PC2)
#define SET_LCD_SDI	PORTC|=(1<<PC1)		//数据
#define CLR_LCD_SDI	PORTC&=~(1<<PC1)
#define SET_LCD_SCK	PORTC|=(1<<PC4)		//时钟
#define CLR_LCD_SCK	PORTC&=~(1<<PC4)

#define NOP					0x00
#define Function_Set		0x20
#define Display_Control		0x08
#define Set_Y_Address		0x40
#define Set_X_Address		0x80
#define Set_V0		        0x80
#define Set_Test_Mode		0x30

#define DIS_PHONEANT_0      0				//天线
#define DIS_PHONEANT_1      4				//信号1格
#define DIS_PHONEANT_2      5				//信号2格
#define DIS_PHONEANT_3      7				//信号3格
#define DIS_PHONEANT_4      8				//信号4格
#define DIS_PHONEANT_5      10
#define DIS_PHONE           20				//电话
#define DIS_LOCK            30				//锁定
#define DIS_Gr              40				//Gr符号
#define DIS_SMS             50				//短信符号
#define DIS_SHAKE           60				//震动符号
#define DIS_HOME            61				// 室内符号
#define DIS_HOME_ANT        65				//室内天线符号
#define DIS_HOME_ANT_1      67				//室内天线1格
#define DIS_HOME_ANT_2      68				//室内天线2格
#define DIS_HOME_ANT_3      69				//室内天线3格
#define DIS_MUTE            70				//静音符号
#define DIS_ALAM            75				//闹钟符号
#define DIS_CELL_0          80				//电池电量符号
#define DIS_CELL_1          94				//电池电量1格
#define DIS_CELL_2          95				//电池电量2格
#define DIS_CELL_3          93				//电池电量3格

#define DIS_ON              0xFF			//显示小图标
#define DIS_OFF				0x00			//关闭小图标

#include "font.h"

/*-----------------------------------------------------------------------
shiftout: 以SPI方式串行输出1字节
输入参数：data    ：要串行发送的字节
-----------------------------------------------------------------------*/
void shiftout(uchar data){
	uchar i=0;
	for (i=0;i<8;i++)
	{
		CLR_LCD_SCK;
		if (data&0x80)
		{
			SET_LCD_SDI;
		}
		else
		{
			CLR_LCD_SDI;
		}
		SET_LCD_SCK;
		data=data<<1;
	}
}
/*-----------------------------------------------------------------------
send_dat: 发送命令
输入参数：	cmd		：要发送的命令类型；
			data    ：要发送的命令数据
-----------------------------------------------------------------------*/
void send_cmd(uchar cmd,uchar data)
{
	CLR_LCD_CS;
	CLR_LCD_DC;
	shiftout(cmd|data);
	SET_LCD_CS;	
}
/*-----------------------------------------------------------------------
send_dat: 发送数据
输入参数：data    ：要发送的数据；
-----------------------------------------------------------------------*/ 
void send_dat(uchar data)
{
	CLR_LCD_CS;
	SET_LCD_DC;
	shiftout(data);
	SET_LCD_CS;
}

/* 清屏 */
void cls(void)
{
	int i;
	send_cmd(Set_X_Address, 0);
	send_cmd(Set_Y_Address, 0);
	for(i=0;i<960;i++)
	send_dat(0x00);
	send_cmd(Set_X_Address, 0);
	send_cmd(Set_Y_Address, 0);
}
/* 初始化LCD */
void LCDInit(void)
{
	CLR_LCD_RST;
	_delay_ms(10);
	SET_LCD_RST;
	_delay_ms(10);
	send_cmd(Function_Set,0x01);
	send_cmd(Set_V0,0b00000);			//对比度
	send_cmd(Set_Test_Mode,0x02);
	send_cmd(Function_Set,0x00);
	send_cmd(Display_Control,0x04);
	cls();
}
/*-----------------------------------------------------------------------
putch: 在LCD上显示单个字符
输入参数：X、Y    ：显示字符的起始X、Y坐标；
          ch      ：要显示的字符
-----------------------------------------------------------------------*/ 
void putch(uchar x,uchar y, unsigned int ch)
{
	unsigned char i;
	unsigned int temp=0;
	send_cmd(Set_X_Address,x);
	send_cmd(Set_Y_Address,y);
	temp=(ch-0x20)*5;
	for(i=0;i<5;i++)
	{
		send_dat(pgm_read_byte(FONT+temp+i));
	}	
}
/*-----------------------------------------------------------------------
putstr: 在LCD上显示字符串
输入参数：X、Y    ：显示字符的起始X、Y坐标；
          *str    ：要显示的字符串
-----------------------------------------------------------------------*/ 
void putstr(uchar x, uchar y, char *str)
{
	while(*str!=0)
	{
		putch(x,y,*str++);
		x=x+6;
	}
}
/*-----------------------------------------------------------------------
LCD_set_XY: 设置坐标地址
输入参数：X、Y    ：设置屏幕地址；
-----------------------------------------------------------------------*/ 
void LCD_set_XY(uchar x,uchar y)
{
	send_cmd(Set_X_Address,x);
	send_cmd(Set_Y_Address,y);
}

/*-----------------------------------------------------------------------
LCD_write_chinese_string: 在LCD上显示汉字
输入参数：X、Y    ：显示汉字的起始X、Y坐标；
          ch_with ：汉字点阵的宽度
          num     ：显示汉字的个数；  
          line    ：汉字点阵数组中的起始行数
          row     ：汉字显示的行间距
-----------------------------------------------------------------------*/ 
void LCD_write_chinese_string(uchar X, uchar Y,uchar ch_with,uchar num,uchar line,uchar row)
{
	unsigned char i,n;
	unsigned int temp=0;
	LCD_set_XY(X,Y);                             //设置初始位置
	for (i=0;i<num;)
	{
		for (n=0; n<ch_with*2; n++)              //写一个汉字
		{
			if (n==ch_with)                      //写汉字的下半部分
			{
				if (i==0) LCD_set_XY(X,Y-1);
				else
				LCD_set_XY((X+(ch_with+row)*i),Y-1);
			}
			temp=(line+i)*24+n;
			send_dat(pgm_read_byte(write_chinese+temp));
		}
		i++;
		LCD_set_XY((X+(ch_with+row)*i),Y);
	}
}
/*-----------------------------------------------------------------------
putdraw	: 显示屏幕上方图标
输入参数：x: 屏幕上方的图标地址
		  s: 图标开关状态
-----------------------------------------------------------------------*/ 
void putdraw(uchar x,uchar s )
{
	send_cmd(Set_X_Address,x);
	send_cmd(Set_Y_Address,8);
	send_dat(s);
}

/*-----------------------------------------------------------------------
show_map	: 在屏幕上显示图像
-----------------------------------------------------------------------*/ 
void show_map(void)
{
	unsigned int i,j,n=0,temp=0;
	for(i=0;i<8;i++)
	for(j=0;j<96;j++)
	{
		send_cmd(Set_Y_Address,i);
		send_cmd(Set_X_Address,j);
		temp=n++;
		send_dat(pgm_read_byte(BMP+temp));
	}
}






#endif /* LCD_96_64_H_ */