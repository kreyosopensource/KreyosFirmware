/*==========================================================================*\
|                                                                            |
|                                                                            |
| PC-side Bootstrap Loader communication Application                         |
|                                                                            |
| See main.c for full version and legal information                          |
|                                                                            |
\*==========================================================================*/
#include "BSL_IO_UART.h"
#include "BSL_Interface.h"
#include <windows.h>

void SetRSTpin(BOOL level);
void SetTESTpin(BOOL level);

#define TIMEOUT_MS 20000

HANDLE       hComPort;    /* COM-Port Handle             */
DCB          comDCB;      /* COM-Port Control-Settings   */
COMSTAT      comState;    /* COM-Port Status-Information */
COMMTIMEOUTS orgTimeouts; /* Original COM-Port Time-out  */
unsigned int        nakDelay;    /* Delay before DATA_NAK will be send */
unsigned int        UART_verbose = 0; /* verbose mode flag, default off */
unsigned int        pinReverted = 0;
int initializeCommPort(unsigned char* comPort, unsigned char parity);
void ReadGargbageUntilTimeout(void);
void clearBuffers();
void delay( unsigned int time );
unsigned long int calcTimeout(unsigned long startTime);

/*******************************************************************************
*Function:    setVerbose
*Description: sets Verbose mode to on or off for debugging
*Parameters: 
*             int val   1 == verbose mode on, 0 == verbose mode off
*Returns:
*             none
*******************************************************************************/
void UART_setVerbose( unsigned int val )
{
  UART_verbose = val;
}
void UART_revert(unsigned int pin)
{
  if (pinReverted & pin)
  {
    pinReverted &= ~pin;
  }
  else
  {
    pinReverted |= pin;
  }
}

void UART_reset()
{
  SetRSTpin(1); 
  SetTESTpin(0);
  delay(250);

    SetRSTpin(1);  /* RST  pin: GND */
    SetTESTpin(0); /* TEST pin: GND */   delay(10);   
    SetTESTpin(0); /* TEST pin: Vcc */   delay(10);   
    SetTESTpin(0); /* TEST pin: GND */   delay(10);   
    SetTESTpin(0); /* TEST pin: Vcc */   delay(10);  
    SetTESTpin(0); /* TEST pin: GND */   delay(10); 
    SetRSTpin (0); /* RST  pin: Vcc */   delay(10);
}

unsigned int UART_GetVerbose()
{
  return UART_verbose;
}
/*******************************************************************************
*Function:    initialize_BSL
*Description: Initializes the COM port and invokes the BSL.
*Parameters: 
*             char* comPort         a string for the COM port ie "COM1"
*Returns:
*             none
*******************************************************************************/
int UART_initialize_BSL_5438(unsigned char* comPort)
{
  if (initializeCommPort(comPort,0))
    return -1;
  invokeBSL();

  return 0;
}
/*******************************************************************************
*Function:    initialize_BSL
*Description: Initializes the COM port and invokes the BSL.
*Parameters: 
*             char* comPort         a string for the COM port ie "COM1"
*Returns:
*             none
*******************************************************************************/
int UART_initialize_BSL(unsigned char* comPort)
{
  if (initializeCommPort(comPort,1))
    return -1;
  invokeBSL();

  return 0;
}

/*******************************************************************************
*Function:    invokeBSL
*Description: toggles the TEST and RESET pins in order to invoke the BSL
*Parameters: 
*             none
*Returns:
*             none
*******************************************************************************/
void invokeBSL()
{
//#define running 1
//#ifndef running
  SetRSTpin(0); 
  SetTESTpin(0);
  delay(250);

    SetRSTpin(0);  /* RST  pin: GND */
    SetTESTpin(0); /* TEST pin: GND */   delay(10);   
    SetTESTpin(1); /* TEST pin: Vcc */   delay(10);   
    SetTESTpin(0); /* TEST pin: GND */   delay(10);   
    SetTESTpin(1); /* TEST pin: Vcc */   delay(10);  
    SetTESTpin(0); /* TEST pin: GND */   delay(10); 
    SetRSTpin (1); /* RST  pin: Vcc */   delay(10);
    
	//SetTESTpin(1);
	//SetRSTpin (0);
	//delay(10);
	//SetTESTpin(0);
	//SetRSTpin (1);

  delay(350); 	
  PurgeComm(hComPort, PURGE_TXCLEAR);
  PurgeComm(hComPort, PURGE_RXCLEAR); 
  changeCommBaudRate(9601);
  changeCommBaudRate(9600);
  delay(350);
//#endif
  //changeCommBaudRate(9600);
}

