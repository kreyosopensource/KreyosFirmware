//*****************************************************************************
//
// Include Files
//
//*****************************************************************************
#include "contiki.h"
#include "memlcd.h"
#include <string.h>

// Initializes the display driver.
// This function initializes the LCD controller
//
// TemplateDisplayFix
void
memlcd_DriverInit(void)
{
    memlcd_InitScreen();
}

static void screenshot(void);
//*****************************************************************************
//
//! Flushes any cached drawing operations.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//!
//! This functions flushes any cached drawing operations to the display.  This
//! is useful when a local frame buffer is used for drawing operations, and the
//! flush would copy the local frame buffer to the display.
//!
//! \return None.
//
//*****************************************************************************
void
Template_DriverFlush(void *pvDisplayData)
{
    screenshot();
}

void halLcdRefresh(int start, int end)
{
    
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

static uint8_t lookup[16] = {
   0x0, 0x8, 0x4, 0xC,
   0x2, 0xA, 0x6, 0xE,
   0x1, 0x9, 0x5, 0xD,
   0x3, 0xB, 0x7, 0xF };

static uint8_t flip( uint8_t n )
{
   //This should be just as fast and it is easier to understand.
   //return (lookup[n%16] << 4) | lookup[n/16];
   return (lookup[n&0x0F] << 4) | lookup[n>>4];
}

#include <stdio.h>

#pragma pack(1)
struct BITMAPFILEHEADER
{
    uint8_t header[2];
    uint32_t filesize;
    uint32_t reserved;
    uint32_t offset;
};

struct BITMAPINFOHEADER
{
    uint32_t headersize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compress;
    uint32_t datasize;
    uint32_t hres, vres;
    uint32_t colors;
    uint32_t important;
};

void screenshot()
{
  static int id;
  FILE* fp;

  char name[30];
  sprintf(name, "unittest/screen/image%03d.bmp", id++);
  fp = fopen(name, "wb");

  // write the BMP header
  struct BITMAPFILEHEADER fh;
  struct BITMAPINFOHEADER ih;

#if CC == GCC
  memset(&fh, 0, sizeof(fh));
  memset(&ih, 0, sizeof(ih));
#else
  bzero(&fh, sizeof(fh));
  bzero(&ih, sizeof(ih));
#endif

  int sizeofline = ((LCD_X_SIZE / 8) + 3) & (~3);
  fh.header[0] = 'B'; fh.header[1] = 'M';
  fh.filesize = sizeof(fh) + sizeof(ih) + 4 * 2 + LCD_Y_SIZE * sizeofline;
  fh.reserved = 0;
  fh.offset = sizeof(fh) + sizeof(ih) + 4 * 2;

  ih.headersize = sizeof(ih);
  ih.width = LCD_X_SIZE;
  ih.height = LCD_Y_SIZE;
  ih.planes = 1;
  ih.bpp = 1;
  ih.compress = 0;
  ih.datasize = sizeofline * LCD_Y_SIZE;
  ih.colors = 2;

  fwrite(&fh, sizeof(fh), 1, fp);
  fwrite(&ih, sizeof(ih), 1, fp);

  // write pallete
  char color[4];
#if CC == GCC
  memset(&color, 0, sizeof(color));
#else
  bzero(&color, sizeof(color));
#endif

  fwrite(color, sizeof(color), 1, fp);
  color[0] = color[1] = color[2] = 0xff;
  fwrite(color, sizeof(color), 1, fp);  

  // write real data, align with 4bytes
  for(int i = LCD_Y_SIZE - 1; i >= 0; i--)
  {
    uint8_t buf[LCD_X_SIZE/8 + 4];
    for(int j = 0; j < LCD_X_SIZE/8; j++)
    {
     buf[j] = flip(lines[i].pixels[j]);
    }
    fwrite(buf, sizeofline, 1, fp);
  }

  fclose(fp);
}
