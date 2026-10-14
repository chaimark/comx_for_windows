#include "CheckCode.h"
#include "NumberBaseLib.h"

uint16_t getModbusCrc(strnew InputBuf, size_t num_bytes) {
    const unsigned char *input_str = (const unsigned char *)InputBuf.Name._uint8_t;
    uint16_t crc = CRC_START_MODBUS;
    if (input_str == NULL) {
        return crc;
    }
    while (num_bytes--) {
        crc ^= (uint16_t)*input_str;
        input_str++;
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ CRC_POLY_16;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

bool isModbusCrc(strnew InputBuf, size_t num_bytes) {
    if ((size_t)InputBuf.MaxLen < num_bytes) {
        return false;
    }
    num_bytes -= 2;
    uint16_t crc = getModbusCrc(InputBuf, num_bytes);
    if (crc == U8_Connect_U8(InputBuf.Name._uint8_t[num_bytes + 1], InputBuf.Name._uint8_t[num_bytes])) {
        return true;
    } else {
        return false;
    }
}

uint8_t getSumCs(strnew InputBuf, size_t num_bytes) {
    uint8_t Sum = 0;
    for (size_t i = 0; i < num_bytes; i++) {
        Sum += InputBuf.Name._uint8_t[i];
    }
    return Sum;
}

bool isSumCs(strnew InputBuf, size_t num_bytes) {
    if ((size_t)InputBuf.MaxLen < num_bytes) {
        return false;
    }
    num_bytes -= 1;
    uint8_t Sum = getSumCs(InputBuf, num_bytes);
    if (Sum == InputBuf.Name._uint8_t[num_bytes]) {
        return true;
    } else {
        return false;
    }
}
