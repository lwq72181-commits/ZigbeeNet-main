#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "proto_common.h"
#include "proto_handle.h"
//#include "dt_param.h"
//#include "wifi_pro.h"
#include "dt_uart_mcu_board.h"
#include "wrapper.h"
static wless_param_t wless_param;
//extern ZIGBEEParam *g_ZIGBEEParam;
extern int g_switch_flag;
wless_param_t *get_wless_param(void)
{
    return &wless_param;
}

static wless_data_t wless_data;
wless_data_t *get_wless_data(void)
{
    return &wless_data;
}

static int mpu_config_mcu_flag = false;
int get_mpu_config_mcu_flag(void)
{
    return mpu_config_mcu_flag;
}

void set_mpu_config_mcu_flag(int flag)
{
    mpu_config_mcu_flag = flag;
}

static int task_run_state;

void init_task_run_state(void)
{
    task_run_state = TASK_SUCCESS;
}

void set_task_run_state(unsigned short state)
{
#if 0
    if(state == TASK_RUNNING)
    {
        task_run_state += TASK_RUNNING;
    }
    else
    {
        if(task_run_state > 0)
            task_run_state -= TASK_RUNNING;
        if((task_run_state & state) == 0x0)
            task_run_state += state;
    }
#else
    task_run_state = state;
#endif

#ifdef SNESOR_DBG_PRINTF_ENABLE
    DT_WLS_LOG("state:%d,  task_run_state:%d\r\n", state, task_run_state);
#endif
}

unsigned short get_task_run_state(void)
{
    return task_run_state;
}

unsigned char get_all_task_run_state(void)
{
    if(get_task_run_state() == TASK_RUNNING)
        return TASK_CAN_NOT_RUN;

    return TASK_CAN_RUN;
}

static volatile unsigned char cur_task_type = 0;
static volatile unsigned char cur_task_state= 0; //00成功

void set_cur_task_type(unsigned char type)
{
    cur_task_type = type;
}

unsigned char get_cur_task_type()
{
    return cur_task_type;
}

void set_cur_task_state(unsigned char state)
{
    cur_task_state = state;
}

unsigned char get_cur_task_state(void)
{
    return cur_task_state;
}
unsigned int ComputeCRC32(unsigned char *buf, unsigned int len)
{
    unsigned int crc;
    unsigned int temp;
    unsigned int j;

    crc = 0xFFFFFFFF;
    for(unsigned int i=0;i<len;i++)
    {
        temp = (unsigned int)((crc & 0xFF) ^ buf[i]);
        for (j = 0; j < 8; j++)
        {
            if (temp & 0x1)
                temp = (temp >> 1) ^ 0xEDB88320;
            else
                temp >>= 1;
        }
        crc = (crc >> 8) ^ temp;
    }
    return crc ^ 0xFFFFFFFF;
}
#if 0
int mpu_get_seneor_config(wls_sensor_info_t *ps_info)
{
    int ret = 0;
    uart_cmd_0x40_0x03_t reply;
    memset(&reply, 0, sizeof(reply));

    uart_cmd_0x40_0x03_ret_t ret_data;
    memset(&ret_data, 0, sizeof(ret_data));
    uint32_t ret_len = 0;

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_DEBUG_TASK;
    reply.header.len = sizeof(reply);
    reply.TaskType = 0x03;

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    // ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    if (DT_EOK != uart_send_with_recv(SERIAL_UART4, &reply, sizeof(reply), &ret_data, &ret_len, 2000)) {
        DT_WLS_ERR("wireless uart send with rec error\r\n");
        return DT_ERROR;
    }



    printf("get sensor info len = %d\r\n", ret_len);
    printf("-----sensor data:\r\n");
    uint8_t *p = (uint8_t *)&ret_data;
    for (uint32_t i = 0; i < ret_len; i++) {
        printf("%02x ", p[i]);
        if ((i + 1) % 32 == 0) {
            printf("\r\n");
        }
    }
    printf("\r\n");
    if (ret_data.header.head != CMD_SOURCE_MCU) {
        return -1;
    }

    if (ret_data.header.cmd != UART_CMD_DEBUG_TASK) {
        return -1;
    }

    // ret_len = puart->recv_len;

    if (ret_data.header.len != ret_len) {
        return -1;
    }

    if (ret_data.tailer.cs != dt_compute_cs((unsigned char *)&ret_data, (ret_len-1))) {
        return -1;
    }

    memcpy(ps_info, (wls_sensor_info_t *)&ret_data.Info, MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t));

    return ret;
}


