#ifndef _I2C_H_
#define _I2C_H_
extern void I2C_Init();
extern int I2C_readbytes(unsigned char reg, unsigned char *data, uint8_t len);
extern int I2C_writebytes(unsigned char reg, const unsigned char *data, uint8_t len);
extern int I2C_write(unsigned char reg, unsigned char data);

extern void I2C_addr(unsigned char address);
extern void I2C_done(); // move I2c module to power-save mode

#define I2C_read8(reg, data) I2C_readbytes(reg, data, 1)
#define I2C_read16(reg, data) I2C_readbytes(reg, (unsigned char*)data, 2); \
                              __swap_bytes(*data);


#endif