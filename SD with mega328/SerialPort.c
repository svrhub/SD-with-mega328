/*
 * SerialPort.c
 * Only For Mega328
 * Created: 2016/4/30 10:26:51
 *  Author: Stone
 */ 


#define F_CPU 16000000UL
#include "avr/io.h"
#include "SerialPort.h"

/*
    串口初始化程序    
*/
void Serial_Init(unsigned long BAUD)
{	
	/*    设置波特率    */
	UBRR0L= (unsigned char)((F_CPU/BAUD/16)-1)%256;
	UBRR0H= (unsigned char)((F_CPU/BAUD/16)-1)/256;
	/*    使能接收中断，使能接收器与发送器    */
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);
	/*    设置帧格式 ：  8个数据位，1个停止位    */
	UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);	
}

void Serial_Send(unsigned char data)
{
	/* 等待发送缓冲器为空 */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* 将数据放入缓冲器，发送数据 */
	UDR0 = data;
}

void Serial_SendString(char *s)
{
	while(*s)
	{
		Serial_Send(*s);
		s++;
	}
}