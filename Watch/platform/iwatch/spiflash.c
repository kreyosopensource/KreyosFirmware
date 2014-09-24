#include "contiki.h"
#include <stdio.h>
#include "sys/rtimer.h"
#include "isr_compat.h"
#include "spiflash.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#define hexdump(...)
#endif

/*
* Exchanges data on SPI connection
* - Busy waits until entire shift is complete
* - This function is safe to use to control hardware lines that rely on shifting being finalised
*/

/******************************************************************************************
*函数名：SPI_FLASH_SendByte()
* 参数：u8 byte        写入的数据
* 返回值：u8 8位数据
* 功能：SPIFLASH读写一个字节函数，外部调用
*********************************************************************************************/
static inline uint8_t SPI_FLASH_SendByte( uint8_t data )
{
  UCA1TXBUF = data;
  while(( UCA1IFG & UCRXIFG ) == 0x00 ); // Wait for Rx completion (implies Tx is also complete)
  return( UCA1RXBUF );
}

static void SPI_FLASH_SendCommandAddress(uint8_t opcode, uint32_t address)
{
  SPI_FLASH_SendByte(opcode);
  SPI_FLASH_SendByte((address >> 16) & 0xFF);
  SPI_FLASH_SendByte((address >> 8) & 0xFF);
  SPI_FLASH_SendByte(address & 0xFF);
}

static inline void SPI_FLASH_CS_LOW()
{
  UCA1CTL1 &= ~UCSWRST;
  SPIOUT &= ~CSPIN;
}

static inline void SPI_FLASH_CS_HIGH()
{
  while(UCA1STAT & UCBUSY);
  UCA1CTL1 |= UCSWRST;
  SPIOUT |= CSPIN;
  __delay_cycles(10);
}


/******************************************************************************************
*函数名：SPI_FLASH_SectorErase()
* 参数：u32 SectorAddr   块地址
* 返回值：void
* 功能：SPIFLASH扇区擦除函数，外部调用
*********************************************************************************************/
void SPI_FLASH_SectorErase(u32 SectorAddr, u32 size)
{
  uint8_t opcode;
  if (size == 4 * 1024UL)
  {
    opcode = W25X_SectorErase;
  }
  else if (size == 32 * 1024UL)
  {
    opcode = W25X_BlockErase32;
  }
  else if (size == 64 * 1024UL)
  {
    opcode = W25X_BlockErase64;
  }
  else
  {
    PRINTF("Error in erase size: %lx\n", size);
    return;
  }

  PRINTF("Erase offset: %lx\n", SectorAddr);
 
  /*发送写数据使能指令*/
  SPI_FLASH_WriteEnable();
  /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送扇区擦除指令*/
  SPI_FLASH_SendCommandAddress(opcode, SectorAddr);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  /* 等待写完毕*/
  SPI_FLASH_WaitForWriteEnd();
}

/******************************************************************************************
*函数名：SPI_FLASH_BulkErase()
* 参数：void
* 返回值：void
* 功能：SPIFLASH整片擦除函数，外部调用
*********************************************************************************************/
void SPI_FLASH_BulkErase(void)
{
  /*使能写入*/
  SPI_FLASH_WriteEnable();
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送整片擦除指令*/
  SPI_FLASH_SendByte(W25X_ChipErase);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  /* 等待写完成*/
  SPI_FLASH_WaitForWriteEnd();
}

/******************************************************************************************
*函数名：SPI_FLASH_PageWrite()
* 参数：u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite 数据指针，写入地址，写入的个数
* 返回值：void
* 功能：SPIFLASH页写入数据函数，外部调用
*********************************************************************************************/
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  PRINTF("Write to Disk: offset:%lx size:%d\n", WriteAddr, NumByteToWrite);
  hexdump(pBuffer, NumByteToWrite);

   /*使能写入*/
  SPI_FLASH_WriteEnable();
  /*使能片选*/
  SPI_FLASH_CS_LOW();
  /* 发送页写入指令*/
  SPI_FLASH_SendCommandAddress(W25X_PageProgram, WriteAddr);

  /*检测写入的数据是否超出页的容量大小*/
  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
  }
  /*循环写入数据*/
  while (NumByteToWrite--)
  {
    /*发送数据*/
    SPI_FLASH_SendByte(~(*pBuffer));
    /* 指针移到下一个写入数据 */
    pBuffer++;
  }
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  /* 等待写完成*/
  SPI_FLASH_WaitForWriteEnd();
}