int mpu_set_seneor_config(uint8_t *ps_info)
{
    int ret = 0;
    uart_cmd_0x40_0x03_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    uart_cmd_0x40_0x04_ret_t ret_data;
    memset(&ret_data, 0, sizeof(ret_data));
    uint32_t ret_len = 0;

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_DEBUG_TASK;
    reply.header.len = sizeof(reply);
    reply.TaskType = 0x04;
    memcpy(reply.Info, ps_info, MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t));

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    // ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));

    printf("-----set_data:\r\n");
    uint8_t *p = (uint8_t *)&reply;
    for (uint32_t i = 0; i < sizeof(reply); i++) {
        printf("%02x ", p[i]);
        if ((i + 1) % 32 == 0) {
            printf("\r\n");
        }
    }

    if (DT_EOK != uart_send_with_recv(SERIAL_UART4, &reply, sizeof(reply), &ret_data, &ret_len, 2000)) {
        DT_WLS_ERR("wireless uart send with rec error\r\n");
        return DT_ERROR;
    }

    printf("ret_len = %d\r\n", ret_len);
    printf("-----ret_data:\r\n");
    p = (uint8_t *)&ret_data;
    for (uint32_t i = 0; i < ret_len; i++) {
        printf("%02x ", p[i]);
        if ((i + 1) % 32 == 0) {
            printf("\r\n");
        }
    }
    printf("\r\n");

    if (ret_data.header.head != CMD_SOURCE_MCU) {
        return -1;
    }

    if (ret_data.header.cmd != UART_CMD_DEBUG_TASK) {
        return -1;
    }

    if (ret_data.header.len != ret_len) {
        return -1;
    }

    if (ret_data.tailer.cs != dt_compute_cs((unsigned char *)&ret_data, (ret_len-1))) {
        return -1;
    }

    if (ret_data.State != 1) {
        return -1;
    }

    return ret;
}
#endif

int mpu_get_param_info_form_mcu(void *arg)
{
    mcu_cmd_0x40_0x01_req_t req;
    mcu_cmd_0x40_0x01_rsp_t rsp;

    memset(&req, 0, sizeof(req));
    memset(&rsp, 0, sizeof(rsp));

    req.TaskType = TASK_TYPE_READ_PARAM;

    if (DT_EOK != uart_mcu_board_data_trx(UART_CMD_DEBUG_TASK, &req, sizeof(req), &rsp, sizeof(rsp), 2000)) {
        DT_WLS_ERR("mcu cmd 0x%0x failed\r\n", UART_CMD_DEBUG_TASK);
        return DT_ERROR;
    }

    dt_uart_mcu_board_t * t = (dt_uart_mcu_board_t *) arg;
    if (DT_ENABLE != t->is_exist) {
        t->is_exist  = DT_ENABLE;
        snprintf(t->version, sizeof(t->version), "%s", rsp.GwParam.mcuSoftVer);
    }

    t->freq_band = rsp.GwParam.zgbFreqBand;
    DT_WLS_LOG("version %s freq_band %d\r\n",t->version, t->freq_band);
    DT_WLS_LOG("param info get ok\r\n");

    return DT_EOK;
}

