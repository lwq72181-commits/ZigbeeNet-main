#ifndef ZIGBEETYPE_H
#define ZIGBEETYPE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include "common.h"
/**
 * @brief 分割波形大小枚举
 */
typedef enum _split_wave_size_def {
    SPLIT_WAVE_SIZE_MIN  = 16 * 1024,   /**< 最小分割波形大小 */
    SPLIT_WAVE_SIZE_MID1 = 64 * 1024,   /**< 中等1分割波形大小 */
    SPLIT_WAVE_SIZE_MID2 = 256 * 1024,  /**< 中等2分割波形大小 */
    SPLIT_WAVE_SIZE_MAX  = 512 * 1024,  /**< 很大的分割波形大小 */
    SPLIT_WAVE_SIZE_PRO  = 1024 * 1024, /**< 最大的分割波形大小*/
} split_wave_size_t;


/* 上传协议类型 */
typedef enum _dt_protocol_type_def{
    PROTOCOL_TYPE_DOTON = 0x00, /* is default */
    PROTOCOL_TYPE_IMOTORLINX,
    PROTOCOL_TYPE_MAX,
}dt_protocol_type_t;


/* 数据上传类型定义 */
typedef enum _dt_upload_data_type_def{
    UPLOAD_DATA_TYPE_EIGEN = 0x00,  /* 指标 */
    UPLOAD_DATA_TYPE_ALARM_WAVE,    /* 报警波形 */
    UPLOAD_DATA_TYPE_WAVE,          /* 波形 */
    UPLOAD_DATA_TYPE_TAG,           /* 时标 */
    UPLOAD_DATA_TYPE_LWAVE,         /* 长波形*/
    UPLOAD_DATA_TYPE_RTU,           /* RTU */
    UPLOAD_DATA_TYPE_TECH,          /* TECH */
    UPLOAD_DATA_TYPE_RPM,           /* RPM */
    UPLOAD_DATA_TYPE_LOG,           /* LOG */
    UPLOAD_DATA_TYPE_TEMP,          /* TEMP */
    UPLOAD_DATA_TYPE_TCP,           /* TCP */
    UPLOAD_DATA_TYPE_AUDIO,
    UPLOAD_DATA_TYPE_WLESS_REAL_EIGEN,    /* wireless real time eigen */
    UPLOAD_DATA_TYPE_WLESS_TREND,         /* wireless trend */
    UPLOAD_DATA_TYPE_WLESS_WAVE,          /* wireless wave data */
    UPLOAD_DATA_TYPE_WLESS_EIGEN,         /* wireless eigen */
    UPLOAD_DATA_TYPE_WLESS_MCU_LOG,       /* wireless mcu log */
    UPLOAD_DATA_TYPE_WLESS_SENSOR_LOG,    /* wireless sensor log */
    UPLOAD_DATA_TYPE_WLESS_BATTERY,       /* wireless battery */
    UPLOAD_DATA_TYPE_WLESS_CFG_TABLE,    /* wireless config table */
    UPLOAD_DATA_TYPE_MAX,
}dt_upload_data_type_t;

typedef struct _dt_upload_data_config_def {
    int  type;                      /* 数据类型 */
    void (*callback)(void);         /* 初始化回调函数*/
    char file_head[16];             /* 文件头 */
    char file_name[128];            /* 文件名类型 */
    char file_path[128];            /* 文件路径 */
    int  indexsize;                 /*  索引文件大小 */
}dt_upload_data_config_t;


/* 数据存储与上传管理-结构定义 */
typedef struct _dt_upload_manager_def{
    struct list_head head;     /* 链表结构:头节点 */
    pthread_t pid;            /* 数据上传线程*/
    pthread_mutex_t keep_mutex;    /* */
    unsigned int keep_tick;  /*用于通讯传输的健康计数*/
    int is_upload; //用于控制是否上传开关
}dt_upload_manager_t;

/* 数据存储与上传节点-结构定义 */
typedef struct _dt_upload_data_def{
    struct list_head list;                    /* 链表结构:节点 */
    pthread_mutex_t mutex;                    /* 互斥锁，用于各个数据的离线保存和上传 */
    int is_mutex;                             /* 表示互斥锁正在使用 */
    dt_upload_data_type_t data_type;          /* 处理采集任务类型，包括指标、波形、RTU */
    dt_protocol_type_t    protocol;           /* 预留接口 */
    int (*online) (void);                     /* 执行上传任务 */
    int (*offline)(void);                     /* 执行上传任务 */
    int (*save)(dt_upload_data_type_t type, void *data, int size, char *ptime);                        /* 执行存储任务 */
}dt_upload_data_t;

/* MODBUS 数据传输协议类型 */
typedef enum _modbus_protocol_type_def {
    RTU_PROTOCOL_TPYE = 5,
    TCP_PROTOCOL_TPYE,
}modbus_protocol_type_t;

#define OFFLINE_INDEX_LABEL_START       (0x5A5A5A5A)
#define OFFLINE_INDEX_LABEL_END         (0xA5A5A5A5)
#define FLASH_REMAIN_SIZE               (32 * 1024)         /* EMMC预留空间 */

/* 离线文件 */
#define STA_OFFLINE_DIR                 "/userdata/AppData"/* 采集站离线数据目录 */
#define EIGEN_DATA_DIR                  STA_OFFLINE_DIR"/eigen"
#define ALARM_WAVE_DATA_DIR             STA_OFFLINE_DIR"/alarm_wave"    /* 报警波形 */
#define WAVE_DATA_DIR                   STA_OFFLINE_DIR"/wave"    /* 波形 */
#define LW_DATA_DIR                     STA_OFFLINE_DIR"/lw"      /* 长波形 */
#define LOG_DATA_DIR                    STA_OFFLINE_DIR"/log"     /* 日志 */
#define RPM_DATA_DIR                    STA_OFFLINE_DIR"/rpm"     /* 转速 */
#define RTU_DATA_DIR                    STA_OFFLINE_DIR"/rtu"     /* rtu */
#define TEMPERATURE_DATA_DIR            STA_OFFLINE_DIR"/temperature"     /* 温度 */
#define TECH_DATA_DIR                   STA_OFFLINE_DIR"/tech"     /* 工艺量 */
#define TAG_DATA_DIR                    STA_OFFLINE_DIR"/tag"      /* 时标 */
#define AUDIO_DATA_DIR                   STA_OFFLINE_DIR"/audio"    /* 音频 */

/* wireless */
#define WLESS_REAL_EIGEN_DATA_DIR        STA_OFFLINE_DIR"/wless_real_eigen"            /* 无线实时指标存储位置 */
#define WLESS_TREND_DATA_DIR             STA_OFFLINE_DIR"/wless_trend"                 /* 无线实趋势数据存储位置 */
#define WLESS_WAVE_DATA_DIR              STA_OFFLINE_DIR"/wless_wave"             /* 无线波形存储位置 */
#define WLESS_EIGEN_DATA_DIR             STA_OFFLINE_DIR"/wless_eigen"            /* 无线指标存储位置 */
#define WLESS_MCU_LOG_DATA_DIR           STA_OFFLINE_DIR"/wless_mcu_log"          /* 无线mcu日志存储位置 */
#define WLESS_SENSOR_LOG_DATA_DIR        STA_OFFLINE_DIR"/wless_sensor_log"       /* 无线传感器日志存储位置 */
#define WLESS_BATTERY_DATA_DIR           STA_OFFLINE_DIR"/wless_battery"          /* 无线传感器电池信息存储位置 */

