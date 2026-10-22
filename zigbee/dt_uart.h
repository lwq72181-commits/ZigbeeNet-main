#ifndef __DT_UART_H__
#define __DT_UART_H__
#ifdef __cplusplus
extern "C" {
#endif

int dt_uart_write(int fd, void *buf, int len);
int dt_uart_read(int fd, void *buf, int len);
int dt_uart_set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);
int dt_uart_open(const char *port);
#ifdef __cplusplus
}
#endif
#endif /* __DT_UART_H__ */
