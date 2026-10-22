#ifndef __COMMON_H__
#define __COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <sched.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <semaphore.h>
#include <math.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/time.h>
#include <linux/watchdog.h>
#include <sys/vfs.h>
#include <sys/mman.h>

#include "cJSON.h"
#include "base64.h"
#include "dt_heap.h"
#include "dt_crc.h"
//#include "dt_misc.h"
/* version information */
#define DT_VERSION                      (1)              /**< major version number */
#define DT_SUBVERSION                   (1)              /**< minor version number */
#define DT_REVISION                     (5)             /**< revise version number */
#define BUILD_TIME_STRING               "2024-8-30 14:48:06"   /* 这个时间也要更新 */


// #define SNESOR_DBG_PRINTF_ENABLE    //传感器调试打印
#define MQTT_TRIGGER_PARAM_ENABLE   //新增触发采集参数获取
#define NEW_EIGEN_TYPE_ENABLE       //新增指标类型
#define MULTI_DEV_MODE_DISABLE      //取消多设备模型创建
#define MULTI_EIGEN_TYPE_ENABLE     //新增多指标类型
#define FPGA_SAMPLE_ONE_FREQ_ENABLE

/* 固件名 */
#define FW_APP_NAME     "MS810_APP_V"
#define FW_FPGA_NAME    "MS810_FPGA_V"
#define FW_TECH_NAME    "ms_tech_v"
#define FW_MODBUS_NAME  "ms_rs485_v"
#define FW_AD_NAME      "ms_ad_v"
#define FW_OTA_NAME     "ms_ota_v"
#define FW_CERT_NAME    "certs.zip"
#define FW_DT_CERT_NAME    "dt_certs.zip" //同时支持certs.zip和dt_certs.zip
#define SENSOR_CONFIG_TAB_NAME  "SensorConfigTab.txt"

/* 临时目录 */
#define TEMP_FW_DIR         "/tmp"

/* 升级区目录 */
#define FW_DIR  "/userdata/fw"
#define FW_FPGA_DIR     FW_DIR"/fpga"
#define FW_TECH_DIR     FW_DIR"/tech"
#define FW_MODBUS_DIR   FW_DIR"/modbus"
#define FW_APP_DIR      FW_DIR"/app"
#define FW_AD_DIR       FW_DIR"/ad"
#define FW_OTA_DIR         FW_DIR"/ota"
#define FW_SENSOR_CFG_DIR  FW_DIR"/sensor_config_table"
#define FW_WLESS_DIR    FW_DIR"/wless"

/* 主程序执行目录 */
#define APP_EXECUTE_DIR     "/usr/bin/app"

/* FPGA执行目录 */
#define FPGA_EXECUTE_DIR     "/usr/bin/fpga"

/* 最终文件名 */
#define FPGA_EXECUTE_NAME    "ms810_fpga.rbf"
#define TECH_EXECUTE_NAME    "ms810_tech.bin"
#define MODBUS_EXECUTE_NAME  "ms810_modbus.bin"
#define AD_EXECUTE_NAME      "ms810_ad.bin"
#define OTA_EXECUTE_NAME     "ms810_ota"
#define APP_EXECUTE_NAME     "ms810"
#define SENSOR_EXECUTE_NAME  "SensorConfigTab.txt"
#define WLESS_SENSOR_CONFIG   "sensor_config"
#define WLESS_MCU             "wless_mcu.bin"

#define DEVICE_TREE_NAME     "ms_device_tree_v"
#define KERNEL_NAME          "ms_kernel_v"
#define ROOT_FS_NAME         "ms_root_fs_v"
#define UPG_DESCRIBE_FILE    "fw_description_file"  /* OTA固件升级所需描述文件 */

/* 新压缩工具路径 */
#define COMPRESS_TOOL_PATH  "/usr/bin/7za"  /* 这个路径为系统默认bin文件路径，使用此路径就不用修改环境变量 */

/* OTA描述文件内容, 以及系统区分环境变量名 */
#define NAND_BRAND_ENV_NAME     "nand_brand"
#define SYS_FW_VER_ENV_NAME     "fw_version"

