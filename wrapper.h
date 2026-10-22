#ifndef WRAPPER_H
#define WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "zigbee/proto_parse.h"
// C 兼容的函数声明
int set(char *name,int value);
int get(char *name);
int get_string(char *name,char *param);
int storeIndexData(uart_0x34_indicator_data_t* buf, char *lsn, int count, int pos);
int storeWaveData(char *pwave_buf,unsigned char DataType,char *sn,int wavfilesize);
#ifdef __cplusplus
}
#endif
#endif // WRAPPER_H