/* 临时数据文件区 */
#define TEMP_DATA_DIR                   STA_OFFLINE_DIR"/temp"              /* 所有临时数据 */
#define RPM_TEMP_DIR                    TEMP_DATA_DIR"/temp_rpm"            /* 转速存储位置 */
#define WAVE_TEMP_DIR                   TEMP_DATA_DIR"/temp_wave"           /* 波形存储位置 */
#define LW_TEMP_DIR                     TEMP_DATA_DIR"/temp_lw"             /* 长波形存储位置 */
#define EIGEN_TEMP_DIR                  TEMP_DATA_DIR"/temp_eigen"          /* 指标存储位置 */
#define RTU_TEMP_DIR                    TEMP_DATA_DIR"/temp_rtu"            /* rtu存储位置 */
#define TEMPERATURE_TEMP_DIR            TEMP_DATA_DIR"/temp_temperature"    /* 温度存储位置 */
#define TECH_TEMP_DIR                   TEMP_DATA_DIR"/temp_tech"           /* 工艺量存储位置 */
#define TAG_TEMP_DIR                    TEMP_DATA_DIR"/temp_tag"            /* 时标存储位置 */
#define AUDIO_TEMP_DIR                  TEMP_DATA_DIR"/temp_audio"            /* 音频存储位置 */

/* wireless */
#define WLESS_REAL_EIGEN_TEMP_DIR             TEMP_DATA_DIR"/temp_wless_real_eigen"            /* 无线实时指标存储位置 */
#define WLESS_TREND_TEMP_DIR                  TEMP_DATA_DIR"/temp_wless_trend"                 /* 无线趋势数据存储位置 */
#define WLESS_WAVE_TEMP_DIR                   TEMP_DATA_DIR"/temp_wless_wave_data"             /* 无线波形存储位置 */
#define WLESS_EIGEN_TEMP_DIR                  TEMP_DATA_DIR"/temp_wless_eigen_data"            /* 无线指标存储位置 */
#define WLESS_MCU_LOG_TEMP_DIR                TEMP_DATA_DIR"/temp_wless_mcu_log_data"          /* 无线mcu日志存储位置 */
#define WLESS_SENSOR_LOG_TEMP_DIR             TEMP_DATA_DIR"/temp_wless_sensor_log_data"       /* 无线传感器日志存储位置 */
#define WLESS_BATTERY_TEMP_DIR                TEMP_DATA_DIR"/temp_wless_battery_data"          /* 无线电池信息存储位置 */

/* 文件后缀 */
#define RPM_TYPE            (".rpm")
#define ALARM_WAVE_TYPE     (".alarm_wave")
#define WAVE_TYPE           (".wave")
#define LW_TYPE             (".lw")
#define EIGEN_TYPE          (".eigen")
#define RTU_TYPE            (".rtu")
#define TEMPERATURE_TYPE    (".temp")
#define TECH_TYPE           (".tech")
#define TAG_TYPE            (".tag")
#define AUDIO_TYPE          (".raw")

#define WLESS_REAL_EIGEN_TYPE   (".wless_real_eigen")
#define WLESS_TREND_TYPE        (".wless_trend")
#define WLESS_WAVE_TYPE         (".wless_wave")
#define WLESS_EIGEN_TYPE        (".wless_eigen")
#define WLESS_MCU_LOG_TYPE      (".wless_mcu_log")
#define WLESS_SENSOR_LOG_TYPE   (".wless_sensor_log")
#define WLESS_BATTERY_TYPE      (".wless_battery")

#define MAX_AUDIO_CH_NUM		2       /* 最大音频通道数 */
/* 文件系统状态 */
typedef enum _file_system_state_def {
    FILE_READ_WRITE = 0x1020,
    FILE_ONLY_READ  = 0x1021,
}file_system_state_t;

#define DISK_SPACE_PATH "/userdata"                       /* 数据文件存放地址 */
#define MAX_WLS_VIB_CH_NUM			45      /* 最大支持检通道 */

#define MAC2STR(a) (uint8_t)((a)[0]), (uint8_t)((a)[1]), (uint8_t)((a)[2]), (uint8_t)((a)[3]), (uint8_t)((a)[4]), (uint8_t)((a)[5])
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

#define BOARD_INFO_STRING_SN         "SN"
#define BOARD_INFO_STRING_DEV_CODE   "DEV_CODE"
#define BOARD_INFO_STRING_NET_TYPE   "NET_TYPE"
#define BOARD_INFO_STRING_PRO_TYPE   "PROTO_TYPE"
#define BOARD_INFO_STRING_PRO_VER    "PROTO_VERSION"

#define BOARD_INFO_STRING_APP_VER     "APP_VERSION"
#define BOARD_INFO_STRING_HW_VER      "HW_VERSION"
#define BOARD_INFO_STRING_FPGA_TYPE   "FPGA_VERSION"
#define BOARD_INFO_STRING_TECH_TYPE   "TECH_VERSION"
#define BOARD_INFO_STRING_GD_VER      "GD_VERSION"
#define BOARD_INFO_STRING_AD1_VER     "AD1_VERSION"
#define BOARD_INFO_STRING_AD2_VER     "AD2_VERSION"

#define BOARD_INFO_STRING_MAC         "MAC_ADDRESS"
#define BOARD_INFO_STRING_SERVER_IP   "SERVER_IP"
#define BOARD_INFO_STRING_HOST        "SERVER_HOST"
#define BOARD_INFO_STRING_LOCAL_IP    "LOCAL_IP"
#define BOARD_INFO_STRING_NETMASK     "NETMASK"
#define BOARD_INFO_STRING_GW          "GATEWAY"
#define BOARD_INFO_STRING_DNS         "DNS_ADDRESS"
#define BOARD_INFO_STRING_SERVER_NAME "SERVER_DOMAIN"

#define BOARD_INFO_STRING_MQ_PREFIX      "MQTT_PREFIX"
#define BOARD_INFO_STRING_MQ_USERID      "MQTT_USERID"
#define BOARD_INFO_STRING_MQ_USERKEY     "MQTT_USERKEY"
#define BOARD_INFO_STRING_MQ_KEEP        "MQTT_KEEPLIVE"
#define BOARD_INFO_STRING_MQ_TLS         "MQTT_TLS"
#define BOARD_INFO_STRING_NTP_SERVER     "NTP_SERVER"
#define BOARD_INFO_STRING_NTP_PORT       "NTP_PORT"

#define BOARD_INFO_STRING_WIFI_SSID     "WIFI_SSID"
#define BOARD_INFO_STRING_WIFI_KEY      "WIFI_KEY"

#define BOARD_INFO_STRING_MB_ADDR1       "GD_MODBUS_ADDR1"
#define BOARD_INFO_STRING_MB_ADDR2       "GD_MODBUS_ADDR2"

#define BOARD_INFO_STRING_SWITCH_TYPE    "SWITCH_TYPE"
#define BOARD_INFO_STRING_TECH_CH        "TECH_CH_"

#define BOARD_INFO_STRING_HAS_CCS        "HAS_CCS"
#define BOARD_INFO_STRING_CC_SWITCH      "CC_SWITCH"

#define BOARD_INFO_STRING_WLESS_SWITCH   "WLESS_SWITCH"
#define SYS_PARAM_SIZE

#define DEFAULT_LOCATION_SERVER
#define DEFAULT_LOCATION_PORT

