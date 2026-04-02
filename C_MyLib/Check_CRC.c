#include "Check_CRC.h"
#include "NumberBaseLib.h"

static uint16_t crc_tab16[256] = {0};
static void     init_crc16_tab(void) {
        uint16_t crc;
        uint16_t c;
        for (uint16_t i = 0; i < 256; i++) {
            crc = 0;
            c = i;
            for (uint16_t j = 0; j < 8; j++) {
                if ((crc ^ c) & 0x0001)
                crc = (crc >> 1) ^ CRC_POLY_16;
            else
                crc = crc >> 1;
            c = c >> 1;
        }
            crc_tab16[i] = crc;
    }
} /* init_crc16_tab */

uint16_t get_crc_modbus(const unsigned char* input_str, size_t num_bytes) {
    uint16_t             crc = CRC_START_MODBUS;
    const unsigned char* ptr = input_str;
    init_crc16_tab();
    if (ptr != NULL)
        for (size_t a = 0; a < num_bytes; a++) {
            crc = (crc >> 8) ^ crc_tab16[(crc ^ (uint16_t)*ptr++) & 0x00FF];
        }
    return (((crc & 0x00FF) << 8) | ((crc & 0xFF00) >> 8));
} /* get_crc_modbus */

bool is_crc_modbus(strnew InputBuf) {
    InputBuf.MaxLen -= 2;
    uint16_t crc = get_crc_modbus((const unsigned char*)InputBuf.Name._char, InputBuf.MaxLen);
    if (crc == U8_Connect_U8(InputBuf.Name._char[InputBuf.MaxLen], InputBuf.Name._char[InputBuf.MaxLen + 1])) {
        return true;
    } else {
        return false;
    }
}
