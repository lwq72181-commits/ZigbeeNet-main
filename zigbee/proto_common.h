#ifndef __PROTO_COMMON_H__
#define __PROTO_COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "proto_parse.h"
//#include "dt_mqtt_protocol.h"
//#include "dt_param.h"
#include "zigbeetype.h"
#define USE_MQTT  1

#define OLD_PATH	"/home/root/"
#define NEW_PATH	"/usrdata/"
#define OLD_APP		"/usr/bin/gateway"
#define NEW_APP		"/usrdata/gateway"

#define FILE_RH605_UPG_NAME         "WS_"
#define FILE_WS2_NAME				"WS2_V"
#define FILE_WS20_NAME				"WS20_V"
#define FILE_WS21S_NAME				"WS21S_V"

#define FILE_MPU_RS485_NAME         "SG_RS485_V"
#define FILE_RH560_WIFI_UPG_NAME    "RH560_WIFI_V"
#define FILE_MCU_UPG_NAME           "SG-MCU_"
#define FILE_MPU_APP_NAME           "SG_MPU_V"
// #define FILE_MPU_FS_NAME            "SG_MPU_FS_V"
#define FILE_MPU_FS_NAME            ".txt"

#define UART_SIZE_PER_PKT               (2048)
#define UART_RECV_LEN                   (32 * 1024)
#define WLESS_SENSOR_CONFIG_PATH    "/userdata/wless_sensor_config"


#define TEMP_DOWNLOAD_DIR    "/usrdata/app/temp_download_dir/"              //从上位机下载的临时目录
#define STATION_SERVER_DIR   "/usrdata/app/DownloadFileDir/"                //从上位机下载的文件存放目录
#define WLESS_DATA_DIR       "/usrdata/wless/data/"                         //存放采集站相关参数及数据
#define DATA_INDEX          "Data.index"

#define PARAMETER_FILE      "Parameter"           //采集站参数目录, 目录中的文件包括在线服务参数类指令获取的参数文件以及解析之后文件param.txt
#define WAVEDATA_FILE       "WaveData"            //采集站波形目录
#define FEATUREDATA_FILE    "FeatureData"         //采集站特征量数据目录
#define EIGENDATA_FILE      "EigenData"           //采集站指标数据目录
#define INFODATA_FILE       "InfoData"            //传感器日志信息
#define BATTERY_INDEX_FILE  "SensorBatteryData"   //传感器电池信息

#define UART_ACK_BUF_LEN             (32 * 1024)

enum {
	PLATFORM_NONE = 0,
	PLATFORM_EC20,
	PLATFORM_EG25,
};

enum {
    TASK_CAN_NOT_RUN    = 0x0,
    TASK_CAN_RUN        = 0x1
};

enum {
    MPU_STATE_IDLE = 0,
    MPU_STATE_BUSY,
    MPU_STATE_WIFI_CONFIG,
    MPU_STATE_WIFI_CONFIG_ONGOING,
    MPU_STATE_NEED_REBOOT
};

enum {
    TASK_SUCCESS                = 0x0,
    TASK_RUNNING                = 0x1,
    TASK_WIFI_START_FAIL        = 0x10,
    TASK_4G_START_FAIL          = 0x20,
    TASK_CONNECT_SERVER_FAIL    = 0x40,
    TASK_STORE_DATA_FAIL        = 0x80,
    TASK_4G_SIGNAL_LOW          = 0x100,
    TASK_UART_FAIL              = 0x200,
    TASK_EMMC_INIT_FAIL         = 0x400
};

enum {
    FILE_UPGRADE_RH605          = 0x0,
    FILE_UPGRADE_MPU_RS485      = 0x1,
    FILE_UPGRADE_RH560_WIFI     = 0x2,
    FILE_UPGRADE_MCU            = 0x3,
    FILE_UPGRADE_MPU            = 0x4,
    FILE_SENSOR_CONFIG          = 0x5
};

typedef struct wless_param_def {
    uint8_t param_time[7];
    uint8_t param_flag;
    uint8_t index_period;
    uint8_t measdef_period;
    uint8_t station_num;

}wless_param_t;

typedef struct wless_data_def {
    uart_0x34_indicator_data_t real_eigen[MAX_WLS_VIB_CH_NUM];
    uart_cmd_0x41_t trend_data[MAX_WLS_VIB_CH_NUM];
}wless_data_t;

void set_cur_task_type(unsigned char type);
unsigned char get_cur_task_type(void);
void set_cur_task_state(unsigned char state);
unsigned char get_cur_task_state(void);
wless_param_t *get_wless_param(void);
wless_data_t *get_wless_data(void);
unsigned short get_task_run_state(void);
unsigned char get_all_task_run_state(void);
void set_task_run_state(unsigned short state);
int check_used_capacity(void);
int get_platform(void);
int CreateDir(const char *sPathName);
int deleteDir(const char *dirName);
void mpu_notify_mcu(void);
int dt_check_time(char *time);
unsigned int ComputeCRC32(unsigned char *buf, unsigned int len);
int mpu_set_seneor_config(uint8_t *ps_info);
int mpu_get_seneor_config(wls_sensor_info_t *ps_info);
int get_mpu_config_mcu_flag(void);
void set_mpu_config_mcu_flag(int flag);
int mpu_set_sensor_info();
int mpu_set_sensor_info_select(int select);
int refresh_sensor_info();
int refresh_sensor_file_info();
int mpu_set_sensor_info_to_mcu(uint8_t *ps_info);
int mpu_get_sensor_info_form_mcu(wls_sensor_info_t *ps_info);
int mpu_get_param_info_form_mcu(void *arg);
int mpu_set_param_info_to_mcu(char *ps_info);
int mpu_set_param_info();
int mpu_set_param_info_Band(int zgbFreqBand);
int dt_check_spi_nor_module(void);
int read_wsl_syspara();
int config_zgb_syspara();
#ifdef __cplusplus
}
#endif
#endif
