/**
 \file HidDevice.h

 \brief       

 \par         Project:
              MSP430 USB HID Interface 

 \par         Developed using:
              MS Visual C++ 6.0

 \author      Rostyslav Stolyar\n
              (c) 2009 by Texas Instruments

 \version     1.1.0.0

 \par         Supported API calls:
			  - HID_Init()
			  - HID_Open()
			  - HID_Close()
			  - HID_GetNumOfDevices()
			  - HID_WriteFile()
			  - HID_ReadFile()

 \par		  Version History:
              - Version 0.0.1.0 - 20 February 2007
			    Internal version, no release

              - Version 0.9.0.0 - 20 March 2009
                Adapted for using with MSP430F5529 HID stack
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __HidDevice_h_
#define __HidDevice_h_

/// HID Device return codes
/// HID action/transfer was successful
#define HID_DEVICE_SUCCESS				0x00
/// HID device was not found
#define HID_DEVICE_NOT_FOUND			0x01
/// HID device is not opened
#define HID_DEVICE_NOT_OPENED			0x02
/// HID device is allready opened
#define HID_DEVICE_ALREADY_OPENED		0x03
/// Timeout occurs during transfer
#define	HID_DEVICE_TRANSFER_TIMEOUT		0x04
/// HID transfer failed
#define HID_DEVICE_TRANSFER_FAILED		0x05
/// Invalid handle
#define HID_DEVICE_HANDLE_ERROR			0x06
/// Unknown error
#define HID_DEVICE_UNKNOWN_ERROR		0xFF

    
#define ERR_BYTE			"Data transmision failure"
#define ERR_HID_WRITE_1		"Hid Device not found."
#define ERR_HID_WRITE_2		"Hid Device not opened."
#define ERR_HID_WRITE_3		"Hid Device is already opened."
#define ERR_HID_WRITE_4		"Hid Device transfer timeout."
#define ERR_HID_WRITE_5		"Hid Device transfer failed."
#define ERR_HID_WRITE_6		"Hid Device: Invalid Handle."
#define ERR_HID_WRITE_FF	"Hid Device unknown error."
#define ERR_HID_READ		"Error Hid Read."
#define ERR_HID_MSP			"Wrong Package received."



/**
 \brief  Device information structure.
*/
struct strHidDevice{

	/// Handle for hid device
	HANDLE hndHidDevice;	
	/// Indicator if device is opened
	BOOL bDeviceOpen;				

	/// Timeout for GetReport requests
	UINT uGetReportTimeout;			
	/// Timeout for SetReport requests
	UINT uSetReportTimeout;			

    OVERLAPPED oRead;
    OVERLAPPED oWrite;

	/// Maximum length of InReport's
	WORD wInReportBufferLength;		
	/// Maximum length of OutReport's
	WORD wOutReportBufferLength;	
	/// Maximum length of FeatureReport's
	WORD wFeatReportBufferLength;		

	/// InBuffer contains data, if InReport provides more data then the application actual need
	BYTE inBuffer[256];					
	/// Number of current used bytes in inBuffer
	WORD inBufferUsed;	
	
};

//	struct strHidDevice m_HidDevice;

/**
\fn      VOID HID_Init(struct strHidDevice* pstrHidDevice);

\brief   Init structure with default values.
         - It is important to call HID_Init before calling HID_Open\n
		   to avoid unpredicted behavoir

\param   pstrHidDevice: Structure which contains important data of an HID device

*/
VOID HID_Init(struct strHidDevice* pstrHidDevice);


/**
\fn      BYTE HID_Open(struct strHidDevice* pstrHidDevice, WORD vid, WORD pid, DWORD deviceIndex)

\brief   Open a Hid Device.

\param   pstrHidDevice: Structure which contains important data of an HID device

\param   vid: Vendor-Id of the device

\param   pid: Product-Id of the device 

\param   deviceIndex: Index of the device.If only one HID device is connected, deviceIndex is 0.
					  - Starts with zero
					  - Maximum value is (HID_GetNumOfDevices()-1)

\return  HID_DEVICE_SUCCESS:.
\n       HID_DEVICE_NOT_FOUND:.
\n       HID_DEVICE_ALREADY_OPENED:.
*/
BYTE HID_Open(struct strHidDevice* pstrHidDevice, WORD VID, WORD PID, DWORD deviceIndex);


