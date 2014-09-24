/*==========================================================================*\
|                                                                            |
|                                                                            |
| PC-side Bootstrap Loader communication Application                         |
|                                                                            |
| See main.c for full version and legal information                          |
|                                                                            |
\*==========================================================================*/

#include "TextFileIO.h"

FILE* file;
unsigned int currentAddr = NO_DATA_READ;

struct datablock blocks[20];
int current_block;

/*******************************************************************************
*Function:    openTI_TextForRead
*Description: Opens a TXT file for reading
*Parameters: 
*             char *filename        The string containing the file name
*Returns:
*             SUCCESSFUL_OPERATION  The file is open for reading
*             ERROR_OPENING_FILE    Some error occured during file open
*******************************************************************************/
int openTI_TextForRead( char *filename )
{
   currentAddr = NO_DATA_READ;
   if( (file = fopen( filename, "rb" )) == 0 )
   {
      return ERROR_OPENING_FILE;
   }
 
  int bytesRead = 0;
  char string[50];
  int block = -1;
   while(1)
   {
      int status = fgets( string, sizeof string, file );
      if (status == 0)
         break;
      if (string[0] == '@')
      {
        block++;
        sscanf(&string[1], "%x\n", &blocks[block].currentAddr);
        blocks[block].offset = 0;
        blocks[block].size = 0;
        blocks[block].data = malloc(256 * 1024);
        bytesRead = 0;
        continue;
      }
      else if (string[0] == 'q' || string[0] == 'Q')
      {
        for(int i = block + 1; i < 20; i++)
          blocks[i].currentAddr = -1;
        break;
      }
      else{
        int stringLength = strlen( string );
        // this is data
        for( int stringPosition = 0; stringPosition < (stringLength-3); stringPosition+=3 )
        {
          sscanf( &string[stringPosition], "%2x", &blocks[block].data[bytesRead]);
          bytesRead++;
        }
        blocks[block].size = bytesRead;
      }
   }

   return OPERATION_SUCCESSFUL;
}

/*******************************************************************************
*Function:    openTI_TextForWrite
*Description: Opens a TXT file for writing with append
*Parameters: 
*             char *filename        The string containing the file name
*Returns:
*             SUCCESSFUL_OPERATION  The file is open for reading
*             ERROR_OPENING_FILE    Some error occured during file open
*******************************************************************************/
int openTI_TextForWrite( char *filename )
{
   currentAddr = NO_DATA_READ;
   if( (file = fopen( filename, "a+" )) == 0 )
   {
      return ERROR_OPENING_FILE;
   }
   else
   {
      return OPERATION_SUCCESSFUL;
   }
}

/*******************************************************************************
*Function:    endTI_TextWrite
*Description: Writes the final 'q' to a TI TEXT file and closes it
*Parameters: 
*             none
*Returns:
*             none
*******************************************************************************/
void endTI_TextWrite()
{
  fprintf(file,"q\n");
  closeTI_Text();
}

/*******************************************************************************
*Function:    closeTI_Text
*Description: closes access to a TI TEXT file
*Parameters: 
*             none
*Returns:
*             none
*******************************************************************************/
void closeTI_Text()
{
   fclose( file );
}

/*******************************************************************************
*Function:    moreDataToRead
*Description: checks whether an end-of-file was hit during read
*Parameters: 
*             none
*Returns:
*             1                     if an EOF has not been hit
*             0                     if an EOF has been hit
*******************************************************************************/
int moreDataToRead()
{
  return !(blocks[current_block].currentAddr == -1);
}

/*******************************************************************************
*Function:    writeTI_TextFile
*Description: writes a block of data in TI TEXT format to the current file
*Parameters: 
*             DataBlock data        The DataBlock structure to write
*Returns:
*             none
*******************************************************************************/
/*
void writeTI_TextFile( DataBlock data )
{

  unsigned int bytesWritten = 0;
  if( (currentAddr == NO_DATA_READ) || (currentAddr != data.startAddr) )
  {
    fprintf(file, "@%05X\n", data.startAddr);
	currentAddr = data.startAddr;
  }
  for( bytesWritten = 0; bytesWritten < data.numberOfBytes; bytesWritten++,currentAddr++ )
  {
    fprintf(file, "%02X", data.data[bytesWritten]);
	if( ((bytesWritten+1)%16 == 0) || (bytesWritten+1 == data.numberOfBytes) )
	{
      fprintf(file, "\n");
	} // if
	else
	{
      fprintf(file, " ");
	}
  }
}
*/
/*******************************************************************************
*Function:    writeTI_TextFile
*Description: writes a block of data in TI TEXT format to the current file
*Parameters: 
*             int length                 The address of bytes
*             unsigned char *data        The array to write
*             int length                 The amount of bytes
*Returns:
*             none
*******************************************************************************/
void writeTI_TextFile( int addr, unsigned char *data, int length )
{
  int i;
   fprintf(file, "@%05X", addr);
  for( i = 0; i < length; i++)
  {
    if( i%16 == 0 )
	{
      fprintf(file, "\n");
	}
	else
	{
      fprintf(file, " ");
	}

    fprintf(file, "%02X", data[i]);
	/*
	if( i == 0 )
	{
      fprintf(file, " ");
    }
	else if( (i%16 == 0) || (i == length-1) || (i == 15))
	{
      fprintf(file, "\n");
	}
	else
	{
      fprintf(file, " ");
	}
	*/
  }
  fprintf(file, "\n");

}

int open_BinaryForRead( char *filename )
{
  for(int i = 0; i < 20; i++)
   blocks[i].currentAddr = -1;

  FILE *fp = fopen(filename, "rb");

  blocks[0].currentAddr = 0;
  blocks[0].offset = 0;
  fseek (fp, 0, SEEK_END);
  blocks[0].size = ftell(fp);
  fseek (fp, 0, SEEK_SET);
  blocks[0].data = malloc(blocks[0].size);

  fread(blocks[0].data, blocks[0].size, 1, fp);
  fclose(fp);

  return OPERATION_SUCCESSFUL;
}