int wless_param_pack(dt_config_param_t *pparam, wls_dbg_param_info_t *pwless_param)
{
#define WLESS_FREQ_BAND_MIN (11)
#define WLESS_FREQ_BAND_MAN (25)

    const uint8_t  gateNetSrcIpDef[4] = {192,168,0,120};
    const uint8_t  gateNetDesIpDef[4] = {192,168,0,105};
    const uint8_t  gateNetMaskDef[4]  = {255,255,255,0};
    const uint8_t  gateNetGateDef[4]  = {192,168,0,1};
    const uint8_t  gateLteDesIpDef[4] = {39,96,180,58};
    const uint8_t  gateNbDesIpDef[4]  = {0,0,0,0};

    const char  gateWifiSSID[32]  = "DT-dev";
    const char  gateWifiKEY[32]   = "doton12345";
    const char  gateLteApn[32]    = "china net";
    const char  gateLteDns[32]    = "2u565793i9.wicp.vip";
    const char gateProgVer[16]    = "1.052T1";
    const char gateHardVer[8]     = "V01";

    if (!pparam || !pwless_param) {
        DT_ERR("pparam || pwless_param is NULL.\r\n");
        return DT_ERROR;
    }

    memcpy(pwless_param->stationCode, pparam->dev_code, sizeof(pwless_param->stationCode));
    snprintf(pwless_param->stationSn, sizeof(pwless_param->stationSn), "%s", pparam->sn);
    memcpy(pwless_param->mainBordVer, pparam->app_ver, sizeof(pwless_param->mainBordVer));
    memcpy(pwless_param->mpuSoftVer, pparam->app_ver, sizeof(pwless_param->mpuSoftVer));
    memcpy(pwless_param->mpuSoftVer, gateProgVer, sizeof(pwless_param->mpuSoftVer));
    pwless_param->proto_ver = 0; //协议版本, 0-老协议V1.0; 1-新协议V1.1

    pwless_param->commMode = WIRELESS_TYPE_ETH;
    //WIFI参数
    memcpy(pwless_param->wifiSrcIp,   gateNetSrcIpDef,4);
    memcpy(pwless_param->wifiDesIp,   gateNetDesIpDef,4);
    memcpy(pwless_param->wifiMask,    gateNetMaskDef,4);
    memcpy(pwless_param->wifiGate,    gateNetGateDef,4);
    pwless_param->wifiPort = 1199;
    memcpy(pwless_param->wifiSSID,    gateWifiSSID,16);
    memcpy(pwless_param->wifiKey,     gateWifiKEY,16);

    //以太网参数
    memcpy(pwless_param->netSrcIp, pparam->local_ip, sizeof(pwless_param->netSrcIp));
    memcpy(pwless_param->netDesIp, pparam->server_ip, sizeof(pwless_param->netDesIp));
    memcpy(pwless_param->netMask, pparam->netmask, sizeof(pwless_param->netMask));
    memcpy(pwless_param->netGate, pparam->gw, sizeof(pwless_param->netGate));
    pwless_param->netPort = pparam->hostPort;
    memcpy(pwless_param->netMac, pparam->mac_addr, sizeof(pwless_param->netMac));

    //4G参数
    memcpy(pwless_param->g4DesIp,     gateLteDesIpDef,4);
    memcpy(pwless_param->g4Apn,       gateLteApn,32);
    memcpy(pwless_param->g4Dns,       gateLteDns,32);
    pwless_param->g4Port = 14794;

    //NB/LORA参数结构体
    memcpy(pwless_param->nbDesIp,     gateNbDesIpDef, 4);
    pwless_param->nbPort = 5683;

    //Zigbee参数结构体
    if (pparam->zigbee_freq < WLESS_FREQ_BAND_MIN || pparam->zigbee_freq > WLESS_FREQ_BAND_MAN)
        pparam->zigbee_freq = WLESS_FREQ_BAND_MIN;
    pwless_param->zgbFreqBand = (unsigned char)pparam->zigbee_freq;//频段参数
    pwless_param->zgbTxpower  = 180;
    pwless_param->proto_ver   = 0;


    return DT_EOK;
}

int mpu_set_param_info()
{
    mcu_cmd_0x40_0x02_req_t req;
    mcu_cmd_0x40_0x02_rsp_t rsp;
    wls_dbg_param_info_t l_wls_param;
    char sn[20] = {0};
    wls_dbg_param_info_t wless_param;
    memset(&wless_param, 0x00, sizeof(wless_param));
//    wless_param_pack(&para, &wless_param);
    get_string("Device/devid",sn);
    DT_WLS_LOG("===get sn %s",sn);
    memset(&req, 0, sizeof(req));
    memset(&rsp, 0, sizeof(rsp));
    strcpy(l_wls_param.stationCode,sn);
    strcpy(l_wls_param.stationSn,sn);
    l_wls_param.zgbFreqBand = get("Device/zgbFreqBand");
    l_wls_param.zgbTxpower = get("Device/zgbTxpower");
    req.TaskType = TASK_TYPE_SET_PARAM;
    memcpy(&req.GwParam, &l_wls_param, sizeof(wls_dbg_param_info_t));

    if (DT_EOK != uart_mcu_board_data_trx(UART_CMD_DEBUG_TASK, &req, sizeof(req), &rsp, sizeof(rsp), 2000)) {
        DT_WLS_ERR("mcu cmd 0x%0x failed\r\n", UART_CMD_DEBUG_TASK);
        return DT_ERROR;
    }
    DT_WLS_LOG("Status: %d\r\n", rsp.State);
    if (DT_EOK != rsp.State) {
        DT_WLS_LOG("sensor info set failed\r\n");
        return DT_ERROR;
    }

    DT_WLS_LOG("config param info set ok\r\n");
    return DT_EOK;
}

int mpu_set_param_info_Band(int zgbFreqBand)
{
    mcu_cmd_0x40_0x02_req_t req;
    mcu_cmd_0x40_0x02_rsp_t rsp;
    wls_dbg_param_info_t l_wls_param;
    char sn[20] = {0};
    wls_dbg_param_info_t wless_param;
    memset(&wless_param, 0x00, sizeof(wless_param));
//    wless_param_pack(&para, &wless_param);
    get_string("Device/devid",sn);
    DT_WLS_LOG("===get sn %s",sn);
    memset(&req, 0, sizeof(req));
    memset(&rsp, 0, sizeof(rsp));
    sprintf(sn, "%d", 1100+g_switch_flag);
    strcpy(l_wls_param.stationCode,sn);
    strcpy(l_wls_param.stationSn,sn);
    l_wls_param.zgbFreqBand = zgbFreqBand;
    l_wls_param.zgbTxpower = get("Device/zgbTxpower");
    req.TaskType = TASK_TYPE_SET_PARAM;
    memcpy(&req.GwParam, &l_wls_param, sizeof(wls_dbg_param_info_t));

    if (DT_EOK != uart_mcu_board_data_trx(UART_CMD_DEBUG_TASK, &req, sizeof(req), &rsp, sizeof(rsp), 2000)) {
        DT_WLS_ERR("mcu cmd 0x%0x failed\r\n", UART_CMD_DEBUG_TASK);
        return DT_ERROR;
    }
    DT_WLS_LOG("Status: %d\r\n", rsp.State);
    if (DT_EOK != rsp.State) {
        DT_WLS_LOG("sensor info set failed\r\n");
        return DT_ERROR;
    }

    DT_WLS_LOG("config param info set ok\r\n");
    return DT_EOK;
}