/**
\fn      BYTE HID_Close(struct strHidDevice* pstrHidDevice)

\brief   Close a Hid Device.

\param   pstrHidDevice: Structure which contains important data of an HID device

\return  HID_DEVICE_SUCCESS:.
\n       HID_DEVICE_NOT_OPENED:.
\n       HID_DEVICE_HANDLE_ERROR:.
*/
BYTE HID_Close(struct strHidDevice* pstrHidDevice);


/**
\fn      BYTE HID_GetNumOfDevices(WORD vid, WORD pid);

\brief   Returns the number of connected devices with the specific vid and pid.

\param   VID: Vendor-Id of the device 

\param   PID: Product-Id of the device

\return  Number of connected devices with the specific vid and pid.

*/
DWORD HID_GetNumOfDevices(WORD vid, WORD pid);

/**
\fn      BYTE HID_GetSerialNumber(struct strHidDevice* pstrHidDevice, BYTE * Buffer, ULONG BufferLength);

\brief   Returns the serial number of a device in UNICODE format. 

\param   pstrHidDevice: Structure which contains important data of an HID device

\param   Buffer: Buffer for serial number (minimum size of 34 bytes)

\param   BufferLength: Length of given buffer

\return  HID_DEVICE_SUCCESS:.
\n       HID_DEVICE_HANDLE_ERROR:.

*/
BYTE HID_GetSerialNumber(struct strHidDevice* pstrHidDevice, BYTE * Buffer, ULONG BufferLength);

/**
\fn      BYTE HID_GetVersionNumber(struct strHidDevice* pstrHidDevice, USHORT * lpVersionNumber);

\brief   Returns the version number of a device.

\param   pstrHidDevice: Structure which contains important data of an HID device

\param   VersionNumber: Pointer to USHORT variable.

\return  HID_DEVICE_SUCCESS:.
\n       HID_DEVICE_HANDLE_ERROR:.

*/
BYTE HID_GetVersionNumber(struct strHidDevice* pstrHidDevice, USHORT * VersionNumber);

/**
\fn      BYTE HID_WriteFile(struct strHidDevice* pstrHidDevice, BYTE* buffer, DWORD bufferSize);

\brief   Writes a data stream to the given hid device. Needed report id's will be generated automatically

\param   pstrHidDevice: Structure which contains important data of an HID device

\param   buffer: Buffer which will be send

\param   bufferSize: Number of bytes to send

\return  HID_DEVICE_SUCCESS:.
\n       HID_DEVICE_NOT_OPENED:.
\n       HID_DEVICE_TRANSFER_TIMEOUT:.
\n       HID_DEVICE_TRANSFER_FAILED:.
*/
BYTE HID_WriteFile(struct strHidDevice* pstrHidDevice, BYTE* buffer, DWORD bufferSize);


/**
\fn      BYTE HID_WriteFile(struct strHidDevice* pstrHidDevice, BYTE* buffer, DWORD bufferSize);

\brief   Reads a data stream from the given hid device. Prefixed report id's will be skipped

\param   pstrHidDevice: Structure which contains important data of an HID device

\param   buffer: Pointer to buffer in which will be written

\param   bufferSize: Number of bytes to read

\param   bytesReturned: Number of actual read bytes 

\return  HID_DEVICE_SUCCESS:.
\n       HID_DEVICE_NOT_OPENED:.
\n       HID_DEVICE_TRANSFER_TIMEOUT:.
\n       HID_DEVICE_TRANSFER_FAILED:.
*/
//BYTE HID_ReadFile(struct strHidDevice* pstrHidDevice, BYTE* buffer, DWORD bufferSize, DWORD* bytesReturned);
BYTE HID_ReadFile(struct strHidDevice* pstrHidDevice, BYTE* buffer);


/**
\fn    BOOL HID_FlushBuffer(struct strHidDevice* pstrHidDevice);

\brief   Flush USB buffer for the given device

\param   pstrHidDevice: Structure which contains important data of an HID device.

\return  HID_DEVICE_SUCCESS:.
\n       HID_DEVICE_HANDLE_ERROR:.
\n       HID_DEVICE_UNKNOWN_ERROR:.
*/
BYTE HID_FlushBuffer(struct strHidDevice* pstrHidDevice);

#endif 
#ifdef __cplusplus
}
#endif