/*******************************************************************************
*Function:    SetRSTpin
*Description: Controls RST Pin
*Parameters: 
*             BOOL level            0: GND; 1: VCC
*Returns:
*             none
*******************************************************************************/
void SetRSTpin(BOOL level)
{
  if (pinReverted & RST_PIN)
  {
    comDCB.fDtrControl = level ? DTR_CONTROL_DISABLE : DTR_CONTROL_ENABLE;
  }
  else
  {
    comDCB.fDtrControl = level ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
  }

  SetCommState(hComPort, &comDCB);
} /* SetRSTpin */

/*******************************************************************************
*Function:    SetTESTpin
*Description: Opens a TXT file for writing with append
*Parameters: 
*             BOOL level            0: GND; 1: VCC
*Returns:
*             none
*******************************************************************************/
void SetTESTpin(BOOL level)
{
  if (pinReverted & TEST_PIN)
  {
    comDCB.fRtsControl = level ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE ;
  }
  else
  {
    comDCB.fRtsControl = level ? RTS_CONTROL_DISABLE : RTS_CONTROL_ENABLE ;
  }
  SetCommState(hComPort, &comDCB);
} /* SetTESTpin */

/*******************************************************************************
*Function:    writeByte
*Description: writes a single byte to the COM port
*Parameters: 
*             char byte             The byte to write to the COM port
*Returns:
*             none
*******************************************************************************/
void writeByte( unsigned char byte  )
{
  unsigned int dwWrite;
  if( UART_verbose )
  {
    printf( "[%2.2x] ", (byte&0xFF));
  }
  //PurgeComm(hComPort, PURGE_RXCLEAR | PURGE_RXABORT);
  WriteFile(hComPort, &byte, 1, &dwWrite, NULL);
}

void writeBytes( unsigned char *byte, int size )
{
  unsigned int dwWrite;
  if( UART_verbose )
  {
    for(int i = 0; i < size; i++)
      printf( "[%2.2x] ", (*(byte+i)&0xFF));
  }
  //PurgeComm(hComPort, PURGE_RXCLEAR | PURGE_RXABORT);
  WriteFile(hComPort, byte, size, &dwWrite, NULL);
}

/*******************************************************************************
*Function:    readByte
*Description: reads a single byte from the COM port buffer
*Parameters: 
*             none
*Returns:
*             char                  The read byte
*******************************************************************************/
unsigned char readByte()
{
  readByte_s(TIMEOUT_MS);
}

unsigned char readByte_s(int timeoutMS)
{
  unsigned char byte = TIMEOUT_ERROR;
  unsigned int dwRead;
  unsigned int errors;
  unsigned int error_flag = 0;
  unsigned int timeout = GetTickCount()+ timeoutMS;
  
  #if 0
  if (timeoutMS != -1)
  {
  do
  {
    ClearCommError(hComPort, &errors, &comState);
	  error_flag = ( GetTickCount() > timeout );
    Sleep(1);
  }
  while(comState.cbInQue == 0 && error_flag == 0);
  }
#endif
  if( error_flag == 0 )
  {
    if (!ReadFile(hComPort, &byte, 1, &dwRead, NULL))
    {
      byte = TIMEOUT_ERROR;
    }
  }
  if( UART_verbose )
  {
    printf( "{%2.2x} ", (byte&0xFF));
  }
  return byte;
}

/*******************************************************************************
*Function:    changeCommBaudRate
*Description: changes the baud rate for communication
*Parameters: 
*             int rate              The rate to change as defined in the header file
*Returns:
*             none
*******************************************************************************/
void changeCommBaudRate(unsigned int rate)
{
  GetCommState(hComPort, &comDCB);
  comDCB.BaudRate = rate;
  /*
  switch( rate )
  {
  case 4800:
    comDCB.BaudRate    = CBR_4800; 
	break;
  case 9600:
    comDCB.BaudRate    = CBR_9600; 
  case 9601:
    comDCB.BaudRate    = 9601; 
	break;
  case 19200:
    comDCB.BaudRate    = CBR_19200; 
	break;
  case 38400:
    comDCB.BaudRate    = CBR_38400; 
	break;
  case 57600:
    comDCB.BaudRate    = CBR_57600; 
	break;
  case 115200:
    comDCB.BaudRate    = CBR_115200; 
	break;
  }
  */
  SetCommState(hComPort, &comDCB);
}