int mpu_set_param_info_to_mcu(char *ps_info)
{
    mcu_cmd_0x40_0x02_req_t req;
    mcu_cmd_0x40_0x02_rsp_t rsp;

    memset(&req, 0, sizeof(req));
    memset(&rsp, 0, sizeof(rsp));

    req.TaskType = TASK_TYPE_SET_PARAM;
    memcpy(&req.GwParam, ps_info, sizeof(wls_dbg_param_info_t));

    if (DT_EOK != uart_mcu_board_data_trx(UART_CMD_DEBUG_TASK, &req, sizeof(req), &rsp, sizeof(rsp), 2000)) {
        DT_WLS_ERR("mcu cmd 0x%0x failed\r\n", UART_CMD_DEBUG_TASK);
        return DT_ERROR;
    }
    DT_WLS_LOG("Status: %d\r\n", rsp.State);
    if (DT_EOK != rsp.State) {
        DT_WLS_LOG("sensor info set failed\r\n");
        return DT_ERROR;
    }

    DT_WLS_LOG("config param info set ok\r\n");
    return DT_EOK;
}


/**
 * @brief 从 MCU 获取传感器信息
 *
 * @param[out] ps_info 传感器信息结构体指针
 * @return 返回执行结果，成功返回 DT_EOK，失败返回 DT_ERROR
 */
int mpu_get_sensor_info_form_mcu(wls_sensor_info_t *ps_info)
{
    mcu_cmd_0x40_0x03_req_t req; /**< MCU 0x40 0x03 请求结构体 */
    mcu_cmd_0x40_0x03_rsp_t rsp; /**< MCU 0x40 0x03 响应结构体 */

    memset(&req, 0, sizeof(req));
    memset(&rsp, 0, sizeof(rsp));

    req.TaskType = TASK_TYPE_READ_SENSOR_INFO;

    if (DT_EOK != uart_mcu_board_data_trx(UART_CMD_DEBUG_TASK, &req, sizeof(req), &rsp, sizeof(rsp), 2000)) {
        DT_WLS_ERR("mcu cmd 0x%0x failed\r\n", UART_CMD_DEBUG_TASK);
        return DT_ERROR;
    }

    memcpy(ps_info, (wls_sensor_info_t *)&rsp.Info, MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t));
    DT_WLS_LOG("sensor info get ok\r\n");
    return DT_EOK;
}

int refresh_sensor_info()
{
    int ret = 0;
    int file_fd       = -1;
    int file_size     = 0;
    char filename[FILE_NAME_LEN] = {0};
        /* 寻找文件，并判断是否有文件 */
    snprintf(filename, sizeof(filename), "%s/%s", FW_SENSOR_CFG_DIR, WLESS_SENSOR_CONFIG);
    if (0 != access(filename, F_OK)) {
        ret = DT_EEMPTY;
        return DT_ERROR;
    }
    file_size = dt_get_file_size(filename);
    /* 将文件读取到内存 */
    file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        DT_WLS_ERR("open cfg table file failed(%s).\r\n", filename);
        return DT_ERROR;
    }
    ZIGBEEParam * lparm = getSharedData();
    DT_PRINTF("memset sensor_info\r\n");
//    memset(lparm->sensor_info, 0, sizeof(ZIGBEEParam));
    DT_PRINTF("memset sensor_infoxxx\r\n");
    DT_PRINTF("m_ZIGBEEParam:%d,sensor:%d\r\n",lparm,lparm->sensor_info);
    memset(lparm->sensor_info, 0, MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t));
    if (file_size != dt_read(file_fd, lparm->sensor_info, file_size)) {
        DT_WLS_ERR("file_fd read fail.\r\n");
        return DT_ERROR;
    }
    DT_PRINTF("memset sensor_info over\r\n");
    for(int i=0;i<MAX_SENSOR_CNT;i++)
    {
        DT_PRINTF("memset sensor_info over1\r\n");
        DT_PRINTF("chan:%d,Sn :%d\r\n",lparm->sensor_info[i].Channel,atoi(lparm->sensor_info[i].Sn));
        if(lparm->sensor_info[i].Channel==0\
                ||atoi(lparm->sensor_info[i].Sn)<=0\
                ||(strcmp(lparm->sensor_info[i].DevNumber,"new")!=0))
        {
            lparm->count = i;
            DT_PRINTF("g_ZIGBEEParam count:%d\r\n",lparm->count);
            break;
        }
    }
    DT_PRINTF("refresh_sensor_info over\r\n");
    return DT_EOK;
}

