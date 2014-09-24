#include "TextFileIO.h"
#include "CRC.h"
#include <stdint.h>

#define SIGNATURE 0xFACE0001

/*
 * When update this structure, also update platform/iwatch/progam.c, check watch/upgrade.c
 */
#pragma pack(1)
struct _header
{
  uint32_t signature;
  uint32_t length;
  uint8_t crch;
  uint8_t crcl;
}header;

void hexdump(void* ptr, int length)
{
  unsigned char *p = (unsigned char*)ptr;
  for(int i = 0; i < length; i++)
  {
    printf("0x%02x,", *p);
    p++;
  }

  printf("\n");
}

void crc(void* ptr, int length)
{
  unsigned char *p = (unsigned char*)ptr;
  for(int i = 0; i < length; i++)
  {
    ROM_crcByte(*p);
    p++;
  }

  //hexdump(ptr, length);
}


int main(int argc, char* argv[])
{
  header.signature = SIGNATURE;
  header.length = 0;

  if (argc < 2)
  {
    printf("Usage\nconvert input.txt output.bin\n");
    return -1;
  }

  ROM_crc_init();

  int ret = openTI_TextForRead(argv[1]);
  if (ret != OPERATION_SUCCESSFUL)
  {
    printf("read input file failed.\n");
    return -1;
  }

  FILE *fp = fopen(argv[2], "wb");
  fwrite(&header, sizeof(header), 1, fp);
  hexdump(&header, sizeof(header));
  for(int i = 0; i < 20; i++)
  {
    struct datablock *d = &blocks[i];
    if (d->currentAddr != -1 && d->currentAddr != 0x1800) // skip 1800
    {
      printf("address: 0x%x length=%d\n", d->currentAddr, d->size);
      fwrite(&d->currentAddr, 4, 1, fp);
      header.length+=4;
      crc(&d->currentAddr, 4);
      fwrite(&d->size, 4, 1, fp);
      header.length+=4;
      crc(&d->size, 4);
      fwrite(d->data, d->size, 1, fp);
      header.length+=d->size;
      crc(d->data, d->size);
    }
  }

  header.crch = ROM_getHighByte();
  header.crcl = ROM_getLowByte();

  // write a copy of header to the end of file
  fwrite(&header, sizeof(header), 1, fp);

  // seek to start
  fseek(fp, 0, SEEK_SET);
  
  fwrite(&header, sizeof(header), 1, fp);
  fclose(fp);

  hexdump(&header, sizeof(header));
  printf("%s is created.\n", argv[2]);
  printf("Length: %d\n", header.length);
  printf("CRC H is %d\nCRC L is %d\n", header.crch, header.crcl);
}
