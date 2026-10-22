
#include "common.h"
#include "proto_common.h"
#include "proto_parse.h"
#include "proto_handle.h"

static sem_t uart_notice_wless_sem;
static sem_t wless_sem_dispose;
static int is_init = DT_DISABLE;

FREQ_PUBLIC void do_wls_sem_release(void)
{
    if (DT_DISABLE == is_init) {
        sem_init(&uart_notice_wless_sem, 0, 0); /* uart4接收数据信号 */
        is_init = DT_ENABLE;
    }
    sem_post(&uart_notice_wless_sem);
}

FREQ_PUBLIC void do_wls_sem_wait(void)
{
    if (DT_DISABLE == is_init) {
        sem_init(&uart_notice_wless_sem, 0, 0);
        is_init = DT_ENABLE;
    }
    sem_wait(&uart_notice_wless_sem);
}

int uart_msg_check(uint8_t *pmsg, uint32_t msg_len)
{
    uint8_t compute_cs = 0;
    uart_proto_head_t *pheader = (uart_proto_head_t *)pmsg;
    if (pheader->head != CMD_SOURCE_MCU) {
        DT_WLS_ERR("head 0x%.x != 0x%.x\r\n", pheader->head, CMD_SOURCE_MCU);
        return -1;
    }

    if (pheader->len != msg_len) {
        DT_WLS_ERR("len 0x%.x != 0x%.x\r\n", pheader->len, msg_len);
        return -1;
    }

    compute_cs = dt_compute_cs(pmsg, msg_len-1);
    if (pmsg[msg_len-1] != compute_cs) {
        DT_WLS_ERR("cs 0x%.x != 0x%.x\r\n", pmsg[msg_len-1], compute_cs);
        return -1;
    }
    return 0;
}

static uart_cmd_handle_t uart_cmd_handle[] = {
    {UART_CMD_COMMUNICATION, do_regular_commun_handle},
    {UART_CMD_INTERNAL_TASK, inter_task_handle},
    {UART_CMD_WIRELESS_TASK, wireless_task_handle},
    {UART_CMD_GET_TASK_STATE, get_task_state_handle},
    // {UART_CMD_NOTIFY_MPU_UPGRADE, notify_mpu_upgrade_handle},
    // {UART_CMD_NOTIFY_MPU_RS485_UPGRADE, notify_mpu_rs485_upgrade_handle},
    {UART_CMD_REQ_GET_FILE, req_get_file_handle},
    {UART_CMD_GET_FILE, get_file_handle},
    {UART_CMD_UPLOAD_REAL_EIGEN, upload_real_eigen_handle},
    {UART_CMD_SPI_FLASH_INFO, spi_flash_info_handle},
    // {UART_CMD_HW_SELF_CHECK, hw_self_check_handle},
    // {UART_CMD_HW_CHECK_QUERY, hw_check_query_handle},
#ifdef DOTON
    //{UART_CMD_DEBUG_TASK, debug_task_handle},   //TODO
    {UART_CMD_UPLOAD_TREND, upload_trend_handle},
    {UART_CMD_UPLOAD_UPGRADE_STATE, upload_upgrade_state_handle},
#endif
    {UART_CMD_UPLOAD_LOG, upload_log_handle}
};


int uart_msg_handle(int cmd, uint8_t *pdata, uint32_t data_len)
{
    for (uint32_t i = 0; i < sizeof(uart_cmd_handle)/sizeof(uart_cmd_handle_t); i++) {
        if (cmd == uart_cmd_handle[i].cmd_type) {
            if (uart_cmd_handle[i].handle == NULL) {
                return -1;
            }
            return uart_cmd_handle[i].handle(pdata, data_len);
        }
    }

    DT_WLS_ERR("cmd: 0x%.x error !\r\n", cmd);
    return -1;
}

static uint32_t dispose_len;
static uint8_t dispose_data[8192];
static uart_proto_head_t dispose_header;
static ZIGBEEParam *g_ZIGBEEParam;

ZIGBEEParam *getSharedData()
{
    return g_ZIGBEEParam;
}