/*******************************************************************************
*Function:    initializeCommPort
*Description: initializes a COM port for communication
*Parameters: 
*             char *comPort         A string for the com port ie. "COM1"
*Returns:
*             none
*******************************************************************************/
int initializeCommPort(unsigned char* comPort, unsigned char parity)
{
  COMMTIMEOUTS timeouts;
  unsigned int dwCommEvents;
  hComPort = CreateFile(comPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
  if (hComPort == INVALID_HANDLE_VALUE)
  {
    return -1;
  }

  SetupComm(hComPort, QUEUE_SIZE, QUEUE_SIZE); 

  GetCommTimeouts(hComPort, &orgTimeouts);
  /* Set Windows timeout values (disable build-in timeouts): */
  timeouts.ReadIntervalTimeout= 10;
  timeouts.ReadTotalTimeoutMultiplier= TIMEOUT_MS;
  timeouts.ReadTotalTimeoutConstant= 10;
  timeouts.WriteTotalTimeoutMultiplier= TIMEOUT_MS;
  timeouts.WriteTotalTimeoutConstant= 10;

  SetCommTimeouts(hComPort, &timeouts);

  dwCommEvents= EV_RXCHAR | EV_TXEMPTY | EV_RXFLAG | EV_ERR;
  SetCommMask(hComPort, dwCommEvents);

  GetCommState(hComPort, &comDCB);
  comDCB.BaudRate    = CBR_9600; /* Startup-Baudrate: 9,6kBaud */
  comDCB.ByteSize    = 8;
  nakDelay= (DWORD)((11*MAX_UART_FRAME_SIZE)/9.6);

  if( parity )
  {
    comDCB.Parity      = EVENPARITY;   
  }
  else
  {
    comDCB.Parity      = NOPARITY; 
  }
  comDCB.StopBits    = ONESTOPBIT;
  comDCB.fBinary     = TRUE; /* Enable Binary Transmission */
  comDCB.fParity     = TRUE; /* Enable Parity Check        */
  comDCB.ErrorChar   = (char)0xEF; 
  /* Char. w/ Parity-Err are replaced with 0xff 
   *(if fErrorChar is set to TRUE) 
   */
  comDCB.fRtsControl = RTS_CONTROL_ENABLE; /* For power supply */
  comDCB.fDtrControl = DTR_CONTROL_ENABLE; /* For power supply */
                       
  comDCB.fOutxCtsFlow= FALSE;        comDCB.fOutxDsrFlow= FALSE;        
  comDCB.fOutX       = FALSE;        comDCB.fInX        = FALSE;
  comDCB.fNull       = FALSE;

  comDCB.fErrorChar  = FALSE; 

  /* Assign new state: */
  SetCommState(hComPort, &comDCB);

  /* Clear buffers: */
  clearBuffers();

  return 0;
}

/*******************************************************************************
*Function:    clearBuffers
*Description: purges the Buffers
*Parameters: 
*             none
*Returns:
*             none
*******************************************************************************/
void clearBuffers(void)
{
  PurgeComm(hComPort, PURGE_TXCLEAR | PURGE_TXABORT);
  PurgeComm(hComPort, PURGE_RXCLEAR | PURGE_RXABORT);
}

/*******************************************************************************
*Function:    ReadGargbageUntilTimeout
*Description: if a problem with RX from the 430 happens, this function reads from the 430
*             until a timeout occurs, so communication can begin again correctly
*Parameters: 
*             none
*Returns:
*             none
*******************************************************************************/
void ReadGargbageUntilTimeout(void)
{
	char rxed_byte = 0;
#ifdef DEBUG_ERRORS
printf( "\n---------------ERROR on Packet RX---------------\n");
//printf( "header:\t%2.2x\n", (RX_header&0xFF) );
//printf( "size:\t%x\n", (RX_size&0xFF) );
printf( "packets:\t");
#endif
  do
  {
    rxed_byte = readByte();
#ifdef DEBUG_ERRORS
    printf( "(%2.2x)", (rxed_byte&0xFF) );
#endif
  }
  while( rxed_byte!= (char)TIMEOUT_ERROR );
#ifdef DEBUG_ERRORS
//printf( "\nchecksum low:\t%2.2x\n", (RX_checksum_low&0xFF) );
//printf( "checksum high:\t%2.2x\n", (RX_checksum_high&0xFF) );
printf( "------------------------------------------------\n");
#endif
}
/*******************************************************************************
*Function:    delay
*Description: delays program execution by a certain number of miliseconds
*Parameters: 
*             DWORD time            Number of miliseconds to pause
*Returns:
*             none
*******************************************************************************/
void delay(unsigned int time) /* exported! */
{ 
#ifndef WIN32
  unsigned long int startTime= GetTickCount();
  while (calcTimeout(startTime) < time);
#else
  Sleep(time);
#endif
}

/*******************************************************************************
*Function:    calcTimeout
*Description: Calculates the difference between startTime and the acutal windows time
*Parameters: 
*             DWORD startTime       the Start time
*Returns:
*             DWORD                 The difference between start Time and current time
*******************************************************************************/
unsigned long int calcTimeout(unsigned long startTime) /* exported! */
{
  return((unsigned long int)(GetTickCount() - startTime));
}