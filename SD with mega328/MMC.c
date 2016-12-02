#define F_CPU 16000000UL
#include <util/delay.h>
#include "MMC.h"



unsigned char SD_Type=0 ;

void SPI_Low(void)		
{//SPI低速模式
	SPCR = 0;
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << SPR1);
	SPSR &= ~(1 << SPI2X);
	//使能SPI,主机方式,MSB在前,模式0,128分频
}

void SPI_High(void)
{//SPI高速模式
	SPCR = 0;
	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR |= (1 << SPI2X);
	//使能SPI,主机方式,MSB在前,模式0,4分频,2倍频
}


void SPI_Init(void)
{//SPI初始化
	PORT_SPI |= (1 << SPI_SS) | (1 << SPI_MISO);		//将SS置位输出拉高,MISO输入带上拉
	DDR_SPI &= ~((1 << SPI_SS) | (1 << SPI_MOSI) | (1 << SPI_MISO)| (1 << SPI_SCK));
	DDR_SPI |= (1 << SPI_SS) | (1 << SPI_MOSI) | (1 << SPI_SCK);
	//将SS SCK MOSI置为输出
}


unsigned char SPI_RW(unsigned char dat)
{
	SPDR = dat;
	while(!(SPSR & (1 << SPIF)))
		;
	return (SPDR);
}

//****************************************************************************
//SD卡初始化(SPI-MODE)
//****************************************************************************
unsigned char SD_Init(void)
{


	unsigned char i,temp;
    unsigned int retry; 
	

	SPI_Init();
	SPI_Low();

	SD_CS_L;
	_delay_ms(1);       //Wait MMC/SD ready...

	for(i=0;i<0xff;i++)
	{
		SPI_RW(0xff);   //send 74 clock at least!!!
	}

	retry = 0;

	do
	{
		temp=SD_Write_Command(0,0);//retry 200 times to send CMD0 command 
		retry++;
		if(retry==2000) return 0xff;//CMD0 Error! 

	}while(temp!=1);      //temp=0x01即说明SD卡进入空闲状态复位成功

	retry = 0;
	do
	{
		temp=SD_Write_Command(1,0);
		retry++;
		if(retry >= 2000) return 0xff;  //retry 100 times to send CMD1 command 
	}while(temp!=0);    //temp=0说明SD可以进行操作 激活成功

	retry = 0;
	SD_Write_Command(16,512);     //设置一次读写BLOCK的长度为512个字节 CMD16
	SD_CS_H;   //MMC_CS_PIN=1;  //set MMC_Chip_Select to high 关闭片选
    return(0); //All commands have been taken.


}


//****************************************************************************
//Send a Command to MMC/SD-Card
//Return: the second byte of response register of MMC/SD-Card
//****************************************************************************
unsigned char SD_Write_Command(unsigned char cmd,unsigned long arg)
{
	unsigned char tmp;
	unsigned char retry=0;
	
	SD_CS_H;   //关闭SD卡
	SPI_RW(0xff);
	SD_CS_L;
	SPI_RW(cmd|0x40);   //送头命令
	SPI_RW(arg>>24);  //最高的8位
	SPI_RW(arg>>16);   //send 6 Byte Command to MMC/SD-Card
	SPI_RW(arg>>8);
	SPI_RW(arg&0xff);
	SPI_RW(0x95);      //仅仅对RESET有效的CRC效验码
		//get 8 bit response 

	do
	{
		tmp = SPI_RW(0XFF);
		retry++;
	}while((tmp==0xff)&&(retry<100));

	if(reading==0) SD_CS_H;
	else
	SD_CS_L;
	return(tmp);

		
}

