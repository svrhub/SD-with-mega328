#ifndef _SD_H_
#define _SD_H_ 1

#include<avr/io.h>

//ATMEGA128 SPI引脚
#define DDR_SPI		DDRB
#define PORT_SPI	PORTB
#define SPI_PIN		PINB
#define SPI_SS		PORTB2
#define SPI_MOSI	PORTB3
#define SPI_MISO	PORTB4
#define SPI_SCK		PORTB5
#define SD_CS_L		PORT_SPI &= ~(1<<PORTB2)
#define SD_CS_H     PORT_SPI |= (1<<PORTB2)

#define SD_DET()            (1)//GET_BIT(PORTA,2)//检测有卡

/* Private define ------------------------------------------------------------*/
/* SD卡类型定义 */
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4


/* SD传输数据结束后是否释放总线宏定义 */
#define NO_RELEASE      0
#define RELEASE         1

/* SD卡指令表 */
#define CMD0    0       //卡复位
#define CMD9    9       //命令9 ，读CSD数据
#define CMD10   10      //命令10，读CID数据
#define CMD12   12      //命令12，停止数据传输
#define CMD16   16      //命令16，设置SectorSize 应返回0x00
#define CMD17   17      //命令17，读sector
#define CMD18   18      //命令18，读Multi sector
#define ACMD23  23      //命令23，设置多sector写入前预先擦除N个block
#define CMD24   24      //命令24，写sector
#define CMD25   25      //命令25，写Multi sector
#define ACMD41  41      //命令41，应返回0x00
#define CMD55   55      //命令55，应返回0x01
#define CMD58   58      //命令58，读OCR信息
#define CMD59   59      //命令59，使能/禁止CRC，应返回0x00


#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

unsigned char reading;   //标记读写,1:读

void SPI_Low(void);
void SPI_High(void);
void SPI_Init(void);
unsigned char SD_Write_Command(unsigned char cmd,unsigned long arg);
unsigned char SPI_RW(unsigned char dat);
unsigned char SD_Init(void);  //SD卡初始化(SPI-MODE)
unsigned char SD_ReceiveData(unsigned char*data,unsigned int len,unsigned char release);
unsigned char SD_SendCommand(unsigned char cmd,unsigned long arg,unsigned char crc);
unsigned char SD_ReadSingleBlock(unsigned long sector,unsigned char*buffer);
unsigned char SD_ReadMultiBlock(unsigned long sector,unsigned char *buffer,unsigned char count);
unsigned char SD_WriteSingleBlock(unsigned long sector,const unsigned char *data);
unsigned char SD_WriteMultiBlock(unsigned long sector,const unsigned char*data,unsigned char count);
unsigned char SD_WaitReady(void);
unsigned long SD_GetCapacity(void);
unsigned char SD_GetCSD(unsigned char *csd_data);









#endif
