void USB_initialize_BSL(unsigned char* comPort);
void USB_setVerbose( unsigned int verb );
void USB_sendData( unsigned char* buf, unsigned int num_bytes );
int USB_receiveData( unsigned char* buf );
void USB_delay(int time);

#define TIMEOUT_ERROR    0xEE
#define MAX_USB_FRAME_SIZE 63

unsigned char USB_dataBuffer[MAX_USB_FRAME_SIZE-1];


//#define DEBUG_VERBOSE
//#define DEBUG_ERRORS