/*******************************************************************************
* Function Name  : SD_SendCommand
* Description    : 向SD卡发送一个命令
* Input          : unsigned char cmd   命令
*                  unsigned long arg  命令参数
*                  unsigned char crc   crc校验值
* Output         : None
* Return         : unsigned char r1 SD卡返回的响应
*******************************************************************************/
unsigned char SD_SendCommand(unsigned char cmd,unsigned long arg,unsigned char crc)
{
    unsigned char r1 ;
    unsigned char Retry=0 ;
    

    SPI_RW(0XFF);
    //片选端置低，选中SD卡
    SD_CS_L;   //PC0 = 0
    
    //发送
    SPI_RW(cmd|0x40);
    //分别写入命令
    SPI_RW(arg>>24);
    SPI_RW(arg>>16);
    SPI_RW(arg>>8);
    SPI_RW(arg);
    SPI_RW(crc);
    
    //等待响应，或超时退出
    while((r1=SPI_RW(0xFF))==0xFF)
    {
        Retry++;
        if(Retry>200)
        {
        break ;
        }
    }

    //关闭片选
    SD_CS_H;
    //在总线上额外增加8个时钟，让SD卡完成剩下的工作
    SPI_RW(0xFF);
    
    //返回状态值
    return r1 ;
}

/*******************************************************************************
* Function Name  : SD_ReceiveData
* Description    : 从SD卡中读回指定长度的数据，放置在给定位置
* Input          : unsigned char *data(存放读回数据的内存>len)
*                  unsigned int len(数据长度）
*                  unsigned char release(传输完成后是否释放总线CS置高 0：不释放 1：释放）
* Output         : None
* Return         : unsigned char
*                  0：NO_ERR
*                  other：错误信息
*******************************************************************************/
unsigned char SD_ReceiveData(unsigned char*data,unsigned int len,unsigned char release)
{
    unsigned int retry ;
    unsigned char r1 ;
    
    // 启动一次传输
    SD_CS_L;
    //等待SD卡发回数据起始令牌0xFE
    retry=0 ;
    do 
    {
        r1=SPI_RW(0xFF);
        retry++;
        //2000次等待后没有应答，退出报错
        if(retry>200)
        {
            SD_CS_H;
            return 1 ;
        }
    }
    while(r1!=0xFE);
    //开始接收数据
    while(len--)
    {
        *data=SPI_RW(0xFF);
        data++;
    }
    //下面是2个伪CRC（dummy CRC）
    SPI_RW(0xFF);
    SPI_RW(0xFF);
    //按需释放总线，将CS置高
    if(release==RELEASE)
    {
        //传输结束
        SD_CS_H;
        SPI_RW(0xFF);
    }
    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_ReadSingleBlock
* Description    : 读SD卡的一个block
* Input          : unsigned long sector 取地址（sector值，非物理地址）
*                  unsigned char *buffer 数据存储地址（大小至少512byte）
* Output         : None
* Return         : unsigned char r1
*                   0： 成功
*                   other：失败
*******************************************************************************/
unsigned char SD_ReadSingleBlock(unsigned long sector,unsigned char*buffer)
{
    unsigned char r1 ;
    
    //设置为高速模式
    SPI_High();
    
    //如果不是SDHC，将sector地址转成byte地址
    sector=sector<<9 ;
    
    r1=SD_SendCommand(CMD17,sector,0);
    //读命令
    
    if(r1!=0x00)
    {
        return r1 ;
    }
    
    r1=SD_ReceiveData(buffer,512,RELEASE);
    if(r1!=0)
    {
        return r1 ;
        //读数据出错！
    }
    else 
    {
        return 0 ;
    }
}

/*******************************************************************************
* Function Name  : SD_ReadMultiBlock
* Description    : 读SD卡的多个block
* Input          : unsigned long sector 取地址（sector值，非物理地址）
*                  unsigned char *buffer 数据存储地址（大小至少512byte）
*                  unsigned char count 连续读count个block
* Output         : None
* Return         : unsigned char r1
*                   0： 成功
*                   other：失败
*******************************************************************************/
unsigned char SD_ReadMultiBlock(unsigned long sector,unsigned char *buffer,unsigned char count)
{
    unsigned char r1 ;
    
    //设置为高速模式
    SPI_High();
    
    //如果不是SDHC，将sector地址转成byte地址
    sector=sector<<9 ;
    //SD_WaitReady();
    //发读多块命令
    r1=SD_SendCommand(CMD18,sector,0);
    //读命令
    if(r1!=0x00)
    {
        return r1 ;
    }
    //开始接收数据
    do 
    {
        if(SD_ReceiveData(buffer,512,NO_RELEASE)!=0x00)
        {
            break ;
        }
        buffer+=512 ;
    }
    while(--count);
    
    //全部传输完毕，发送停止命令
    SD_SendCommand(CMD12,0,0);
    //释放总线
    SD_CS_H;
    SPI_RW(0xFF);
    
    if(count!=0)
    {
        return count ;
        //如果没有传完，返回剩余个数
    }
    else 
    {
        return 0 ;
    }
}

/*******************************************************************************
* Function Name  : SD_WriteSingleBlock
* Description    : 写入SD卡的一个block
* Input          : unsigned long sector 扇区地址（sector值，非物理地址）
*                  unsigned char *buffer 数据存储地址（大小至少512byte）
* Output         : None
* Return         : unsigned char r1
*                   0： 成功
*                   other：失败
*******************************************************************************/
unsigned char SD_WriteSingleBlock(unsigned long sector,const unsigned char *data)
{
    unsigned char r1 ;
    unsigned int i ;
    unsigned int retry ;
    
    //设置为高速模式
    SPI_High();
    
    //如果不是SDHC，给定的是sector地址，将其转换成byte地址
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector=sector<<9 ;
    }
    
    r1=SD_SendCommand(CMD24,sector,0x00);
    if(r1!=0x00)
    {
        return r1 ;
        //应答不正确，直接返回
    }
    
    //开始准备数据传输
    SD_CS_L;
    //先放3个空数据，等待SD卡准备好
    SPI_RW(0xff);
    SPI_RW(0xff);
    SPI_RW(0xff);
    //放起始令牌0xFE
    SPI_RW(0xFE);
    
    //放一个sector的数据
    for(i=0;i<512;i++)
    {
        SPI_RW(*data++);
    }
    //发2个Byte的dummy CRC
    SPI_RW(0xff);
    SPI_RW(0xff);
    
    //等待SD卡应答
    r1=SPI_RW(0xff);
    if((r1&0x1F)!=0x05)
    {
        SD_CS_H;
        return r1 ;
    }
    
    //等待操作完成
    retry=0 ;
    while(!SPI_RW(0xff))
    {
        retry++;
        //如果长时间写入没有完成，报错退出
        if(retry>0xfffe)
        {
            SD_CS_H;
            return 1 ;
            //写入超时返回1
        }
    }
    
    //写入完成，片选置1
    SD_CS_H;
    SPI_RW(0xff);
    
    return 0 ;
}

