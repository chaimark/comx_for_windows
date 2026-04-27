#ifndef __CHECK_CRC_H__
#define __CHECK_CRC_H__

#include "StrLib.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define CRC_POLY_16 0xA001
#define CRC_START_MODBUS 0xFFFF
extern uint16_t get_crc_modbus(const unsigned char* input_str, size_t num_bytes);
extern bool     is_crc_modbus(strnew InputBuf);

#endif
