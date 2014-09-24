#include "BSL_Device_File.h"
#include "BSL430_API.h"
#include "BSL430_Command_Definitions.h"

/*******************************************************************************
*  Change Log:
*  2.2.09  LCW   removed RAM erase
*             Changed mass erase and info segments
*  2.2.09  LCW   worked on RAM Loader function
*             Changed Password to XOR
*             Changed Password addr to 0xFFE0
*  --------------------------------------------------------------------------------
*  Version 3 work begins
*  07.08.09 LCW  Removed info erase
*  11.08.09 LCW  bugfix in CRC algorithm
*  --------------------------------------------------------------------------------
*  Version 4 work begins
*  21.12.09 LCW  Removed VPE check
*  15.03.10 LCW  Misc comment cleanup for source release
*  --------------------------------------------------------------------------------
*  Version 4.1 (no output change, source/comment modified)
*  15.04.10 LCW Changed Version defines to remove warning
*******************************************************************************/
#ifdef RAM_BASED_BSL
#    ifdef RAM_WRITE_ONLY_BSL
#        error Can NOT have RAM write and RAM based BSL
#    endif
#    define DEFAULT_STATE (UNLOCKED)
#else
#    define DEFAULT_STATE (LOCKED)
#endif
#include "spiflash.h"

volatile unsigned int LockedStatus;
unsigned int FwRamKey;

#define API_VERSION (0x04)
#ifdef RAM_WRITE_ONLY_BSL
#    define API_V (API_VERSION + RAM_WRITE_ONLY_BSL)
#else
#    define API_V API_VERSION
#endif


/*******************************************************************************
* *Function:    BSL430_API_init
* *Description: sets the key for writing to flash,  sets device state
*******************************************************************************/

void BSL430_API_init()
{
    LockedStatus = DEFAULT_STATE;
    FwRamKey = FWKEY;
}

/*******************************************************************************
* *Function:    BSL430_lock_BSL
* *Description: Locks the BSL
* *Returns:
*             SUCCESSFUL_OPERATION  BSL Locked
*******************************************************************************/

char BSL430_lock_BSL()
{
    
    return SUCCESSFUL_OPERATION;
}

/*******************************************************************************
* *Function:    BSL430_unlock_BSL
* *Description: Causes the BSL to compare the data buffer against the BSL password
*             BSL state will be UNLOCKED if successful
****************Parameters:
*             char* data            A pointer to an array containing the password
****************Returns:
*             SUCCESSFUL_OPERATION  All data placed into data array successfully
*             BSL_PASSWORD_ERROR    Correct Password was not given
*******************************************************************************/

char BSL430_unlock_BSL(char* data)
{
        return BSL_PASSWORD_ERROR;    
}

/*******************************************************************************
* *Function:    BSL430_toggleInfoLock
* *Description: Toggles the LOCKA bit for writing/erasing info A segment
* *Returns:
*             SUCCESSFUL_OPERATION  Info A is now open for writing or erasing.
*             BSL_LOCKED            Correct Password has not yet been given
*******************************************************************************/

char BSL430_toggleInfoLock()
{
    char exceptions =  BSL_LOCKED;
    return exceptions;
}

/*******************************************************************************
* *Function:    BSL430_openMemory
* *Description: Unlocks the Flash for writing
* *Returns:
*             SUCCESSFUL_OPERATION  Flash is now open for writing.
*             BSL_LOCKED            Correct Password has not yet been given
*******************************************************************************/

char BSL430_openMemory()
{
    char exceptions = SUCCESSFUL_OPERATION;

    return exceptions;
}

/*******************************************************************************
* *Function:    BSL430_closeMemory
* *Description: Locks the Flash against writing
* *Returns:
*             SUCCESSFUL_OPERATION  Flash is now locked.
*******************************************************************************/

char BSL430_closeMemory(void)
{

    return SUCCESSFUL_OPERATION;
}

/*******************************************************************************
* *Function:    BSL430_readMemory
* *Description: Reads an array of bytes from memory into a supplied array
* *Parameters:
*             unsigned long addr    The address from which the read should begin
*             char length           The amount of bytes to read
*             char* data            The array into which the data will be saved
****************Returns:
*             SUCCESSFUL_OPERATION  All Data placed into data array successfully
*             BSL_LOCKED            Correct Password has not yet been given
*******************************************************************************/

char BSL430_readMemory(unsigned long addr, unsigned int length, char* data)
{
    char exceptions = SUCCESSFUL_OPERATION;

    SPI_FLASH_BufferRead(data, addr, length);

    return exceptions;
}

