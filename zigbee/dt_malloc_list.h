#ifndef _DT_MALLOC_LIST_H
#define _DT_MALLOC_LIST_H
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

FREQ_PUBLIC unsigned char *modbus_tcp_read_buffer_get(unsigned int length);
FREQ_PUBLIC unsigned char *tcp_up_buf_get(int size);
FREQ_PUBLIC char *ver_temp_buf_get(int index, int size);
FREQ_PUBLIC char *audio_temp_buf_get(unsigned int size);
FREQ_PUBLIC char *audio_upload_buf_get(unsigned int size);
FREQ_PUBLIC short *db_calc_buf_get(int index, unsigned int size);
FREQ_PUBLIC char *wless_eigen_buf_get(int index, unsigned int size);
#ifdef __cplusplus
}
#endif
#endif // !_DT_MALLOC_LIST_H