#define DT_DEFAULT_SELFCHECK_TIME  (3600) /* 自诊断周期 */
#define DT_DEFAULT_EIGEN_TIME      (1)    /* 默认指标周期 */
#define DT_DEFAULT_TEMP_CALC_TIME  (10)   /* 温度周期 */
#define DT_DEFAULT_RPM_TIME        (1)    /* 默认转速周期 */
#define DT_DEFAULT_MANUAL_SAMPLE_TIME (1) /*手动采集检测周期*/
#define DT_WAVE_PERIOD_MIN         (60)   /*波形最新周期间隔时间*/
#define DT_LWAVE_PERIOD_MIN        (10*60) /*长波形最新周期间隔时间*/

#ifndef FREQ_PRIVATE
#define FREQ_PRIVATE static      /*item with scope limited to the module*/
#endif

#ifndef FREQ_PUBLIC
#define FREQ_PUBLIC              /*specifies an item of public API scope*/
#endif

#define FILE_NAME_LEN                   (128)   /* 文件名长度 */
#define INDEX_FILE                      "index.dat"

#define ArrayLength(Array) (sizeof(Array) / sizeof(Array[0]))
#define ZeroFloat  (0.000001f)//
#define IS_FLOAT_ZERO(x) ((x) > ZeroFloat ? 1 : 0)
#define DT_ABS(val1)   ((val1 >= ZeroFloat) ? (val1 - 0) : (0 - val1))

/* 内存相关 */
#define DT_MICRON_NAND_BLOAK_SIZE   (0x40000)  /* 镁光 */
#define DT_WINBOND_NAND_BLOAK_SIZE  (0x20000)  /* 华邦 */
#define DT_CRC_PACK_SIZE           (0x100000)  /* 1M */

#define USE_PAHO_MQTT_C
#define USE_WIRELESS_ENABLE         /*无线数据接入功能启用*/
#if defined USE_PAHO_MQTT_C
#define USE_MQTT_TLS_ENABLE         /* MQTT数据加密功能启用 */
#endif
#define USE_CCS_ENABLE              /*恒流源开关功能启动*/

/* WIFI调试相关 */
#define DEBUG_TIME_OUT_COUNT    (1800)
#define DEBUG_SLEEP_TIME        (5)

/**
 * @brief 判断是否进入调试模式，进入则休眠
 * 该宏判断是否进入调试模式，进入则会令调用该宏的线程休眠，休眠时间与Debug时间保持一致
 */
#define DEBUG_ENTER_CHECK \
do { \
    static int is_first = 1; \
    while (SYS_DBG == sys_status_get()) { \
        if (is_first) { \
            is_first = 0; \
            DT_LOG("system DBG ==>sleep.\r\n"); \
        } \
        sleep(DEBUG_SLEEP_TIME); \
    } \
    is_first = 1; \
} while (0)

/* MS810默认从2开始, 2：v2.1 */
typedef enum _main_board_hw_type_def {
    HW_VERSION_0  = 0x00,   /* ms800使用 */
    HW_VERSION_1,           /* ms800使用 */
    HW_VERSION_2,           /* ms810默认版本V2.1 */
}main_board_hw_type_t;

//enum {
//    false	= 0,
//    true	= 1
//};

enum {
    DT_HIGH_FREQ    = 0,
    DT_LOW_FREQ,
    DT_MAX_FREQ,
};

#define HIGH_FREQ DT_HIGH_FREQ
#define LOW_FREQ  DT_LOW_FREQ

enum {
    SAMPLE_MODE_REGULAR    = 1,  /* 定时采集模式 */
    SAMPLE_MODE_CONTINUE,        /* 连续采集模式 */
    SAMPLE_MODE_TRIGGER    = 6,  /* 触发采集模式 */
    SAMPLE_MODE_MAX,
};

enum {
    DT_DISABLE = 0,
    DT_ENABLE,
};

#define IS_ENABLE_CHECK(x) ((x) ==  DT_ENABLE ? DT_ENABLE : DT_DISABLE)

enum {
    THRESHOLD_VALUE_L1   = 0x01,
    THRESHOLD_VALUE_L2,
    THRESHOLD_VALUE_L3,
    THRESHOLD_VALUE_L4,
};