typedef struct _sample_param_def{
    unsigned char is_sample; /*1表示采集，0表示不采集*/
    unsigned int freq;
    unsigned int len;      /* high&low freq, diff length  */
    unsigned int plus_len;
    unsigned int sample_len;
}sample_param_t;

typedef struct _wave_sample_param_def{
    sample_param_t wave[DT_MAX_FREQ];
    unsigned int rate;                /* 降采样率 */
    unsigned int wave_len;            /* 波形采集长度 */
}wave_sample_param_t;

#pragma pack (1)
#define PARAM_SIZE_LEN_MAX (2048)

typedef struct _dt_config_param_old_def{
#define MAX_AD_BOARD_NUM    (3)
    //device param
    char sn[16];        /* SN号 */
    char dev_code[10];  /* 设备编码 */
    char net_type;      /*网络类型：0是以太网，1是4G, 2是wifi*/
    char protocol_type; /*协议类型 可能后期支持其他类型*/
    char protocol_ver;  /*协议版本*/
    //board version
    char     app_ver[32];   /*主板软件版本号 */
    char     hw_ver[16];    /*主板硬件版本号*/
    char     fpga_ver[20];  /*fpga版本号 */
    char     tech_ver[20];  /*工艺板卡-版本号 */

    char     modbus_ver[20];/*底板板卡-版本号 */
    char     ad_ver[MAX_AD_BOARD_NUM][10];  /* AD板卡1-版本号, 810的AD板带有主控，直接读即可 */
    char     sensor[20];    /* 占个坑位，未来可能zigbee传感器接入 */
    // net param
    int hostPort;                  /*服务器端口*/
    unsigned char mac_addr[6];     /*mac地址*/
    unsigned char local_ip[4];     /*IP地址*/
    unsigned char netmask[4];      /*掩码*/
    unsigned char gw[4];           /*网关*/
    unsigned char server_ip[4];    /*服务器地址*/
    unsigned char dns[4];         /*dns服务器地址*/
    char server_domain[64];        /*域名解析地址*/
     // mqtt param
    char mqtt_prefix [128];          /* 主题前缀 */
    char mqtt_userID [32];           /* MQTT用户名 */
    char mqtt_userKEY[32];           /* MQTT密码 */
    int  mqtt_keepalive;             /* MQTT心跳时间 */

     // wifi param
    char wifi_ssid[32];             /* wifi名称 */
    char wifi_key[32];              /* wifi密码 */
     // 4G&5G param
     char apn[32];                 /* apn名称 */
    // modbus param
    unsigned char  modbusAddr;    /* MODBUS设备1地址 */
    unsigned short modbusBaud;    /* MODBUS设备1波特率*/
    unsigned char  modbusAddr2;
    unsigned short modbusBaud2;
    unsigned char  modbusAddr3;
    unsigned short modbusBaud3;
    unsigned char  modbusAddr4;
    unsigned short modbusBaud4;
    //hardware param
    unsigned char upload_sd;      /*0-不上传; 1-上传*/
    unsigned char rtu_ver;        /*RTU版本类型:通用协议和定制协议*/
    unsigned char switch_type;    /* 1-干节点；2-湿节点 */
#define  TECH_CH_NUM_MAX (4)      /* 目前支持4个工艺通道配置使用 */
    unsigned char tech_type[TECH_CH_NUM_MAX];   /* 0-测电流；1-测电压 */
    char  bias_check;            /* 传感器是否自检偏置电压。0-不自检；1-自检，当偏置电压正常时保存该通道数据，异常不保存 */
}__attribute__((packed))dt_config_param_old_t;

typedef struct _dt_board_info_old_param_def{
    dt_config_param_old_t body;
    unsigned int magic;
#define MAGIC_CODE (0xF0F0A5A5)
    char rsvd[PARAM_SIZE_LEN_MAX - sizeof(dt_config_param_old_t) - sizeof(unsigned int)];
} __attribute__((packed)) dt_board_info_old_param_t;

/**------------------------------------------------------------------------------------------------------------------------------------*/
typedef struct _dt_config_param_def{
#define MAX_AD_BOARD_NUM    (3)
    //device param
    char sn[16];        /* SN号 */
    char dev_code[10];  /* 设备编码 */
    char net_type;      /*网络类型：0是以太网，1是4G, 2是wifi*/
    char protocol_type; /*协议类型 可能后期支持其他类型*/
    char protocol_ver;  /*协议版本*/
    //board version
    char     app_ver[32];   /*主板软件版本号 */
    char     hw_ver;    /*主板硬件版本号*/
    char     rsvd0[15];    /* 暂时预留 */

    char     fpga_ver[20];  /*fpga版本号 */
    char     tech_ver[20];  /*工艺板卡-版本号 */

    char     modbus_ver[20];/*底板板卡-版本号 */
    char     ad_ver[MAX_AD_BOARD_NUM][10];  /* AD板卡1-版本号, 810的AD板带有主控，直接读即可 */
    char     sensor[20];    /* 占个坑位，未来可能zigbee传感器接入 */
    // net param
    int hostPort;                  /*服务器端口*/
    unsigned char mac_addr[6];     /*mac地址*/
    unsigned char local_ip[4];     /*IP地址*/
    unsigned char netmask[4];      /*掩码*/
    unsigned char gw[4];           /*网关*/
    unsigned char server_ip[4];    /*服务器地址*/
    unsigned char dns[4];         /*dns服务器地址*/
    char server_domain[64];        /*域名解析地址*/
     // mqtt param
    char mqtt_prefix [128];          /* 主题前缀 */
    char mqtt_userID [32];           /* MQTT用户名 */
    char mqtt_userKEY[32];           /* MQTT密码 */
    int  mqtt_keepalive;             /* MQTT心跳时间 */

     // wifi param
    char wifi_ssid[32];             /* wifi名称 */
    char wifi_key[32];              /* wifi密码 */
     // 4G&5G param
     char apn[32];                 /* apn名称 */
    // modbus param
    unsigned char  modbusAddr;    /* MODBUS设备1地址 */
    unsigned short modbusBaud;    /* MODBUS设备1波特率*/
    unsigned char  modbusAddr2;
    unsigned short modbusBaud2;
    unsigned char  modbusAddr3;
    unsigned short modbusBaud3;
    unsigned char  modbusAddr4;
    unsigned short modbusBaud4;
    //hardware param
    unsigned char upload_sd;      /*0-不上传; 1-上传*/
    unsigned char rtu_ver;        /*RTU版本类型:通用协议和定制协议*/
    unsigned char switch_type;    /* 1-干节点；2-湿节点 */
#define  TECH_CH_NUM_MAX (4)      /* 目前支持4个工艺通道配置使用 */
    unsigned char tech_type[TECH_CH_NUM_MAX];   /* 0-测电流；1-测电压 */
    char  bias_check;            /* 传感器是否自检偏置电压。0-不自检；1-自检，当偏置电压正常时保存该通道数据，异常不保存 */
#ifdef USE_CCS_ENABLE
    unsigned char has_ccs;
    unsigned char ccs;           /*Constant current source 恒流源类型开关，0是关闭，1是开启*/
#endif

#ifdef USE_MQTT_TLS_ENABLE
    unsigned char   mqtt_tls;   /* TLS是否功能开启:  */
    char	ntp_server[64];     /* NTP服务器地址*/
    unsigned short ntp_port;    /* NTP服务器端口*/
#endif

#ifdef USE_WIRELESS_ENABLE
    char mcu_ver[16]; /*无线版本号*/
    int zigbee_freq;           /*zigbee-频段*/
    char wireless; /* 是否启用无线 0:关闭，1:启用 */
#endif // DEBUG

}__attribute__((packed))dt_config_param_t;