int refresh_sensor_file_info()
{
    int ret = 0;
    int file_fd       = -1;
    char filename[FILE_NAME_LEN] = {0};
        /* 寻找文件，并判断是否有文件 */
    snprintf(filename, sizeof(filename), "%s/%s", FW_SENSOR_CFG_DIR, WLESS_SENSOR_CONFIG);
    if (0 != access(filename, F_OK)) {
        ret = DT_EEMPTY;
        return DT_ERROR;
    }
    DT_PRINTF("refresh_sensor_file_info start\r\n");
    /* 将文件读取到内存 */
    file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        DT_WLS_ERR("open cfg table file failed(%s).\r\n", filename);
        return DT_ERROR;
    }
    ZIGBEEParam * lparm = getSharedData();
    if (dt_write(file_fd, lparm->sensor_info, MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t))<=0) {
        DT_WLS_ERR("file_fd read fail.\r\n");
        return DT_ERROR;
    }
    return DT_EOK;
}

int refresh_wsl_syspara()
{
    int file_fd = -1;
    int ret = DT_MQTT_OK;
    char param_path[128] = {0};
//    wless_common_param_t syspara;

    snprintf(param_path, sizeof(param_path), "%s%s", WLESS_PARAM_PATH, WLESS_PARAM_FILE);

    if (access(param_path, F_OK) != 0) {
        DT_LOG("wless param file not found.\r\n");
        goto _exit;
    }

    file_fd = open(param_path, O_RDONLY);
    if(file_fd < 0) {
        DT_ERR("open file: %s error!\r\n", param_path);
        goto _exit;
    }
    ZIGBEEParam * lparm = getSharedData();
    memset(&lparm->syspara, 0x00, sizeof(wless_common_param_t));
    if (sizeof(wless_common_param_t) != read(file_fd, &lparm->syspara, sizeof(wless_common_param_t))) {
        DT_ERR("read failed.\r\n");
        goto _exit;
    }
_exit:
    if (file_fd > 0) {
        close(file_fd);
        file_fd = -1;
    }

    return ret;
}

int config_zgb_syspara()
{
    int file_fd = -1;
    int ret = DT_MQTT_OK;
    char param_path[128] = {0};
    int file_size = 0;
//  wless_common_param_t syspara;

    snprintf(param_path, sizeof(param_path), "%s%s", WLESS_PARAM_PATH, WLESS_PARAM_FILE);

    if (access(param_path, F_OK) != 0) {
        DT_LOG("wless param file not found.\r\n");
        goto _exit;
    }

    file_fd = open(param_path, O_WRONLY);
    if(file_fd < 0) {
        DT_ERR("open file: %s error!\r\n", param_path);
        goto _exit;
    }
    time_t seconds = time(NULL);
    int time = seconds;
    ZIGBEEParam * lparm = getSharedData();
    memcpy(lparm->syspara.UpTimeBuf,&time,4);
    DT_LOG("set zgbtype:%d.\r\n",lparm->zgbtype);
    if(lparm->zgbtype==2)
    {
        lparm->syspara.VibIndexPeriod = 600;
        lparm->syspara.MeasDefPeriod = 7200;
        lparm->syspara.TemperaturePeriod = 600;
        lparm->syspara.LongWavePeriod = 12;
    }
    else if(lparm->zgbtype==1)
    {
        lparm->syspara.VibIndexPeriod = 300;
        lparm->syspara.MeasDefPeriod = 3600;
        lparm->syspara.TemperaturePeriod = 300;
        lparm->syspara.LongWavePeriod = 6;
    }
    else
    {
        lparm->syspara.VibIndexPeriod = 600;
        lparm->syspara.MeasDefPeriod = 7200;
        lparm->syspara.TemperaturePeriod = 600;
        lparm->syspara.LongWavePeriod = 12;
    }
//    lparm->syspara.VibIndexPeriod = 180;
//    lparm->syspara.MeasDefPeriod = 12000;
//    lparm->syspara.TemperaturePeriod = 180;
//    lparm->syspara.LongWavePeriod = 12;
    if (sizeof(wless_common_param_t) != write(file_fd, &lparm->syspara, sizeof(wless_common_param_t))) {
        DT_ERR("write failed.\r\n");
        goto _exit;
    }
    printf("update value :%d!!",lparm->update);
_exit:
    if (file_fd > 0) {
        close(file_fd);
        file_fd = -1;
    }

    return ret;
}