/******************************************************************************************
*函数名：SPI_FLASH_BufferWrite()
* 参数：u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite 数据指针，写入地址，写入的个数
* 返回值：void
* 功能：SPIFLASH多个数据函数，外部调用
*********************************************************************************************/
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  Addr = WriteAddr % SPI_FLASH_PageSize;                           //计算写入的页的对应初始地址
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;                //计算总共要写的页数
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;               //计算剩余单个页写的数据个数
  if (Addr == 0) /* 如果要写入的页地址为0，说明正好整页写数据，没有偏移*/
  {
    if (NumOfPage == 0) /* 如果计算的写入页数为0，说明数据量在一个页的范围内，可直接进行页的写*/
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);      //进行页写数据
    }
    else /* 如果要写的页数大于0*/
    { 
      /*先将开头数据进行整页写入*/
      while (NumOfPage--)
      { 
        //整页写入
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        //地址偏移
        WriteAddr +=  SPI_FLASH_PageSize;
        //数据指针偏移
        pBuffer += SPI_FLASH_PageSize;
      }
       //将剩余数据个数写入
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*如果写入的地址不在页的开头位置*/
  {
    if (NumOfPage == 0) /*如果写入数据页的个数为0，即数据小于一页容量*/
    {
      if (NumOfSingle > count) /*如果剩余数据大于当前页的剩余容量*/
      {
        temp = NumOfSingle - count;     //计算超出的数据个数
        /*写满当前页*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        /*设置地址偏移*/
        WriteAddr +=  count;
        /*设置数据指针偏移*/
        pBuffer += count;
        /*将剩余量写入新的页*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else  /*如果剩余数据小于当前页的剩余容量*/
      {
        /*直接写入当前页*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*如果写入数据页的个数大于0，即数据大于一页容量*/
    {
      NumByteToWrite -= count;         //总数据减去当前页剩余的容量
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;  //计算要写的整页个数
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize; //计算剩余数据个数
      /*将开头数据写入当前页剩余字节个数*/
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      /*设置地址偏移*/
      WriteAddr +=  count;
      /*设置数据指针偏移*/
      pBuffer += count;
       /*开始剩下数据的整页写入*/
      while (NumOfPage--)
      {
        /*写入一个页的字节数*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        /*设置地址偏移*/
        WriteAddr +=  SPI_FLASH_PageSize;
        /*设置指针偏移*/
        pBuffer += SPI_FLASH_PageSize;
      }
      /*如果剩余数据大于0，将剩余的个数写入下一个页*/
      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/******************************************************************************************
*函数名：SPI_FLASH_BufferRead()
* 参数：u8* pBuffer, u32 ReadAddr, u16 NumByteToRead 数据指针，读出的地址，读出的个数
* 返回值：void
* 功能：SPIFLASH多个数据函数，外部调用
*********************************************************************************************/
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  PRINTF("Read from Disk: offset:%lx size:%d\n", ReadAddr, NumByteToRead);
  //u16 n = NumByteToRead;

   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送读数据指令*/
  SPI_FLASH_SendCommandAddress(W25X_ReadData, ReadAddr);
  while (NumByteToRead--) /* 循环读取数据*/
  {
    /*读取一个字节数据*/
    *pBuffer = ~SPI_FLASH_SendByte(Dummy_Byte);
    /*数据指针加1*/
    pBuffer++;
  }
  /*失能片选*/
  SPI_FLASH_CS_HIGH();

  hexdump(pBuffer - n, n);
}

/******************************************************************************************
*函数名：SPI_FLASH_BufferRead()
* 参数：u8* pBuffer, u32 ReadAddr, u16 NumByteToRead 数据指针，读出的地址，读出的个数
* 返回值：void
* 功能：SPIFLASH多个数据函数，外部调用
*********************************************************************************************/
void SPI_FLASH_BufferRead_Raw(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  PRINTF("Read from Disk: offset:%lx size:%d\n", ReadAddr, NumByteToRead);
  //u16 n = NumByteToRead;

   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送读数据指令*/
  SPI_FLASH_SendCommandAddress(W25X_ReadData, ReadAddr);
  while (NumByteToRead--) /* 循环读取数据*/
  {
    /*读取一个字节数据*/
    *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
    /*数据指针加1*/
    pBuffer++;
  }
  /*失能片选*/
  SPI_FLASH_CS_HIGH();

  hexdump(pBuffer - n, n);
}

/******************************************************************************************
*函数名：SPI_FLASH_ReadID()
* 参数：void
* 返回值：u32 器件ID
* 功能：SPIFLASH读取ID函数，外部调用
*********************************************************************************************/
u32 SPI_FLASH_ReadID(void)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* 使能片选 */
  SPI_FLASH_CS_LOW();

  /*发送识别器件ID号*/
  SPI_FLASH_SendByte(W25X_JedecDeviceID);
  /* 读取一个字节*/
  Temp0 = SPI_FLASH_SendByte(Dummy_Byte);
  /* 读取一个字节*/
  Temp1 = SPI_FLASH_SendByte(Dummy_Byte);
   /* 读取一个字节*/
  Temp2 = SPI_FLASH_SendByte(Dummy_Byte);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
  return Temp;
}
/******************************************************************************************
*函数名：SPI_FLASH_ReadDeviceID()
* 参数：void
* 返回值：u32 设备ID
* 功能：SPIFLASH读取设备ID函数，外部调用
*********************************************************************************************/
u32 SPI_FLASH_ReadDeviceID(void)
{
  u32 Temp = 0;
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送读取ID指令*/
  SPI_FLASH_SendCommandAddress(W25X_DeviceID, 0UL);
  /*读取8位数据*/
  Temp = SPI_FLASH_SendByte(Dummy_Byte);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  return Temp;
}

static void SPI_FLASH_WaitForFlag(uint8_t flag)
{
  u16 tick = 0;
  u8 FLASH_Status = 0;
  do
  {
     /* 使能片选 */
    SPI_FLASH_CS_LOW();
    /*发送读状态指令 */
    SPI_FLASH_SendByte(W25X_ReadStatusReg);
    /*循环发送空数据直到FLASH芯片空闲*/
    /* 发送空字节 */
    FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);
    tick++;
    /*失能片选*/
    SPI_FLASH_CS_HIGH();
  }while ((FLASH_Status & flag) == flag); /* 检测是否空闲*/

  PRINTF("operation takes %d ticks\n", tick);
}

/******************************************************************************************
*函数名：SPI_FLASH_WriteEnable()
* 参数：void
* 返回值：void
* 功能：SPIFLASH写使能函数，外部调用
*********************************************************************************************/
void SPI_FLASH_WriteEnable(void)
{
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送写使能指令*/
  SPI_FLASH_SendByte(W25X_WriteEnable);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();

  /*check WEL */
  //SPI_FLASH_WaitForFlag(WEL_Flag);
}

/******************************************************************************************
*函数名：SPI_FLASH_WaitForWriteEnd()
* 参数：void
* 返回值：void
* 功能：SPIFLASH等待写完毕函数，外部调用
*********************************************************************************************/
void SPI_FLASH_WaitForWriteEnd(void)
{
  SPI_FLASH_WaitForFlag(WIP_Flag);
}

/******************************************************************************************
*函数名：SPI_Flash_PowerDown()
* 参数：void
* 返回值：void
* 功能：SPIFLASH进入掉电模式函数，外部调用
*********************************************************************************************/
void SPI_Flash_PowerDown(void)   
{ 
  /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送掉电指令 */
  SPI_FLASH_SendByte(W25X_PowerDown);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
}   

/******************************************************************************************
*函数名：SPI_Flash_WAKEUP()
* 参数：void
* 返回值：void
* 功能：SPIFLASH唤醒掉电模式函数，外部调用
*********************************************************************************************/
void SPI_Flash_WAKEUP(void)   
{
  /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /* 发送退出掉电模式指令 */
  SPI_FLASH_SendByte(W25X_ReleasePowerDown);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();              
}  

/******************************************************************************************
*函数名：SPI_Flash_Reset()
* 参数：void
* 返回值：void
* 功能：SPIFLASH Reset函数，外部调用
*********************************************************************************************/
void SPI_Flash_Reset(void)   
{
  /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /* 发送退出掉电模式指令 */
  SPI_FLASH_SendByte(W25X_EnableReset);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /* 发送退出掉电模式指令 */
  SPI_FLASH_SendByte(W25X_Reset);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  
}  

void SPI_FLASH_Init(void)
{
  // init SPI
  UCA1CTL1 = UCSWRST;

  UCA1CTL0 |= UCMST + UCSYNC + UCMSB + UCCKPL; // master, 3-pin SPI mode, LSB //UCCKPH
  UCA1CTL1 |= UCSSEL__SMCLK; // SMCLK for now
  UCA1BR0 = 4; // 8MHZ / 4 = 2Mhz
  UCA1BR1 = 0;
  UCA1MCTL = 0;

  //Configure ports.
  CLKDIR |= CLKPIN;
  CLKSEL |= CLKPIN;
  CLKOUT &= ~CLKPIN;

  SPIDIR |= SIPIN | CSPIN;
  SPIDIR &= ~SOPIN; // SO is input
  SPISEL |= SIPIN | SOPIN;
  SPIOUT &= ~SIPIN;
  SPIOUT |= CSPIN; // pull CS high to disable chip

  //SPI_Flash_Reset();

  printf("\n$$OK SPIFLASH\n");
#if 1
  uint8_t FLASH_Status;
  SPI_FLASH_CS_LOW();
  /*发送读状态指令 */
  SPI_FLASH_SendByte(W25X_ReadStatusReg);
  /* 发送空字节 */
  FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  printf("status register 1 = %x ", FLASH_Status);

  SPI_FLASH_CS_LOW();
  /*发送读状态指令 */
  SPI_FLASH_SendByte(W25X_ReadStatusReg2);
  /* 发送空字节 */
  FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  printf("status register 2 = %x ", FLASH_Status);  
#endif
  printf("Find SPI Flash DeviceId = %x\n", (uint16_t)SPI_FLASH_ReadDeviceID());

//  SPI_FLASH_BulkErase();
}
