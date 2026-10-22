#ifndef __PROTO_PARSE_H__
#define __PROTO_PARSE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "dt_uart_manager.h"
#include "zigbeetype.h"
#include "wless_param.h"
#define WIRELESS_UART       (SERIAL_UART9)

#define UART_BUF_LEN         4096
#define CMD_SOURCE_MCU       0x55    // MCU发送
#define CMD_SOURCE_MPU       0xAA    // MPU发送

#define DOTON

/* uart cmd */
#define UART_CMD_COMMUNICATION          0x1
#define UART_CMD_INTERNAL_TASK          0x20
#define UART_CMD_WIRELESS_TASK          0x21
#define UART_CMD_GET_TASK_STATE         0x22
#define UART_CMD_REQ_UPGRADE            0x10
#define UART_CMD_GET_UPGRADE_PEOGRAM    0x11
#define UART_CMD_NOTIFY_MPU_UPGRADE     0x12
#define UART_CMD_NOTIFY_MPU_RS485_UPGRADE     0x13

#define UART_CMD_REQ_GET_FILE           0x31
#define UART_CMD_GET_FILE               0x32
#define UART_CMD_REQ_UPLOAD_DATA        0x33
#define UART_CMD_UPLOAD_REAL_EIGEN      0x34
#define UART_CMD_UPLOAD_STATE           0x35
#define UART_CMD_SPI_FLASH_INFO         0x36
#define UART_CMD_HW_SELF_CHECK          0x37
#define UART_CMD_HW_CHECK_QUERY         0x38
#ifdef DOTON
#define UART_CMD_DEBUG_TASK             0x40
#define UART_CMD_UPLOAD_TREND		    0x41
#define UART_CMD_UPLOAD_UPGRADE_STATE	0x42
#endif

#define UART_CMD_UPLOAD_LOG	            0x43
/*****************uart proto struct ***********/
typedef struct {
    unsigned char               head; /*命令头*/
    unsigned char               cmd;  /*命令码*/
    unsigned short              len;  /*长度*/
}__attribute__((packed)) uart_proto_head_t;


/* 协议数据帧格式: 头和数据 */
typedef struct _mcu_board_frame_def{
    uart_proto_head_t   header;
    unsigned char      data[0];
} __attribute__((packed))uart_proto_frame_t, mcu_board_frame_t;

typedef struct {
    unsigned char               cs;
}__attribute__((packed)) uart_proto_tail_t, mcu_board_tail_t;

#ifdef DOTON
#define FIRST_POWER_ON  1
#endif

/* cmd: 0x01 */
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
}__attribute__((packed)) uart_cmd_0x01_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           MpuState;
    char                    MpuVersion[10];
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x01_ret_t;

/* cmd: 0x20 */
#define TASK_GET_SYSTEM_TIME        0x01
#define TASK_GET_560G_PARAMETER     0x02
#define TASK_GET_UPGRADE_VERSION    0x03
#define TASK_GET_560_PARAMETER      0x04
#define TASK_READY_SHUTDOWN         0x05
#define TASK_SAVE_SENSOR_CONFIG     0x06

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           RunState;
    unsigned char           SystemTime[7];
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x20_get_system_time_ret_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           RunState;
    unsigned int            IndexPeriod;
    unsigned int            MeasDefPeriod;
    unsigned char           StationNum;
    char                    StationCode[10][8];
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x20_get_560g_parameter_ret_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           RunState;
    char                    MpuProgramName[32];
    char                    McuProgramName[32];
    char                    MpuRs485ProgramName[32];
    char                    StationWifiProgramName[32];
    char                    SensorProgramName[32];
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x20_get_upgrade_version_ret_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           RunState;
    unsigned char           StationParamTime[7];
    unsigned char           DeviceState;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x20_get_560_parameter_ret_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           RunState;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x20_ready_shutdown_ret_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           RunState;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x20_save_sensor_config_ret_t;


/* cmd: 0x21 */
#define TASK_WIFI_CHECK             0xFF
#define TASK_4G_CHECK               0xFE
#define TASK_RECV_DEBUG_FILE        0xFD
#define TASK_WIFI_SIGNAL_DETECT     0xFC
#define TASK_4G_SIGNAL_DETECT       0xFB
#define TASK_GET_PARAMETER          0x01
#define TASK_GET_UPGRADE_FILE       0x02
#define TASK_RECV_STATION_DATA      0x04
#define TASK_REQ_PUT_FILE           0x08
#define TASK_UPLOAD_DATA            0x10
#define TASK_UPLOAD_EIGEN           0x20
#define TASK_CONNECT_ONLINE         0x40

