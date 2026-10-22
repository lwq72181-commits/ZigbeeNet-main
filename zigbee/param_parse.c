#include "param_parse.h"
//#include "dt_upgrade_manager.h"

static sensor_config_info_t sensor_info;

FREQ_PUBLIC sensor_config_info_t *sensor_info_get(void)
{
    return &sensor_info;
}

FREQ_PRIVATE void _sensor_config_table_printf(void)
{
    DT_WLS_LOG("*********sensor config table info:************\n");
    DT_WLS_LOG("  is_valid:%d, station:%s\n", sensor_info.is_valid, sensor_info.station);
    for (int i = 0; i < MAX_WLS_VIB_CH_NUM; i++) {
        if (sensor_info.sensor[i].channel != 0) {
            DT_WLS_LOG(" channel:%d, sn:%s, devnumber:%s\n",
                sensor_info.sensor[i].channel, sensor_info.sensor[i].sn, sensor_info.sensor[i].devnumber);
        }
    }
    DT_WLS_LOG("*********************************************\n");
}

/**
 * @brief 传感器配置表文件解析
 *        主要功能，检查配置表是否存在，若存在则将配置信息加载到内存
 *
 * @return int
 */
FREQ_PRIVATE int _sensor_config_table_parse(char *pfilename)
{
    int ret = DT_ERROR;
    int sensor_fd  = -1;
    int sensor_num = 0;
    char *pbuff    = NULL;
    int buffer_size = 0;
    sensor_config_info_t temp_info;

    if (!pfilename) {
        DT_WLS_ERR("pfilename is null.\n");
        goto _exit;
    }

    memset(&temp_info, 0x00, sizeof(temp_info));
    //判断配置表是否存在
    if (access(pfilename, F_OK) != 0) {
        DT_WLS_ERR("sensor config table inexist.\n");
        goto _exit;
    }

    //判断文件大小
    buffer_size = dt_get_file_size(pfilename);
    pbuff = (char *)calloc(1, buffer_size);
    if (!pbuff) {
        DT_WLS_LOG("pbuff malloc failed.\n");
        goto _exit;
    }

    //读数据到内存
    sensor_fd = open(pfilename, O_RDONLY);
    if (sensor_fd < 0) {
        DT_WLS_ERR("open(%s) failed.\n", pfilename);
        goto _exit;
    }
    if (-1 == read(sensor_fd, pbuff, buffer_size)) {
        DT_WLS_ERR("read(%s) failed.\n", pfilename);
        goto _exit;
    }
    // DT_WLS_LOG("pbuff:%s\n", pbuff);

    //解析json格式内容
    cJSON *root = cJSON_Parse((char *)pbuff);
    if (!root) {
        DT_WLS_ERR("json format err.\n");
        goto _exit;
    }

    cJSON *sensor = cJSON_GetObjectItem(root, "sensor");
    if (!sensor) {
        DT_WLS_ERR("missing field(sensor).\n");
        goto _exit;
    }

    if (!cJSON_IsArray(sensor)) {
        DT_WLS_ERR("sensor is not array.\r\n");
        goto _exit;
    }

    for (int i = 0; i < MAX_WLS_VIB_CH_NUM; i++) {
        cJSON *obj = cJSON_GetArrayItem(sensor, i);
        if (!obj)
            break;
        cJSON *devnumber = cJSON_GetObjectItem(obj, "devnumber");
        cJSON *channel = cJSON_GetObjectItem(obj, "channel");
        cJSON *sn = cJSON_GetObjectItem(obj, "sn");

        if (!devnumber || !channel || !sn) {
            DT_WLS_ERR("missing fields(%d).\r\n", i);
            goto _exit;
        }

        if (channel->valueint <=0 || channel->valueint > MAX_WLS_VIB_CH_NUM) {
            DT_WLS_ERR("config tab channel err(%d).\r\n", channel->valueint);
            goto _exit;
        }
        int index = channel->valueint - 1;
        temp_info.sensor[index].bind = 1;
        temp_info.sensor[index].channel = channel->valueint;
        memcpy(temp_info.sensor[index].devnumber, devnumber->valuestring, sizeof(temp_info.sensor[index].devnumber));
        memcpy(temp_info.sensor[index].sn, sn->valuestring, sizeof(temp_info.sensor[index].sn));
        sensor_num++;
    }

    temp_info.is_valid = DT_EOK;
    memset(&sensor_info, 0x00, sizeof(sensor_info));
    memcpy(&sensor_info, &temp_info, sizeof(sensor_info));

    DT_WLS_LOG("get %d sensor info.\n", sensor_num);
    sensor_param_printf();
    ret = DT_EOK;

_exit:
    if (sensor_fd > 0) {
        close(sensor_fd);
        sensor_fd = -1;
    }
    if (pbuff) {
        dt_free(pbuff);
        pbuff = NULL;
    }

    return  ret;
}

static sensor_param_info_t sensor_param_info = {
    .parse  = _sensor_config_table_parse,
    .printf = _sensor_config_table_printf,
};

FREQ_PUBLIC int sensor_param_parse(char *pfilename)
{
    if (sensor_param_info.parse)
        return sensor_param_info.parse(pfilename);
    else
        DT_WLS_ERR("parse is null.\n");

    return DT_ERROR;
}

FREQ_PUBLIC void sensor_param_printf(void)
{
    if (sensor_param_info.printf)
        sensor_param_info.printf();
    else
        DT_WLS_ERR("printf is null.\n");
}
