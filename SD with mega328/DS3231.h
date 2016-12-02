/*
 * DS3231.h
 *
 * Created: 2016/3/25 21:38:04
 *  Author: Stone
 */ 


#ifndef DS3231_H_
#define DS3231_H_

//器件地址
#define DS3231_ADDR				0xD0

//闹铃1
#define CMD_SALARM1ECOND		0x07	//秒
#define CMD_ALARM1MINUTE		0x08	//分
#define CMD_ALARM1HOUR			0x09	//时
#define CMD_ALARM1WEEK			0x0A	//星期/日
//闹铃2
#define CMD_ALARM2MINUTE		0x0b	//分
#define CMD_ALARM2HOUR			0x0c	//时
#define CMD_ALARM2WEEK			0x0d	//星期/日
//控制 状态 命令
#define CMD_CONTROL				0x0e	//控制寄存器
#define CMD_STATUS				0x0f	//状态寄存器
#define CMD_XTAL				0x10	//晶体老化寄存器
#define BSY						0x04	//忙
#define OSF						0x80	//振荡器停止标志
//温度
#define CMD_TEMPERATUREH	0x11	//温度寄存器高字节(8位)
#define CMD_TEMPERATUREL	0x12	//温度寄存器低字节(高2位)

struct{
	unsigned char year;
	unsigned char month;
	unsigned char date;
	unsigned char week;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
}Time={0x16,0x11,0x30,0x03,0x18,0x00,0x00,};
	
unsigned char DS3231_Date[15]={"2000年00月00日"};
unsigned char DS3231_Time[9]={"00:00:00"};
unsigned char DS3231_Week[7]={"星期一"};
unsigned char weeks[7][7]={{"星期一"},{"星期二"},{"星期三"},{"星期四"},{"星期五"},{"星期六"},{"星期日"}};
unsigned char DS3231_Temperature[11]={"25;"};
char 		  Temperature_Sign; 		 				   								//温度正负
unsigned char Temperature_Integer, Temperature_Decimals; 	//温度整数，温度小数


unsigned char HEX_BCD(unsigned char val)		//B码转换为BCD码
{
	unsigned char k;
	k = (val/10)<<4 | (val%10);
	return k;
}


unsigned char BCD_HEX(unsigned char val)   //BCD转换为HEX
{
	unsigned char t;
	t = ((val&0x70) >> 4)*10 + (val&0x0F);	
	return t;
}

void Initialize_DS3231(void)
{	
	/*I2C_Start();
	I2C_SendByte(DS3231_ADDR);
	I2C_SendByte(0x00);
	I2C_SendByte(Time.second);	//写入时间
	I2C_SendByte(Time.minute);
	I2C_SendByte(Time.hour);
	I2C_SendByte(Time.week);
	I2C_SendByte(Time.date);
	I2C_SendByte(Time.month);
	I2C_SendByte(Time.year);	
	I2C_Stop();	*/
	I2C_Start();
	I2C_SendByte(DS3231_ADDR);
	I2C_SendByte(0x0E);
	I2C_SendByte(0x00);
	I2C_SendByte(0x0F);
	I2C_SendByte(0x08);
	I2C_Stop();
}


void WriteTime_DS3231(void)
{
	I2C_Start();
	I2C_SendByte(DS3231_ADDR);	
	I2C_SendByte(0x00);		//从秒的地址开始写，地址指针自动++	
	I2C_SendByte(Time.second);	//写入时间
	I2C_SendByte(Time.minute);
	I2C_SendByte(Time.hour);
	I2C_SendByte(Time.week);
	I2C_SendByte(Time.date);
	I2C_SendByte(Time.month);
	I2C_SendByte(Time.year);
	I2C_Stop();
}


void ReadTime_DS3231(void)
{
	I2C_Start();
	I2C_SendByte(DS3231_ADDR);
	I2C_SendByte(0x00);			//从秒的地址开始读，地址指针自动++
	I2C_Start();
	//在写入读地址
	I2C_SendByte(DS3231_ADDR | 1);
	//读出时间
	Time.second =	I2C_ReceiveByte();
	I2C_Ack();	
	Time.minute =	I2C_ReceiveByte();
	I2C_Ack();
	Time.hour   =	I2C_ReceiveByte();
	I2C_Ack();
	Time.week   =	I2C_ReceiveByte();
	I2C_Ack();
	Time.date   =	I2C_ReceiveByte();
	I2C_Ack();
	Time.month  =	I2C_ReceiveByte();
	I2C_Ack();
	Time.year   =	I2C_ReceiveByte();
	I2C_NoAck();
	I2C_Stop();
}

void ReadTemp_DS3231(void)
{
	I2C_Start();
	I2C_SendByte(DS3231_ADDR);
	I2C_SendByte(CMD_TEMPERATUREH);			//直接读温度高字节寄存器
	I2C_Start();
	//在写入读地址
	I2C_SendByte(DS3231_ADDR | 1);
	//读出温度
	Temperature_Integer=I2C_ReceiveByte();
	I2C_Ack();
	Temperature_Decimals=I2C_ReceiveByte()>>6;
	I2C_NoAck();
	I2C_Stop();
}

void Time_DisplayString()
{	
	ReadTime_DS3231();
	DS3231_Date[2]	=	(Time.year>>4)+0x30;
	DS3231_Date[3]	=	(Time.year&0x0F)+0x30;
	DS3231_Date[6]	=	(Time.month>>4)+0x30;
	DS3231_Date[7]	=	(Time.month&0x0F)+0x30;
	DS3231_Date[10]	=	(Time.date>>4)+0x30;
	DS3231_Date[11]	=	(Time.date&0x0F)+0x30;
	DS3231_Time[0]	=	(Time.hour>>4)+0x30;
	DS3231_Time[1]	=	(Time.hour&0x0F)+0x30;
	DS3231_Time[3]	=	(Time.minute>>4)+0x30;
	DS3231_Time[4]	=	(Time.minute&0x0F)+0x30;
	DS3231_Time[6]	=	(Time.second>>4)+0x30;
	DS3231_Time[7]	=	(Time.second&0x0F)+0x30;
	//strcpy(DS3231_Week,weeks[Time.week-1]);	
	ReadTemp_DS3231();
	DS3231_Temperature[0]=(HEX_BCD(Temperature_Integer)>>4)+0x30;
	DS3231_Temperature[1]=(HEX_BCD(Temperature_Integer)&0x0F)+0x30;
}


#endif /* DS3231_H_ */