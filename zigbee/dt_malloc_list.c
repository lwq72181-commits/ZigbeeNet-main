#include "dt_malloc_list.h"
#include "dt_heap.h"
#include "common.h"
#include "zigbeetype.h"
//#include "dt_mqtt_protocol.h"



static unsigned char *modbus_tcp_read_buffer = NULL;
FREQ_PUBLIC unsigned char *modbus_tcp_read_buffer_get(unsigned int length)
{
    const int max_size = 8*1024;

    if (length > max_size) {
        DT_ERR("length (%d )> MAX_CALC_LENGTH  %d \r\n", length, max_size);
        return NULL;
    }

    if (NULL == modbus_tcp_read_buffer) {
        modbus_tcp_read_buffer = (unsigned char*)dt_calloc(1,  max_size);
        if (NULL == modbus_tcp_read_buffer) {
            DT_ERR("malloc failed %d\r\n");
            return NULL;
        }
    }

    memset(modbus_tcp_read_buffer, 0x00, max_size *sizeof(unsigned char));
    return modbus_tcp_read_buffer;
}

/**
 * @brief tcp数据上传静态缓冲区
 *
 */
static unsigned char *tcp_up_buf = NULL;
FREQ_PUBLIC unsigned char *tcp_up_buf_get(int size)
{
    const int max_size = 32*1024;

    if (size > max_size) {
        DT_ERR("tcp_up_buf_get failed.\r\n");
        return NULL;
    }

    if (NULL == tcp_up_buf) {
        tcp_up_buf = (unsigned char *)dt_calloc(1, max_size);
        if (NULL == tcp_up_buf) {
            DT_ERR("malloc failed\r\n");
            return NULL;
        }
    }

    memset(tcp_up_buf, 0, max_size);
    return (unsigned char *)tcp_up_buf;
}

static char *ver_temp_buf[2] = {NULL};
FREQ_PUBLIC char *ver_temp_buf_get(int index, int size)
{
#define TEMP_BUFFER_SIZE (4*1024*1024)

    if (size > TEMP_BUFFER_SIZE) {
        DT_ERR("size(%d) > %d\r\n", size, TEMP_BUFFER_SIZE);
        return NULL;
    }

    if (NULL == ver_temp_buf[index]) {
        ver_temp_buf[index] = (char *)dt_calloc(1, TEMP_BUFFER_SIZE);
        if (NULL == ver_temp_buf[index]) {
            DT_ERR("malloc failed\r\n");
            return NULL;
        }
    }

    memset(ver_temp_buf[index], 0, TEMP_BUFFER_SIZE);
    return ver_temp_buf[index];
}


/**
 * @brief 音频采集存放两个声道的临时buff
 *
 */
static char *audio_temp_buf = NULL;
FREQ_PUBLIC char *audio_temp_buf_get(unsigned int size)
{
    const int max_size = 9*1024*1024;

    if (size > max_size) {
        DT_ERR("buf get fail, size(%d)\r\n", max_size);
        return NULL;
    }

    if (NULL == audio_temp_buf) {
        audio_temp_buf = (char *)dt_calloc(1, max_size);
        if (NULL == audio_temp_buf) {
            DT_ERR("audio malloc failed\r\n");
            return NULL;
        }
    }

    memset(audio_temp_buf, 0x00, max_size);
    return audio_temp_buf;
}

/**
 * @brief 音频获取上传缓冲区
 *        该函数返回一个指向无符号字符类型的指针，用于上传操作。
 *        这里使用的buff有两种 0:普通音频, 1:报警音频
 *
 * @return 指向上传缓冲区的指针，如果内存分配失败则返回 NULL
 */
static char *audio_ul_buf = NULL;
FREQ_PUBLIC char *audio_upload_buf_get(unsigned int size)
{
    const int max_size = 9*1024*1024;

    if (size > max_size) {
        DT_ERR("buf get fail, size(%d)\r\n", max_size);
        return NULL;
    }

    if (NULL == audio_ul_buf) {
        audio_ul_buf = (char *)dt_calloc(1, max_size);
        if (NULL == audio_ul_buf) {
            DT_ERR("audio malloc failed\r\n");
            return NULL;
        }
    }

    memset(audio_ul_buf, 0x00, max_size);
    return audio_ul_buf;
}

/**
 * @brief 存放两个声道的临时buff
 *
 */
static short *db_calc_buf[MAX_AUDIO_CH_NUM] = {NULL};
FREQ_PUBLIC short *db_calc_buf_get(int index, unsigned int size)
{
    const int max_size = 5*1024*1024;

    if ((size > max_size) || (index < 0 || index >= 2)) {
        DT_ERR("buf get fail, size(%d)\r\n", max_size);
        return NULL;
    }

    if (NULL == db_calc_buf[index]) {
        db_calc_buf[index] = (short *)dt_calloc(1, max_size);
        if (NULL == db_calc_buf[index]) {
            DT_ERR("audio malloc failed\r\n");
            return NULL;
        }
    }

    memset(db_calc_buf[index], 0x00, max_size);
    return db_calc_buf[index];
}

/* 无线指标读取 */
static char *wless_eigen_buf[3] = {NULL};
FREQ_PUBLIC char *wless_eigen_buf_get(int index, unsigned int size)
{
    const int max_size = 1024*1024;

    if (size > max_size) {
        DT_ERR("buf get fail, size(%d)\r\n", max_size);
        return NULL;
    }

    if (NULL == wless_eigen_buf[index]) {
        wless_eigen_buf[index] = (char *)dt_calloc(1, max_size);
        if (NULL == wless_eigen_buf[index]) {
            DT_ERR("audio malloc failed\r\n");
            return NULL;
        }
    }

    memset(wless_eigen_buf[index], 0x00, max_size);
    return wless_eigen_buf[index];
}