int read_wsl_syspara()
{
    int file_fd = -1;
    int ret = DT_MQTT_OK;
    char param_path[128] = {0};
    int file_size = 0;
//    wless_common_param_t syspara;
    refresh_sensor_info();
    snprintf(param_path, sizeof(param_path), "%s%s", WLESS_PARAM_PATH, WLESS_PARAM_FILE);

    if (access(param_path, F_OK) != 0) {
        DT_LOG("wless param file not found.\r\n");
        goto _exit;
    }

    file_fd = open(param_path, O_RDWR);
    if(file_fd < 0) {
        DT_ERR("open file: %s error!\r\n", param_path);
        goto _exit;
    }
    time_t seconds = time(NULL);
    int time = seconds;
    ZIGBEEParam * lparm = getSharedData();
    memcpy(lparm->syspara.UpTimeBuf,&time,4);
    int mid  = get("Device/zgbtype");
    lparm->zgbtype = mid;
    if(lparm->zgbtype==2)
    {
        lparm->syspara.VibIndexPeriod = 600;
        lparm->syspara.MeasDefPeriod = 7200;
        lparm->syspara.TemperaturePeriod = 600;
        lparm->syspara.LongWavePeriod = 12;
    }
    else if(lparm->zgbtype==1)
    {
        lparm->syspara.VibIndexPeriod = 300;
        lparm->syspara.MeasDefPeriod = 3600;
        lparm->syspara.TemperaturePeriod = 300;
        lparm->syspara.LongWavePeriod = 6;
    }
    else
    {
        lparm->syspara.VibIndexPeriod = 600;
        lparm->syspara.MeasDefPeriod = 7200;
        lparm->syspara.TemperaturePeriod = 600;
        lparm->syspara.LongWavePeriod = 12;
    }
//    lparm->syspara.VibIndexPeriod = 180;
//    lparm->syspara.MeasDefPeriod = 12000;
//    lparm->syspara.TemperaturePeriod = 180;
//    lparm->syspara.LongWavePeriod = 12;


    lparm->syspara.LongWaveLength = 65536;
    lparm->syspara.LongWaveFreq = 12500;
    lparm->syspara.LowFreqDaqLength = 16384;
    lparm->syspara.LowFreqSaveLength = 0;
    lparm->syspara.LowDaqFreq = 3125;
    lparm->syspara.HighFreqDaqLength = 32768;
    lparm->syspara.HighFreqSaveLength = 0;
    lparm->syspara.HighDaqFreq = 12500;
    lparm->syspara.IndexResetPeriod = 00;
    lparm->syspara.VibChannelNum = lparm->count;

    if (sizeof(wless_common_param_t) != write(file_fd, &lparm->syspara, sizeof(wless_common_param_t))) {
        DT_ERR("write failed.\r\n");
        goto _exit;
    }
_exit:
    if (file_fd > 0) {
        close(file_fd);
        file_fd = -1;
    }

    return ret;
}

int mpu_set_sensor_info()
{
    mcu_cmd_0x40_0x04_req_t req; /**< MCU 0x40 0x04 请求结构体 */
    mcu_cmd_0x40_0x04_rsp_t rsp; /**< MCU 0x40 0x04 响应结构体 */

    memset(&req, 0, sizeof(req));
    memset(&rsp, 0, sizeof(rsp));
    refresh_sensor_info();
    req.TaskType = TASK_TYPE_SET_SENSOR_INFO; /**< 设置任务类型为设置传感器信息 */
    ZIGBEEParam * lparm = getSharedData();
    memcpy(req.Info, lparm->sensor_info, lparm->count*sizeof(uart_cmd_ret_sensor_info_t));
    //refresh_sensor_file_info();
    if (DT_EOK != uart_mcu_board_data_trx(UART_CMD_DEBUG_TASK, &req, sizeof(req), &rsp, sizeof(rsp), 2000)) {
        DT_WLS_ERR("mcu cmd 0x%0x failed\r\n", UART_CMD_DEBUG_TASK);
        return DT_ERROR;
    }

    DT_WLS_LOG("Status: %d\r\n", rsp.State);
    if (DT_EOK != rsp.State) {
        DT_WLS_LOG("sensor info set failed\r\n");
        return DT_ERROR;
    }

    DT_WLS_LOG("sensor info set ok\r\n");
    return DT_EOK;
}

