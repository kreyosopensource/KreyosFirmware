void Flash_crcInit( unsigned short int init );
void Flash_crcInput( unsigned char data);
unsigned char Flash_getHighByte();
unsigned char Flash_getLowByte();

void ROM_crc_init();
void ROM_crcByte( unsigned char byte );
unsigned char ROM_getLowByte();
unsigned char ROM_getHighByte();