/*******************************************************************************
* *Function:    BSL430_crcCheck
* *Description: return a CRC check on the memory specified
* *Parameters:
*           unsigned long addr    The address from which to start the check
*           int length            The length of the data area to check
*           int* return           variable in which to put the return value
****************Returns:
*           SUCCESSFUL_OPERATION  CRC check done correctly
*           BSL_LOCKED            Correct Password has not yet been given
*******************************************************************************/

int BSL430_crcCheck(unsigned long addr, unsigned int length, int* result)
{

    return SUCCESSFUL_OPERATION;
}

/*******************************************************************************
* *Function:    BSL430_callAddress
* *Description: Loads the Program Counter with the supplied address
* *Parameters:
*           unsigned long addr    The address to which the function call should go
****************Returns:
*           SUCCESSFUL_OPERATION  Called location has returned
*           BSL_LOCKED            Correct Password has not yet been given
****************Note:
*           If successful, this function does not return.
*******************************************************************************/

char BSL430_callAddress(unsigned long addr)
{
    char exceptions = BSL_LOCKED;

    return exceptions;
}

/*******************************************************************************
* *Function:    BSL430_writeMemory
* *Description: Writes a byte array starting at a given address.
*             Note: the function will write in word mode if possible
*             (when start address is even)
****************Parameters:
*           unsigned long startAddr        The address to which the write should begin
*           int size                       The number of bytes to write
*           char* data                     The array of bytes to write (must be even aligned)
****************Returns:
*           SUCCESSFUL_OPERATION           Bytes written successfully
*           MEMORY_WRITE_CHECK_FAILED      A byte in data location post-write does not match data
***************parameter
*                                          Note: write stops immediatly after a byte check fails
*           BSL_LOCKED                     Correct Password has not yet been given
*           VOLTAGE_CHANGE_DURING_PROGRAM  Voltage changed during write (of a single byte/word)
*******************************************************************************/

char BSL430_writeMemory(unsigned long startAddr, unsigned int size,  char* data)
{
    unsigned long i;
    char exceptions = SUCCESSFUL_OPERATION;

    SPI_FLASH_BufferWrite(data, startAddr, size);
    
    return exceptions;
}

/*******************************************************************************
* *Function:    BSL430_writeByte
* *Description: Writes a byte at a given address
* *Parameters:
*           unsigned long addr             The address to which the byte should be written
*           char data                      The byte to write
****************Returns:
*           SUCCESSFUL_OPERATION           Byte written successfully
*           MEMORY_WRITE_CHECK_FAILED      Byte in data location post-write does not match data
***************parameter
*           VOLTAGE_CHANGE_DURING_PROGRAM  Voltage changed during write
*           BSL_LOCKED                     Correct Password has not yet been given
*******************************************************************************/

char BSL430_writeByte(unsigned long addr, char data)
{
    char exceptions = BSL_LOCKED;

    return exceptions;
}

/*******************************************************************************
* *Function:    BSL430_writeWord
* *Description: Writes a word at a given address
* *Parameters:
*           unsigned long addr             The address to which the word should be written
*           int data                       The byte to write
****************Returns:
*           SUCCESSFUL_OPERATION           Word written successfully
*           MEMORY_WRITE_CHECK_FAILED      Word in data location post-write does not match data
***************parameter
*           VOLTAGE_CHANGE_DURING_PROGRAM  Voltage changed during write
*           BSL_LOCKED                     Correct Password has not yet been given
*******************************************************************************/

char BSL430_writeWord(unsigned long addr, int data)
{
      char exceptions = BSL_LOCKED;

    return exceptions;
}

/*******************************************************************************
* *Function:    BSL430_eraseLocation
* *Description: Erases a memory segment which contains a given address
* *Parameters:
*           char block_erase               currently unused 1: erase block 0: erase segment
*           unsigned long addr             An address which is within the segment to be erased
****************Returns:
*           SUCCESSFUL_OPERATION           Segment erased
*           BSL_LOCKED                     Correct Password has not yet been given
*******************************************************************************/

char BSL430_eraseLocation(char block_erase, unsigned long addr)
{
    char exceptions;

   
    return exceptions;
}

/*******************************************************************************
* *Function:    BSL430_massErase
* *Description: Mass erases the entire MSP430 device
* *Returns:
*           SUCCESSFUL_OPERATION           Flash erased
*           BSL_LOCKED                     Correct Password has not yet been given
*******************************************************************************/

char BSL430_massErase()
{
    char exceptions = SUCCESSFUL_OPERATION;

    SPI_FLASH_BulkErase();
    
    return exceptions;
}

