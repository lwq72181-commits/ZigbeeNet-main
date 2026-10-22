#ifndef _DT_UART_MCU_BOARD_H
#define _DT_UART_MCU_BOARD_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "proto_parse.h"

typedef enum _debug_task_type_def{
    TASK_TYPE_READ_PARAM = 0x01,    // 读取网关参数
    TASK_TYPE_SET_PARAM,            // 设置网关参数
    TASK_TYPE_READ_SENSOR_INFO,     //读取传感器绑定信息
    TASK_TYPE_SET_SENSOR_INFO,      //设置传感器绑定信息
    TASK_TYPE_START_SAMPLE,         //临时采集
    TASK_TYPE_SAMPLE_QUERY,         //临时采集状态查询
    TASK_TYPE_TEND_SAMPLE,          //趋势采集
    TASK_TYPE_DL_FILE_INFO,         //下达升级文件信息
    TASK_TYPE_DL_FILE_DATA,         //下达升级文件内容
}debug_task_type_t;


typedef enum sys_net_type_def{
    WIRELESS_TYPE_4G    = 0x00,
    WIRELESS_TYPE_WIFI   = 0x01,
    WIRELESS_TYPE_ETH    = 0x02,
}sys_net_type_t;

typedef struct wls_dbg_param_info_def{ //配置系统参数
    //系统参数结构体
    char stationCode[8];
    char stationSn[16];
    char mainBordVer[16];
    char mcuSoftVer[16];
    char mpuSoftVer[16];
    char commMode; //通讯方式，0x00-4G; 0x01-wifi; 0x02-eth
    uint8_t proto_ver;     //协议版本, 0-老协议V1.0；1-新协议V1.1
    uint8_t res1[62];
    //WIFI参数结构体
    uint8_t wifiSrcIp[4]; //本机IP
    uint8_t wifiDesIp[4]; //服务器IP
    uint8_t wifiMask[4];  //子网掩码
    uint8_t wifiGate[4];  //网关
    uint16_t wifiPort;    //端口
    uint8_t wifiMac[6];   //MAC地址
    char    wifiSSID[16];
    char    wifiKey[16];
    uint8_t res2[64];
    //以太网参数结构体
    uint8_t netSrcIp[4]; //本机IP
    uint8_t netDesIp[4]; //服务器IP
    uint8_t netMask[4];  //子网掩码
    uint8_t netGate[4];  //网关
    uint16_t netPort;    //端口
    uint8_t netMac[6];  //MAC地址
    uint8_t res3[64];
    //4G参数结构体
    uint8_t  g4DesIp[4];
    uint16_t g4Port;
    char     g4Apn[32];
    char     g4Dns[32];
    uint8_t  res4[64];
    //NB/LORA参数结构体
    uint8_t  nbDesIp[4];
    uint16_t nbPort;
    char     nbImei[16];
    uint8_t  res5[64];
    //Zigbee参数结构体
    uint8_t  zgbFreqBand;
    uint8_t  zgbTxpower;
    uint8_t  res6[16];
}__attribute__((packed))wls_dbg_param_info_t;

typedef struct {
    unsigned char           TaskType;
    wls_dbg_param_info_t    GwParam;
}__attribute__((packed)) mcu_cmd_0x40_0x02_req_t, mcu_cmd_0x40_0x01_rsp_t;

typedef struct {
    unsigned char           TaskType;
    uart_cmd_ret_sensor_info_t Info[MAX_SENSOR_CNT];
}__attribute__((packed)) mcu_cmd_0x40_0x04_req_t, mcu_cmd_0x40_0x03_rsp_t;

typedef struct {
    unsigned char           TaskType;
    unsigned char           State;  // 0, fail; 1, ok
}__attribute__((packed)) mcu_cmd_0x40_0x04_rsp_t, mcu_cmd_0x40_0x02_rsp_t;

typedef struct {
    unsigned char           TaskType;
}__attribute__((packed)) mcu_cmd_0x40_0x03_req_t, mcu_cmd_0x40_0x01_req_t;

typedef struct {
    unsigned char           McuTime[7];
    char                    GatewayCode[8];
    /* wifi parameter */
    char                    WifiSsid[32];
    char                    WifiKey[32];
    unsigned char           WifiSrcIP[4];
    unsigned char           WifiDestIP[4];
    unsigned short          WifiPort;
    unsigned char           WifiMask[4];
    unsigned char           WifiGate[4];
    /* 4g parameter */
    unsigned char           PbDestIP[4];
    unsigned short          PbPort;
    char                    APN[8];

    char                    McuVersion[10];
    char                    G4Apn[16];   //4g apn
    unsigned short          rs485_device_id;
    char                    station_type;
#ifdef DOTON
    char                    FirstPowerOn;
    unsigned char           EthMac[6];
    unsigned char           ProtoVer; // 0-old proto, 1-new proto
    char                    Rsvd[37];
#else
    char                    Rsvd[45];
#endif
}__attribute__((packed)) mcu_cmd_0x01_rsp_t;

typedef struct {
    unsigned char           MpuState;
    char                    MpuVersion[10];
}__attribute__((packed)) mcu_cmd_0x01_req_t;

typedef struct dt_uart_mcu_board_def dt_uart_mcu_board_t;
struct dt_uart_mcu_board_def {
    char is_exist;       /*存在标志位，0表示底板不存在，1表示存在*/
    char version[16];  /* 无线版本信息 */
    unsigned char freq_band; /* 频段参数*/
};

FREQ_PUBLIC dt_uart_mcu_board_t * dt_uart_mcu_board_get(void);
FREQ_PUBLIC int uart_mcu_board_data_trx(int cmd, void *src, unsigned int src_len, void *dest, unsigned int dest_len, int timeout);
FREQ_PUBLIC int uart_mcu_board_data_reply(int cmd, void *src, unsigned int src_len);
FREQ_PUBLIC int dt_uart_mcu_board_version_read(void *data, unsigned int len);
#ifdef __cplusplus
}
#endif
#endif // !_DT_UART_MCU_BOARD_H
