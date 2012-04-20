#include "quadrocore.h"

uint8_t ReadCalibrationByte(uint8_t index)
{
	uint8_t value = 0;
	NVM.CMD = NVM_CMD_READ_CALIB_ROW_gc; 
	value = pgm_read_byte(index); 
	NVM.CMD = NVM_CMD_NO_OPERATION_gc; 
	return value;
}