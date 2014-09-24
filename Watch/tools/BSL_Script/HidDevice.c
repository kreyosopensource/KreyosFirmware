/**
 \file HidDevice.c

 \brief

 \par         Project:
              MSP430 USB HID Interface

 \par         Developed using:
              MS Visual C++ 6.0

 \author      Rostyslav Stolyar\n
              (c) 2009 by Texas Instruments

 \version     1.0.1.0

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

              - Version 1.0.1.0 - 24 April 2009
                Adapted for using with multiple USB devices
*/

#include <windows.h>
#include "hiddevice.h"
#include "hidsdi.h"
#include "setupapi.h"

#include <stdio.h>


BYTE SetFeature(struct strHidDevice* pstrHidDevice,BYTE* buffer);

//-------------------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------------------
DWORD err = 0; // for debug



VOID HID_Init(struct strHidDevice* pstrHidDevice)
{
	pstrHidDevice->hndHidDevice = NULL;
	pstrHidDevice->bDeviceOpen = FALSE;
	pstrHidDevice->inBufferUsed = 0;
	pstrHidDevice->uGetReportTimeout = 600;
	pstrHidDevice->uSetReportTimeout = 500;
	pstrHidDevice->wFeatReportBufferLength = 0;
	pstrHidDevice->wInReportBufferLength = 0;
	pstrHidDevice->wOutReportBufferLength = 0;

    //clean overlapped structures
    memset(&pstrHidDevice->oRead, 0, sizeof(pstrHidDevice->oRead));
    memset(&pstrHidDevice->oWrite, 0, sizeof(pstrHidDevice->oWrite));
}


//-------------------------------------------------------------------------------------------------------------

BOOL GetHidDevicePath(WORD VID, WORD PID, DWORD index, char* devicePath)
{
	// This function will obtain the device path of an HID device by it's index
	// TRUE is returned if the index is found, and contains a valid device path
	// FALSE is returned if the index is not found

	BOOL status = FALSE;

	GUID		hidGuid;
	HDEVINFO	hHidDeviceInfo = NULL;

	// Obtain the HID GUID
	HidD_GetHidGuid(&hidGuid);

	// Use the HID GUID to get a handle to a list of all HID devices connected
	hHidDeviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	// If the device info was NULL, return an error code
	if (hHidDeviceInfo != NULL)
	{
		SP_DEVICE_INTERFACE_DATA hidDeviceInterfaceData;
		hidDeviceInterfaceData.cbSize = sizeof(hidDeviceInterfaceData);

		// Check that the index is less than the max number of USB devices connectable
		if (index < 128)
		{
			// Query the device using the index to get the interface data
			BOOL hidResult;
			hidResult = SetupDiEnumDeviceInterfaces(hHidDeviceInfo, 0, &hidGuid, index, &hidDeviceInterfaceData);

			// If a successful query was made, use it to get the detailed data of the device
			if (hidResult)
			{
				BOOL detailResult;
				DWORD length, required;
				PSP_DEVICE_INTERFACE_DETAIL_DATA hidDeviceInterfaceDetailData;

				// Obtain the length of the detailed data structure, then allocate space and retrieve it
				SetupDiGetDeviceInterfaceDetail(hHidDeviceInfo, &hidDeviceInterfaceData, NULL, 0, &length, NULL);
				hidDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(length);

				if (hidDeviceInterfaceDetailData)
				{
					char strVidPid[40];
					char strTmp[40];

					hidDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
					detailResult = SetupDiGetDeviceInterfaceDetail(hHidDeviceInfo, &hidDeviceInterfaceData, hidDeviceInterfaceDetailData, length, &required, NULL);

					// Create our vid-pid string like "vid_0451&pid_f401"
					strcpy(strVidPid, "vid_");
					sprintf(strTmp, "%04x", VID);
					strcat(strVidPid, strTmp);

					strcat(strVidPid, "&pid_");
					sprintf(strTmp, "%04x", PID);
					strcat(strVidPid, strTmp);


					// If getting the device detail provides a valid result, then copy it's device
					// path into the device path argument pointer and set status to TRUE
					if (detailResult && ( strstr(hidDeviceInterfaceDetailData->DevicePath, strVidPid) ))
					{
						strcpy(devicePath, hidDeviceInterfaceDetailData->DevicePath);
						status = TRUE;
					}

					// Deallocate space for the detailed data structure
					free(hidDeviceInterfaceDetailData);
				}
			}
        } // index <128
	}

	return status;
}

