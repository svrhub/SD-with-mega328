/*
 * SerialPort.h
 * Only For Mega328
 * Created: 2016/4/30 10:26:51
 *  Author: Stone
 */ 


#ifndef SERIALPORT_H_
#define SERIALPORT_H_

/*
    串口初始化程序    
*/
void Serial_Init(unsigned long BAUD);
void Serial_Send(unsigned char data);
void Serial_SendString(char *s);
#endif /* SERIALPORT_H_ */