typedef struct _dt_board_info_param_def{
    unsigned int magic;
    dt_config_param_t body;
    char rsvd[PARAM_SIZE_LEN_MAX - sizeof(dt_config_param_t) - sizeof(unsigned int)];
} __attribute__((packed)) dt_board_info_param_t;

typedef struct _wls_sensor_info_def{
   unsigned char            bind;
    char                    sn[10];
    unsigned char           channel;
    char                    dev_number[32];
} __attribute__((packed)) wls_sensor_info_t;

#pragma pack()
/**
 * @brief 参数类型定义枚举
 * 这是一个参数类型定义的枚举，包含了振动、音频、工艺、转速和 RTU 的参数类型。
 */
typedef enum _param_type_def {
    PARAM_TYPE_VIB = 0x00,
    PARAM_TYPE_AUDIO,
    PARAM_TYPE_TECH,
    PARAM_TYPE_RPM,
    PARAM_TYPE_RTU,
    PARAM_TYPE_TCP,
    PARAM_TYPE_TEMP,
    PARAM_TYPE_MAX
} param_type_t;

/**
 * @brief 参数状态
 *
 */
typedef enum _param_status_def {
    PARAM_VALID     = 0x00,     /* 有效 */
    PARAM_INVALID,              /* 无效 */
} param_status_t;

//=================================================================
#define SYSTEM_LINUX

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
// typedef unsigned char bool;

struct mqtt_param {
    char prefix[128];/* 16->128 */
    char clientid[144];/* 32->144  prefix+sn */
    char username[32];/* 16->32 */
    char password[32];/* 16->32 */
    char stationcode[16];
    unsigned char qos;
    unsigned short keepalive;
#ifdef USE_PAHO_MQTT_C
    int ssl;
#endif
};
extern struct mqtt_param mp;

#ifdef USE_PAHO_MQTT_C
#else
extern MQTTClient g_mqtt_client;
#endif

// 升级接口回调
typedef u8 (*upgrade_cb)(int fd, char *data, u32 size, u32 packcount, u32 packindex);

// 上位机主动命令回调
typedef u8 (*host_cmd)(int cmd, void *data);
extern char g_mqtt_topic[];

#define MAX_TRANSFER_SEGMENT_SIZE	(1024 * 1024 * 2)  /* 最大传输字节数: 上传/下载每个包包含的数据内容部分 */
#define MAX_FILE_TRANSFER_SEGMENT_SIZE (32*1024)       /*固件文件大小分包传输字节数*/

#define MAX_BOARD_CNT	7	   /* 最大板卡数量 */
#define MAX_UPDATE_FILE_LIST 3 /* 最大升级文件列表数量 */

/* 板卡信息列表 */
struct BoardInfo {
    int Code;			// 板卡编码
    char  AppVer[16]; 	// 软件版本
    char 	HardVer[16];	// 硬件版本
    char  IMEI[32];
};

/* LteInfo 4、5G信息 */
struct LteInfo {
    char Revision[32];
    char ICCID[32];
    char IMEI[32];
};

/* 注册信息 */
struct RegisterInfo {
#define DT_MQTT_PROTO_VER	"V1.0"
    char Ver[16];		// 协议版本号
    char Code[16];	// 采集站编码
    char Mac[20];		// Mac地址
    char SN[16];		// 采集器SN
    int ConnType;	// 0: none, 1: 4g, 2: wifi, 3: eth
    int DasType;    // 0: 无线SG500Z, 20: 有线MS800, 24: MS810 ,100: NB101, 120: LoraSG500L

    char LocalIp[16]; 	// 本机IP
    char ServerIp[16]; 	// 服务器IP
    char GateWay[16]; 	// 网关地址
    char SubnetMask[16]; 	// 子网掩码
    u16 ServerPort; 	// 服务器端口
    int RSSI; 			// 信号强度
    struct BoardInfo BoardInfoList[MAX_BOARD_CNT];
    struct LteInfo LteInfo; /* LteInfo 4、5G信息 */
};

/* 策略参数 */
struct SysPara {
    char  SysTime[32];	// 系统时间
    char  ParaTime[32];	// 参数时间
    int IndexPeriod;	// 指标间隔(s)
    int WavePeriod;		// 波形间隔(s)
    int LongWavePeriod;	// 长波形间隔(s)
    int LongWaveOffset; // 时间偏移(s)
    int CollectType;	// 采集类型(0-定时，1-连续)
    int OutType485;		// 485输出类型（0-高频；1-低频；2速度）
    int TechPeriod;		// 工艺数据采集间隔（单位：s）
    int RpmPeriod;		// 转速数据采集间隔（单位：s）
    int RtuPeriod;		// Rtu数据轮询间隔（单位：s）
    int UserCustom;     // 0表示我们主板本，123456789表示 其他各种客户定制版本
    int AlarmCount;     // 报警上传间隔
    int WaveCompress;	//是否使⽤压缩波形传输；0--不⽀持压缩 1 ⽀持压缩
    int WlessIndexPeriod;	//无线传感器指标间隔
    int WlessWavePeriod;	//无线传感器波形间隔
};

/* 采集参数 */
#define MAX_MONITOR_DEV_NUM		4       /* 最大支持检测设备 */
#define MAX_VIB_CH_NUM			24      /* 最大支持检通道 */
#define MAX_TECH_CH_NUM 		16       /* 最大支持工艺通道 */
#define MAX_SPEED_CH_NUM		2
#define MAX_RTU_CH_NUM			8
#define MAX_TRIGGER_PARA_NUM	4
#define MAX_VIB_PARA_NUM		10 					/* 振动采集参数列表最大值 */
/* 报警参数（一个通道包含三个测点（X、Y、Z），一个测点有13个报警参数（7个低频，6个高频）） */
#define MAX_LIMIT_PARA_NUM		(MAX_VIB_PARA_NUM * 4)			/* 报警门限参数最大值 */
#define MAX_MODBUS_TCP_CH_NUM  (4)		/*最大支持8路modbus tcp通道*/
#define MAX_MODBUS_TCP_POINT_NUM  (16)  /*每路通道最大支持测点个数*/
#define MAX_AUDIO_CH_NUM		2       /* 最大音频通道数 */

// 振动通道
struct VibPara {	// 振动采集参数
    int LowFreq;	// 下限频率（HZ）
    int HighFreq;	// 上限频率（HZ）
    int Length;		// 采集长度
    int PlusLength;	// 附加长度
#define VIB_DIR_NONE 0
#define VIB_DIR_X	 2
#define VIB_DIR_Y	 3
#define VIB_DIR_Z	 1
    int Dir;		// 方向（0-无方向；1-Z轴；2-X轴；3-Y轴）
#define VIB_CODE_LONGWAVE 1 //高频长波形
#define VIB_CODE_LW_LOW_FREQ  2 //低频长波形
#define VIB_CODE_HIGH_ACC 11
#define VIB_CODE_LOW_ACC  12
#define VIB_CODE_VEL_WAVE  (15)
    int Code;		// 指标类型（0-音频波形; 1-长波形；11-高频加速度；12低频加速度；15-速度波形）
    char Unit[10];	// 工程单位（m/s^2）
};

