#include "contiki.h"

#include <stdint.h>
#include <msp430.h>
#include <string.h>
#include "spiflash.h"
#include "board.h"
#include "battery.h"
#include "backlight.h"
#include <stdio.h>

#pragma segment="FLASHCODE"                 // Define flash segment code
#pragma segment="RAMCODE"

void FlashFirmware();

#define FIRMWARE_BASE (4UL * 1024 * 1024 - 256UL * 1024) // last 256KB in first 4 M

static inline uint8_t SPI_FLASH_SendByte( uint8_t data )
{
  UCA1TXBUF = data;
  while(( UCA1IFG & UCRXIFG ) == 0x00 ); // Wait for Rx completion (implies Tx is also complete)
  return( UCA1RXBUF );
}

static inline void SPI_FLASH_SendCommandAddress(uint8_t opcode, uint32_t address)
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

typedef enum 
{
    STATE_NEEDSIGNATURE,
    STATE_NEEDADDR,
    STATE_WRITE,
    STATE_DONE
}WriteState;

#define INTERRUPT_VECTOR_START 0xFFE0
#define SIGNATURE 0xFACE0001

#pragma pack(1)
struct _header
{
  uint32_t signature;
  uint32_t length;
  uint8_t crch;
  uint8_t crcl;
};

int CheckUpgrade(void)
{
  struct _header h;
  // check if the firmware is there
  SPI_FLASH_BufferRead((void*)&h, FIRMWARE_BASE, sizeof(h));

  if (h.signature != SIGNATURE)
    return 0;

  printf("Found firmware, length = %lu\n", h.length);

  SPI_FLASH_BufferRead((void*)&h, FIRMWARE_BASE + h.length + sizeof(h), sizeof(h));

  if (h.signature != SIGNATURE)
    return 1;

  SPI_FLASH_BufferRead((void*)&h, FIRMWARE_BASE + h.length + 2 * sizeof(h), sizeof(h));
  if (h.signature == SIGNATURE)
    return 2; // ignore flag

  if (battery_state() == BATTERY_STATE_DISCHARGING
    && battery_level(BATTERY_STATE_DISCHARGING) < 4)
    return 3; // not enough power

  // check CRC

  return 0xff;
}

//------------------------------------------------------------------------------
// Copy flash function to RAM.
//------------------------------------------------------------------------------
void Upgrade(void)
{
  unsigned char *flash_start_ptr;           // Initialize pointers
  unsigned char *flash_end_ptr;
  unsigned char *RAM_start_ptr;

  if (CheckUpgrade() != 0xff)
    return;

  //Initialize flash and ram start and end address
  flash_start_ptr = (unsigned char *)__segment_begin("FLASHCODE");
  flash_end_ptr = (unsigned char *)__segment_end("FLASHCODE");
  RAM_start_ptr = (unsigned char *)__segment_begin("RAMCODE");

  //calculate function size
  unsigned long function_size = (unsigned long)(flash_end_ptr) - (unsigned long)(flash_start_ptr);

  // Copy flash function to RAM
  printf("Copy From %p to %p size=%ld\n", flash_start_ptr, RAM_start_ptr, function_size);
  memcpy(RAM_start_ptr,flash_start_ptr,function_size);

  motor_on(0, 0);
  printf("Jump to %p\n", FlashFirmware);

  // remove the flag of firmware
  struct _header h;
  SPI_FLASH_BufferRead((void*)&h, FIRMWARE_BASE, sizeof(h));
  SPI_FLASH_BufferWrite((void*)&h, FIRMWARE_BASE + h.length + 2 * sizeof(h), sizeof(h));

  FlashFirmware();
}

void WriteFirmware(void* data, uint32_t offset, int size)
{
  SPI_FLASH_BufferWrite(data, offset + FIRMWARE_BASE, size);
}

void EraseFirmware()
{
  long size = 64UL * 1024;
  for(long i = 0; i < 256UL *1024; i += size)
  {
    SPI_FLASH_SectorErase(i + FIRMWARE_BASE, size);
  }
}

#pragma segment="FLASHCODE"                 // Define flash segment code
#pragma segment="RAMCODE"

#if FLASHDEBUG
#define DCO_SPEED F_CPU

#define BitTime_115200   (DCO_SPEED / 115200)
#define BitTime_5_115200 (BitTime_115200 / 2)

#pragma location="RAMCODE"
int putchar_(int data)
{
    int tempData;

    int parity_mask = 0x200;
    char bitCount = 0xB;                    // Load Bit counter, 8data + ST/SP +parity
    int flag;
    //while (TA0CCTL0 & CCIE);                                    // Ensure last char got TX'd

    TA0CCR0 = TA0R;                       // Current state of TA counter
    TA0CCR0 += BitTime_115200;
    tempData = 0x200 + (int)data;           // Add mark stop bit to Data
    tempData = tempData << 1;
    //TZNCCTL_TX = OUTMOD0;

    //int x = splhigh();
    while (bitCount != 0)
    {
        while (!(TA0CCTL0 & CCIFG)) ;
        TA0CCTL0 &= ~CCIFG;
        TA0CCR0 += BitTime_115200;
        if (tempData & 0x01)
        {
            tempData ^= parity_mask;
            TA0CCTL0 &= ~OUTMOD2;         // TX '1'
        }
        else
        {
            TA0CCTL0 |=  OUTMOD2;             // TX '0'
        }

        parity_mask = parity_mask >> 1;
        tempData = tempData >> 1;
        bitCount--;
    }
    while (!(TA0CCTL0 & CCIFG)) ;         // wait for timer
    //splx(x);

    return data;
}

