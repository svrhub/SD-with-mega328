/*
 * SD with mega328.c
 *
 * Created: 2016/11/19 17:03:56
 * Author : Stone
 */ 
#define F_CPU 16000000UL
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
typedef unsigned char PROGMEM prog_uchar;
#include <string.h>
#include "SerialPort.h"
#include "IIC.h"
#include "DS3231.h"
#include "diskio.h"
#include "ff.h"
#include "MMC.h"
#include "LCD_96_64.h"

FATFS FatFs; 
FRESULT scan_files (char* path);
FRESULT get_file_size();
FRESULT read();
FRESULT write();
int main(void)
{
	DDRD|=(1<<PORTD6);
	DDRC=0xff;
	LCDInit();	
	unsigned char temp1=0,temp2=0,i=0;
    Serial_Init(57600);
	Initialize_DS3231();
	//WriteTime_DS3231();
	_delay_ms(1000);
	Serial_SendString("Ready!\r\n");
	char line[29]; 
	FIL fil;   	
	FRESULT fr;   
	FILINFO fileinfo;
	fr=f_mount(0,&FatFs);
	for (i=0;i<5;i++)
	{
		fr=f_open(&fil,"/test1.txt",FA_OPEN_ALWAYS);
		if (!fr) break;
	}
	if (i==5)
	{	
		Serial_SendString("mount error!");
		return 0;
	}
	f_close(&fil);
	f_mount(0, NULL);
    while(1)
	{
		Time_DisplayString();
		temp1=Time.second;
		if (temp1!=temp2)
		{	
			line[0]=32;
			for(i=0;i<14;i++)
			{
				line[i+1]=DS3231_Date[i];
			}
			for(i=0;i<8;i++)
			{
				line[i+15]=DS3231_Time[i];
			}
			line[23]=32;
			line[24]=DS3231_Temperature[0];
			line[25]=DS3231_Temperature[1];
			line[26]='\r';
			line[27]='\n';
			line[28]='\0';
			Serial_SendString(line);
			for (i=0;i<5;i++)
			{	
				fr=f_mount(0,&FatFs);
				f_stat("test1.txt",&fileinfo);
				fr=f_open(&fil,"/test1.txt",FA_WRITE);				
				f_lseek(&fil,fileinfo.fsize);
				if (!fr)
				{
					fr=f_write(&fil,line,27,&i);
					if (!fr)
					{
						putstr(20,7,"Write OK!");
					}
					else
					{
						putstr(16,7,"Write Error!");
					}
					f_close(&fil);
					f_mount(0, NULL);
					putstr(20,4,DS3231_Time);
					putstr(40,2,DS3231_Temperature);
					break;
				}
				f_close(&fil);
				f_mount(0, NULL);
			}
			if (i==5)
			{
				Serial_SendString("mount error!");
				//return 0;
			}							
			for (i=0;i<28;i++)
			{
				line[i]=0;
			}						
			temp2=temp1;
		}		
		_delay_ms(200);
	}
	f_close(&fil);
	f_mount(0, NULL);
	return 0;
}

FRESULT scan_files (char* path)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i;
	char *fn;
	#if _USE_LFN
	static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
	#endif

	res = f_opendir(&dir, path);
	if (res == FR_OK) {
		i = strlen(path);
		for (;;) {
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0) break;
			#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
			#else
			fn = fno.fname;
			#endif
			if (*fn == '.') continue;
			if (fno.fattrib & AM_DIR) {
				Serial_SendString(&path[i]);
				Serial_SendString("/");
				Serial_SendString(fn);
				res = scan_files(path);
				if (res != FR_OK) break;
				path[i] = 0;
				} else {
				Serial_SendString(path);
				Serial_SendString("/");
				Serial_SendString(fn);
				Serial_SendString("\r\n");
			}
		}
	}

	return res;
}

FRESULT get_file_size()
{	
	FRESULT fr;
	FILINFO fno;
	fr=f_stat("/old/TEST2.txt",&fno);
	DWORD temp=fno.fsize;
	Serial_Send(temp);
	Serial_Send(temp>>8);
	Serial_Send(temp>>16);
	Serial_Send(temp>>24);
	return fr;
}

FRESULT read()
{	
	FIL fil;       /* File object */
	char line[100]; /* Line buffer */
	FRESULT fr;    /* FatFs return code */
	fr=f_mount(0,&FatFs);
	fr=f_open(&fil,"/test1.txt", FA_READ);
	_delay_ms(2);		//等待文件打开
	while (f_gets(line, sizeof line, &fil))
	{
		Serial_SendString(line);
	}
	f_close(&fil);
	f_mount(0, NULL);
}

FRESULT write()
{
	 FIL fil;       /* File object */
	 char line[]="The f_puts() is a wrapper function of f_put\r\n"; /* Line buffer */
	 FRESULT fr;    /* FatFs return code */
	 
	 /* Register work area to the default drive */
	fr=f_mount(0,&FatFs);	
	//fr=f_open(&fil,"/test1.txt",FA_CREATE_ALWAYS);
	//f_close(&fil);
	fr=f_open(&fil,"/test1.txt",FA_CREATE_ALWAYS|FA_WRITE);
	if (fr) return 0;
	//Serial_Send(fr);
	//_delay_ms(20);
	unsigned int temp=0;
	unsigned int i=0;
	for (i=0;i<50000;i++)
	{	
		//fr=f_write(&fil,"Hello, World!\r\n",15,&temp);
		//f_sync(&fil);
		fr=f_write(&fil,line,(sizeof line)-1,&temp);
		//f_sync(&fil);
	}	
	Serial_SendString("OK");
	Serial_Send(temp);
	f_close(&fil);
    f_mount(0, NULL);
	return 0;
}