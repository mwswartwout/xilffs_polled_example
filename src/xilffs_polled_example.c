/******************************************************************************
*
* Copyright (C) 2013 - 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilffs_polled_example.c
*
*
* @note This example uses file system with SD to write to and read from
* an SD card using ADMA2 in polled mode.
* To test this example File System should not be in Read Only mode.
*
* This example was tested using SD2.0 card and eMMC (using eMMC to SD adaptor).
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.00a hk  10/17/13 First release
* 2.2   hk  07/28/14 Make changes to enable use of data cache.
* 2.5   sk  07/15/15 Used File size as 8KB to test on emulation platform.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xsdps.h"		/* SD device driver */
#include "xil_printf.h"
#include "ff.h"
#include "xil_cache.h"
#include "xplatform_info.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FfsSdPolledExample(void);
void handle_fresult(FRESULT result);

/************************** Variable Definitions *****************************/
static FIL fil;		/* File object */
static FATFS fatfs;
static char FileName[32] = "Test.txt";
static char *SD_File;
u32 Platform;

//#ifdef __ICCARM__
//#pragma data_alignment = 32
char DestinationAddress[32];
char SourceAddress[32] = "Hello World";
//#pragma data_alignment = 4
//#else
//u8 DestinationAddress[10*1024*1024] __attribute__ ((aligned(32)));
//u8 SourceAddress[10*1024*1024] __attribute__ ((aligned(32)));
//#endif

#define TEST 7

/*****************************************************************************/
/**
*
* Main function to call the SD example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("SD Polled File System Example Test \r\n");

	Status = FfsSdPolledExample();
	if (Status != XST_SUCCESS) {
		xil_printf("SD Polled File System Example Test failed \r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SD Polled File System Example Test \r\n");

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* File system example using SD driver to write to and read from an SD card
* in polled mode. This example creates a new file on an
* SD card (which is previously formatted with FATFS), write data to the file
* and reads the same data back to verify.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int FfsSdPolledExample(void)
{
	FRESULT Res;
	UINT NumBytesRead;
	UINT NumBytesWritten;
	u32 BuffCnt;
	u32 FileSize = 32;
	TCHAR *Path = "0:/";

	Platform = XGetPlatform_Info();
	if (Platform == XPLAT_ZYNQ_ULTRA_MP) {
		/*
		 * Since 8MB in Emulation Platform taking long time, reduced
		 * file size to 8KB.
		 */
		FileSize = 8*1024;
	}

	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		SourceAddress[BuffCnt] = TEST + BuffCnt;
	}

	/*
	 * Register volume work area, initialize device
	 */
	Res = f_mount(&fatfs, Path, 0);
	if (Res != FR_OK) {
		print("f_mount failed\n\r");
		handle_fresult(Res);
		return XST_FAILURE;
	}

	/*
	 * Open file with required permissions.
	 * Here - Creating new file with read/write permissions. .
	 * To open file with write permissions, file system should not
	 * be in Read Only mode.
	 */
	SD_File = (char *)FileName;

	Res = f_open(&fil, SD_File, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (Res) {
		print("f_open failed\n\r");
		handle_fresult(Res);
		return XST_FAILURE;
	}

	/*
	 * Pointer to beginning of file .
	 *
	Res = f_lseek(&fil, 0);
	if (Res) {
		print("f_lseek failed\n\r");
		handle_fresult(Res);
		return XST_FAILURE;
	}*/

	/*
	 * Write data to file.
	 */
	Res = f_write(&fil, (const void*)SourceAddress, FileSize,
			&NumBytesWritten);
	if (Res) {
		print("f_write failed\n\r");
		handle_fresult(Res);
		return XST_FAILURE;
	}

	/*
	 * Pointer to beginning of file .
	 */
	Res = f_lseek(&fil, 0);
	if (Res) {
		print("f_lseek failed the second time\n\r");
		handle_fresult(Res);
		return XST_FAILURE;
	}

	/*
	 * Read data from file.
	 */
	Res = f_read(&fil, (void*)DestinationAddress, FileSize,
			&NumBytesRead);
	if (Res) {
		print("f_read failed\n\r");
		handle_fresult(Res);
		return XST_FAILURE;
	}

	/*
	 * Data verification
	 */
	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		if(SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]){
			print("Data comparison failed\n\r");
			return XST_FAILURE;
		}
	}

	/*
	 * Close file.
	 */
	Res = f_close(&fil);
	if (Res) {
		print("f_close failed\n\r");
		handle_fresult(Res);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

void handle_fresult(FRESULT result) {
	switch (result) {
		case FR_OK:
			print("FRESULT = FR_OK\n\r");
			break;
		case FR_DISK_ERR:
			print("FRESULT = FR_DISK_ERR\n\r");
			break;
		case FR_INT_ERR:
			print("FRESULT = FR_INT_ERR\n\r");
			break;
		case FR_NOT_READY:
			print("FRESULT = FR_NOT_READY\n\r");
			break;
		case FR_NO_FILE:
			print("FRESULT = FR_NO_FILE\n\r");
			break;
		case FR_NO_PATH:
			print("FRESULT = FR_NO_PATH\n\r");
			break;
		case FR_INVALID_NAME:
			print("FRESULT = FR_INVALID_NAME\n\r");
			break;
		case FR_DENIED:
			print("FRESULT = FR_DENIED\n\r");
			break;
		case FR_EXIST:
			print("FRESULT = FR_EXIST\n\r");
			break;
		case FR_INVALID_OBJECT:
			print("FRESULT = FR_INVALID_OBJECT\n\r");
			break;
		case FR_WRITE_PROTECTED:
			print("FRESULT = FR_WRITE_PROTECTED\n\r");
			break;
		case FR_INVALID_DRIVE:
			print("FRESULT = FR_INVALID_DRIVE\n\r");
			break;
		case FR_NOT_ENABLED:
			print("FRESULT = FR_NOT_ENABLED\n\r");
			break;
		case FR_NO_FILESYSTEM:
			print("FRESULT = FR_NO_FILESYSTEM\n\r");
			break;
		case FR_MKFS_ABORTED:
			print("FRESULT = FR_MKFS_ABORTED\n\r");
			break;
		case FR_TIMEOUT:
			print("FRESULT = FR_TIMEOUT\n\r");
			break;
		case FR_LOCKED:
			print("FRESULT = FR_LOCKED\n\r");
			break;
		case FR_NOT_ENOUGH_CORE:
			print("FRESULT = FR_NOT_ENOUGH_CORE\n\r");
			break;
		case FR_TOO_MANY_OPEN_FILES:
			print("FRESULT = FR_TOO_MANY_OPEN_FILES\n\r");
			break;
		case FR_INVALID_PARAMETER:
			print("FRESULT = FR_INVALID_PARAMETER\n\r");
			break;
		default:
			print("Returned FRESULT value is unknown\n\r");
	}
}