/*******************************************************************************
* Function Name  : SD_WaitReady
* Description    : 等待SD卡Ready
* Input          : None
* Output         : None
* Return         : unsigned char
*               0： 成功
*           other：失败
*******************************************************************************/

unsigned char SD_WaitReady(void)
{
    unsigned char r1 ;
    unsigned char retry ;
    retry=0 ;
    do 
    {
        r1=SPI_RW(0xFF);
        if(retry==255)//如果卡异常，会死循坏！
        {
            return 1 ;
        }
    }
    while(r1!=0xFF);
    
    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_WriteMultiBlock
* Description    : 写入SD卡的N个block
* Input          : unsigned long sector 扇区地址（sector值，非物理地址）
*                  unsigned char *buffer 数据存储地址（大小至少512byte）
*                  unsigned char count 写入的block数目
* Output         : None
* Return         : unsigned char r1
*                   0： 成功
*                   other：失败
*******************************************************************************/
unsigned char SD_WriteMultiBlock(unsigned long sector,const unsigned char*data,unsigned char count)
{
    unsigned char r1 ;
    unsigned int i ;
    
    //设置为高速模式
    SPI_High();
    
    //如果不是SDHC，给定的是sector地址，将其转换成byte地址
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector=sector<<9 ;
    }
    //如果目标卡不是MMC卡，启用ACMD23指令使能预擦除
    if(SD_Type!=SD_TYPE_MMC)
    {
        r1=SD_SendCommand(ACMD23,count,0x00);
    }
    //发多块写入指令
    r1=SD_SendCommand(CMD25,sector,0x00);
    if(r1!=0x00)
    {
        return r1 ;
        //应答不正确，直接返回
    }
    
    //开始准备数据传输
    SD_CS_L;
    //先放3个空数据，等待SD卡准备好
    SPI_RW(0xff);
    SPI_RW(0xff);
    SPI_RW(0xff);
    //--------下面是N个sector写入的循环部分
    do 
    {
        //放起始令牌0xFC 表明是多块写入
        SPI_RW(0xFC);
        
        //放一个sector的数据
        for(i=0;i<512;i++)
        {
            SPI_RW(*data++);
        }
        //发2个Byte的dummy CRC
        SPI_RW(0xff);
        SPI_RW(0xff);
        
        //等待SD卡应答
        r1=SPI_RW(0xff);
        if((r1&0x1F)!=0x05)
        {
            SD_CS_H;
            //如果应答为报错，则带错误代码直接退出
            return r1 ;
        }
        
        //等待SD卡写入完成
        if(SD_WaitReady()==1)
        {
            SD_CS_H;
            //等待SD卡写入完成超时，直接退出报错
            return 1 ;
        }
        
        //本sector数据传输完成
    }
    while(--count);
    
    //发结束传输令牌0xFD
    r1=SPI_RW(0xFD);
    if(r1==0x00)
    {
        count=0xfe ;
    }
    
    if(SD_WaitReady())
    {
        while(1){}
    }
    
    //写入完成，片选置1
    SD_CS_H;
    SPI_RW(0xff);
    
    return count ;
    //返回count值，如果写完则count=0，否则count=1
}

