#ifndef SPI_MANAGER_H
#define SPI_MANAGER_H
#ifdef __cplusplus
extern "C" {
#endif
#include "proto_parse.h"
//#include "dt_mqtt_protocol.h"
#define SPI_NOR_FLASH_SIZE           (64*1024*1024)

#define WLESS_WAVE_BUFF_SIZE        (4*1024*1024+sizeof(uart_0x34_indicator_data_t))
#define WLESS_EIGEN_BUFF_SIZE       (1024*1024)
#define WLESS_MCU_LOG_BUFF_SIZE     (4*1024*1024)
#define WLESS_SENSOR_LOG_BUFF_SIZE  (1024*1024)

typedef enum
{
    A_IND  = 0x01,
    B_IND  = 0x02,
    NORMAL_SYNC_MEA  = 0x03,
    NORMAL_ASYNC_MEA = 0x04,
    NORMAL_LW = 0x05,
    START_MEA = 0x06,
    TEMP_ALARM_MEA = 0x07,
    IND_ALARM_MEA  = 0x08,
    IND_ALARM_LW   = 0x09,
}SPI_FLASH_DATA_TYPE;

#define SPI_WAVE_INFO_HEAD                   0xCCDDEFEF

#define SPI_AIND_DATA_HEAD                   0x11223344
#define SPI_BIND_DATA_HEAD                   0xAABBCCDD
#define SPI_STATION_LOG_HEAD                 0x44DD33AA
#define SPI_SENSOR_LOG_HEAD                  0x66778899

typedef struct
{
    unsigned char   ChannelNo;          // 1
    unsigned int    MeasPointID;        // 5
    unsigned int    MeasDefID;          // 9
    unsigned char   SampleTime[7];      // 16
    float           SampleValue;        // 20
    float           ConverCoef;         // 24
    unsigned int    RPM;                // 28
    unsigned char   DataSource;         // 29
    unsigned char   DataSection;        // 30
    unsigned char   WaveType;           // 31
    unsigned char   UpTag;              // 32
    unsigned int    TimeTag;            // 36
    unsigned int    TimeBase;           // 40
    unsigned char   DataType;           // 41
    unsigned int    SensorSN;           // 45
    unsigned int    WaveFileSize;       // 49
    unsigned int	Freq;
    unsigned char   Rsvd[75];           // 128
}__attribute__((packed)) wavedata_index_t;

// =========spi nor flash storage struct, start========
// 波形存放区域头信息
typedef struct
{
    uint32_t head;
    uint8_t indexVer;  //索引版本号
    uint8_t channel;   //传感器通道号
    uint8_t senDir;    //传感器数据方向
    uint8_t dataSaveOk;//传感器数据是否存储成功
    uint8_t dataUpOk;  //传感器数据是否上传成功
    uint8_t packSeq;   //64KB已存储的包序号
    uint8_t packNum;   //64KB总包数
    uint8_t dataType;  //数据类型
    uint32_t dataTime; //数据时间
    uint32_t dataSize; //数据大小
    uint16_t startBlk; //起始块
    uint16_t endBlk;   //终止块
    uint32_t curSavePage; //当前存储页号
    uint16_t blkNum;      //块数量
    //uint16_t blkBuf[SIG_WAVE_MAX_BLK_NUM];  //块号,不需要，没用，直接用startblk和endblk，中间没有坏块
}__attribute__((packed)) spi_storage_wave_head_t;
    //uint8_t  chk;

// 波形索引
typedef struct
{
    uint8_t  head[4];   //68 01 57 68
    uint8_t  isSave;
    uint8_t  dataFlag;
    uint32_t meaPointID;
    uint32_t meaDefID;
    uint8_t  industUnit[6];
    uint8_t  reserved;
    uint8_t  addLenMult;
    uint16_t addLen;
    uint8_t  sampleTime[7];
    float    sampleVal;
    uint8_t  alarmLevel;
    uint8_t  isSpectrum;
    uint32_t sampleLen;
    float    sampleFreq;
    uint32_t runRPM;
    uint32_t highestRPM;
    uint32_t lowestRPM;
    uint16_t chkSum;    //16位波形校验和
    float    convCoe;
    uint8_t  isCompress;
    uint8_t  signalType;//信号轴向
    uint8_t  tag[4];
    uint8_t  dataSource;
    uint8_t  isSteel;
    uint16_t gear;
    uint16_t errorCode;
    char senSn[10];
    uint8_t  freqType;
    uint8_t  backUp[4];
    uint8_t  meaEnd[6]; //FE EF FD DF FC CF

    uint8_t rvsd[160];  //保留，保证该结构体大小为 256Byte
}__attribute__((packed)) spi_storage_measdef_index_t;

typedef struct
{
    uint8_t  head[4];   //68 01 55 68
    uint8_t  isSave;
    uint32_t meaPointID;
    uint8_t  industUnit[6];
    uint8_t  freqType;
    uint8_t  dataRange;
    uint8_t  signalType;
    uint8_t  res;
    uint8_t  sampleTime[7];
    float    sampleVal;
    uint8_t  alarmLevel;
    uint32_t dataLength;
    uint32_t freq;
    uint32_t runRPM;
    float    convCoe;
    uint8_t  isCompress;
    uint8_t  steady1Intv;
    uint8_t  steady2Intv;
    uint8_t  steady3Intv;
    uint16_t chkSum;    //16位波形校验和
    uint8_t  sampleMult;
    uint32_t tag;
    uint8_t  longEnd[6]; //FE EF FD DF FC CF

    uint8_t rvsd[192];  //保留，保证该结构体大小为 256Byte
}__attribute__((packed)) spi_storage_longwave_index_t;

typedef struct
{
    uint32_t indTime;
    float piez_high_rms;
    uint8_t piez_high_rms_alarm;
    float piez_ver;
    uint8_t piez_ver_alarm;
    float memsx_ver;
    uint8_t memsx_ver_alarm;
    float memsy_ver;
    uint8_t memsy_ver_alarm;
}__attribute__((packed)) spi_storage_a_ind_data_t;

typedef struct
{
    uint32_t  head; //0x11223344
    spi_storage_a_ind_data_t aindData;
    uint8_t res[4];
}__attribute__((packed)) spi_storage_a_ind_t;//32B

typedef struct {
    float dataVal;                  //指标值
    unsigned char isValue;          //是否有指标值
    unsigned char isWave;           //该指标是否存在波形(只有报警才会有波形)
} indicator_value_t;

typedef struct
{
    unsigned char ChannelNo;          // 1
    unsigned int  MeasPointID;        // 5
    unsigned char SampleTime[7];      // 12
    unsigned char UpTag;              // 13
    indicator_value_t stIndData[13];  // 91   6个高频指标(0-5), 7个低频指标(6-12)
	indicator_value_t temp;
	unsigned char Rsvd[31];
} eigendata_index_t;

typedef struct
{
    uint32_t indTime;
    float piez_high_kurt;
    uint8_t piez_high_kurt_alarm;
    float piez_high_e1;
    uint8_t piez_high_e1_alarm;
    float piez_high_e2;
    uint8_t piez_high_e2_alarm;
    float piez_high_e3;
    uint8_t piez_high_e3_alarm;
    float piez_high_e4;
    uint8_t piez_high_e4_alarm;
    float piez_low_rms;
    uint8_t piez_low_rms_alarm;
    float piez_low_e1;	// 低频包络峰值
    uint8_t piez_low_e1_alarm;
    float piez_low_e2;	// 高频包络峰值
    uint8_t piez_low_e2_alarm;
    float memsx_rms;
    uint8_t memsx_rms_alarm;
    float memsx_e1;		// WS2, 音频分贝
    uint8_t memsx_e1_alarm;
    float memsx_e2;		// WS2, 转频频率
    uint8_t memsx_e2_alarm;
    float memsy_rms;
    uint8_t memsy_rms_alarm;
    float memsy_e1;		// WS2, 转频rms
    uint8_t memsy_e1_alarm;
    float memsy_e2;
    uint8_t memsy_e2_alarm;
    signed char temp;
}__attribute__((packed)) spi_storage_b_ind_data_t;

typedef struct
{
    uint32_t  head; //0xAABBCCDD
    spi_storage_b_ind_data_t bindData;
    uint8_t res[49];
}__attribute__((packed)) spi_storage_b_ind_t;//128B

typedef struct
{
    char station[32];
    char sn[32]; // sensor sn
    char channel[32];
    char dir[32];
    char freq[32];
    char len[32];
    char sampletime[32];
    char coe[32];
    uint8_t data[1024];
} SampleInfo;

typedef struct
{
    unsigned char    head[4];
    unsigned short   battery_num;
    unsigned char    Rsvd;
}__attribute__((packed)) battery_info_head_t;

typedef struct
{
    unsigned char   ChannelNo;
    unsigned char   SN[10];
    unsigned char   SampleTime[7];
    unsigned char   State;
    float           PeakCurrent;
    float           PeakVol;
    float           EnergyConsumption;
    unsigned int    WorkTime;
    float           Resistor;
}__attribute__((packed)) battery_info_index_t;

typedef struct
{
    unsigned int    ChannelNo;          // 4
    unsigned int    MeasPointID;        // 8
    unsigned int    SensorSN;           // 12
    unsigned int    SoftwareVer;        // 16
    unsigned int    SoftwareBuild;      // 20
    unsigned int    HardwareVer;        // 24
    float           AccCoef;            // 28
    unsigned int    CommFreq;           // 32
    unsigned int    SendPower;          // 36
    unsigned int    StartState;         // 40
    float           BatteryVol;         // 44
    unsigned int    Current;            // 48
    int             Temperature;        // 52
    char            SensorTime[7];      // 59
    int             LQI;                // 63
    int             RSSI;               // 67
    unsigned int    TransTime;          // 71
    unsigned int    RetransTimes;       // 75
    unsigned int    TotalPackets;       // 79
    unsigned char   Rsvd[45];           // 124
}__attribute__((packed)) sensor_info_index_t;

typedef struct
{
    uint32_t  head; //0x66778899
    sensor_info_index_t sensorlog;
}__attribute__((packed)) spi_storage_sensor_log_t;//128B

typedef struct //日志
{
    int logIndex;
    int logValue;
    char systemWork;
    char logTime[7];
}__attribute__((packed)) mcu_log_index_t;

typedef struct
{
    uint32_t  head; //0x44DD33AA
    mcu_log_index_t stationlog;
    uint8_t res[12];
}__attribute__((packed)) spi_storage_station_log_t;//32B

// spi manager

int spi_read_flash_data(unsigned char type, uart_cmd_0x36_t info, uart_cmd_new_0x36_t new_info);

int spi_nor_flash_check(void);

#ifdef __cplusplus
}
#endif
#endif // SPI_MANAGER_H