struct LimitPara   {// 报警门限参数
    int Dir;		// 方向（0-无方向；1-Z轴；2-X轴；3-Y轴）
    int Level;		// 报警等级
    float Limit;	// 报警门限
#define EIGEN_DEFAULT_CODE       (0)
#define EIGEN_TEMPERATURE_CODE   (100)
#define EIGEN_LOW_FREQ_RMS_CODE  (101)
#define EIGEN_VER_RMS_CODE       (107)
#define EIGEN_HIGH_FREQ_RMS_CODE (112)
#define EIGEN_BIAS_VAL_CODE      (127) /* 位移偏置电压 */
#define EIGEN_FREQ_PEAK_CODE (202)
#define EIGEN_FREQ_MEAN_CODE (206)
#define EIGEN_STD_DEV_CODE   (217)

#define EIGEN_HIGH_ENVELOPE_PEAK_CODE (121) //高频包络峰值
#define EIGEN_HIGH_FREQ_P_CODE  (130) //高频峰值
#define EIGEN_HIGH_FREQ_PP_CODE (131) //高频峰峰值
#define EIGEN_HIGH_FREQ_MAX_CODE (132) //高频最大值
#define EIGEN_HIGH_FREQ_MIN_CODE  (133) //高频最小值
#define EIGEN_HIGH_FREQ_Root_CODE    (134) //方根幅值
#define EIGEN_HIGH_FREQ_Kur_CODE     (135) //峭度幅值
#define EIGEN_HIGH_FREQ_Peak_CODE    (136) //峰值幅值
#define EIGEN_HIGH_FREQ_Pulse_CODE   (137) //脉冲幅值
#define EIGEN_HIGH_FREQ_Wave_A_CODE  (138) //波形幅值
#define EIGEN_HIGH_FREQ_Margin_CODE  (139) //裕度幅值
#define EIGEN_HIGH_FREQ_Skewed_CODE  (140) //歪度幅值
#define EIGEN_HIGH_FREQ_Mean_A_CODE  (141) //平均幅值
#define EIGEN_HIGH_FREQ_Mean_CODE    (142) //均值
    int Code;		// 指标类型（0-默认； 100-温度；101-低频总值；107-速度总值；112-高频总值; 202加速度峰值; 206加速度均值; 217加速度标准差）
};

struct VibChannalPara {					// 振动通道参数
    int ChNo;							// 通道号
    int SpeedNo;						// 绑定转速通道号
    int SensType;						// 传感器类型（0-加速度；1-速度；2-位移）
    float SensCoef;						// 灵敏度系数
    int CnstCurSrcState;				// 控制恒流源状态（0-关闭；1-打开）
    struct VibPara VibParaList[MAX_VIB_PARA_NUM];		// 振动采集参数列表
    int VibParaCnt;	// 自定义字段
    struct LimitPara LimitParaList[MAX_LIMIT_PARA_NUM];	// 报警门限参数
    int LimitParaCnt; // 自定义字段
    int DevId;							//设备id
    int IsTem;							//是否测温，针对不同的传感器
};

// 工艺量通道
struct TechChannelPara {//工艺通道参数
    int ChNo;		// 通道号
    float SensCoef;	// 灵敏度系数
    float Amendment;// 修正项
};

// 转速通道
struct SpeedChannelPara {//转速通道参数
    int ChNo;			// 通道号
    int TriggerCount;	// 每圈触发数
};

// RTU通道
struct RtuChannelPara {//RTU通道参数
    int ChNo;			// Rtu通道号
    int RTUDeviceAddr;	// 设备地址
    int FunCode;		// 功能码
    int RegAddr;		// 寄存器起始地址
    int DataLength;		// 数据长度
};

// 触发采集参数
struct TriggerLimitPara {//触发门限
    int ChannelCnt;					//通道个数
    int Channels[MAX_VIB_CH_NUM];	// 通道集合（这些通道同步触发,多设备不同参数）
    float AccLimit;					// 加速度门限值
    float VerLimit;					// 速度门限值
    float RpmLimit;					// 转速门限值
    float ShockLimit;				// 冲击门限值
    float KurLimit;					// 翘度门限值
};

struct TriggerPara {							// 触发采集参数
    int Freq;									// 采集频率(HZ)
    int Length;									// 采集长度
    int TriggerGroup;							// 触发采集组数
    int TriggerType;							// 触发类型 1-加速度；2-速度；3-转速 4-加速度\转速；5-速度\转速；
    struct TriggerLimitPara TriggerLimitList[MAX_VIB_CH_NUM];	// 触发门限列表
};

struct StopLimitInfo {
    int ChNo;					// 通道号
    int ChType;					// 通道类型（0=振动 1:转速 2:工艺,3-温度，4-音频，5-RTU）
    int Code;					// 指标类型合集（0-默认； 100-温度；101-低频总值；107-速度总值；112-高频总值；）
    float StopLimit;			// 停机门限，为null时为该通道不判断
    float StopLimitChangeRate;	// 变化率
};

struct ModbusTCPPointPara {
    int DataType;	//数据类型（0是int, 1是unsigned int，3是short ,4是float）
    int RegAddr;	//寄存器地址(作为主机去读取别人数据的寄存器地址)
    float Coef_K;	//灵敏度系数K
    float Coef_B;	//灵敏度系数B
};

struct ModbusTCPChannelPara {
    int ChNo;			// 通道号
    int DeviceAddr;		// 设备地址
    char IPAddr[16]; 	// IP
    int Port;			//Port
    int Status;			//读写状态 （0是读，1是写）
    int FunCode;		// 功能码
    int Mode;			//主从模式（0主机，1从机）
    int StartAddr;		// 起始偏移地址
    int ModbusTCPParaCnt;		// 自定义字段
    struct ModbusTCPPointPara ModbusTCPParaList[MAX_MODBUS_TCP_POINT_NUM];
};

/* 采集参数 */
/* 这个相当于是有线的参数集合*/
struct CollectPara {
    int VibChannelCnt;	// 自定义字段
    struct VibChannalPara VibChannelList[MAX_VIB_CH_NUM];		// 振动/音频通道列表
    int TechChannelCnt; // 自定义字段
    struct TechChannelPara TechChannelList[MAX_TECH_CH_NUM];	// 工艺通道列表
    int SpeedChannelCnt; // 自定义字段
    struct SpeedChannelPara SpeedChannelList[MAX_SPEED_CH_NUM];	// 转速通道列表
    int RtuChannelCnt;	// 自定义字段
    struct RtuChannelPara RtuChannelList[MAX_RTU_CH_NUM];		// Modbus Rtu通道列表
    int TriggerListCnt; // 自定义字段
    struct TriggerPara TriggerList[MAX_TRIGGER_PARA_NUM];		// 触发采集参数列表
    int StopLimitListCnt; // 自定义字段
    struct StopLimitInfo StopLimitList[MAX_VIB_CH_NUM];			// 启停机
    int TcpChannelCnt;	// 自定义字段
    struct ModbusTCPChannelPara ModbusTCPChannelList[MAX_MODBUS_TCP_CH_NUM];
};

#ifdef USE_WIRELESS_ENABLE
#define MAX_WLS_VIB_CH_NUM			45      /* 最大支持检通道 */
/* 这个相当于是无线的参数集合*/
struct Wireless_CollectPara {
    int VibChannelCnt;	// 自定义字段
#define START_CH_NO	 (101)	/*起始通道编号 */
#define END_CH_NO	 (200)  /*结束通道编号 */
    struct VibChannalPara VibChannelList[MAX_WLS_VIB_CH_NUM];		//无线振动通道列表
    int StopLimitListCnt; // 自定义字段
    struct StopLimitInfo StopLimitList[MAX_WLS_VIB_CH_NUM];			// 启停机
};

