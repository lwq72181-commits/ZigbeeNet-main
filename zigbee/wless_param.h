#ifndef __WLESS_PARAM_H__
#define __WLESS_PARAM_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
#include "zigbeetype.h"
#define WLESS_PARAM_PATH            "/userdata/"
#define WLESS_PARAM_FILE            "/etc/wless_param"
#define WLESS_PARAM_FLAG_FILE       "param_update_flag"

#define UNIT_LENGTH 9

#define LOW_FREQ_RMS          10    //总值（低频）
#define LOW_FREQ_KURT         11    //峭度（低频）
#define LOW_ENERGY_1          12    //超低频段能量（低频）
#define LOW_ENERGY_2          13    //低频段能量（低频）
#define LOW_ENERGY_3          14    //中频段能量（低频）
#define LOW_ENERGY_4          15    //高频段能量（低频）
#define VEL_RMS               16    //总值（速度）

#define HIGH_FREQ_RMS         21    //总值（高频）
#define HIGH_FREQ_KURT        22    //峭度（高频）
#define HIGH_ENERGY_1         23    //低频段能量（高频）
#define HIGH_ENERGY_2         24    //中频段能量（高频）
#define HIGH_ENERGY_3         25    //高频段能量（高频）
#define HIGH_ENERGY_4         26    //超高频段能量（高频）

typedef struct {
    unsigned char                   UpTimeBuf[7];            //参数时间
    unsigned int                    VibIndexPeriod;          //指标采集周期
    unsigned int                    MeasDefPeriod;           //测量定义采集周期
    unsigned int                    TemperaturePeriod;       //温度采集周期
    unsigned int                    LongWavePeriod;          //长波形采集周期
    unsigned int                    LongWaveLength;          //长波形采集长度
    unsigned int                    LongWaveFreq;            //长波形采集频率
    unsigned int                    LowFreqDaqLength;        //低频采集长度
    unsigned int                    LowFreqSaveLength;       //低频保存长度
    unsigned int                    LowDaqFreq;              //低频采集频率
    unsigned int                    HighFreqDaqLength;       //高频采集长度
    unsigned int                    HighFreqSaveLength;      //高频保存长度
    unsigned int                    HighDaqFreq;             //高频采集频率
    unsigned char                   IndexResetPeriod;        //指标门限重置周期
    unsigned char                   VibChannelNum;          //振动通道个数
}__attribute__((packed)) wless_common_param_t;


typedef struct wless_param_flag_def {
#define FRAM_WLESS_PARAM_FLAG_HEAD (0xADADACAC)
#define FRAM_WLESS_PARAM_FLAG_TAIL (0xCACADADA)
    int head;
    int flag;
    int tail;
}wless_param_flag_t;

FREQ_PUBLIC int wless_param_update_flag_get(int *value);
FREQ_PUBLIC void wless_param_update_flag_set(int value);
int wless_param_convert_to_file(struct SysPara *pSysPara, struct Wireless_CollectPara *pCollectPara);

#ifdef __cplusplus
}
#endif
#endif
