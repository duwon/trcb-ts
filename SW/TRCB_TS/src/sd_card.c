#include "sd_card.h"
#include "xsdps.h"		/* SD device driver */
#include "ff.h"
#include "interrupt.h"
int FfsSdPolledExample(void);

/************************** Variable Definitions *****************************/
static FIL fil;		/* File object */
static FATFS fatfs;
/*
 * To test logical drive 0, FileName should be "0:/<File name>" or
 * "<file_name>". For logical drive 1, FileName should be "1:/<file_name>"
 */
static char FileName[32] = "BOOT.bin";
static char *SD_File;

int init_SDCard(void)
{
    FRESULT Res;
    BYTE work[FF_MAX_SS];

	TCHAR *Path = "0:/";

	Res = f_mount(&fatfs, Path, 0); /* Register volume work area, initialize device */
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

    /* Path - Path to logical driver, 0 - FDISK format.  0 - Cluster size is automatically determined based on Vol size. */
	Res = f_mkfs(Path, FM_FAT32, 0, work, sizeof work);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

    /* Open file with required permissions. - Creating new file with read/write permissions. */
	SD_File = (char *)FileName;

	Res = f_open(&fil, SD_File, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (Res) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int write_SDCard(uint8_t *data, uint32_t writeAddress, uint32_t dataSize)
{
	FRESULT Res;
	UINT NumBytesWritten;

	Res = f_lseek(&fil, writeAddress); /* Pointer to beginning of file. */
	if (Res) {
		return XST_FAILURE;
	}


	Res = f_write(&fil, (const void*)data, dataSize, &NumBytesWritten); /* Write data to file. */
	if (Res) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

//uint32_t read_SDCard(uint8_t *readData, uint32_t readAddress, uint32_t dataSize)
//{
//}

int close_SDCard(void)
{
	FRESULT Res;

	Res = f_close(&fil); /* Close file. */
	if (Res) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

#pragma pack(push, 1)
typedef struct
{
    uint16_t stx;
	uint8_t length;
	uint8_t type;
	uint16_t identifier;
	uint16_t trbID;
	uint16_t totImgSize_H;
	uint16_t totImgSize_L;
	uint16_t startAddress_H;
	uint16_t startAddress_L;
	uint16_t currentImgSize;
	uint8_t data[1024];
	uint16_t checksum;
} firmwarePacket_TypeDef;
#pragma pack(pop)

firmwarePacket_TypeDef firmRxPacket;

int write_BootImage(uint16_t *packet)
{
	int Res=0;

//	memcpy((void *)&firmRxPacket, (void *)packet, (packet[2]*128 + 10)*2);
//
//	uint32_t startAddress = ((uint32_t)firmRxPacket.startAddress_H << 16) + ((uint32_t)firmRxPacket.startAddress_L << 0);
//	uint32_t imgSize = ((uint32_t)firmRxPacket.totImgSize_H << 16) + ((uint32_t)firmRxPacket.totImgSize_L << 0);
//	uint16_t totalIdentifier = (imgSize / 1024);
//
//	static uint8_t writeStage = 0;
//	static uint64_t LT = 0;
//	xil_printf("STAGE: %d, TI: %d, CI: %d A: %d\r\n", writeStage, totalIdentifier, firmRxPacket.identifier, startAddress);
//	switch(writeStage)
//	{
//		case 1:
//			Res |= write_SDCard(firmRxPacket.data, startAddress, firmRxPacket.currentImgSize);
//			if(firmRxPacket.identifier == totalIdentifier)
//			{
//				Res |= close_SDCard();
//				xil_printf("Write Done. TI: %d, CI: %d A: %d\r\n",totalIdentifier, firmRxPacket.identifier, startAddress);
//				writeStage = 0;
//			}
//			/* writing 중 5초 타이웃 지정 */
//			if((tick_1ms - LT) > 5000 )
//			{
//				writeStage = 0;
//				Res |=  XST_FAILURE;
//			}
//			else
//			{
//				LT = tick_1ms;
//			}
//			break;
//		default:
//			if(firmRxPacket.identifier == 0)
//			{
//				Res |= init_SDCard();
//				Res |= write_SDCard(firmRxPacket.data, startAddress, firmRxPacket.currentImgSize);
//				writeStage = 1;
//				LT = tick_1ms;
//			}
//			else
//			{
//				Res = XST_FAILURE;
//			}
//			break;
//	}
//
//	return Res;













	uint32_t startAddress = ((uint32_t)packet[6] << 16) + ((uint32_t)packet[7] << 0);
	uint16_t identifier = (uint32_t)packet[2];
	uint32_t imgSize = ((uint32_t)packet[4] << 16) + ((uint32_t)packet[5] << 0);
	uint16_t totalIdentifier = (imgSize / 1024);
	uint16_t currentImgSize = packet[8];
	
	static uint8_t writeStage = 0;
	static uint64_t LT = 0;

	//xil_printf("STAGE: %d, TI: %d, CI: %d A: %d\r\n", writeStage, totalIdentifier, identifier, startAddress);
	switch(writeStage)
	{
		case 1:
			Res |= write_SDCard((uint8_t *)&packet[9], startAddress, 1024);
			if(identifier == totalIdentifier)
			{
				Res |= close_SDCard();
				xil_printf("Write Done. TI: %d, CI: %d A: %d\r\n",totalIdentifier, identifier, startAddress);
				writeStage = 0;
			}
			/* writing 중 5초 타이웃 지정 */
			if((tick_1ms - LT) > 5000 )
			{
				writeStage = 0;
				Res |=  XST_FAILURE;
			}
			else
			{
				LT = tick_1ms;
			}
			break;
		default:
			if(identifier == 0)
			{
				Res |= init_SDCard();
				Res |= write_SDCard((uint8_t *)&packet[9], 0, 1024);
				writeStage = 1;
				LT = tick_1ms;
			}
			else
			{
				Res = XST_FAILURE;
			}
			break;
	}

	return Res;
}

//int FfsSdPolledExample(void)
//{
//	FRESULT Res;
//	UINT NumBytesRead;
//	UINT NumBytesWritten;
//	u32 BuffCnt;
//	BYTE work[FF_MAX_SS];
//	u32 FileSize = (8*1024);
//
//	/*
//	 * To test logical drive 0, Path should be "0:/"
//	 * For logical drive 1, Path should be "1:/"
//	 */
//	TCHAR *Path = "0:/";
//
//	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
//		SourceAddress[BuffCnt] = BuffCnt;
//	}
//
//	/*
//	 * Register volume work area, initialize device
//	 */
//	Res = f_mount(&fatfs, Path, 0);
//
//	if (Res != FR_OK) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Path - Path to logical driver, 0 - FDISK format.
//	 * 0 - Cluster size is automatically determined based on Vol size.
//	 */
//	Res = f_mkfs(Path, FM_FAT32, 0, work, sizeof work);
//	if (Res != FR_OK) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Open file with required permissions.
//	 * Here - Creating new file with read/write permissions. .
//	 * To open file with write permissions, file system should not
//	 * be in Read Only mode.
//	 */
//	SD_File = (char *)FileName;
//
//	Res = f_open(&fil, SD_File, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
//	if (Res) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Pointer to beginning of file .
//	 */
//	Res = f_lseek(&fil, 0);
//	if (Res) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Write data to file.
//	 */
//	Res = f_write(&fil, (const void*)SourceAddress, FileSize,
//			&NumBytesWritten);
//	if (Res) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Pointer to beginning of file .
//	 */
//	Res = f_lseek(&fil, 0);
//	if (Res) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Read data from file.
//	 */
//	Res = f_read(&fil, (void*)DestinationAddress, FileSize,
//			&NumBytesRead);
//	if (Res) {
//		return XST_FAILURE;
//	}
//
//	/*
//	 * Data verification
//	 */
//	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
//		if(SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]){
//			return XST_FAILURE;
//		}
//	}
//
//	/*
//	 * Close file.
//	 */
//	Res = f_close(&fil);
//	if (Res) {
//		return XST_FAILURE;
//	}
//
//	return XST_SUCCESS;
//}
