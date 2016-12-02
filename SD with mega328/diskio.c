#include "diskio.h"
#include "MMC.h"
#include "SerialPort.h"


unsigned char disk_initialize(unsigned char drv)

{
	unsigned char state;

	if(drv)
	{
		 return STA_NOINIT;  //仅支持磁盘0的操作
	}

	state = SD_Init();      //SD卡的初始化
	 
	 
	
	if(state == STA_NODISK)
    {
        return STA_NODISK;
    }
    else if(state != 0)
    {
	
		Serial_SendString("InitializeFail\r\n");
        return STA_NOINIT;  //其他错误：初始化失败
    }
    else
    {
	
		//Serial_SendString("InitializeSuccess\r\n");
        return 0;           //初始化成功
    }


	
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
//Get disk status
DSTATUS disk_status (               
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
    if(drv)
    {
        return STA_NOINIT;  //仅支持磁盘0操作
    }

    //检查SD卡是否插入
    if(!SD_DET())
    {
        return STA_NODISK;
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	unsigned char res=0;
	//count=count;      //不知为何,count会被优化为0,故加上这条语句
	//sector=sector;
    if ((drv) || (!count))
    {    
		Serial_Send(5+0x30);
        return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
    }
    if(!SD_DET())
    {
        return RES_NOTRDY;  //没有检测到SD卡，报NOT READY错误
    }

    
	
    if(count==1)            //1个sector的读操作      
    {                                                
        res = SD_ReadSingleBlock(sector, buff);      
    }                                                
    else                    //多个sector的读操作     
    {                                                
        res = SD_ReadMultiBlock(sector, buff, count);
    }                                                
	/*
    do                           
    {                                          
        if(SD_ReadSingleBlock(sector, buff)!=0)
        {                                      
            res = 1;                           
            break;                             
        }                                      
        buff+=512;                             
    }while(--count);                                         
    */
    //处理返回值，将SPI_SD_driver.c的返回值转成ff.c的返回值
    if(res == 0x00)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	        /* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	unsigned char res;
//	count = 1;
//	sector = 2;

    if (drv || !count)
    {    
        return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
    }

    if(!SD_DET())
    {
        return RES_NOTRDY;  //没有检测到SD卡，报NOT READY错误
    }


    // 读写操作
    if(count == 1)
    {
        res = SD_WriteSingleBlock(sector, buff);
    }
    else
    {
        res = SD_WriteMultiBlock(sector, buff, count);
    }
    // 返回值转换
    if(res == 0)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)

{
    DRESULT res;


    if (drv)
    {    
        return RES_PARERR;  //仅支持单磁盘操作，否则返回参数错误
    }
    
    //FATFS目前版本仅需处理CTRL_SYNC，GET_SECTOR_COUNT，GET_BLOCK_SIZ三个命令
    switch(ctrl)
    {
    case CTRL_SYNC:
        SD_CS_L;
        if(SD_WaitReady()==0)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }
        SD_CS_H;
        break;
        
    case GET_BLOCK_SIZE:
    case GET_SECTOR_SIZE:
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:
        *(DWORD*)buff = SD_GetCapacity();
        res = RES_OK;
        break;
    default:
        res = RES_PARERR;
        break;
    }

    return res;
}


/*-----------------------------------------------------------------------*/
/* User defined function to give a current time to fatfs module      */
/* 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */
/* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */ 

DWORD get_fattime (void)
{
 /*   struct tm t;
    DWORD date;
    t.tm_year=2009-1980;		//年份改为1980年起
    t.tm_mon=8;         	//0-11月改为1-12月
	t.tm_mday=3;
	t.tm_hour=15;
	t.tm_min=30;
    t.tm_sec=20;      	//将秒数改为0-29
	date =t.tm_year;
	date=date<<7;
	date+=t.tm_mon;
	date=date<<4;
	date+=t.tm_mday;
	date=date<<5;
	date+=t.tm_hour;
	date=date<<5;
	date+=t.tm_min;
	date=date<<5;
	date+=t.tm_sec;
	date=1950466004;
*/
    return 1950466005;
}
