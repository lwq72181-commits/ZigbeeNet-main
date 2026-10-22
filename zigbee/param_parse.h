#ifndef _DT_PARAM_PARSE_H
#define _DT_PARAM_PARSE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
#include "proto_parse.h"

#pragma pack (1)
/***************无线传感器信息***********************/
typedef struct sensor_info_def {
    unsigned char bind;
    char sn[10];
    unsigned char channel;
    char devnumber[32];
}sensor_info_t;

typedef struct sensor_config_info_def {
    char is_valid;      //0:无效， 1：有效
    char station[16];
    sensor_info_t sensor[MAX_SENSOR_CNT];
}sensor_config_info_t;

typedef struct sensor_param_info_def {
    int (*parse)(char *pfilename);
    void (*printf)(void);
}sensor_param_info_t;

#pragma pack ()

FREQ_PUBLIC sensor_config_info_t *sensor_info_get(void);
FREQ_PUBLIC int sensor_param_parse(char *pfilename);
FREQ_PUBLIC void sensor_param_printf(void);
#ifdef __cplusplus
}
#endif
#endif // !_DT_PARAM_PARSE_H
