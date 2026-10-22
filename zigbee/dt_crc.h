#ifndef _DT_CRC_H
#define _DT_CRC_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

/* 秘钥结构体 */
typedef struct _rc4_key_def {
    unsigned char  state[256];
    unsigned char  x;
    unsigned char  y;
}rc4_key_t;

unsigned int dt_crc32(void *buf, unsigned int len);
unsigned int dt_sub_crc32(unsigned int crc, void *buf, unsigned int len);
void DecodeRC4(unsigned char *data, unsigned int len);
#ifdef __cplusplus
}
#endif
#endif
