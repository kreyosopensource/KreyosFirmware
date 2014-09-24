/*==========================================================================*\
|                                                                            |
|                                                                            |
| PC-side Bootstrap Loader communication Application                         |
|                                                                            |
|                                                                            |
|----------------------------------------------------------------------------|
| Developed using:      Microsoft Visual C++ Version 6.0                     |
|----------------------------------------------------------------------------|
| Author:               L.Westlund                                           |
*/
#define VERSION 1.07                     
/*                         
| Initial Version:      09-22-08                                             |
| Last Change:          08-18-10                                             |
|----------------------------------------------------------------------------|
| Version history:                                                           |
| 1.00 09/22/08 L.Westlund   Initial version, published as 5xxBSL_Demo       |
| 1.01 01/16/09 L.Westlund   Added Scripting Language and support for ROM BSL|
| 1.02 02/06/09 L.Westlund   Added support for USB / Major interface redesign|
| 1.04 03/09/10 L.Westlund   Removed USB Timeout error                       |
|      04/26/10 L.Westlund   USB/UART support for RX from BSL in USB/5438    |
| 1.05 05/31/10 L.Westlund   Invoke error fixed                              |
| 1.06 08/18/10 L.Westlund   Changed 5438 invoke command for clarity         |
| 1.07 02/19/13 L.Westlund   Added baud rate invoke flexibility              |
|                            Added support for COMs > 9                      |
|----------------------------------------------------------------------------|
| Designed 2009 by Texas Instruments Germany                                 |
\*==========================================================================*/

#include "BSL_Interface.h"
#include "TextFileIO.h"
#include <windows.h>
#include <setupapi.h>

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <devguid.h>

void openFile( unsigned char *fileName );
void setMode();
void errorExit( unsigned char *error );

FILE* scriptFile;
unsigned char *token;
unsigned char line[100];
unsigned int reply;
unsigned char verbose_mode = 0;

/*
#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}
*/

static char portname[30] = "\\\\.\\";

static const char* getPort()
{
	HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i;

	hDevInfo = SetupDiGetClassDevs((LPGUID) &GUID_DEVCLASS_PORTS, 0, 0,DIGCF_PRESENT);

	if (hDevInfo == INVALID_HANDLE_VALUE)
		return NULL;

	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
     for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,
         &DeviceInfoData);i++)
     {
         DWORD DataT;
         //LPTSTR buffer = NULL;
 		char buffer[2048];
        DWORD buffersize =sizeof(buffer);
 
		SetupDiGetDeviceInstanceId(hDevInfo, &DeviceInfoData, (PTSTR)buffer, buffersize, &buffersize);
		if (strstr(buffer, "VID_10C4&PID_EA60") == NULL)
		{
			continue;
		}

 		buffersize =sizeof(buffer);
        SetupDiGetDeviceRegistryProperty(
             hDevInfo,
             &DeviceInfoData,
             SPDRP_FRIENDLYNAME,
             &DataT,
             (PBYTE)buffer,
             buffersize,
             &buffersize);
        printf("User Port: %s\n", buffer);

        // Open device parameters reg key - Added after fact from post on CodeGuru - credit to Peter Wurmsdobler
		HKEY hKey = SetupDiOpenDevRegKey(hDevInfo, &DeviceInfoData, DICS_FLAG_GLOBAL,0,DIREG_DEV,KEY_READ);
		TCHAR szPortName[MAX_PATH];
		if (hKey)
		{
			DWORD dwType = REG_SZ;
			DWORD dwReqSize = sizeof(szPortName);
			// Query for portname
			long lRet = RegQueryValueEx(hKey,"PortName", 0, &dwType, (LPBYTE)&szPortName, &dwReqSize);
			if (lRet == ERROR_SUCCESS)
			{
				strcat(portname, szPortName);
 				SetupDiDestroyDeviceInfoList(hDevInfo);
				return &portname;
			}
		}
    }