void *msg_handle_thread(void *param)
{
    uint32_t data_len = 0;
    uint8_t data[8192] = {0};
    uart_proto_head_t header;
    uint32_t head_offset = sizeof(uart_proto_head_t);
    uint32_t tail_offset = sizeof(uart_proto_tail_t);
    dt_uart_t * puart = dt_uart_get(WIRELESS_UART);

    while (1) {
        do_wls_sem_wait();

        /* do not handle when debug mode */
//        if (get_mpu_config_mcu_flag()) {
//            continue;
//        }

        // uint8_t_data_printf(puart->recv_buff, printf_len);
#ifdef SNESOR_DBG_PRINTF_ENABLE
        printf("-----rec buf(len:%d):\r\n", puart->recv_len);
        int printf_len = puart->recv_len;
//        if (printf_len > puart->recv_len) {
//            printf_len = puart->recv_len;
//        }

        for (uint32_t i = 0; i < printf_len; i++) {
            printf("%02x ", puart->recv_buff[i]);
            if ((i + 1) % 32 == 0) {
                printf("\r\n");
            }
        }
        printf("\r\n");
#endif
        int i=0;
        while((printf_len-i)>4)
        {
            uart_proto_head_t *pheader = (uart_proto_head_t *)&puart->recv_buff[i];
            if (pheader->head != CMD_SOURCE_MCU) {
                i++;
            }
            else
            {
                if (!uart_msg_check(&puart->recv_buff[i], pheader->len)) {
                    memcpy(&header, &puart->recv_buff[i], head_offset);
                    data_len = pheader->len - head_offset - tail_offset;
                    memcpy(&data, &puart->recv_buff[i] + head_offset, data_len);

                    //如果让读取flash数据，则单独去处理，防止MCU问状态时，没有回复
                    if (UART_CMD_SPI_FLASH_INFO == header.cmd) {
                        memcpy(&dispose_header, &header, head_offset);
                        dispose_len = data_len;
                        memcpy(dispose_data, data, data_len);
                        sem_post(&wless_sem_dispose);
                        i+=pheader->len;
                        //printf_len-=i;
                        continue;
                    }
                    uart_msg_handle(header.cmd, data, data_len);
                }
                i+=pheader->len;

            }
//            printf_len-=i;
        }
        usleep(10);
    }
}

void *msg_dispose_handle_thread(void *param)
{
    uint32_t offset = sizeof(uart_proto_head_t);
    dt_uart_t * puart = dt_uart_get(WIRELESS_UART);

    while (1) {
        sem_wait(&wless_sem_dispose);
        // DT_WLS_LOG("spi_flash_read_start.\r\n");

        //设置当前任务为执行状态
        set_cur_task_state(TASK_RUNNING);

        uart_msg_handle(dispose_header.cmd, dispose_data, dispose_len);

        //处理完后，清除数据
        memset(dispose_data, 0x00, sizeof(dispose_data));
        memset(&dispose_header, 0x00, sizeof(dispose_header));
        dispose_len = 0;

        //设置当前任务为成功状态
        set_cur_task_state(TASK_SUCCESS);
        // DT_WLS_LOG("spi_flash_read_end.\r\n");
        usleep(10);
    }
}

FREQ_PUBLIC int dt_wireless_access_init(ZIGBEEParam * Param)
{
    pthread_t pid_wireless, pid_wless_dispose;
    //g_sensor_info = Param->sensor_info;
    g_ZIGBEEParam = Param;
    //DT_PRINTF("m_ZIGBEEParam:%d,sensor:%d",g_ZIGBEEParam,g_ZIGBEEParam->sensor_info);
    DT_PRINTF("refresh_sensor_info");
    refresh_sensor_info();
//    DT_PRINTF("refresh_sensor_info over");


    sem_init(&wless_sem_dispose, 0, 0);

    read_wsl_syspara();
//    dt_config_param_t *pparam = get_board_config_param();

//    if (!pparam->wireless) {
//        DT_LOG("wireless function is not enabled.\r\n");
//        return DT_ERROR;
//    }
    DT_LOG("msg_handle_thread.\r\n");
    if(0 != pthread_create(&pid_wireless, NULL, msg_handle_thread, NULL)) {
        DT_WLS_ERR("msg handle thread crteate fail!\r\n");
        return DT_ERROR;
    }
    DT_LOG("msg_dispose_handle_thread.\r\n");
    if(pthread_create(&pid_wless_dispose, NULL, msg_dispose_handle_thread, NULL)) {
        DT_WLS_ERR("wless dispose handle thread crteate fail!\r\n");
        return DT_ERROR;
    }
    DT_LOG("dt_wireless_access_init over.\r\n");
    return DT_EOK;
}
