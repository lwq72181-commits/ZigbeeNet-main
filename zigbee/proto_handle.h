#ifndef __PROTO_HANDLE_H__
#define __PROTO_HANDLE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "dt_uart_manager.h"
//#define ZIGBEE_TEST_SWITCH
typedef struct inter_task_def {
    int task_type;
    int (* handle)(uint8_t *pdata, uint32_t len);
} inter_task_t;

typedef struct wireless_task_def {
    int task_type;
    int (* handle)(uint8_t *pdata, uint32_t len);
} wireless_task_t;

FREQ_PUBLIC int do_regular_commun_handle(unsigned char *pdata, unsigned int data_len);
FREQ_PUBLIC int set_sys_status(int status);
int inter_task_handle(uint8_t *pdata, uint32_t data_len);
int wireless_task_handle(uint8_t *pdata, uint32_t data_len);
int get_task_state_handle(uint8_t *pdata, uint32_t data_len);
int notify_mpu_upgrade_handle(uint8_t *pdata, uint32_t data_len);
int notify_mpu_rs485_upgrade_handle(uint8_t *pdata, uint32_t data_len);
int req_get_file_handle(uint8_t *pdata, uint32_t data_len);
int get_file_handle(uint8_t *pdata, uint32_t data_len);
int upload_data_handle(uint8_t *pdata, uint32_t data_len);
int spi_flash_info_handle(uint8_t *pdata, uint32_t data_len);
int hw_self_check_handle(uint8_t *pdata, uint32_t data_len);
int hw_check_query_handle (uint8_t *pdata, uint32_t data_len);
int debug_task_handle(uint8_t *pdata, uint32_t data_len);
int upload_trend_handle(uint8_t *pdata, uint32_t data_len);
int upload_upgrade_state_handle(uint8_t *pdata, uint32_t data_len);
int upload_log_handle(uint8_t *pdata, uint32_t data_len);
int upload_real_eigen_handle(uint8_t *pdata, uint32_t data_len);
FREQ_PUBLIC void sys_status_set(int status);
FREQ_PUBLIC int sys_status_get(void);
#ifdef __cplusplus
}
#endif
#endif