/*******************************************************************************
* Function Name  : SD_GetCSD
* Description    : 获取SD卡的CSD信息，包括容量和速度信息
* Input          : unsigned char *cid_data(存放CID的内存，至少16Byte）
* Output         : None
* Return         : unsigned char
*                  0：NO_ERR
*                  1：TIME_OUT
*                  other：错误信息
*******************************************************************************/
unsigned char SD_GetCSD(unsigned char *csd_data)
{
    unsigned char r1 ;
    
    //发CMD9命令，读CSD
    r1=SD_SendCommand(CMD9,0,0xFF);
    if(r1!=0x00)
    {
        return r1 ;
        //没返回正确应答，则退出，报错
    }
    //接收16个字节的数据
    SD_ReceiveData(csd_data,16,RELEASE);
    
    return 0 ;
}


/*******************************************************************************
* Function Name  : SD_GetCapacity
* Description    : 获取SD卡的容量
* Input          : None
* Output         : None
* Return         : unsigned long capacity
*                   0： 取容量出错
*******************************************************************************/
unsigned long SD_GetCapacity(void)
{
    unsigned char csd[16];
    unsigned long Capacity ;
    unsigned char r1 ;
    unsigned int i ;
    unsigned int temp ;
    
    //取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd)!=0)
    {
        return 0 ;
    }

    //如果为SDHC卡，按照下面方式计算
    if((csd[0]&0xC0)==0x40)
    {
        Capacity=((((unsigned long)csd[8])<<8)+(unsigned long)csd[9]+1)*(unsigned long)1024 ;
    }
    else 
    {
        //下面代码为网上版本
        ////////////formula of the capacity///////////////
        //
        //  memory capacity = BLOCKNR * BLOCK_LEN
        //
        //	BLOCKNR = (C_SIZE + 1)* MULT
        //
        //           C_SIZE_MULT+2
        //	MULT = 2
        //
        //               READ_BL_LEN
        //	BLOCK_LEN = 2
        /**********************************************/
        //C_SIZE
        i=csd[6]&0x03 ;
        i<<=8 ;
        i+=csd[7];
        i<<=2 ;
        i+=((csd[8]&0xc0)>>6);
        
        //C_SIZE_MULT
        r1=csd[9]&0x03 ;
        r1<<=1 ;
        r1+=((csd[10]&0x80)>>7);
        
        //BLOCKNR
        r1+=2 ;
        temp=1 ;
        while(r1)
        {
            temp*=2 ;
            r1--;
        }
        Capacity=((unsigned long)(i+1))*((unsigned long)temp);
        
        // READ_BL_LEN
        i=csd[5]&0x0f ;
        //BLOCK_LEN
        temp=1 ;
        while(i)
        {
            temp*=2 ;
            i--;
        }
        //The final result
        Capacity*=(unsigned long)temp ;
        //Capacity /= 512;
    }
    return (unsigned long)Capacity ;
}