//  Cleanup
 	SetupDiDestroyDeviceInfoList(hDevInfo);

 	return NULL;
}



unsigned int main(unsigned int argc, unsigned char* argv[])
{
  SYSTEMTIME lt;
  setvbuf(stdout, NULL, _IONBF, 0);
  printf( "\n------------------------------------------\n");
  printf("BSL Scripting application %3.2f\n", VERSION);
  GetLocalTime(&lt);
  printf( "The local time is: %02d:%02d on %02d.%02d.%04d\n", lt.wHour, lt.wMinute, lt.wDay, lt.wMonth, lt.wYear);
  printf( "------------------------------------------\n");

  if( argc == 1 )
  {
    printf( "Usage: BSL_Scripter watchprog\n\n" );
	return 1;
  }

  openFile( argv[1] );
  while( fgets( line, sizeof line, scriptFile ) )
  {
    fflush(stdout);
	if( line[strlen(line)-1] == '\n' ){
		line[strlen(line)-1] =' ';
	}
	if( line[strlen(line)-2] == '\r' ){
		line[strlen(line)-2] =' ';
	}
    if( (memcmp( line, "//", 2 )==0) || line[0] == '#')
	{
		// skip comments
	}

/*********MODE******************************************************************/
    else if( (memcmp( line, "MODE", 4 )==0) )
	{
	  setMode();
	}
/*********SET_MEMORY_OFFSET*****************************************************/
    else if( memcmp( line, "SET_MEMORY_OFFSET", 17 )==0 )
	{
      unsigned int addr;
      sscanf( line, "%*s %x", &addr);
      printf("Setting offset to %x\t\t\t",addr);
	  if( BSL_setMemOffset(addr) == BSL_ACK)
	  {
        printf("DONE\n");
	  }
      else
	  {
        printf("FAIL\n");
	  }
	}
/*********ERASE_CHECK*********************************************************/
    else if( memcmp( line, "ERASE_CHECK", 11 )==0 )
	{
      unsigned long int length, addr;
      sscanf( line, "%*s %x %x", &addr, &length );
      printf("Checking %4.4u bytes starting at %4.4x for erase:\t\t",length, addr);
      //reply = ROM_eraseCheck(addr, length );
	  if( reply == BSL_ACK )
	  {
        printf("PASSED\n");
	  }
	  else
	  {
        printf("FAIL\n");
	  }
	}
/*********CHANGE_BAUD_RATE******************************************************/
    else if( memcmp( line, "CHANGE_BAUD_RATE", 16 )==0 )
	{
	  unsigned int rate;
      sscanf( line, "%*s %u", &rate);
      printf("Changing Baud Rate to %u\t\t", rate);
	  if ( BSL_changeBaudRate( rate ) == BSL_ACK )
	  {
        printf("DONE\n");
	  } 
      else
	  {
        printf("FAIL\n");
	  }

	} 
/*********TX_BUFFER_SIZE********************************************************/
    else if( memcmp( line, "TX_BUFFER_SIZE", 14 )==0 )
	{
	  signed int bufSize = BSL_TX_BufferSize();
	  printf("Getting Buffer Size:\t\t\t");
	  if( bufSize > 0 )
	  {
      printf("[%u]\n", bufSize);
	  }
	  else
	  {
        printf("FAIL\n");
	  }
      
	}
/*********BSL_VERSION***********************************************************/
    else if( memcmp( line, "TX_BSL_VERSION", 14 )==0 )
	{
      printf("BSL Version:\t\t\t\t%s\n",BSL_TX_BSL_Version_String());
	}
/*********SET_PC****************************************************************/
    else if( memcmp( line, "SET_PC", 6 )==0 )
	{
      unsigned int addr;
      sscanf( line, "%*s %x", &addr);
      printf("Sending Set PC to %x:\t\t\t", addr);
      BSL_LoadPC( addr );
      printf("SENT\n");
	}
/*********CRC*******************************************************************/
    else if( memcmp( line, "CRC", 3 )==0 )
	{
      unsigned int addr, length, value, outputValue;
      if( sscanf( line, "%*s %x %x %x", &addr, &length, &value) == 3)
	  {
        outputValue = 0;
        printf("CRC from %x of %u bytes to %4.4x\t",addr, length,value);
	  }
      else
	  {
        outputValue = 1;
        printf("CRC from %x of %u bytes\t\t",addr, length);
	  }
      if( BSL_CRC_Check( addr, length) == BSL_ACK )
	  {
	    unsigned int crc;
		dataBuffer db = BSL_get_RX_Buffer();
	    crc = db.data[1];
	    crc |= db.data[2]<<8;
        if( outputValue )
		{
          printf("%4.4x\n", crc );
		}
	    else if( crc == value )
		{
          printf("MATCH\n");
		}
	    else if( crc != value )
		{
          printf("NOT A MATCH (%4.4x)\n", crc);
		}
	  }
      else
	  {
        printf("FAIL\n");
	  }
	}
/*********ERASE_MAIN_OR_INFO*********************************************************/
    else if( memcmp( line, "ERASE_MAIN_OR_INFO", 18 )==0 )
	{
      unsigned int addr;
      sscanf( line, "%*s %x", &addr );
      printf("Erasing main/info (addr:%x)\t\t", addr);
	  if ( BSL_eraseMainOrInfo( addr ) == BSL_ACK )
	  {
        printf("DONE\n");
	  } 
      else
	  {
        printf("FAIL\n");
	  }

	}
/*********ERASE_SEGMENT*********************************************************/
    else if( memcmp( line, "ERASE_SEGMENT", 13 )==0 )
	{
      unsigned int addr;
      sscanf( line, "%*s %x", &addr );
      printf("Erasing segment at addr %x\t\t", addr);
	  if ( BSL_eraseSegment( addr ) == BSL_ACK )
	  {
        printf("DONE\n");
	  } 
      else
	  {
        printf("FAIL\n");
	  }

	}
/*********TOGGLE_INFO*********************************************************/
    else if( memcmp( line, "TOGGLE_INFO", 11 )==0 )
	{
      printf("Toggling info flag\t\t");
	  if ( BSL_toggleInfo() == BSL_ACK )
	  {
        printf("DONE\n");
	  } 
      else
	  {
        printf("FAIL\n");
	  }
	}
/*********TX_DATA_BLOCK*********************************************************/
    else if( memcmp( line, "TX_DATA_BLOCK", 13 )==0 )
	{
      unsigned int length, addr;
	  char filename[50];
      sscanf( line, "%*s %x %x %s", &addr, &length, filename );
      printf("Reading %4.4u bytes starting at %4.4x to file %s:\t\t",length, addr, filename);
      reply = BSL_TX_TXT_File( filename, addr, length );
	  if( reply == BSL_ACK )
	  {
        printf("DONE\n");
	  }
	  else
	  {
        printf("FAIL writing data block starting at %4.4x\n",reply);
	  }
	}
/*********RX_DATA_BLOCK_FAST*****************************************************/
    else if( memcmp( line, "RX_DATA_BLOCK_FAST", 17 )==0 )
	{
	  char filename[50];
      sscanf( line, "%*s %s", filename );
      printf("Writing %s to device (fast):\t", filename);
      reply = BSL_RX_TXT_File( filename, 1 );
	  printf("SENT\n");
	}
/*********RX_DATA_BLOCK*********************************************************/
    else if( memcmp( line, "RX_DATA_BLOCK", 13 )==0 )
	{
	  char filename[50];
      sscanf( line, "%*s %s", filename );
      printf("Writing %s to device:\t", filename);
      reply = BSL_RX_TXT_File( filename, 0 );
	  if( reply == BSL_ACK )
	  {
        printf("DONE\n");
	  }
	  else
	  {
        printf("FAIL writing data block starting at %4.4x\n",reply);
	  }
	}
/*********RX_PASSWORD***********************************************************/
    else if( memcmp( line, "RX_PASSWORD", 11 )==0 )
	{
      char filename[50];
      DataBlock pass = default_pass;
	  printf("RX Password:\t\t\t\t");
      if( sscanf( line, "%*s %s", filename ) == 1 )
	  {
        openTI_TextForRead( filename );
        pass = readTI_TextFile(32);
	  }
      reply = BSL_RX_Password(pass);
	  if( reply == BSL_ACK )
	  {
        printf("DONE\n");
	  }
	  else
	  {
        printf("FAIL(%2.2x)\n",reply);
	  }

	}
/*********MASS_ERASE************************************************************/
    else if( memcmp( line, "MASS_ERASE", 10 )==0 )
	{
	  printf("Mass Erase:\t\t\t\t");
      reply = BSL_massErase();
	  if( reply == BSL_ACK )
	  {
        printf("DONE\n");
	  }
	  else
	  {
        printf("FAIL(%2.2x)\n",reply);
	  }
	}
/*********DELAY*****************************************************************/
    else if( memcmp( line, "DELAY", 5 )==0 )
	{
	  int del;
	  sscanf( line, "%*s %d", &del );
      printf( "Waiting %u milliseconds\t\t", del );
	  //delay(del);
	  Sleep(del);  // to do: more universal
	  printf("DONE\n");
      
	}    
/*********VERBOSE***************************************************************/
	else if( memcmp( line, "VERBOSE", 7 )==0 )
	{
	  printf("Verbose mode ");
      verbose_mode ^= 0x01;
	  BSL_SetVerbose( verbose_mode );
	  if( verbose_mode )
	  {
	    printf("on \n");
	  }
	  else
	  {
	    printf("off \n");
	  }
	}
/*********REVERT***************************************************************/
	else if( memcmp( line, "REVERT", 6)==0 )
	{
		char name[50];
		if( sscanf( line, "%*s %s", name ) == 1 )
	  	{
	  		if (strcmp(name, "TEST") == 0)
	  		{
	  			UART_revert(TEST_PIN);
	  		}
	  		else if (strcmp(name, "RST") == 0)
	  		{
	  			UART_revert(RST_PIN);
	  		}

	  		printf("Revert %s\n", name);
	  	}
	  	else
	  	{
	  		printf("ERROR\n");
	  	}
	}
/*********RESET**************************************************************/
	else if ( memcmp( line, "RESET", 5)==0 )
	{
		printf("Reset \t\t\t");
		BSL_Reset();
		printf("Done\n");
	}
/*********DUMP**************************************************************/
	else if ( memcmp( line, "DUMP", 4)==0 )
	{
	  unsigned int rate;
      sscanf( line, "%*s %u", &rate);
      printf("Changing Baud Rate to %u\n", rate);
		changeCommBaudRate(rate);
		unsigned char c;
		while(1)
		{
			c = readByte_s(-1);
			if (c != 0xEE)
				putchar(c);
      fflush(stdout);
		}
		printf("Done\n");
	}
/*********SEND_FLASH************************************************************/
	else if ( memcmp( line, "SEND_FLASH", 10)==0 )
	{
	  char flashname[50];
	  char srcfile[50];
      sscanf( line, "%*s %s %s", flashname, srcfile );
      printf("Writing %s to flash %s:\t", srcfile, flashname);
      reply = MY_RX_TXT_File( flashname , srcfile, 0);
	  if( reply == BSL_ACK )
	  {
        printf("DONE\n");
	  }
	  else
	  {
        printf("FAIL writing data block starting at %4.4x\n",reply);
	  }
	}
/*********SEND_FLASH************************************************************/
	else if ( memcmp( line, "SEND_FLASH_FAST", 15)==0 )
	{
	  char flashname[50];
	  char srcfile[50];
      sscanf( line, "%*s %s %s", flashname, srcfile );
      printf("Writing %s to flash %s:\t", srcfile, flashname);
      reply = MY_RX_TXT_File( flashname , srcfile, 1);
	  if( reply == BSL_ACK )
	  {
        printf("DONE\n");
	  }
	  else
	  {
        printf("FAIL writing data block starting at %4.4x\n",reply);
	  }
	}
/*********SEND_FLASH************************************************************/
	else if ( memcmp( line, "RECV_FLASH", 10)==0 )
	{
	  char flashname[50];
	  char destfile[50];
      sscanf( line, "%*s %s %s", flashname, destfile );
      printf("Read %s from flash to %s:\t", flashname, destfile);
      reply = MY_TX_TXT_File( flashname , destfile);
	  if( reply == BSL_ACK )
	  {
        printf("DONE\n");
	  }
	  else
	  {
        printf("FAIL read data block starting at %4.4x\n",reply);
	  }
	}
/*********REMOVE_FLASH************************************************************/
	else if ( memcmp( line, "REMOVE_FLASH", 12)==0 )
	{
	  char flashname[50];
      sscanf( line, "%*s %s", flashname );
      printf("Remove %s from flash:\t", flashname);
      reply = MY_RemoveFile( flashname);
	  if( reply == BSL_ACK )
	  {
        printf("DONE\n");
	  }
	  else
	  {
        printf("FAIL delete file\n",reply);
	  }
	}
	else
	{
	  printf("Hit Uknown Command\n");
	}
  }
  return 1;
}

