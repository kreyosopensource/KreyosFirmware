#ifndef __MEMLCD_H_
#define __MEMLCD_H_
#include <stdint.h>
#include "grlib/grlib.h"
//*****************************************************************************
//
// Include Files
//
//*****************************************************************************

/*Put any necessary include files here*/

//*****************************************************************************
//
// User Configuration for the LCD Driver
//
//*****************************************************************************
#ifdef PRODUCT_W003
  // Number of pixels on LCD X-axis
  #define LCD_X_SIZE 128
  // Number of pixels on LCD Y-axis
  #define LCD_Y_SIZE 128
#else
  // Number of pixels on LCD X-axis
  #define LCD_X_SIZE 144
  // Number of pixels on LCD Y-axis
  #define LCD_Y_SIZE 168
#endif

#define BPP 1

//*****************************************************************************
//
// Defines for the pins that are used to communicate with the LCD Driver
//
//*****************************************************************************

/*Define Pin Names here i.e. #define LCD_RESET_OUT	P6OUT*/

//*****************************************************************************
//
// Defines for LCD driver configuration
//
//*****************************************************************************

/* Defines for pixels, colors, masks, etc. Anything Template_Driver.c needs*/


//*****************************************************************************
//
// This driver operates in four different screen orientations.  They are:
//
// * Portrait - The screen is taller than it is wide. This is selected by defining
//              PORTRAIT.
//
// * Landscape - The screen is wider than it is tall. This is selected by defining
//               LANDSCAPE.
//
// * Portrait flip - The screen is taller than it is wide. This is
//                   selected by defining PORTRAIT_FLIP.
//
// * Landscape flip - The screen is wider than it is tall. This is
//                    selected by defining LANDSCAPE_FLIP.
//
// These can also be imagined in terms of screen rotation; if landscape mode is
// 0 degrees of screen rotation, portrait flip is 90 degrees of clockwise
// rotation, landscape flip is 180 degrees of rotation, and portrait is
// 270 degress of clockwise rotation.
//
// If no screen orientation is selected, "landscape" mode will be used.
//
//*****************************************************************************
#if ! defined(PORTRAIT) && ! defined(PORTRAIT_FLIP) && \
    ! defined(LANDSCAPE) && ! defined(LANDSCAPE_FLIP)
#define LANDSCAPE
#endif

//*****************************************************************************
//
// Various definitions controlling coordinate space mapping and drawing
// direction in the four supported orientations.
//
//*****************************************************************************
#ifdef PORTRAIT
#define MAPPED_X(x, y) (LCD_X_SIZE - (y) - 1)
#define MAPPED_Y(x, y) (x)
#define LCD_WIDTH LCD_Y_SIZE
#define LCD_HEIGHT LCD_X_SIZE
#endif
#ifdef LANDSCAPE
#define MAPPED_X(x, y) (x)
#define MAPPED_Y(x, y) (y)
#define LCD_WIDTH LCD_X_SIZE
#define LCD_HEIGHT LCD_Y_SIZE
#endif
#ifdef PORTRAIT_FLIP
#define MAPPED_X(x, y)  (y)
#define MAPPED_Y(x, y)  (LCD_Y_SIZE - (x) - 1)
#define LCD_WIDTH LCD_Y_SIZE
#define LCD_HEIGHT LCD_X_SIZE
#endif
#ifdef LANDSCAPE_FLIP
#define MAPPED_X(x, y)  (LCD_X_SIZE - (x) - 1)
#define MAPPED_Y(x, y)  (LCD_Y_SIZE - (y) - 1)
#define LCD_WIDTH LCD_X_SIZE
#define LCD_HEIGHT LCD_Y_SIZE
#endif


//*****************************************************************************
//
// Various LCD Controller command name labels and associated control bits
//
//*****************************************************************************

//*****************************************************************************
//
// Macros for the Display Driver
//
//*****************************************************************************

/* All macros can go here. This is typically the color translation function (example below)
and could also include Set_Address(), Write_Data(), etc. */


//
// Translates a 24-bit RGB color to a display driver-specific color.
//
// \param c is the 24-bit RGB color.  The least-significant byte is the blue
// channel, the next byte is the green channel, and the third byte is the red
// channel.
//
// This macro translates a 24-bit RGB color into a value that can be written
// into the display's frame buffer in order to reproduce that color, or the
// closest possible approximation of that color. This particular example
// requires the 8-8-8 24 bit RGB color to convert into 5-6-5 16 bit RGB Color
// Your conversion should be made to fit your LCD settings.
//
// \return Returns the display-driver specific color

#define DPYCOLORTRANSLATE(c)    ((((c) & 0x00f80000) >> 8) |               \
                                 (((c) & 0x0000fc00) >> 5) |               \
                                 (((c) & 0x000000f8) >> 3))

//*****************************************************************************
//
// Prototypes for the globals exported by this driver.
//
//*****************************************************************************
extern void memlcd_InitScreen(void);

// need define in driver
extern void halLcdRefresh(int start, int end);
extern void Template_DriverFlush(void *pvDisplayData);
extern void memlcd_DriverInit(void);

#pragma pack(push, 1)
typedef struct
{
  uint8_t opcode;
  uint8_t linenum;
  uint8_t pixels[LCD_X_SIZE/8];
}linebuf;
#pragma pack(pop)

#define MLCD_WR 0x01          // MLCD write line command
#define MLCD_CM 0x04          // MLCD clear memory command
#define MLCD_SM 0x00          // MLCD static mode command
#define MLCD_VCOM 0x02          // MLCD VCOM bit

extern linebuf lines[LCD_Y_SIZE];

extern const tDisplay g_memlcd_Driver;

#endif