typedef struct {
    unsigned char           TaskType;
    unsigned char           WirelessType;
    char                    StationCode[8];
}__attribute__((packed)) uart_cmd_0x21_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    char                    StationCode[8];
    unsigned char           TaskState;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x21_ret_t;

/* cmd: 0x22 */
typedef struct {
    char                    StationCode[8];
}__attribute__((packed)) uart_cmd_0x22_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned short          RunState;
    char                    StationCode[8];
    char                    SignalStrength;
    float                   TransferRate;
    unsigned char           LossRate;
    unsigned int            costtime;   //发送数据所用时间, 单位: ms
    unsigned char           wificonnectstate;  //wifi连接状态, 1-wifi连接成功；0-失败
    unsigned char           tcpconnectstate;   //tcp连接状态,  1-tcp连接成功；0-失败
    unsigned char           headconnectstate;  //头命令发送状态, 1-head命令发送成功；0-失败
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x22_ret_t;

/* cmd: 0x10 */
typedef struct {
    uart_proto_head_t       header;
    unsigned int            McuUpgradeSize;
    unsigned int            McuUpgradeCrc;
    unsigned int            McuUpgradeTotalPacket;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x10_ret_t;

/* cmd: 0x11 */
typedef struct {
    unsigned int            CurPacket;
}__attribute__((packed)) uart_cmd_0x11_t;

/* cmd: 0x12 */
typedef struct {
    uart_proto_head_t       header;
    unsigned char           UpgradeState;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x12_ret_t;

/* cmd: 0x13 */
typedef struct {
   unsigned char           FileType;
}__attribute__((packed)) uart_cmd_0x13_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           UpgradeState;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x13_ret_t;


/* cmd: 0x31, req get file */
typedef struct {
    char                    StationCode[8];
    unsigned char           FileType;
}__attribute__((packed)) uart_cmd_0x31_t;

typedef struct {
    uart_proto_head_t       header;
    char                    StationCode[8];
    char                    FileName[32];
    unsigned int            FileSize;
    unsigned int            FileCrc;
    unsigned int            TotalPacket;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x31_ret_t;

/* cmd: 0x32, get file */
typedef struct {
    char                    StationCode[8];
    unsigned char           FileType;
    unsigned int            CurPacketNum;
}__attribute__((packed)) uart_cmd_0x32_t;

typedef struct {
    uart_proto_head_t       header;
    char                    StationCode[8];
}__attribute__((packed)) uart_cmd_0x32_ret_t;

/* cmd: 0x33, req upload data */
typedef struct {
    char                    StationCode[8];
    unsigned char           SensorType;
    unsigned char           Channel;
    unsigned char           DataType;
    unsigned int            DataSize;
    unsigned int            DataCrc;
    unsigned int            PacketSize;
    unsigned int            TotalPacket;
}__attribute__((packed)) uart_cmd_0x33_t;

typedef struct {
    uart_proto_head_t       header;
    char                    StationCode[8];
    unsigned char           SensorType;
    unsigned char           Channel;
    unsigned char           UploadState;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x33_ret_t;

/* cmd: 0x34, data type */
/* index data */
typedef struct {
    float                   Value;              //指标值
    unsigned char           HaveWave;           //该指标是否存在波形(只有报警才会有波形)
}__attribute__((packed)) uart_indicator_value_t;

typedef struct {
    char                    StationCode[8];
    unsigned char           SensorType;      //0x03：RH605；0x02：RH505
    unsigned char           ChannelNo;
    unsigned char           DataType;
    unsigned int            CurPacket;
    unsigned int            TotalPacket;

    unsigned char           SampleTime[7];
    uart_indicator_value_t  Z_HighFreqAcc;
    uart_indicator_value_t  Z_HighFreqKurt;
    uart_indicator_value_t  Z_HighFreq_1;
    uart_indicator_value_t  Z_HighFreq_2;
    uart_indicator_value_t  Z_HighFreq_3;
    uart_indicator_value_t  Z_HighFreq_4;
    uart_indicator_value_t  Z_LowFreqAcc;
    uart_indicator_value_t  Z_LowFreqVel;
//    uart_indicator_value_t  Z_LowFreq_1;	// 低频包络峰值
//    uart_indicator_value_t  Z_LowFreq_2;	// 高频包络峰值
    char                    Version[10];

    uart_indicator_value_t  X_LowFreqAcc;
    uart_indicator_value_t  X_LowFreqVel;
    uart_indicator_value_t  X_LowFreq_1;	// WS2, 音频分贝
    uart_indicator_value_t  X_LowFreq_2;	// WS2, 转频频率

    uart_indicator_value_t  Y_LowFreqAcc;
    uart_indicator_value_t  Y_LowFreqVel;
    uart_indicator_value_t  Y_LowFreq_1;	// WS2, 转频rms
    uart_indicator_value_t  Y_LowFreq_2;  // float Volt;unsigned char rssi;

    uart_indicator_value_t temperature;
}__attribute__((packed)) uart_0x34_indicator_data_t;

typedef struct {
    uart_proto_head_t       header;
    char                    StationCode[8];
    unsigned char           RecvState;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x34_ret_t;

/* cmd: 0x35, upload state */
typedef struct {
    char                    StationCode[8];
    unsigned char           State;
}__attribute__((packed)) uart_cmd_0x35_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           State;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x35_ret_t;

/* cmd: 0x36, spi nor flash info */
typedef struct {
    uart_proto_head_t       header;
    char                    StationCode[8];
    unsigned short          spi_flash_sector_size;   // 每页多少字节
    unsigned char           save_sensor_info_num;    //存储空间按照多少传感器划分
    unsigned int            vaild_sensor_bit;        //有效传感器标志

    unsigned char           is_read_wave;
    unsigned short          wave_data_start_sector;
    unsigned short          wave_data_end_sector;

    unsigned char           is_read_a_eigen;
    unsigned short          a_eigen_start_sector;
    unsigned short          a_eigen_end_sector;

    unsigned char           is_read_b_eigen;
    unsigned short          b_eigen_start_sector;
    unsigned short          b_eigen_end_sector;

    unsigned char           is_read_station_log;
    unsigned short          station_log_start_sector;
    unsigned short          station_log_end_sector;

    unsigned char           is_read_sensor_log;
    unsigned short          sensor_log_start_sector;
    unsigned short          sensor_log_end_sector;

    unsigned char           is_read_sta_battery;
    unsigned short          sta_battery_start_sector;
    unsigned short          sta_battery_end_sector;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x36_t;

typedef struct {
    uart_proto_head_t       header;
    char                    StationCode[8];
    unsigned short          spi_flash_sector_size;   // 每页多少字节
    unsigned char           save_sensor_info_num;    //存储空间按照多少传感器划分
    unsigned int            vaild_sensor_bit;        //有效传感器标志

    unsigned char           is_read_wave;
    unsigned short          wave_data_start_sector;
    unsigned short          wave_data_end_sector;//20

    unsigned char           is_read_a_eigen;
    unsigned short          a_eigen_start_sector;
    unsigned short          a_eigen_end_sector;

    unsigned char           is_read_b_eigen;
    unsigned short          b_eigen_start_sector;
    unsigned short          b_eigen_end_sector;//30

    unsigned char           is_read_station_log;
    unsigned short          station_log_start_sector;
    unsigned short          station_log_end_sector;

    unsigned char           is_read_sensor_log;
    unsigned short          sensor_log_start_sector;
    unsigned short          sensor_log_end_sector;//40

    unsigned char           is_read_sta_battery;
    unsigned short          sta_battery_start_sector;
    unsigned short          sta_battery_end_sector;
    unsigned int			high_vaild_sensor_bit;//49
    //================================================================
    unsigned char           ChannelNo;          // 1
    unsigned int            MeasPointID;        // 5
    unsigned int            MeasDefID;          // 9
    unsigned char           SampleTime[7];      // 16
    float                   SampleValue;        // 20
    float                   ConverCoef[3];      // 24
    unsigned int            RPM;                // 28
    unsigned char           DataSource;         // 29
    unsigned char           DataSection;        // 30
    unsigned char           WaveType;           // 31
    unsigned char           UpTag;              // 32
    unsigned int            TimeTag;            // 36
    unsigned int            TimeBase;           // 40
    unsigned char           DataType;           // 41
    unsigned int            SensorSN;           // 45
    unsigned int            WaveFileSize;       // 49
    unsigned int            Freq[3];

    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_new_0x36_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           ChannelNo;
    unsigned char           WaveType;
    unsigned char           State;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x36_ret_t;

/* cmd: 0x37, hw check */
typedef struct {
    uart_proto_head_t       header;
    unsigned char           hw_check_type;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x37_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           recv_state;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x37_ret_t;

/* cmd: 0x38, hw check result query */
typedef struct {
    uart_proto_head_t       header;
    unsigned char           hw_check_type;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x38_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           hw_check_type;
    unsigned char           check_result;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x38_ret_t;

#ifdef DOTON
#define TASK_GET_GW_CONFIG          0x01
#define TASK_SET_GW_CONFIG          0x02
#define TASK_GET_SENSOR_CONFIG      0x03
#define TASK_SET_SENSOR_CONFIG      0x04
#define TASK_TEMP_SAMPLE            0x05
#define TASK_TEMP_SAMPLE_QUERY      0x06
#define TASK_TREND_SAMPLE           0x07
#define TASK_FW_UPGRADE_INFO        0x08
#define TASK_FW_UPGRADE_DATA        0x09

#define DEBUG_TASK_OK   1

/* cmd: 0x40, debug task */
typedef struct {
    unsigned char           TaskType;
}__attribute__((packed)) uart_cmd_0x40_0x01_t;

typedef struct {
    char                    StationCode[8];
    char                    StationSN[16];
    char                    MainBoardVersion[16];
    char                    McuVersion[16];
    char                    MpuVersion[16];
#ifdef DOTON
    char                    CommMode; // 0: 4G, 1: WIFI, 2: Eth
    char                    ProtoVer;
    char                    Rsvd[62];
#else
    char                    Rsvd[64];
#endif
}__attribute__((packed)) uart_cmd_ret_sys_param;

typedef struct {
    unsigned char           Ip[4];
    unsigned char           ServerIp[4];
    unsigned char           NetMask[4];
    unsigned char           NetGw[4];
    unsigned short          ServerPort;
    unsigned char           Mac[6];
    char                    Ssid[16];
    char                    Key[16];
    char                    Rsvd[64];
}__attribute__((packed)) uart_cmd_ret_wifi_param;

typedef struct {
    unsigned char           Ip[4];
    unsigned char           ServerIp[4];
    unsigned char           NetMask[4];
    unsigned char           NetGw[4];
    unsigned short          ServerPort;
    unsigned char           Mac[6];
    char                    Rsvd[64];
}__attribute__((packed)) uart_cmd_ret_eth_param;

typedef struct {
    unsigned char           ServerIp[4];
    unsigned short          ServerPort;
    char                    Apn[32];
    char                    Dns[32];
    char                    Rsvd[64];
}__attribute__((packed)) uart_cmd_ret_4g_param;

typedef struct {
    unsigned char           ServerIp[4];
    unsigned short          ServerPort;
    char                    Imei[16];
    char                    Rsvd[64];
}__attribute__((packed)) uart_cmd_ret_nb_lora_param;

typedef struct {
    unsigned char           FreqBand;
    unsigned char           TxPower;
    char                    Rsvd[16];
}__attribute__((packed)) uart_cmd_ret_zigbee_param;

typedef struct {
    uart_cmd_ret_sys_param  sys;
    uart_cmd_ret_wifi_param wifi;
    uart_cmd_ret_eth_param  eth;
    uart_cmd_ret_4g_param   _4g;
    uart_cmd_ret_nb_lora_param nl;
    uart_cmd_ret_zigbee_param zigbee;
}__attribute__((packed)) uart_gw_param;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    uart_gw_param           GwParam;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x01_ret_t;

typedef struct {
    unsigned char           TaskType;
    uart_gw_param           GwParam;
}__attribute__((packed)) uart_cmd_0x40_0x02_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           State;  // 0, fail; 1, ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x02_ret_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x03_t;



typedef struct {
    unsigned char           Bind;
    char                    Sn[10];
    unsigned char           Channel;
    char                    DevNumber[32];
}__attribute__((packed)) uart_cmd_ret_sensor_info_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    uart_cmd_ret_sensor_info_t Info[MAX_SENSOR_CNT];
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x03_ret_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    uart_cmd_ret_sensor_info_t Info[MAX_SENSOR_CNT];
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x04_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           State;  // 0, fail; 1, ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x04_ret_t;

enum {
    SAMPLE_DIR_YADIAN = 0,
    SAMPLE_DIR_X,
    SAMPLE_DIR_Y,
    SAMPLE_DIR_TEMP
};

typedef struct {
    unsigned char           TaskType;
    char                    SensorSn[10];
    unsigned char           SensorChannel;
    unsigned char           Dir;
    unsigned short          SampleLength;
    unsigned short          SampleFreq;
    unsigned char           SampleTime[7];
}__attribute__((packed)) uart_cmd_0x40_0x05_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           State;  // 0, fail; 1, ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x05_ret_t;

typedef struct {
    unsigned char           TaskType;
}__attribute__((packed)) uart_cmd_0x40_0x06_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           State;  // 0, sampling; 1, ok; 2, fail
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x06_ret_t;

typedef struct {
		unsigned char           TaskType;
    char                    SensorSn[10];
    unsigned char           SensorChannel;
    unsigned short          SampleLength;
    unsigned short          SampleFreq;
    unsigned short          SampleInterval;
    unsigned char           SampleType; // 1-state trend 2-vib trend
    unsigned char           Dir;
    unsigned char           SampleTime[7];
    unsigned char           Start;
}__attribute__((packed)) uart_cmd_0x40_0x07_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           State;  // 0, fail; 1, ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x07_ret_t;

typedef struct {
		unsigned char           TaskType;
    unsigned char						FwType;         // 1-MCU, 2-Sensor, 3-BT, 4-MPU
    char          					FileName[32];   // WS_ZGB_V1.001, SG_MCU_V1.001
    unsigned int						TotalSize;
    unsigned int						Crc;
    unsigned int						TotalPackets;   // 1 packet 2KB
}__attribute__((packed)) uart_cmd_0x40_0x08_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           State;  // 0, fail; 1, ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x08_ret_t;

typedef struct {
		unsigned char           TaskType;
    unsigned char						FwType;         // 1-MCU, 2-Sensor
    unsigned int						PacketNo;
    unsigned char						Data[2048];
}__attribute__((packed)) uart_cmd_0x40_0x09_t;

typedef struct {
    uart_proto_head_t       header;
    unsigned char           TaskType;
    unsigned char           State;  // 0, fail; 1, ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x40_0x09_ret_t;

/* cmd: 0x41, upload trend data MCU -> MPU */
typedef struct {
    unsigned char           TrendType;
    char                    SensorSn[10];
    unsigned char           SensorChannel;
    unsigned char           SampleTime[7];
    float					HighIndicator;
    float					LowIndicator;
    float					VelIndicator;
    float					Temperature;
    float					Voltage;
    float					Current;
    float					Impendence;
    float					Signal;
    unsigned char           Rsvd[64];
}__attribute__((packed)) uart_cmd_0x41_t;

typedef struct {
		uart_proto_head_t       header;
    unsigned char           State;	// 0 fail, 1 ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x41_mpu_ack;

/* cmd: 0x42, upload upgrade state MCU -> MPU */
typedef struct {
    unsigned char           FwType;
    unsigned char           State;  // 0 fail, 1 ok
    char           					FwVer[10];
    char                    SensorSn[10];
    unsigned char           SensorChannel;
}__attribute__((packed)) uart_cmd_0x42_t;

typedef struct {
		uart_proto_head_t       header;
    unsigned char           State;	// 0 fail, 1 ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x42_mpu_ack;
#endif

typedef struct {
	uart_proto_head_t       header;
    unsigned char           State;	// 0 fail, 1 ok
    uart_proto_tail_t       tailer;
}__attribute__((packed)) uart_cmd_0x43_mpu_ack;

typedef struct {
    char                    StationCode[8];
    unsigned char           SensorType;
    unsigned char           Channel;
    unsigned char           DataType;
    unsigned int            DataSize;
    unsigned int            DataCrc;
    unsigned int            PacketSize;
    unsigned int            TotalPacket;
}__attribute__((packed)) uart_req_upload_data_info_t;

typedef struct uart_proto_time {
    unsigned short year;
    unsigned char mon;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
}__attribute__((packed)) uart_proto_time_t;


typedef struct uart_cmd_handle_def {
    int cmd_type;
    int (*handle)(uint8_t *pdata, uint32_t data_len);
}uart_cmd_handle_t;

//#pragma pack(push, 1)
typedef struct _ZIGBEEParam{
    uart_cmd_ret_sensor_info_t sensor_info[MAX_SENSOR_CNT];
    wless_common_param_t syspara;
    uint8_t  zgbFreqBand;
    uint8_t  zgbTxpower;
    uint8_t  zgbtype;
    uint8_t  count;
    uint8_t  update;
    char McuVersion[10];
}__attribute__((packed)) ZIGBEEParam;
//#pragma pack(pop)

FREQ_PUBLIC ZIGBEEParam *getSharedData();
FREQ_PUBLIC void do_wls_sem_init(void);
FREQ_PUBLIC void do_wls_sem_release(void);
FREQ_PUBLIC int dt_wireless_access_init(ZIGBEEParam * Param);

#ifdef __cplusplus
}
#endif
#endif