/* 这个相当于是总参数包括的有线+无线 */
struct All_CollectPara {
    int VibChannelCnt;	// 自定义字段
    struct VibChannalPara VibChannelList[MAX_VIB_CH_NUM + MAX_WLS_VIB_CH_NUM];		// 振动/音频/无线 通道列表
    int TechChannelCnt; // 自定义字段
    struct TechChannelPara TechChannelList[MAX_TECH_CH_NUM];	// 工艺通道列表
    int SpeedChannelCnt; // 自定义字段
    struct SpeedChannelPara SpeedChannelList[MAX_SPEED_CH_NUM];	// 转速通道列表
    int RtuChannelCnt;	// 自定义字段
    struct RtuChannelPara RtuChannelList[MAX_RTU_CH_NUM];		// Modbus Rtu通道列表
    int TriggerListCnt; // 自定义字段
    struct TriggerPara TriggerList[MAX_TRIGGER_PARA_NUM];		// 触发采集参数列表
    int StopLimitListCnt; // 自定义字段
    struct StopLimitInfo StopLimitList[MAX_VIB_CH_NUM + MAX_WLS_VIB_CH_NUM];			// 启停机
    int TcpChannelCnt;	// 自定义字段
    struct ModbusTCPChannelPara ModbusTCPChannelList[MAX_MODBUS_TCP_CH_NUM];
};
#endif

#ifdef MQTT_TRIGGER_PARAM_ENABLE
struct ProcessChTrigger {
    int ChNo;      //通道号
    float LowLimit;   //低值⻔限（-1为默认值)
    float HighLimit;  //⾼值⻔限（-1为默认值)
};

struct SpeedChTrigger {
    int ChNo;      //通道号
    int LowLimit;         //转速低值（-1为默认值）
    int HighLimit;        //转速高值（-1为默认值）
};

/* 触发采集参数 */
struct MS810TriggerParam {
    int DevId;          //设备id
    int AlarmCount;     //进入报警次数（1为默认值）
    int AlarmOutCount;  //取消报警次数（1为默认值)
    struct ProcessChTrigger ProcessChTriggers[MAX_TECH_CH_NUM];
    struct SpeedChTrigger SpeedChTriggers;
};

/* 触发采集参数 */
struct MS810_Trigger_Param {
    int MS810TriggerListCnt;    // 自定义字段
    struct MS810TriggerParam MS810TriggerParamList[MAX_MONITOR_DEV_NUM];
};
#endif

/* 指标数据 */
struct IndexTime {
    char Time[32];
};

struct IndexContent {
    int Dir;
    int Code;
    float *Value;
    int pos;
};

struct IndexData {
#define MAX_INDEX_CONTENT 32
    int ChNo;
    int ChType;
    int SampleTimeCount;
    struct IndexTime *SampleTime;
    int cur_pos;

    struct IndexContent Content[MAX_INDEX_CONTENT];
    int cur_content_pos;
};

/* 时序数据 */
struct TimeSeqData {
    int ChNo;		// 通道号
#define CH_TYPE_VIB		0
#define CH_TYPE_SPEED	1
#define CH_TYPE_GY		2
#define CH_TYPE_TEMP	3
#define CH_TYPE_AUDIO	4
#define CH_TYPE_RTU		5
#define CH_TYPE_WLESS	7
    int ChType;		// 通道类型（0=振动 1:转速 2:工艺,3-温度，4-音频，5-RTU）
#define INDEX_TYPE_NONE          0
#define VIB_CODE_AUDIO           0
#define INDEX_TYPE_TEMP          100
#define INDEX_TYPE_LOW_RMS       101
#define INDEX_TYPE_VEL_RMS       107
#define INDEX_TYPE_HIGH_RMS      112
#define INDEX_TYPE_HIGH_ENV_PEAK 121 /* 高频包络峰值 */
#define INDEX_TYPE_LOW_ENV_PEAK  123 /* 低频包络峰值 */
#define INDEX_TYPE_AUDIO_DB  	 124 /* 音频分贝 */
#define INDEX_TYPE_SPEED_FREQ    125 /* 转速频率 */
#define INDEX_TYPE_SPEED_RMS     126 /* 转速rms */
    int Code; 		// 指标类型（0-默认； 100-温度；101-低频总值；107-速度总值；112-高频总值；）
    int Dir;		// 方向（0-无方向；1-Z轴；2-X轴；3-Y轴）
    char Time[32];	// 开始时间
    int Interval; 	// 时间间隔（s）
    int Length;
    float *Value;	// 数值集合,由开始时间通过时间间隔叠加的得到每一个时间的值
};

/* 波形采集触发类型*/
typedef enum _wave_sample_trigger_type_def{
    TRIGGER_SRC_TYPE_NONE     = 0x01,
    TRIGGER_SRC_TYPE_ACC  	  = 0x02,
    TRIGGER_SRC_TYPE_VER  	  = 0x03,
    TRIGGER_SRC_TYPE_RPM  	  = 0x04,
    TRIGGER_SRC_TYPE_ACC_RPM  = 0x05,
    TRIGGER_SRC_TYPE_VER_RPM  = 0x06,
    TRIGGER_SRC_TYPE_SHOCK    = 0x07,
    TRIGGER_SRC_TYPE_RUNING   = 0x08,
    TRIGGER_SRC_TYPE_TEMP  	  = 0x09,
    TRIGGER_SRC_TYPE_PLC      = 0x0A,   /* PLC触发采集 */
    TRIGGER_SRC_TYPE_MANUAL   = 0x0B,   /* 手动触发采集 */
    TRIGGER_SRC_TYPE_ALARM    = 0x0C,   /* 报警触发采集 */
}wave_sample_trigger_type_t;

/* 波形数据 */
struct WaveDataReq {// 波形数据请求
    int Id;			// 波形唯一标识符，用于标识多个分包属于同一个波形
    int PackCount;	// 总分包数
    int PackIndex;	// 当前包序号
    int ChNo;		// 通道号
    int Dir;		// 波形方向（0-无方向；1-X轴；2-Y轴；3-Z轴）
    int Code;		// 指标类型（1-长波形；11-高频加速度；12低频加速度；15-速度波形；）
    char  Time[32];	// 开始时间
    int PlusLength;	// 附件长度
    int Freq;		// 频率（HZ）
    int RPM;		// 转速（RPM）
    int TimeMark;	// 转速时标
    float Coef;		// 转换系数
    int Src;		// 1:默认，2:加速度触发，3：速度触发，4：转速触发，5：加速度/转速触发，6：速度/转速，7：冲击触发，8：启机触发，9：温度触发
    int IsSensor;	// 0:未接传感器的空采数据，1：传感器数据
    u32 crc;		// 校验码，建议最后一包给出
    int Size;		// Content的长度
    int SendModel;	// 0-普通模式,     1-快速模式
    int ZipType;	// 0-不压缩,       1-压缩
    int AlarmLevel; //报警等级
    char Content[MAX_TRANSFER_SEGMENT_SIZE];
};

struct WaveDataResp  {// 波形数据响应
    int Id;			// 波形唯一标识符，用于标识多个分包属于同一个波形
    int PackIndex;	// 已接受包序号
};