int mpu_set_sensor_info_select(int select)
{
    mcu_cmd_0x40_0x04_req_t req; /**< MCU 0x40 0x04 请求结构体 */
    mcu_cmd_0x40_0x04_rsp_t rsp; /**< MCU 0x40 0x04 响应结构体 */

    memset(&req, 0, sizeof(req));
    memset(&rsp, 0, sizeof(rsp));
    refresh_sensor_info();
    req.TaskType = TASK_TYPE_SET_SENSOR_INFO; /**< 设置任务类型为设置传感器信息 */
    ZIGBEEParam * lparm = getSharedData();
    printf("===set set_sensor_info chan:%d sn:%s\n", lparm->sensor_info[select].Channel, lparm->sensor_info[select].Sn);
    memcpy(&req.Info[1], &(lparm->sensor_info[select]), sizeof(uart_cmd_ret_sensor_info_t));

    req.Info[1].Channel = 2;
    req.Info[0].Bind=2;//=sensor_info[date[pos]-1].Bind = 2;
    req.Info[0].Channel=1;
    sprintf(req.Info[0].Sn,"%d",2100+g_switch_flag);
//    req.Info[0].Sn[0] = 0x31;
//    req.Info[0].Sn[1] = 0x30;
//    req.Info[0].Sn[2] = 0x30;
//    req.Info[0].Sn[3] = 0x31;
    strcpy(req.Info[0].DevNumber,"new");
    printf("===set set_sensor_info chan:%d sn:%s\n", req.Info[0].Channel, req.Info[0].Sn);
    printf("===set set_sensor_info chan:%d sn:%s\n", req.Info[1].Channel, req.Info[1].Sn);
    //refresh_sensor_file_info();
    if (DT_EOK != uart_mcu_board_data_trx(UART_CMD_DEBUG_TASK, &req, sizeof(req), &rsp, sizeof(rsp), 2000)) {
        DT_WLS_ERR("mcu cmd 0x%0x failed\r\n", UART_CMD_DEBUG_TASK);
        return DT_ERROR;
    }

    DT_WLS_LOG("Status: %d\r\n", rsp.State);
    if (DT_EOK != rsp.State) {
        DT_WLS_LOG("sensor info set failed\r\n");
        return DT_ERROR;
    }

    DT_WLS_LOG("sensor info set ok\r\n");
    return DT_EOK;
}

/**
 * @brief 向 MCU 设置传感器信息
 *
 * @param[in] ps_info 传感器信息数组指针
 * @return 返回执行结果，成功返回 DT_EOK，失败返回 DT_ERROR
 */
int mpu_set_sensor_info_to_mcu(uint8_t *ps_info)
{
    mcu_cmd_0x40_0x04_req_t req; /**< MCU 0x40 0x04 请求结构体 */
    mcu_cmd_0x40_0x04_rsp_t rsp; /**< MCU 0x40 0x04 响应结构体 */

    memset(&req, 0, sizeof(req));
    memset(&rsp, 0, sizeof(rsp));

    req.TaskType = TASK_TYPE_SET_SENSOR_INFO; /**< 设置任务类型为设置传感器信息 */
    memcpy(req.Info, ps_info, MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t));

    if (DT_EOK != uart_mcu_board_data_trx(UART_CMD_DEBUG_TASK, &req, sizeof(req), &rsp, sizeof(rsp), 2000)) {
        DT_WLS_ERR("mcu cmd 0x%0x failed\r\n", UART_CMD_DEBUG_TASK);
        return DT_ERROR;
    }

    DT_WLS_LOG("Status: %d\r\n", rsp.State);
    if (DT_EOK != rsp.State) {
        DT_WLS_LOG("sensor info set failed\r\n");
        return DT_ERROR;
    }

    DT_WLS_LOG("sensor info set ok\r\n");
    return DT_EOK;
}

void mpu_notify_mcu(void)
{
	dt_system("echo 197 > /sys/class/gpio/export");
    usleep(100);
	dt_system("echo out > /sys/class/gpio/gpio197/direction");
    usleep(100);
	dt_system("echo 0 > /sys/class/gpio/gpio197/value");
    usleep(100);
	dt_system("echo 1 > /sys/class/gpio/gpio197/value");
    usleep(100);
	dt_system("echo 0 > /sys/class/gpio/gpio197/value");
}


int g_platform = PLATFORM_NONE;
char SYSCONF_DIR[128];
char WLESS_WIFI_CONFIG_DIR[128];
char WLESS_WIFI_STA_CONF[256];
char WLESS_WIFI_AP_CONF[256];

