#include "dt_uart_mcu_board.h"


FREQ_PRIVATE int _mcu_board_protocol_format_check(void *buff, unsigned int len)
{
    if (buff == NULL || len == 0)
        return DT_ERROR;

    /* 判断接收的内容是否符合协议 */
    mcu_board_frame_t *recv = (mcu_board_frame_t *)buff;

    /* 头错误 */
    if (CMD_SOURCE_MCU != recv->header.head) {
        DT_ERR("uart recv header error,0x%02x\r\n", recv->header.head);
        return DT_ERROR;
    }

    /* 头长度 */
    if (recv->header.len != len) {
        DT_ERR("uart len error!%d,%d\r\n", recv->header.len, len);
        return DT_ERROR;
    }

    /* cs错误 */
    if (0 != dt_compute_cs((unsigned char *)buff, len)) {
        DT_ERR("uart cs error!\r\n");
        return DT_ERROR;
    }

    return DT_EOK;
}

FREQ_PUBLIC int uart_mcu_board_data_trx(int cmd, void *src, unsigned int src_len, void *dest, unsigned int dest_len, int timeout)
{
    unsigned char tx_buf[4096] = {0}; /* 发送缓冲区 */
    unsigned char rx_buf[4096] = {0}; /* 接收缓冲区 */
    unsigned  int rx_len = 0;        /* 接收长度 */
    unsigned int send_len = 0;

    /* 发送数据包头赋值 */
    void *send = (void *)tx_buf;

    /* 新协议打包 */
    /* 头 */
    ((mcu_board_frame_t *)send)->header.head = CMD_SOURCE_MPU;
    ((mcu_board_frame_t *)send)->header.cmd  = cmd;
    ((mcu_board_frame_t *)send)->header.len  = src_len + sizeof(mcu_board_frame_t) + sizeof(mcu_board_tail_t);     /* 长度包括头、包体、尾 */

    /* 尾 */
    mcu_board_tail_t *tailer = (mcu_board_tail_t *)&((mcu_board_frame_t *)send)->data[src_len];

    /* 如果有包体，需要给包体赋值 */
    if (src && src_len > 0) {
        memcpy(((mcu_board_frame_t *)send)->data, src, src_len);
    }

    /* 发送长度 */
    send_len = ((mcu_board_frame_t *)send)->header.len;

    /* 计算尾cs */
    tailer->cs = dt_compute_cs(tx_buf, send_len);
    uint8_t_data_printf(tx_buf, send_len);
    if (DT_EOK != uart_send_with_recv(SERIAL_UART9, tx_buf, send_len, rx_buf, &rx_len, timeout)) {
        return DT_ERROR;
    }
#if 1
    uint8_t_data_printf(rx_buf, rx_len);
#endif
    if (DT_EOK != _mcu_board_protocol_format_check(rx_buf, rx_len))
        return DT_ERROR;
    /* 如果都正常，将包体拷贝到dest */
    if (dest) {
        mcu_board_frame_t *recv = (mcu_board_frame_t *)rx_buf;
        memcpy(dest, recv->data, dest_len);
    }

    return DT_EOK;
}

FREQ_PUBLIC int uart_mcu_board_data_reply(int cmd, void *src, unsigned int src_len)
{
    unsigned char tx_buf[4096] = {0}; /* 发送缓冲区 */
    unsigned int send_len = 0;

    /* 发送数据包头赋值 */
    void *send = (void *)tx_buf;

    /* 新协议打包 */
    /* 头 */
    ((mcu_board_frame_t *)send)->header.head = CMD_SOURCE_MPU;
    ((mcu_board_frame_t *)send)->header.cmd  = cmd;
    ((mcu_board_frame_t *)send)->header.len  = src_len + sizeof(mcu_board_frame_t) + sizeof(mcu_board_tail_t);     /* 长度包括头、包体、尾 */

    /* 尾 */
    mcu_board_tail_t *tailer = (mcu_board_tail_t *)&((mcu_board_frame_t *)send)->data[src_len];

    /* 如果有包体，需要给包体赋值 */
    if (src && src_len > 0) {
        memcpy(((mcu_board_frame_t *)send)->data, src, src_len);
    }

    /* 发送长度 */
    send_len = ((mcu_board_frame_t *)send)->header.len;

    /* 计算尾cs */
    tailer->cs = dt_compute_cs(tx_buf, send_len);

    if (DT_EOK != uart_send(SERIAL_UART9, tx_buf, send_len)) {
        return DT_ERROR;
    }
#if 1
    uint8_t_data_printf(tx_buf, send_len);
#endif
    return DT_EOK;
}

static dt_uart_mcu_board_t board;
static pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;

FREQ_PUBLIC dt_uart_mcu_board_t * dt_uart_mcu_board_get(void)
{
    return &board;
}

FREQ_PUBLIC int dt_uart_mcu_board_version_read(void *data, unsigned int len)
{
    if (board.is_exist) {
         memcpy(data, board.version, len);
         return DT_EOK;
    }

    return DT_ERROR;
}