#define DT_WAVE_QUICK_MODE /* 启用快速模式 */
struct WaveDataQuickRespDto       {  // 波形数据快速响应
#define MAX_WAVE_LENGTH (4*1024*1024)
#define MIN_WAVE_LENGTH (16*1024)
#define MAX_SEG_NUM             (MAX_WAVE_LENGTH / MIN_WAVE_LENGTH)  // 最大支持分包数
    int UnRceivePack[MAX_SEG_NUM];   // 未接收包
};

/* 时标数据 */
struct TimeMarkData {
    int Id;         //时标唯一标识符
    int PackCount;  //总包数
    int PackIndex;  //当前包数
    int ChNo;		// 通道号
    int Type;		// 时标类型（0-AD时标, 1-转速时标）
    char Time[32];	// 采样时间
    int *Content;	// 时标数据
    int TimeBase;	// 时基
    int Code;		// 指标类型（11-高频加速度；12低频加速度）
    int Length;		// 数据长度
};

/* 自诊断数据 */
struct __CheckData {
    int CheckType; 		// 自检项目类型（1-系统；2-偏执电压；3-音频通道）
    int Id; 			// 编号（通道号）
    float CheckValue; 	// 值
    int State; 			// 状态（0-异常；1-正常）
};

struct CheckData {
    int Count;
    struct __CheckData *data;
};

/* 传感器信息 */
struct SensorInfo {
    int ChNo;			// 通道号
    int SN;				// 传感器序列号
    int AppVer;			// 控制器版本
    int AppBuildCode;	// 控制器Build号
    int HardVer;		// 硬件版本信息
    float AccCoef;		// 加速度校准系数
    int DigitalFreq;	// 数传频段
    int SendPower;		// 发射功率（dbm）
    int StartState;		// 启动状态
    float Vol;			// 电池电压（V）
    int Cur;			// 电流（mA）
    int Temp;			// 温度（摄氏度）
    char Clock[32];		// 传感器时钟
    int LQI;			// LQI
    int RSSI;			// RSSI
    int TransTime;		// 传输时间
    int RetransCount;	// 重传次数
    int TotalPackets;	// 总包数
};

/* 电池信息 */
struct BatteryInfo {
    int ChNo;		// 通道号
    char Time[32];	// 时间
    float Vol;		// 电压（V）
    float Cur;		// 电流（mA）
    float Res ;		// 内阻（Ω）
    float RSSI;		// 信号强度
};

/* RTU */
struct RtuDataReq {
    int ChNo;		// 通道号
    char Time[32];	// 时间
    int DataLength; // 从上位机下发的参数中获取
    char Content[128];
    int ChType;  //通道类型（5-RTU, 6-TCP）
};

/* 日志 */
struct LogFileReq {
    char Md5Code[64];	// MD5码
    char FileName[64];
    int PackCount;	// 总分包数
    int PackIndex;	// 当前包序号
    int Size;		// 当前Content实际长度
    char  Content[MAX_FILE_TRANSFER_SEGMENT_SIZE];
};

struct LogFileResp {
    char Md5Code[64];	// MD5码
    int PackIndex;	// 已接受包序号
};

/* 升级 */
struct UpdateFile {
    char Name[32]; 	// 文件名称
    int Size;	 	// 字节数
    char Md5Code[64]; // MD5码
};

struct UpdateFileReq {
    char Md5Code[64];	// MD5码
    int PackCount;	// 总分包数
    int PackIndex;	// 当前包序号
    int PackMaxLength; // 每包最大传输字节长度
    int IsPlan;		// 1: 升级计划请求文件 0: 上位机主动下发
    int IsConfirm;	// 0: 常规请求包 1: 最后的确认包, 不回复
};

struct UpdateFileResp {
    char Md5Code[64];	// MD5码
    int PackCount;	// 总分包数
    int PackIndex;	// 当前包序号
    u32 size;		// 有效数据长度, 自己添加
    char  Content[MAX_FILE_TRANSFER_SEGMENT_SIZE];	// 内容
};

/* 上位机请求 */
struct IssueRestart {
    u8 type;	// 重启方式:0-硬重启,1-软重启
};

struct IssueUpdateFile {
    char Name[32]; 	// 文件名称
    int Size;	 	// 字节数
    char Md5Code[64]; // MD5码
};

struct NetPara {
    char LocalIp[16]; 	// 本机IP
    char ServerIp[16]; 	// 服务器IP
    char GateWay[16]; 	// 网关地址
    char SubnetMask[16]; 	// 子网掩码
    int ServerPort; 	// 服务器端口
    char Mac[20];			// Mac地址
};

/* 业务ID */
enum {
    /* 采集器请求 */
    DT_MQTT_CMD_ID_REGISTER = 0,
    DT_MQTT_CMD_ID_SYSPARA,
    DT_MQTT_CMD_ID_COLLECTPARA,
#ifdef MQTT_TRIGGER_PARAM_ENABLE
    DT_MQTT_CMD_ID_TRIGGERPARA,
#endif
    DT_MQTT_CMD_ID_INDEXDATA,
    DT_MQTT_CMD_ID_TIMESEQDATA,
    DT_MQTT_CMD_ID_WAVEDATA,
    DT_MQTT_CMD_ID_TIMEMARKDATA,
    DT_MQTT_CMD_ID_CHECKDATA,
    DT_MQTT_CMD_ID_SENSORINFO,
    DT_MQTT_CMD_ID_BATTERYINFO,
    DT_MQTT_CMD_ID_RTUDATA,
    DT_MQTT_CMD_ID_LOGFILEREQ,
    DT_MQTT_CMD_ID_UPDATEFILELIST,
    DT_MQTT_CMD_ID_UPDATEFILE,
    /* 上位机请求 */
    DT_MQTT_CMD_ID_ISSUERESTART,
    DT_MQTT_CMD_ID_ISSUEPARA,
    DT_MQTT_CMD_ID_ISSUEUPDATEFILE,
    DT_MQTT_CMD_ID_ISSUECHECKDATA,
    DT_MQTT_CMD_ID_ISSUENETPARA,
    DT_MQTT_CMD_ID_STARTWAVE,
    DT_MQTT_CMD_ID_MAX
};
#define MAX_SENSOR_CNT  45
#define DT_MQTT_CMD_REGISTER			"Register"
#define DT_MQTT_CMD_SYSPARA				"SysPara"
#define DT_MQTT_CMD_COLLECTPARA			"CollectPara"
#ifdef MQTT_TRIGGER_PARAM_ENABLE
#define DT_MQTT_CMD_TRIGGERPARA			"MS810TriggerParams"
#endif
#define DT_MQTT_CMD_INDEXDATA			"IndexData"
#define DT_MQTT_CMD_TIMESEQDATA			"TimeSeqData"
#define DT_MQTT_CMD_WAVEDATA			"WaveData"
#define DT_MQTT_CMD_TIMEMARKDATA		"TimeMarkData"
#define DT_MQTT_CMD_CHECKDATA			"CheckData"
#define DT_MQTT_CMD_SENSORINFO			"SensorInfo"
#define DT_MQTT_CMD_BATTERYINFO			"BatteryInfo"
#define DT_MQTT_CMD_RTUDATA				"RtuData"
#define DT_MQTT_CMD_LOGFILEREQ			"LogFile"
#define DT_MQTT_CMD_UPDATEFILELIST		"UpdateFileList"
#define DT_MQTT_CMD_UPDATEFILE			"UpdateFile"
#define DT_MQTT_CMD_ISSUERESTART		"IssueRestart"
#define DT_MQTT_CMD_ISSUEPARA			"IssuePara"
#define DT_MQTT_CMD_ISSUEUPDATEFILE		"IssueUpdateFile"
#define DT_MQTT_CMD_ISSUECHECKDATA		"IssueCheckData"
#define DT_MQTT_CMD_ISSUENETPARA		"IssueNetPara"
#define DT_MQTT_CMD_STARTWAVE			"StartWave"