//-------------------------------------------------------------------------------------------------------------

HANDLE OpenDeviceByVidPid(char* devicePath, WORD vid, WORD pid, DWORD * err)
{
	// This function attempts to open a device using a specific VID and PID. If it succeeds
	// then a handle is returned. If it fails, then INVALID_HANDLE_VALUE is returned
	// Open the device

	HANDLE hHidDeviceHandle = CreateFile(devicePath, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

	*err = GetLastError();

	// Check that the device opened successfully
	if (hHidDeviceHandle != INVALID_HANDLE_VALUE)
	{
		HIDD_ATTRIBUTES	hidDeviceAttributes;

		// Get the attributes of the HID device
		if (HidD_GetAttributes(hHidDeviceHandle, &hidDeviceAttributes))
		{
			// Close the handle if either the VID or the PID don't match
			if ((hidDeviceAttributes.VendorID != vid) || (hidDeviceAttributes.ProductID != pid))
			{
				CloseHandle(hHidDeviceHandle);
				hHidDeviceHandle = INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			CloseHandle(hHidDeviceHandle);
			hHidDeviceHandle = INVALID_HANDLE_VALUE;
		}
	}

	return hHidDeviceHandle;
}


//-------------------------------------------------------------------------------------------------------------

DWORD HID_GetNumOfDevices(WORD vid, WORD pid)
{
	// This function will return the number of devices connected with a specified VID and
	// PID, if no devices are connected, it will return a 0

	DWORD deviceNum = 0;
	char devicePath[MAX_PATH];
	DWORD i = 0;
	DWORD error = 0;

	// Loop through to check all of the HID Device paths available
    for (i=0; i< 128; i++)
    {
        if (GetHidDevicePath(vid, pid, i, devicePath))
        {
            // Increment the number of devices found then close the handle
	        deviceNum++;
        }
    }

	return deviceNum;
}

//-------------------------------------------------------------------------------------------------------------

BYTE HID_Close(struct strHidDevice* pstrHidDevice)
{
	BYTE status = HID_DEVICE_SUCCESS;
	BYTE stat2 = 0;

	// Check to see if the device is opened, otherwise return an error code
	if (pstrHidDevice->bDeviceOpen == TRUE)
	{
		// Check that we have valid handle values, otherwise return an error code
		if ((pstrHidDevice->hndHidDevice != INVALID_HANDLE_VALUE) && (pstrHidDevice->hndHidDevice != NULL))
		{
			stat2 = CloseHandle(pstrHidDevice->hndHidDevice);
			stat2 = CloseHandle(pstrHidDevice->oRead.hEvent);
			stat2 = CloseHandle(pstrHidDevice->oWrite.hEvent);
			pstrHidDevice->hndHidDevice = INVALID_HANDLE_VALUE;
			pstrHidDevice->bDeviceOpen = FALSE;
		}
		else
		{
			status = HID_DEVICE_HANDLE_ERROR;
		}

		// Reset the device data
	    //	ResetDeviceData();
	}
	else
	{
		status = HID_DEVICE_NOT_OPENED;
	}
	return status;
}

//-------------------------------------------------------------------------------------------------------------

BYTE HID_Open(struct strHidDevice* pstrHidDevice, WORD VID, WORD PID, DWORD deviceIndex)
{
	// This function will open a device by it's index and VID and PID. In addition, the number
	// of inp

	BYTE status = HID_DEVICE_SUCCESS;
	char devicePath[MAX_PATH];
	DWORD i = 0;
	DWORD deviceNum = 0;
	BOOL found = FALSE;
	DWORD error = 0;

	// Ensure that the we don't already have an open device
	if (pstrHidDevice->bDeviceOpen)
	{
		status = HID_DEVICE_ALREADY_OPENED;
	}

	// Begin to look for the device if it is not opened
	if (status == HID_DEVICE_SUCCESS)
	{
        for (i=0; i<128; i++)
        {
            if (GetHidDevicePath(VID, PID, i, devicePath))
            {
                // Set found to true to stop the loop if the device num is correct
                if (deviceNum == deviceIndex)
                {
                    // Attempt to open the device by VID and PID
			        HANDLE hHidDeviceHandle = OpenDeviceByVidPid(devicePath, VID, PID, &error);
                    found = TRUE;

                    if (error == ERROR_SHARING_VIOLATION)
			        {
                        status = HID_DEVICE_ALREADY_OPENED;
                        break;
                    }

			        // Check that the device is opened and the handle is valid
			        if (hHidDeviceHandle != INVALID_HANDLE_VALUE)
			        {
				        // Set found to true to stop the loop if the device num is correct
				        if (deviceNum == deviceIndex)
				        {
					        PHIDP_PREPARSED_DATA preparsedData;
					        OSVERSIONINFO osVer;
					        found = TRUE;

					        // Set the member variables to an opened device and handle
					        pstrHidDevice->bDeviceOpen = TRUE;
					        pstrHidDevice->hndHidDevice = hHidDeviceHandle;

					        pstrHidDevice->oRead.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
					        pstrHidDevice->oWrite.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);


					        if (HidD_GetPreparsedData(hHidDeviceHandle, &preparsedData))
					        {
						        HIDP_CAPS capabilities;

						        // Used the preparsed data structure to get the device capabilities
						        if (HidP_GetCaps(preparsedData, &capabilities))
						        {
							        // Allocate memory for the input, output and feature reports
							        if (capabilities.InputReportByteLength)
							        {
								        pstrHidDevice->wInReportBufferLength = capabilities.InputReportByteLength;
							        }

							        if (capabilities.OutputReportByteLength)
							        {
								        pstrHidDevice->wOutReportBufferLength = capabilities.OutputReportByteLength;
							        }

							        if (capabilities.FeatureReportByteLength)
							        {
								        pstrHidDevice->wFeatReportBufferLength = capabilities.FeatureReportByteLength;
							        }
						        }

						        osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
						        GetVersionEx(&osVer);

						        // Only set the max report requests if we are using 2K or later
						        if ((osVer.dwPlatformId == 2) && (osVer.dwMajorVersion == 5))
						        {
							        if (osVer.dwMinorVersion >= 1)
							        {
								        // XP or later supports 512 input reports
								        err = HidD_SetNumInputBuffers(pstrHidDevice->hndHidDevice, 512);
							        }
							        else if (osVer.dwMinorVersion == 0)
							        {
								        // 2K supports 200 input reports
								        err = HidD_SetNumInputBuffers(pstrHidDevice->hndHidDevice, 200);
							        }
						        }

						        // Cleanup the preparesed data
						        HidD_FreePreparsedData(preparsedData);
					        }
					        break;
				        }

                    } //if (hHidDeviceHandle != INVALID_HANDLE_VALUE
                }//if (deviceNum == deviceIndex)

                // Increment the device number
                deviceNum++;

            }//if (GetHidDevicePath(VID, PID, i, devicePath))

        } //for (i=0; i<128; i++)

		// If the device wasnt found, return an error code
		if (!found)
		{
			status = HID_DEVICE_NOT_FOUND;
		}
	}

	return status;
}

//-------------------------------------------------------------------------------------------------------------

BYTE HID_WriteFile(struct strHidDevice* pstrHidDevice, BYTE* buffer, DWORD bufferSize)
{
    DWORD time1 , time2; //- for debug

	BYTE status = HID_DEVICE_SUCCESS;
	WORD stillToSend = (WORD)bufferSize;
    BYTE bytesCount;
	BYTE report[256], ReportId;

	while((stillToSend >0) && (status == HID_DEVICE_SUCCESS))
	{
        // ONLY 63ReportID is available!
        ReportId = 63;

		report[0] = ReportId;									// Set ReportId to the 1fst element
        bytesCount = ReportId > stillToSend ? stillToSend : ReportId; // how many bytes will be sent with the report;
        report[1] = bytesCount;                                 // Set how many valid bytes in the Report
		memcpy(&report[1],buffer,bytesCount);			        // Copy data from buffer to the report
		buffer += bytesCount;								    // Set pointer to the next data
		stillToSend -= bytesCount;						    	// decrement "stillToSend" for outstanding data

		// Check to see that the device is opened
		if (pstrHidDevice->bDeviceOpen)
		{
			DWORD bytesWritten = 0;


			// Try to write the file
			//TRACE("HID_WriteFile:Try to write the file");
			if (!WriteFile(pstrHidDevice->hndHidDevice, report, pstrHidDevice ->wOutReportBufferLength, &bytesWritten, &pstrHidDevice->oWrite))
			{
				// If the write fails, see if it is because IO is pending
				if (GetLastError() == ERROR_IO_PENDING)
				{
                    DWORD waitStatus;
                    time1 = GetTickCount();

                    //If there is still data to be written, wait on the event for 3 seconds
					waitStatus = WaitForSingleObject(pstrHidDevice->oWrite.hEvent, pstrHidDevice->uSetReportTimeout);

                    time2 = GetTickCount();
                    //printf("\nTime wr debug() = %d\n", (int)(time2-time1));

					// If the object is signaled, then get the overlapped result, the write succeeded
					// Otherwise determine if the error was a timeout, or another error
					if (waitStatus == WAIT_OBJECT_0)
					{


						GetOverlappedResult(pstrHidDevice ->hndHidDevice, &pstrHidDevice->oWrite, &bytesWritten, FALSE);


					}
					else if (waitStatus == WAIT_TIMEOUT)
					{
						//TRACE("HID_WriteFile:waitStatus = WAIT_TIMEOUT");
						status = HID_DEVICE_TRANSFER_TIMEOUT;
						CancelIo(pstrHidDevice ->hndHidDevice);
					}
					else
					{
						//TRACE("HID_WriteFile:HID_DEVICE_TRANSFER_FAILED");
						status = HID_DEVICE_TRANSFER_FAILED;
						CancelIo(pstrHidDevice ->hndHidDevice);
					}

                }
				else
				{
					//TRACE("HID_WriteFile:HID_DEVICE_TRANSFER_FAILED");
					status = HID_DEVICE_TRANSFER_FAILED;
				}
			}

		}
		else
		{
			//TRACE("HID_WriteFile:HID_DEVICE_NOT_OPENED");
			status = HID_DEVICE_NOT_OPENED;
		}
	}

	return status;
}

//-------------------------------------------------------------------------------------------------------------
#if 0
BYTE HID_ReadFile(struct strHidDevice* pstrHidDevice, BYTE* buffer, DWORD bufferSize, DWORD* bytesReturned)
{
	BYTE status = HID_DEVICE_SUCCESS;
	DWORD time[5],bytesRead = 0;
	UINT reportSize;

	DWORD stillToRead = bufferSize;
	if(pstrHidDevice->inBufferUsed > stillToRead)
	{
		memcpy(buffer,pstrHidDevice->inBuffer,bufferSize);
		pstrHidDevice->inBufferUsed -= (WORD)bufferSize;
		memcpy(pstrHidDevice->inBuffer,&pstrHidDevice->inBuffer[bufferSize],pstrHidDevice->inBufferUsed);
		stillToRead -= bufferSize;
		buffer += bufferSize;
	}
	else if(pstrHidDevice->inBufferUsed > 0)
	{
		memcpy(buffer,pstrHidDevice->inBuffer,pstrHidDevice->inBufferUsed);
		stillToRead -= pstrHidDevice->inBufferUsed;
		buffer += pstrHidDevice->inBufferUsed;
		pstrHidDevice->inBufferUsed = 0;
	}
	while((stillToRead>0) && (status==HID_DEVICE_SUCCESS))
	{
		// Check to see that the device is opened
		if (pstrHidDevice->bDeviceOpen)
		{

			// Try to read input data

			if (!ReadFile(pstrHidDevice->hndHidDevice, pstrHidDevice->inBuffer, pstrHidDevice->wInReportBufferLength, &bytesRead, &pstrHidDevice->oRead))
			{
				DWORD error = GetLastError();
				// If the read fails, see if it is because IO is pending
				time[0] = GetTickCount();
				if (error == ERROR_IO_PENDING)
				{
					// If there is still data to read, wait on the event object for 3 seconds
					DWORD waitStatus = WaitForSingleObject(pstrHidDevice->oRead.hEvent, pstrHidDevice->uGetReportTimeout);
					// If the object is signaled, then get the overlapped result, the read succeeded
					// Otherwise determine if the error was a timeout, or another error
					if (waitStatus == WAIT_OBJECT_0)
					{
						GetOverlappedResult(pstrHidDevice->hndHidDevice, &pstrHidDevice->oRead, &bytesRead, FALSE);
					}
					else if (waitStatus == WAIT_TIMEOUT)
					{
						status = HID_DEVICE_TRANSFER_TIMEOUT;
						CancelIo(pstrHidDevice->hndHidDevice);
					}
					else
					{
						status = HID_DEVICE_TRANSFER_FAILED;
						CancelIo(pstrHidDevice->hndHidDevice);
					}
				}

				else
				{
					status = HID_DEVICE_TRANSFER_FAILED;
				}
			}

			if(status == HID_DEVICE_SUCCESS)
			{
				reportSize = (pstrHidDevice->inBuffer[1]);
				/*if(reportSize > stillToRead)
				{
					memcpy(buffer,&pstrHidDevice->inBuffer[2],stillToRead);
					pstrHidDevice->inBufferUsed = pstrHidDevice->inBuffer[1];
					pstrHidDevice->inBufferUsed -= (WORD)stillToRead;
					memcpy(pstrHidDevice->inBuffer,&pstrHidDevice->inBuffer[stillToRead+2],pstrHidDevice->inBufferUsed);
					stillToRead = 0;
				}
				else
				{ */
					memcpy(buffer,(pstrHidDevice->inBuffer + 1),reportSize+1); //2
					pstrHidDevice->inBufferUsed = 0;
					stillToRead -= reportSize;
					buffer += reportSize;
				//}
			}

		}
		else
		{
			status = HID_DEVICE_NOT_OPENED;
		}
	}

	*bytesReturned = bufferSize - stillToRead;

	return status;
}
#endif

//buffer size should be 64 b ytes , returened always 64 bytes
BYTE HID_ReadFile(struct strHidDevice* pstrHidDevice, BYTE* buffer)
{
	DWORD bufferSize = 64;
	BYTE status = HID_DEVICE_SUCCESS;
	DWORD time[5],bytesRead = 0;
	//UINT reportSize;

	DWORD stillToRead = bufferSize;
	/*
	if(pstrHidDevice->inBufferUsed > stillToRead)
	{
		memcpy(buffer,pstrHidDevice->inBuffer,bufferSize);
		pstrHidDevice->inBufferUsed -= (WORD)bufferSize;
		memcpy(pstrHidDevice->inBuffer,&pstrHidDevice->inBuffer[bufferSize],pstrHidDevice->inBufferUsed);
		stillToRead -= bufferSize;
		buffer += bufferSize;
	}
	else if(pstrHidDevice->inBufferUsed > 0)
	{
		memcpy(buffer,pstrHidDevice->inBuffer,pstrHidDevice->inBufferUsed);
		stillToRead -= pstrHidDevice->inBufferUsed;
		buffer += pstrHidDevice->inBufferUsed;
		pstrHidDevice->inBufferUsed = 0;
	} */
	//while(/*(stillToRead>0) &&*/ (status==HID_DEVICE_SUCCESS))
	{
		// Check to see that the device is opened
		if (pstrHidDevice->bDeviceOpen)
		{

			// Try to read input data

			if (!ReadFile(pstrHidDevice->hndHidDevice, pstrHidDevice->inBuffer, 64, &bytesRead, &pstrHidDevice->oRead))
			{
				DWORD error = GetLastError();
				// If the read fails, see if it is because IO is pending
				time[0] = GetTickCount();
				if (error == ERROR_IO_PENDING)
				{
					// If there is still data to read, wait on the event object for 3 seconds
					DWORD waitStatus = WaitForSingleObject(pstrHidDevice->oRead.hEvent, pstrHidDevice->uGetReportTimeout);
					// If the object is signaled, then get the overlapped result, the read succeeded
					// Otherwise determine if the error was a timeout, or another error
					if (waitStatus == WAIT_OBJECT_0)
					{
						GetOverlappedResult(pstrHidDevice->hndHidDevice, &pstrHidDevice->oRead, &bytesRead, FALSE);
					}
					else if (waitStatus == WAIT_TIMEOUT)
					{
						status = HID_DEVICE_TRANSFER_TIMEOUT;
						CancelIo(pstrHidDevice->hndHidDevice);
					}
					else
					{
						status = HID_DEVICE_TRANSFER_FAILED;
						CancelIo(pstrHidDevice->hndHidDevice);
					}
				}

				else
				{
					status = HID_DEVICE_TRANSFER_FAILED;
				}
			}

			if(status == HID_DEVICE_SUCCESS)
			{
				//reportSize = (pstrHidDevice->inBuffer[1]);
				/*if(reportSize > stillToRead)
				{
					memcpy(buffer,&pstrHidDevice->inBuffer[2],stillToRead);
					pstrHidDevice->inBufferUsed = pstrHidDevice->inBuffer[1];
					pstrHidDevice->inBufferUsed -= (WORD)stillToRead;
					memcpy(pstrHidDevice->inBuffer,&pstrHidDevice->inBuffer[stillToRead+2],pstrHidDevice->inBufferUsed);
					stillToRead = 0;
				}
				else
				{ */
					//memcpy(buffer,(pstrHidDevice->inBuffer + 1),reportSize+1); //2
					memcpy(buffer,(pstrHidDevice->inBuffer),64);
					pstrHidDevice->inBufferUsed = 0;
					//stillToRead -= reportSize;
					//buffer += reportSize;
				//}
			}

		}
		else
		{
			status = HID_DEVICE_NOT_OPENED;
		}
	}

	//*bytesReturned = bufferSize - stillToRead;

	return status;
}


//-------------------------------------------------------------------------------------------------------------

BYTE HID_FlushBuffer(struct strHidDevice* pstrHidDevice)
{
    BYTE status = HID_DEVICE_SUCCESS;
	pstrHidDevice->inBufferUsed = 0;

	if(FALSE == HidD_FlushQueue(pstrHidDevice->hndHidDevice))
	{
		if(ERROR_INVALID_HANDLE == GetLastError())
		{
			status = HID_DEVICE_HANDLE_ERROR;
		}
		else
			status = HID_DEVICE_UNKNOWN_ERROR;
		if(status != HID_DEVICE_SUCCESS)
			return status;
	}

	return HID_DEVICE_SUCCESS;
}

//-------------------------------------------------------------------------------------------------------------

BYTE HID_GetSerialNumber(struct strHidDevice* pstrHidDevice, BYTE * Buffer, ULONG BufferLength)
{
	if(HidD_GetSerialNumberString(pstrHidDevice->hndHidDevice, Buffer, BufferLength) == FALSE)
		return HID_DEVICE_HANDLE_ERROR;
	return HID_DEVICE_SUCCESS;
}

//-------------------------------------------------------------------------------------------------------------

BYTE HID_GetVersionNumber(struct strHidDevice* pstrHidDevice, USHORT * VersionNumber)
{
	HIDD_ATTRIBUTES Hid_Attributes;

	if(HidD_GetAttributes(pstrHidDevice->hndHidDevice,&Hid_Attributes) == FALSE)
		return HID_DEVICE_HANDLE_ERROR;

	*VersionNumber = Hid_Attributes.VersionNumber;
	return HID_DEVICE_SUCCESS;
}

//-------------------------------------------------------------------------------------------------------------

BYTE GetFeature(struct strHidDevice* pstrHidDevice,BYTE* buffer)
{
	BYTE status = HID_DEVICE_SUCCESS;
	DWORD bytesRead;

	if(pstrHidDevice->bDeviceOpen)
	{

		if (!HidD_GetFeature(pstrHidDevice->hndHidDevice, buffer, pstrHidDevice->wFeatReportBufferLength))
		{
			// If the write fails, see if it is because IO is pending
			if (GetLastError() == ERROR_IO_PENDING)
			{
				//If there is still data to be written, wait on the event for 3 seconds
				DWORD waitStatus = WaitForSingleObject(pstrHidDevice->oRead.hEvent, pstrHidDevice->uGetReportTimeout);

				// If the object is signaled, then get the overlapped result, the write succeeded
				// Otherwise determine if the error was a timeout, or another error
				if (waitStatus == WAIT_OBJECT_0)
				{
					GetOverlappedResult(pstrHidDevice ->hndHidDevice, &pstrHidDevice->oRead, &bytesRead, FALSE);
				}
				else if (waitStatus == WAIT_TIMEOUT)
				{
					status = HID_DEVICE_TRANSFER_TIMEOUT;
					CancelIo(pstrHidDevice ->hndHidDevice);
				}
				else
				{
					status = HID_DEVICE_TRANSFER_FAILED;
					CancelIo(pstrHidDevice ->hndHidDevice);
				}
			}
			else
			{
				status = HID_DEVICE_TRANSFER_FAILED;
			}
		}
	}
	else
	{
		status = HID_DEVICE_NOT_OPENED;
	}
	return status;
}

//-------------------------------------------------------------------------------------------------------------

BYTE SetFeature(struct strHidDevice* pstrHidDevice,BYTE* buffer)
{
	BYTE status = HID_DEVICE_SUCCESS;
	DWORD bytesWritten;

	if(pstrHidDevice->bDeviceOpen)
	{
		if (!HidD_SetFeature(pstrHidDevice->hndHidDevice, buffer, pstrHidDevice->wFeatReportBufferLength))
		{
			// If the write fails, see if it is because IO is pending
			if (GetLastError() == ERROR_IO_PENDING)
			{
				//If there is still data to be written, wait on the event for 3 seconds
				DWORD waitStatus = WaitForSingleObject(pstrHidDevice->oWrite.hEvent, pstrHidDevice->uSetReportTimeout);

				// If the object is signaled, then get the overlapped result, the write succeeded
				// Otherwise determine if the error was a timeout, or another error
				if (waitStatus == WAIT_OBJECT_0)
				{
					GetOverlappedResult(pstrHidDevice ->hndHidDevice, &pstrHidDevice->oWrite, &bytesWritten, FALSE);
				}
				else if (waitStatus == WAIT_TIMEOUT)
				{
					status = HID_DEVICE_TRANSFER_TIMEOUT;
					CancelIo(pstrHidDevice ->hndHidDevice);
				}
				else
				{
					status = HID_DEVICE_TRANSFER_FAILED;
					CancelIo(pstrHidDevice ->hndHidDevice);
				}
			}
			else
			{
				status = HID_DEVICE_TRANSFER_FAILED;
			}
		}
	}
	else
	{
		status = HID_DEVICE_NOT_OPENED;
	}
	return status;
}

// end of file