int get_platform(void)
{
	char cmd_result[512];

	if (g_platform != PLATFORM_NONE)
		goto out;

    if(!get_cmd_result("cat /etc/quectel-project-version | grep \"Project Name\"", cmd_result)) {
		DT_WLS_ERR("fatal error, reboot.\r\n");
		// reboot_notify_mcu(false);
		goto out;
	}

	memset(SYSCONF_DIR, 0, sizeof(SYSCONF_DIR));
	memset(WLESS_WIFI_CONFIG_DIR, 0, sizeof(WLESS_WIFI_CONFIG_DIR));
	memset(WLESS_WIFI_STA_CONF, 0, sizeof(WLESS_WIFI_STA_CONF));
	memset(WLESS_WIFI_AP_CONF, 0, sizeof(WLESS_WIFI_AP_CONF));

	if (strstr(cmd_result,"EG25")) {
		g_platform = PLATFORM_EG25;
		sprintf(SYSCONF_DIR, "%s%s", NEW_PATH, "app/config/");
		sprintf(WLESS_WIFI_CONFIG_DIR, "%s%s", NEW_PATH, "wifi_conf/");
	} else {
		g_platform = PLATFORM_EC20;
		sprintf(SYSCONF_DIR, "%s%s", OLD_PATH, "app/config/");
		sprintf(WLESS_WIFI_CONFIG_DIR, "%s%s", OLD_PATH, "wifi_conf/");
	}
	sprintf(WLESS_WIFI_STA_CONF, "%s%s", WLESS_WIFI_CONFIG_DIR, "wpa_supplicant.conf");
	sprintf(WLESS_WIFI_AP_CONF, "%s%s", WLESS_WIFI_CONFIG_DIR, "hostapd.conf");

	DT_WLS_LOG("g_platform: %d, SYSCONF_DIR: %s, WIFI_STA_CONF: %s\r\n", g_platform, SYSCONF_DIR, WLESS_WIFI_STA_CONF);
out:
	return g_platform;
}

int check_used_capacity(void)
{
    int ret = true;
    struct statfs diskInfo;

    int stat_ret = -1;

#ifdef DEBUG_LOCAL
    stat_ret = statfs("/usrdata", &diskInfo);
#else
    stat_ret = statfs("/media/sdcard", &diskInfo);
#endif

    if(stat_ret < 0)
    {
        DT_WLS_ERR("statfs error!\r\n");
        return false;
    }
    unsigned long long block_size = diskInfo.f_bsize;
    unsigned long long totalSize = block_size * diskInfo.f_blocks;  /* Uint is Byte */

    //unsigned long long availSize = diskInfo.f_bavail*block_size;    /* Uint is Byte not root user block */
    unsigned long long availSize = diskInfo.f_bfree*block_size;       /* Uint is Byte , root user block*/

    double capacity_total = 1.0*totalSize/(1024*1024);                     /* Uint MB */
    double capacity_used  = 1.0*(totalSize - availSize)/(1024*1024);       /* Uint MB */

#ifdef DEBUG_LOCAL
    if(capacity_used > 70)
    {
        DT_WLS_ERR("capacity_total:%f, used:%f\r\n", capacity_total, capacity_used);
        ret = false;
    }
#else
    if(capacity_used > 7*1024)
    {
        DT_WLS_ERR("capacity_total:%f, used:%f\r\n", capacity_total, capacity_used);
        ret = false;
    }
#endif
    return ret;
}

int CreateDir(const char *sPathName)
{
    int ret = -1;
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", sPathName);
    ret = dt_system(cmd);
    return ret;
}

int deleteDir(const char *dirName)
{
    int ret = -1;
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "rm -rf %s", dirName);
    ret = dt_system(cmd);
    if(ret < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int dt_check_time(char *time)
{
	u16 year;
	u8 mon, day, hour, minute, sec;
	mon = time[2];
	day = time[3];
	hour = time[4];
	minute = time[5];
	sec = time[6];
	if (mon < 1 || mon > 12 ||
		day < 1 || day > 31 ||
		hour > 23 || minute > 59 || sec > 59)
	{
		DT_WLS_ERR("Wrong Time: %02d%02d-%02d-%02d %02d:%02d:%02d\r\n",
			time[0], time[1], time[2], time[3], time[4], time[5], time[6]);
		return 0;
	}
	return 1;
}

/**
 * @brief 加载共享nor-flash设备
 *        在MCU通知MPU去读取数据时，先确认sip-nor是否已经加载
 *        若已加载，则不做处理
 *        若未加载，则尝试加载两次
 *
 * @return DT_EOK/DT_ERROR
 */
int dt_check_spi_nor_module(void)
{
#define SPI_NOR_DEVICE_NAME     "/dev/mtd0"
#define SPI_NOR_INSMOD_CNT_MAX  (2)
    char cmd[256] = {0};
    int retry_cnt = SPI_NOR_INSMOD_CNT_MAX;

    //检查是否已经加载过
    //if (access(SPI_NOR_DEVICE_NAME, F_OK) != 0) {
    if(1)
    {
        DT_LOG("WIRELESS device not found, start insmod.\r\n");
        snprintf(cmd, sizeof(cmd), "rmmod /oem/dma/qt/ko/m25p80.ko;usleep 500000;insmod -f /oem/dma/qt/ko/m25p80.ko;usleep 500000");
        do {
            retry_cnt--;
            dt_system(cmd);
            if (0 == access(SPI_NOR_DEVICE_NAME, F_OK)) {
                DT_LOG("spi-nor device insmod ok.\r\n");
                return DT_EOK;
            }
            sleep(1);
            //usleep(500000);
        } while(retry_cnt > 0);
        DT_LOG("spi-nor device insmod failed.\r\n");
        return DT_ERROR;
    }
    return DT_EOK;
}