/* 业务执行结果 */
enum {
    DT_MQTT_CMD_STATUS_NONE = 0,			/* 每条命令发送前的状态 */
    DT_MQTT_CMD_STATUS_SENDING,				/* 命令发送中 */
    DT_MQTT_CMD_STATUS_SENDING_FAILED,		/* 命令发送失败 */
    DT_MQTT_CMD_STATUS_SENDING_OK,			/* 命令发送成功 */
    DT_MQTT_CMD_STATUS_RECV_TIMEOUT,		/* 没有在规定时间内接收到命令回复 */
    DT_MQTT_CMD_STATUS_RECV_OK,				/* 命令执行成功 */
    DT_MQTT_CMD_STATUS_RECV_STATUS_FAILED,	/* 接收到命令回复, 但是status = 0 */
    DT_MQTT_CMD_STATUS_RECV_MSGID_FAILED,	/* 接收到命令回复, 但是msgid不匹配 */
    DT_MQTT_CMD_STATUS_PARSE_ERROR,			/* 接收到的数据json解析失败 */
    DT_MQTT_CMD_STATUS_NETWORK_ISSUE,
};
struct dt_mqtt_ops {
    u8 id;		/* 业务ID */
    char *key;	/* 业务关键字 */
    u8 (*callback)(struct dt_mqtt_ops *ops, char *payload);
    u8 status;	/* 业务执行结果 */
    u8 flag;	/* 0: 采集器请求, 1: 上位机请求 */
    u32 msgid;
    char *payload;/* 收到的数据指针 */
    u32 max_len;
#ifdef SYSTEM_LINUX
    pthread_mutex_t mtx;
    pthread_cond_t cond;
#endif
};

/* mqtt命令执行结果 */
enum {
    DT_MQTT_ERR = 0,
    DT_MQTT_OK,
    DT_MQTT_OK_SAME_SYSPARA, /* 策略参数专用 */
    DT_MQTT_OK_SAME_COLLECPARA, /* 采集参数专用 */
#ifdef MQTT_TRIGGER_PARAM_ENABLE
    DT_MQTT_OK_SAME_TRIGGERPARA, /* MS810触发参数专用 */
#endif
};

/* 上位机主动命令 */
enum {
    HOST_CMD_REBOOT = 0,	/* 重启 */
    HOST_CMD_GETPARA,		/* 获取参数 */
    HOST_CMD_UPGRADE,		/* 升级 */
    HOST_CMD_SEFL_CHECK,	/* 自诊断 */
    HOST_CMD_NETPARA,		/* 网络信息 */
    HOST_CMD_MANUAL_SAMPLE, /*手动触发采集*/
    MQTT_SERVER_START = 0xFF
};

/* 系统工作状态 */
typedef enum _dt_sys_status_def {
    SYS_IDLE = 0X00,    /* 表示系统空闲 */
    SYS_RUN,            /* 表示系统正常运行 */
    SYS_DBG,            /* 表示进入DBG模式 */
    SYS_UPG,            /* 表示进入UPG模式 */
    SYS_SAMPLE,         /* 表示正在采样 */
    SYS_DATA_UPLOAD,    /* 表示在线数据上传 */
    SYS_OFFLINE_UPLOAD, /* 表示离线数据上传 */
    SYS_MAX,
}dt_sys_status_t;

//=====================================
/* 参数结果 */
typedef enum _param_error_code_def{
    DT_PARAM_SUCCESS = 0x00,
    DT_PARAM_OPEN_FAILURE,
    DT_PARAM_WRITE_FAILURE,
    DT_PARAM_GENERIC_FAILURE,
}param_error_code_t;

/* 魔幻数 */
#define SYS_PARAM_MAGIC             0x11223344
#define MQTT_SYS_PARAM_MAGIC        0x22345678
#define MQTT_COLLECT_PARAM_MAGIC    0x32345678
#define MQTT_TRIGGER_PARAM_MAGIC    0x42345678

/* 参数 */
//#define UBNUTU_ENVIRONMENT
#ifdef UBNUTU_ENVIRONMENT
#define CONFIG_DIR              "./usr/config"
#else
#define CONFIG_DIR              "/root/AppConfig"          /* 这里考虑实际的存放目录       */
#endif
#define SYS_CONF_FILE           CONFIG_DIR"/sysconf.ini"        /* 系统配置 */
#define MQTT_SYS_PARA_FILE      CONFIG_DIR"/syspara.ini"        /* MQTT策略参数 */
#define MQTT_COLLECT_PARA_FILE  CONFIG_DIR"/collectpara.ini"    /* MQTT采集参数 */
#define MQTT_COLLECT_WLS_PARA_FILE  CONFIG_DIR"/wireless_collectpara.ini"    /* 无线MQTT采集参数 */
#ifdef MQTT_TRIGGER_PARAM_ENABLE
#define MQTT_TRIGGER_PARA_FILE  CONFIG_DIR"/triggerpara.ini"    /* MQTT触发参数 */
#endif

#define DELETE_FILE             "/root/del_file.ini"

/* 参数备份 */
#ifdef UBNUTU_ENVIRONMENT
#define BACK_CONFIG_DIR         "./usr/backconfig"
#else
#define BACK_CONFIG_DIR         "/root/backup"          /* 这里考虑实际的存放目录       */
#endif
#define BACK_SYS_CONF_FILE          BACK_CONFIG_DIR"/sysconf.ini"       /* 系统配置 */
#define BACK_MQTT_SYS_PARA_FILE     BACK_CONFIG_DIR"/syspara.ini"       /* MQTT策略参数 */
#define BACK_MQTT_COLLECT_PARA_FILE BACK_CONFIG_DIR"/collectpara.ini"   /* MQTT采集参数 */
#define BACK_MQTT_COLLECT_WLS_PARA_FILE  BACK_CONFIG_DIR"/wireless_collectpara.ini"    /* 无线MQTT采集参数 */
#ifdef MQTT_TRIGGER_PARAM_ENABLE
#define BACK_MQTT_TRIGGER_PARA_FILE  BACK_CONFIG_DIR"/triggerpara.ini"    /* MQTT触发参数 */
#endif

typedef struct _dt_file_content_head_def {
    /* 魔幻数 */
    unsigned int  magic;
    /* 参数版本 */
    unsigned char  version;
    /* 系统参数校验       */
    unsigned int crc;
    unsigned int len;
}__attribute__((packed)) dt_file_head_t;

typedef struct _dt_file_content_def{
    void *data;
    unsigned len;
}__attribute__((packed)) dt_file_content_t;


/* 参数版本记录*/
#define PARAM_VERSION_MAJOR (1)

typedef struct _file_info_def{
    unsigned char  version; /* 参数版本 */
    unsigned int   magic;   /* 魔幻数 */
    unsigned int   length;  /* 比较长度 */
}__attribute__((packed)) file_info_t;








#ifdef __cplusplus
}
#endif

#endif // ZIGBEETYPE_H