#pragma location="RAMCODE"
static void puth_(unsigned n)
{
  n &= 15;

  if (n >= 10)
    putchar_('A' + n - 10);
  else
    putchar_('0' + n);
}

#pragma location="RAMCODE"
static void putx_(uint32_t n)
{
  uint8_t * t = (uint8_t*)&n;
  for (int i = 0; i < 4; i++)
  {
    uint8_t k = *t++;
    puth_(k >> 4);
    puth_(k);
  }
  putchar_('\n');
}
#else
#pragma location="RAMCODE"
static void putx_(uint32_t n){}
#pragma location="RAMCODE"
static void puth_(unsigned n){}
#pragma location="RAMCODE"
int putchar_(int data){return 0;}
#endif
//------------------------------------------------------------------------------
// This portion of the code is first stored in Flash and copied to RAM then
// finally executes from RAM.
//-------------------------------------------------------------------------------

#pragma location="RAMCODE"
void BSL430_massErase()
{
    volatile char *Flash_ptr;                   // Flash pointer

    while (FCTL3 & BUSY) ;
    FCTL3 = FWKEY;
    while (FCTL3 & BUSY) ;
    Flash_ptr = (char *)INTERRUPT_VECTOR_START; // Initialize Flash pointer
    FCTL1 = FWKEY + MERAS + ERASE;           // Set Mass Erase bit
    *Flash_ptr = 0;                             // Dummy write to erase main flash
    while (FCTL3 & BUSY) ;
    FCTL3 = FWKEY + LOCK;                    // Set LOCK bit
}

#pragma location="RAMCODE"
void BSL430_writeByte(uint32_t addr, char data)
{
    while (FCTL3 & BUSY) ;
    __data20_write_char(addr, data);
    while (FCTL3 & BUSY) ;
    if (data != __data20_read_char(addr))
    {
       putchar_('E');
    }
}

#pragma location="RAMCODE"
void FlashFirmware()
{
  unsigned int i;
  char *pBuffer;
  WriteState state;

  uint16_t NumByteToRead;
  uint32_t NumByteToWrite;

  uint32_t write_ptr;
  uint32_t buffer[32]; // 32 * 4 = 128
  
  __disable_interrupt();                    // 5xx Workaround: Disable global
                                            // interrupt while erasing. Re-Enable
                                            // GIE if needed

  state = STATE_NEEDSIGNATURE;
  // Start the loop
  while(state != STATE_DONE)
  {
    putchar_('a' + state);
    switch(state)
    {
      case STATE_NEEDSIGNATURE:
      NumByteToRead = 10; // the size of file header, sync with main.c under convert tool src
      SPI_FLASH_CS_LOW();
      SPI_FLASH_SendCommandAddress(W25X_ReadData, FIRMWARE_BASE);

      break;
      case STATE_NEEDADDR:
      NumByteToRead = 8; // one address and one 
      break;
      case STATE_WRITE:
      if (NumByteToWrite > 128)
        NumByteToRead = 128;
      else
        NumByteToRead = NumByteToWrite;
      break;
    }
 
    pBuffer = (char*)&buffer[0];;
    for(int i = 0; i < NumByteToRead; i++)
    {
      *pBuffer = ~SPI_FLASH_SendByte(Dummy_Byte);
      pBuffer++;
    }

    switch(state)
    {
      case STATE_NEEDSIGNATURE:
      {
        if (buffer[0] != SIGNATURE)
        {
          SPI_FLASH_CS_HIGH();
          state = STATE_DONE; // error
          continue;
        }

        // Erase Flash
        BSL430_massErase();
        state = STATE_NEEDADDR;
      }
      break;
      case STATE_NEEDADDR:
      {
        if (buffer[0] == 0 
          || buffer[0] == SIGNATURE) // hit the end
        {
          state = STATE_DONE;
          continue;
        }

        // first uint32 is start address, second uint32 is length
        write_ptr = buffer[0];
        NumByteToWrite = buffer[1];
        //putx_(write_ptr);
        //putx_(NumByteToWrite);
        state = STATE_WRITE;
      }
      break;
      case STATE_WRITE:
      {
        while(BUSY & FCTL3);                 // Test wait until ready for next byte
        // Write Flash
        FCTL1 = FWKEY+WRT;                 // Enable block write
        FCTL3 = FWKEY;                       // Set LOCK
        char* src = (char*)&buffer[0];
        for(i = 0; i < NumByteToRead; i++)
        {
          BSL430_writeByte(write_ptr++, *src++);
        }
        FCTL1 = FWKEY;
        FCTL3 = FWKEY+LOCK;                       // Set LOCK
        while(BUSY & FCTL3);                      // Check for write completion

        NumByteToWrite -= NumByteToRead;

        if (NumByteToWrite == 0)
          state = STATE_NEEDADDR;
      }
      break;
    }
  }

  // reboot
  WDTCTL = 0;
}