/*error code definitions */
#define DT_ERROR                        0               /**< A generic error happens */
#define DT_EOK                          1               /**< There is no error */
#define DT_ETIMEOUT                     2               /**< Timed out */
#define DT_EFULL                        3               /**< The resource is full */
#define DT_EEMPTY                       4               /**< The resource is empty */
#define DT_ENOMEM                       5               /**< No memory */
#define DT_ENOSYS                       6               /**< No system */
#define DT_EBUSY                        7               /**< Busy */
#define DT_EIO                          8               /**< IO error */
#define DT_EINTR                        9               /**< Interrupted system call */
#define DT_EINVAL                       10              /**< Invalid argument */

#define DT_FILE_SPACE_CHECK      /* 开始存储文件检查功能*/

int bind_thread_to_core(pthread_t thread, int core);
int get_cmd_result(const char *cmd,  char *results);
int dt_epoll_create(int size);
int dt_epoll_add(int epoll_fd, int target_fd, unsigned int events);
int current_time_check(char *t);
int testdir(const char *path);
void dt_current_time_get(char *t);
void dt_get_sys_time(char *t);
FREQ_PUBLIC int dt_sys_sync_to_rtc(void);
FREQ_PUBLIC int dt_rtc_sync_to_sys(void);
void dt_set_sys_time(int *time);
int dt_system(const char *cmd_line);
int dt_read(int fd, void *buf, unsigned int len);
int dt_write(int fd, const void *buf, unsigned int len);
int dt_write_check(int fd, const void *buf, unsigned int len);
void dt_log(const char *opt, const char *func, int line, const char *fmt, ...);
int dt_create_dir(const char *dirname);
int dt_delete_dirfile(const char *filename);
int dt_get_file_size(const char *file);
void uint8_t_data_printf(unsigned char *data, int len);
unsigned int dt_second_tick_get(void);
unsigned char dt_compute_cs(unsigned char *pdata, unsigned int len);
unsigned char dt_cusum(unsigned char *pdata, unsigned long len);
FREQ_PUBLIC void s16_swap(short *pdata, unsigned int len);
int toInt(char *str);
int toString(int x, char *buf, int len);
int toIp(uint8_t *ip, char *buf, int len);
int fromIp(uint8_t *ip, char *buf);
int toMac(uint8_t *mac, char *buf, int len);
int fromMac(uint8_t *mac, char *buf);
FREQ_PUBLIC int dt_system_cmd_match(const char *cmd, const char *parttern);

#define DT_DBG(fmt, ...)  dt_log("DBG", __func__, __LINE__, fmt, ##__VA_ARGS__)
#define DT_LOG(fmt, ...)  dt_log("LOG", __func__, __LINE__, fmt, ##__VA_ARGS__)
#define DT_ERR(fmt, ...)  dt_log("ERR", __func__, __LINE__, fmt, ##__VA_ARGS__)
#define DT_WARN(fmt, ...) dt_log("WARN", __func__, __LINE__, fmt, ##__VA_ARGS__)
#define DT_PRINTF(fmt, ...) dt_log("PRINTF", __func__, __LINE__, fmt, ##__VA_ARGS__)

#define DT_WLS_LOG(fmt, ...)  dt_log("WLS_LOG", __func__, __LINE__, fmt, ##__VA_ARGS__)
#define DT_WLS_ERR(fmt, ...)  dt_log("WLS_ERR", __func__, __LINE__, fmt, ##__VA_ARGS__)

void short_to_float_t_data_printf(short *data, int len, float coef);
void int_to_float_t_data_printf(int *data, int len, float coef);
void float_to_float_t_data_printf(float *data, int len, float coef);
FREQ_PUBLIC int bzip2_compress(const char*filename);
FREQ_PUBLIC int encryption_compress(const char *filename);
FREQ_PUBLIC int dt_df_check(const char *path, long long *size);
FREQ_PUBLIC int dt_param_content_sync(const char *src, const char *dst);
FREQ_PUBLIC void dt_file_dir_init(void);


#define SNESOR_DBG_PRINTF_ENABLE
#ifdef __cplusplus
}
#endif

#endif
