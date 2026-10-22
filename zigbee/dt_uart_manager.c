#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dt_uart.h"
#include "dt_uart_manager.h"
//#include "dt_misc.h"
#include "dt_heap.h"
#include <sys/epoll.h>
//#include "dt_uart_baseboard.h"
#include "proto_parse.h"

/*uart 接收最大数据包大小*/
#define UART_RECV_BUFFER_SIZE    (4096)

/* uart串口实体，在初始化时，根据uart_config进行分配初始化 */
static dt_uart_t uart_obj[SERIAL_UART_MAX] = {
    {
        .config = {
            .name       = "/dev/ttyS9",     \
            .speed      = 115200,           \
            .data_bit   = 8,                \
            .eve_bit    = 'N',              \
            .stop_bit   = 1,                \
        },
        .type       = SERIAL_UART9,
        .send_mutex = PTHREAD_MUTEX_INITIALIZER,
        .send_cond  = PTHREAD_COND_INITIALIZER,
    }
};

FREQ_PUBLIC dt_uart_t * dt_uart_get(int type)
{
    if (type >= 0 && type < SERIAL_UART_MAX)
        return &uart_obj[type];
    return NULL;
}


FREQ_PRIVATE int _uart_send_wait_recv(dt_uart_t *t,  /*串口结构体对象*/
                                  void *send_buf, unsigned int  send_len,/*发送数据缓存区及长度*/
                                  void *recv_buf, unsigned int *recv_len,/*包体回复数据缓存区及长度*/
                                  int timeout   /* 如果有接收，表示超时时间. 单位:ms */
                                  )
{
    int ret = false;        /*返回值*/
    int send_ret = -1;      /*发送返回值*/

    struct timespec now;        /* 当前时间 */
    struct timespec time;      /* 接收超时时间 */

    if (t->fd <= 0) {
        return DT_ERROR;
    }

    /* Step 1: obtain send mutex */
    pthread_mutex_lock(&t->send_mutex);

    /* Step 2: call socket to send data */
    send_ret = dt_uart_write(t->fd, send_buf, send_len);

    /*发送失败*/
    if (send_ret < 0)
        goto end;

    if (0 == timeout) {
        ret = true;
        goto end;
    }

    /* Step 3: wait event for 1 s */
    /* 等待信号量，获取到信号量*/
    clock_gettime(CLOCK_MONOTONIC, &now);
    time.tv_sec = now.tv_sec + timeout/1000;
    time.tv_nsec = now.tv_nsec + timeout%1000 * 1000*1000;
    if (pthread_cond_timedwait(&t->send_cond, &t->send_mutex, &time) != 0) {
        DT_ERR("uart wait timeout\r\n");
        goto end;
    } else {
        /* 分别给buff和长度赋值 */
        memcpy(recv_buf, t->recv_buff, t->recv_len);
        if (recv_len != NULL)
            *recv_len = t->recv_len;
    }
    ret = true;
end:
    pthread_mutex_unlock(&t->send_mutex);
    return ret;
}

FREQ_PRIVATE int _uart_send(dt_uart_t *t, void *send_buf, unsigned int  send_len)
{
    int ret = false;        /*返回值*/
    int send_ret = -1;      /*发送返回值*/


    if (t->fd <= 0) {
        return DT_ERROR;
    }

    /* Step 1: obtain send mutex */
    pthread_mutex_lock(&t->send_mutex);

    /* Step 2: call socket to send data */
    send_ret = dt_uart_write(t->fd, send_buf, send_len);

    /*发送失败*/
    if (send_ret < 0)
        goto end;
    ret = true;
end:
    pthread_mutex_unlock(&t->send_mutex);
    return ret;
}

static void *serial_thread_entry(void *parameter)
{
    pthread_detach(pthread_self());
    dt_uart_t *t = (dt_uart_t *)parameter;
    int epoll_fd = -1;
    int nfds     = 0;
    int recv_len = 0;
    struct epoll_event events[1];

     /* 创建epoll */
    epoll_fd = dt_epoll_create(1);
    if (epoll_fd < 0) {
        DT_ERR("epoll create failed\r\n");
        return NULL;
    }
    /* 加入epoll监听 */
    if (dt_epoll_add(epoll_fd, t->fd, EPOLLIN)) {
        close(epoll_fd);
        epoll_fd = -1;
	    t->deinit(t);   /* 释放资源 */
		return NULL;
    }
    while(1) {
            /* 等待数据 */
        nfds = epoll_wait(epoll_fd, events, 1, -1);
        for (int i = 0; i < nfds; i++) {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
                break;
            } else if(events[i].events & EPOLLIN) {
                //TODO: 数据处理
                //DT_DBG("get send mutex_lock\r\n");
                pthread_mutex_lock(&t->send_mutex);
                recv_len = dt_uart_read(t->fd, t->recv_buff, UART_RECV_BUFFER_SIZE-1);
                if (recv_len > 0) {
                    t->recv_len = recv_len;
                    pthread_cond_signal(&t->send_cond);
                }
                pthread_mutex_unlock(&t->send_mutex);
                //DT_DBG("release send mutex_lock\r\n");
                if (SERIAL_UART9 == t->type)
                    do_wls_sem_release();
            }
        }
    }
     if (epoll_fd > 0) {
        close(epoll_fd);
        epoll_fd = -1;
    }
    return NULL;
}



