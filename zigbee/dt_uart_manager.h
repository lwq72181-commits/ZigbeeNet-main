#ifndef __DT_UART_MANAGER_H__
#define __DT_UART_MANAGER_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
#include <pthread.h>


typedef enum _serial_uart_port_def{
    SERIAL_UART9 = 0x00,    /* to zigbee soc */
    SERIAL_UART_MAX,
}serial_uart_port_t;


typedef struct _uart_config_def{
    const char *name;   /* 串口路径 */
    int speed;          /* 波特率 */
    int data_bit;       /* 数据位 */
    char eve_bit;       /* 校验位 */
    int stop_bit;       /* 停止位 */
} uart_config_t;

/*串口结构体*/
typedef struct dt_uart_def dt_uart_t;
struct dt_uart_def {
    /*端口配置参数*/
    uart_config_t config;

    /*串口设备句柄*/
    int fd;

    /*串口类型，485或其他串口*/
    int type;

    /* 用于接收消息的锁和信号量 */
    pthread_mutex_t     send_mutex;
    pthread_cond_t      send_cond;

    /*接收缓存区*/
    unsigned char *recv_buff;
    unsigned int  recv_len;

    /*uart 接收数据处理线程，在执行init函数时创建。如果已经创建，则直接返回*/
    pthread_t tid_uart_thread;

    void (*init)(dt_uart_t *t);            /*串口初始化函数*/
    void (*deinit)(dt_uart_t *t);          /*串口反初始化函数*/

    int (*send_with_recv)(dt_uart_t *t, void *send_buf, unsigned int  send_len, \
                               void *recv_buf, unsigned int *recv_len, int timeout);
    int (*send)(dt_uart_t *t, void *send_buf, unsigned int  send_len);
};

FREQ_PUBLIC dt_uart_t * dt_uart_get(int type);


FREQ_PUBLIC void dt_uart_module_init(void);
FREQ_PUBLIC int uart_send_with_recv(int type, void *tx_buf, unsigned int tx_len, void *rx_buf, unsigned int *rx_len, int timeout);
FREQ_PUBLIC int uart_send(int type, void *tx_buf, unsigned int tx_len);
#ifdef __cplusplus
}
#endif
#endif // !__DT_UART_MANAGER_H__