void setMode()
{
  token = strtok( line, " ");  // get command
  token = strtok( NULL, " ");  // get family
  printf("Initializing, Mode: ");
  if( memcmp( token, "543x_family", 11) == 0 )
  {
    BSL_setFamily(FAMILY_5438);
	printf("5438  ");
  }
  else if( memcmp( token, "ROM", 3) == 0 )
  {
    BSL_setFamily(FAMILY_ROM);
	printf("ROM   ");
  }
  else if( memcmp( token, "5xx", 3) == 0 )
  {
    BSL_setFamily(FAMILY_FLASH);
	printf("5xx ");
  } 
  else if( memcmp( token, "6xx", 3) == 0 )
  {
    BSL_setFamily(FAMILY_FLASH);
	printf("6xx ");
  } 
  else
  {
    errorExit("Error parsing MODE");
  }
  token = strtok( NULL, " ");  // get COM
  if( memcmp( token, "COM", 3) == 0 )
  {

	char pre_Com[] = "\\\\.\\";
	char* final_com;
	
	if( strlen(token) > 4 )
	{
		final_com = malloc(strlen(token)+strlen(pre_Com));
		strcpy( final_com, pre_Com );
		strcat( final_com, token);
	}
	else
	{
		if (strcmp(token, "COM?") == 0)
			final_com = getPort();
		else
			final_com = &token[0];
	}
    
    BSL_setCom( COM_UART );
    printf("COM: %s", final_com);

	if (BSL_initialize_BSL( final_com ))
	{
		printf("\tERROR\n");
		errorExit("Cannot open COM port.");
	}
	else
	{
		printf("\tDONE");
	}
  }
  else if( memcmp( token, "USB", 3) == 0 )
  {
    BSL_setCom( COM_USB );
    printf("USB: %s", token);
	BSL_initialize_BSL("");
  }
  else
  {
    errorExit("Error parsing COMM");
  }
  printf("\n");

}

void openFile( unsigned char *fileName )
{
  scriptFile = fopen( fileName, "rb" );
}

void errorExit( unsigned char *error )
{
  printf("-------------ERROR-------------\n%s\n-------------ERROR-------------\n",error);
  exit(-1);
}