FREQ_PRIVATE void _uart_init(dt_uart_t *t)
{
    t->fd = dt_uart_open(t->config.name);
    if (t->fd < 0) {
        DT_ERR("open %s failed!\n", t->config.name);
        t->deinit(t);   /* 如果出错则反初始化释放相关资源 */
        return;
    }
    DT_PRINTF("open1\n");
    if (dt_uart_set_opt(t->fd, t->config.speed, t->config.data_bit, \
            t->config.eve_bit, t->config.stop_bit) < 0) {
        DT_ERR("uart set opt fail,%s!\n", t->config.name);
        t->deinit(t);   /* 如果出错则反初始化释放相关资源 */
        return;
    }
    DT_PRINTF("open2\n");
    /* 创建 serial 线程 */
    if (0 != pthread_create(&t->tid_uart_thread, NULL, &serial_thread_entry, t)) {
        DT_ERR("thread create fail!\r\n");
    }
    DT_PRINTF("open3\n");
}

FREQ_PRIVATE void _uart_deinit(dt_uart_t *t)
{
    /* 表示已释放，直接返回 */
	if (t == NULL)
		return;

    /* 释放接收buff缓冲区 */
    if (t->recv_buff) {
        dt_free (t->recv_buff);
        t->recv_buff = NULL;
    }
    /* 关闭打开的文件句柄 */
    if (t->fd > 0) {
        close(t->fd);
        t->fd = -1;
    }
    t = NULL;
}

FREQ_PRIVATE void do_uart_object_init(int type)
{
    dt_uart_t *t =  dt_uart_get(type);
    /* 句柄 */
    t->fd = -1;

    /*分配接收缓冲区*/
    if (NULL == t->recv_buff) {
        t->recv_buff = (unsigned char *)dt_calloc(1, UART_RECV_BUFFER_SIZE);
        if (NULL == t->recv_buff) {
            DT_ERR("malloc failed!\r\n");
            return;
        }
    }

    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&t->send_cond, &condattr);
    pthread_condattr_destroy(&condattr);

    t->recv_len = 0;

    /*回调函数赋值*/
    t->init    = _uart_init;
    t->deinit  = _uart_deinit;
    t->send_with_recv = _uart_send_wait_recv;
    t->send    = _uart_send;
    /*启动*/
    t->init(t);
}

/**
 * @brief  串口模块功能初始化
 * @note   
 * @retval None
 */
FREQ_PUBLIC void dt_uart_module_init(void)
{//TODO init uart
    int num = sizeof(uart_obj) / sizeof(uart_obj[0]);
    DT_PRINTF("uart num:%d\n", num);
    for (int i = 0; i < num; i++) {
        if (uart_obj[i].type == i)
            do_uart_object_init(i);
    }
    DT_PRINTF("dt_uart_module_init over\n");
//    dt_uart_baseboard_init();
}

FREQ_PUBLIC int uart_send_with_recv(int type, void *tx_buf, unsigned int tx_len, void *rx_buf, unsigned int *rx_len, int timeout)
{
    dt_uart_t *t =  dt_uart_get(type);

    /* 如果资源不存在，直接返回 */
    if (NULL == t)
        return DT_ERROR;

    if (tx_buf == NULL || tx_len == 0)
        return DT_EOK;

    /* 发送 */
    if (t->send_with_recv(t, tx_buf, tx_len, rx_buf, rx_len, timeout))
        return DT_EOK;

    return DT_ERROR;
}

FREQ_PUBLIC int uart_send(int type, void *tx_buf, unsigned int tx_len)
{
    dt_uart_t *t =  dt_uart_get(type);

    /* 如果资源不存在，直接返回 */
    if (NULL == t)
        return DT_ERROR;

    if (tx_buf == NULL || tx_len == 0)
        return DT_EOK;

    /* 发送 */
    if (t->send(t, tx_buf, tx_len))
        return DT_EOK;

    return DT_ERROR;
}
