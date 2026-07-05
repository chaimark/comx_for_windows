#include "CrcCheck.h"
#include "NumberBaseLib.h"

uint16_t get_crc_modbus(const unsigned char *input_str, size_t num_bytes) {
    uint16_t crc = CRC_START_MODBUS;    
    if (input_str == NULL) {
        return crc;
    }
    while(num_bytes--) {
        crc ^= (uint16_t)*input_str++;
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

bool is_crc_modbus(strnew InputBuf, size_t num_bytes) {
    if(InputBuf.MaxLen < num_bytes){
        return false;
    }
    num_bytes -= 2;
    uint16_t crc = get_crc_modbus((const unsigned char *)InputBuf.Name._char, num_bytes);
    if (crc == U8_Connect_U8(InputBuf.Name._char[num_bytes], InputBuf.Name._char[num_bytes + 1])) {
        return true;
    } else {
        return false;
    }
}
