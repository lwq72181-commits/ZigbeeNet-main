#include "common.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dt_uart.h"
//#include "dt_param.h"
#include "proto_common.h"
#include "proto_parse.h"
#include "proto_handle.h"
//#include "dt_upload_manager.h"
#include "spi_manager.h"
#include "wless_param.h"
#include "param_parse.h"
#include "dt_uart_mcu_board.h"
#include "wrapper.h"

unsigned char global_hw_check_result = 0x00;
static int sys_status = SYS_IDLE;/* 系统当前状态 */
static pthread_mutex_t sys_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct tm g_next_hour;
static time_t g_next_time;
int g_switch_flag = -1;
static int g_chan[] = {18,19,20,24,25,26};
//static int g_chan[] = {11,12,13,14,15,16};
//extern ZIGBEEParam *g_ZIGBEEParam;
int uart_reply_mcu(uint8_t *reply_buf, uint32_t len)
{

#ifdef SNESOR_DBG_PRINTF_ENABLE
    int printf_len = 64;
    printf("-----send buf(len:%d):\r\n", len);
    if (len <= 32)
        printf_len = len;
    for (uint32_t i = 0; i < printf_len; i++) {
        printf("%02x ", reply_buf[i]);
        if ((i + 1) % 32 == 0) {
            printf("\r\n");
        }
    }
    printf("\r\n");
#endif

    dt_uart_t *t =  dt_uart_get(WIRELESS_UART);
    return dt_uart_write(t->fd, reply_buf, len);
}

#if 0
int regular_commun_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x01_t *pcommun = (uart_cmd_0x01_t *)pdata;
    uart_cmd_0x01_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    // if (pcommun->FirstPowerOn == FIRST_POWER_ON)
    //     return ret;

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_COMMUNICATION;
    reply.header.len = sizeof(reply);
    reply.MpuState = MPU_STATE_IDLE;
    if (SYS_DBG == sys_status_get()) {
        reply.MpuState = MPU_STATE_WIFI_CONFIG;
        set_mpu_config_mcu_flag(true);
    }

    memset(reply.MpuVersion, 0, sizeof(reply.MpuVersion));

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));

    return ret;
}
#endif

FREQ_PUBLIC int set_sys_status(int status)
{
    sys_status = status;
    return 0;
}


/**
 * @brief 响应例行通讯回复处理
 */
FREQ_PUBLIC int do_regular_commun_handle(unsigned char *pdata, unsigned int data_len)
{
    int ret = 0;
    int status = 0;
    mcu_cmd_0x01_rsp_t *prsp = (mcu_cmd_0x01_rsp_t*)pdata;
    mcu_cmd_0x01_req_t req;
    memset(&req, 0, sizeof(req));
    DT_WLS_LOG("----------------------------------\r\n");

    req.MpuState = MPU_STATE_IDLE;
    //status = get("Device/zgbUpdate");
    DT_PRINTF("==============status:%d\n",status);
    // 如果系统调试状态为 SYS_DBG，则设置 MPU 状态为 WIFI 配置，并将 MPU 配置 MCU 标志位设为true
    //if (SYS_DBG == sys_status) {
    //if (status) {
        req.MpuState = MPU_STATE_WIFI_CONFIG;
        set_mpu_config_mcu_flag(true);
    //}

    /*这里版本号清空*/
    memset(req.MpuVersion, 0, sizeof(req.MpuVersion));
    uart_mcu_board_data_reply(UART_CMD_COMMUNICATION, &req, sizeof(req));

    dt_uart_mcu_board_t * t = dt_uart_mcu_board_get();
    if (DT_ENABLE != t->is_exist) {
        t->is_exist = DT_ENABLE;
        snprintf(t->version, sizeof(t->version),"%s", prsp->McuVersion);
    }
    ZIGBEEParam * lparm = getSharedData();
    memcpy(lparm->McuVersion,prsp->McuVersion,10);
    DT_WLS_LOG("mcu version %s\r\n", t->version);

    if(status)
    {
        //usleep(500000);
        sleep(1);
        mpu_set_param_info();
        usleep(500000);
        //sleep(1);
        ret = mpu_set_sensor_info();
        if(status)
        {
            if(DT_EOK == ret)
            {
                set("Device/zgbUpdate",0);
                //dt_system("echo 0 > /sys/class/gpio/gpio101/value;sleep 2;echo 1 > /sys/class/gpio/gpio101/value");
            }
        }
    }

    return ret;
}

static int get_system_time_task(uint8_t *pdata, uint32_t len)
{
    int ret = 0;
    uart_cmd_0x20_get_system_time_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_INTERNAL_TASK;
    reply.header.len = sizeof(reply);
    reply.TaskType = pdata[0];
    reply.RunState = TASK_CAN_RUN;
//    dt_current_time_get((char *)reply.SystemTime);
    time_t seconds = time(NULL);
    int now = seconds;
    memcpy(reply.SystemTime,&now,4);
//    DT_WLS_LOG("systime: %hhd%hhd_%hhd_%hhd: %hhd:%hhd:%hhd\r\n",
//    reply.SystemTime[0],reply.SystemTime[1], reply.SystemTime[2],
//    reply.SystemTime[3],reply.SystemTime[4], reply.SystemTime[5],reply.SystemTime[6]);
    DT_WLS_LOG("systime :%ld,%d\n",seconds,now);
    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    int status = 0;
    status = get("Device/zgbUpdate");
    DT_PRINTF("==============status:%d\n",status);
    if(status)
    {
        usleep(500000);
        mpu_set_param_info();
        ret = mpu_set_sensor_info();
        if(status)
        {
            if(DT_EOK == ret)
            {
                set("Device/zgbUpdate",0);
            }
        }
    }
#ifdef ZIGBEE_TEST_SWITCH
    time_t nowtime = time(NULL);
    if(g_switch_flag == -1)
    {
        g_switch_flag++;
        //g_next_time = ((nowtime / 3600) + 1) * 3600+120;
        g_next_time = ((nowtime / 300) + 1) * 300 + 60*g_switch_flag +10;
        usleep(500000);
        printf("===set zigbee band :%d\n", g_chan[g_switch_flag]);
        mpu_set_param_info_Band(g_chan[g_switch_flag]);
        mpu_set_sensor_info();
        //mpu_set_sensor_info_select(g_switch_flag);
    }
    int diff = (int)difftime(g_next_time, nowtime);
    if(abs(diff)>3600)
    {
        //g_next_time = ((nowtime / 3600) + 1) * 3600+120;
        g_next_time = ((nowtime / 300) + 1) * 300 + 60*g_switch_flag +10;
        //g_next_time = ((nowtime / 60) + 1) * 60;
    }
    if(diff<=0)
    {
        //g_next_time = ((nowtime / 3600) + 1) * 3600+120;
        //g_next_time = ((nowtime / 60) + 1) * 60;
        usleep(500000);
        g_switch_flag=(++g_switch_flag)%4;
        if(g_switch_flag==0)
        {
            g_next_time = ((nowtime / 300) + 1) * 300 +10;
        }
        else
        {
            g_next_time += 70;
        }
        printf("===set zigbee band :%d\n", g_chan[g_switch_flag]);
        mpu_set_param_info_Band(g_chan[g_switch_flag]);
        mpu_set_sensor_info();
        //mpu_set_sensor_info_select(g_switch_flag);
    }
    // 计算下一个整点时间

#endif
    return ret;
}

static int get_560G_parameter_task(uint8_t *pdata, uint32_t len)
{
    int ret = 0;
    uart_cmd_0x20_get_560g_parameter_ret_t reply;
    memset(&reply, 0, sizeof(reply));
    wless_param_t *pwless_param = get_wless_param();
    //dt_config_param_t *pconfig_param = get_board_config_param();

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_INTERNAL_TASK;
    reply.header.len = sizeof(reply);
    reply.TaskType = pdata[0];
    reply.RunState = TASK_CAN_RUN;
    reply.IndexPeriod = pwless_param->index_period;
    reply.MeasDefPeriod = pwless_param->measdef_period;
    reply.StationNum = pwless_param->station_num;
//    for(int j = 0; j < pwless_param->station_num; j++)
//        memcpy(reply.StationCode, pconfig_param->dev_code, 8);

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}


/* check weather have upgrade file according file_type\
    and get file name and size */
static int get_upgrade_file(unsigned char file_type, char *upg_file, int *size)
{
    int ret = 0;
    int find = false;
    char file_dir_path[128] = FW_WLESS_DIR;
    char filename[512] = {0};
    memset(filename, 0, sizeof(filename));

    if (0 != access(file_dir_path, F_OK)) {
        return -1;
    }

    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    dir = opendir(file_dir_path);
    if (dir == NULL){
        goto end;
    }

    while((ptr = readdir(dir)) != NULL) {
        if (ptr->d_type == DT_REG) {
            if(file_type == FILE_UPGRADE_RH605) {
                if (strstr(ptr->d_name, FILE_RH605_UPG_NAME) || \
                    strstr(ptr->d_name, FILE_WS2_NAME) || \
                    strstr(ptr->d_name, FILE_WS20_NAME) || \
                    strstr(ptr->d_name, FILE_WS21S_NAME)) {
                    // upg_file = ptr->d_name;
                    snprintf(filename, sizeof(filename), "%s", ptr->d_name);
                    find = true;
                }
            } else if (file_type == FILE_UPGRADE_MPU_RS485) {
                if (strstr(ptr->d_name, FILE_MPU_RS485_NAME)) {
                    // upg_file = ptr->d_name;
                    snprintf(filename, sizeof(filename), "%s", ptr->d_name);
                    find = true;
                }
            } else if (file_type == FILE_UPGRADE_MCU) {
                if (strstr(ptr->d_name, FILE_MCU_UPG_NAME)) {
                    // upg_file = ptr->d_name;
                    snprintf(filename, sizeof(filename), "%s", ptr->d_name);
                    find = true;
                }
            } else if (file_type == FILE_UPGRADE_MPU) {
                if (strstr(ptr->d_name, FILE_MPU_APP_NAME)) {
                    // upg_file = ptr->d_name;
                    snprintf(filename, sizeof(filename), "%s", ptr->d_name);
                    find = true;
                }
            } else if (file_type == FILE_SENSOR_CONFIG) {
                if (strstr(ptr->d_name, FILE_MPU_FS_NAME)) {
                    // upg_file = ptr->d_name;
                    snprintf(filename, sizeof(filename), "%s", ptr->d_name);
                    find = true;
                }
            }
        }
        if (find) {
            /* 找到对应的固件，赋值文件名 */
            sprintf(upg_file, "%s", filename);
            DT_WLS_LOG("upg_file[%s]\n", upg_file);
            memset(filename, 0x00, sizeof(filename));
            snprintf(filename, sizeof(filename), "%s/%s", FW_WLESS_DIR, upg_file);

            /* 根据文件名获取文件大小 */
            struct stat info;
            stat(filename, &info);
            if (size)
                *size = info.st_size;
            ret = true;
            break;
        }
    }
end:
    if (dir) {
        closedir(dir);
        dir = NULL;
    }
    return ret;
}


static int get_upgrade_version_task(uint8_t *pdata, uint32_t len)
{
    int ret = 0;
    uart_cmd_0x20_get_upgrade_version_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_INTERNAL_TASK;
    reply.header.len = sizeof(reply);
    reply.TaskType = pdata[0];
    reply.RunState = TASK_CAN_RUN;

    // TODO 升降级文件版本信息获取并处理
    char file_name[128] = {0};
    int file_size;
    char mpu_ver[32] = {0};
    if(get_upgrade_file(FILE_UPGRADE_MPU, file_name, &file_size))
    {
        snprintf(mpu_ver, sizeof(mpu_ver), "%d.%d.%d", DT_VERSION, DT_SUBVERSION, DT_REVISION);/* 获取内存中主程序版本号 */
        if(strstr(file_name, mpu_ver) == NULL)
        {
            memcpy(reply.MpuProgramName, file_name, strlen(file_name));
        }
        DT_WLS_LOG("mpu file name: %s\r\n", file_name);
    }
    if(get_upgrade_file(FILE_UPGRADE_MCU, file_name, &file_size))
    {
        memcpy(reply.McuProgramName, file_name, strlen(file_name));
        DT_WLS_LOG("mcu file name: %s\r\n", file_name);
    }
    if(get_upgrade_file(FILE_UPGRADE_MPU_RS485, file_name, &file_size))
    {
        memcpy(reply.MpuRs485ProgramName, file_name, strlen(file_name));
        DT_WLS_LOG("rs485 file name: %s\r\n", file_name);
    }
    if(get_upgrade_file(FILE_UPGRADE_RH560_WIFI, file_name, &file_size))
    {
        memcpy(reply.StationWifiProgramName, file_name, strlen(file_name));
        DT_WLS_LOG("station wifi file name: %s\r\n", file_name);
    }
    if(get_upgrade_file(FILE_UPGRADE_RH605, file_name, &file_size))
    {
        memcpy(reply.SensorProgramName, file_name, strlen(file_name));
        DT_WLS_LOG("ensor file name: %s\r\n", file_name);
    }
    if(get_upgrade_file(FILE_SENSOR_CONFIG, file_name, &file_size))
    {
        memcpy(reply.StationWifiProgramName, file_name, strlen(file_name));
        DT_WLS_LOG("sensor config file name: %s\r\n", file_name);
    }

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

static int get_560_param_task(uint8_t *pdata, uint32_t len)
{
    int ret = 0;
    char station_code[8];
    uart_cmd_0x20_get_560_parameter_ret_t reply;
    memset(&reply, 0, sizeof(reply));
    wless_param_t *pwless_param = get_wless_param();
    //dt_config_param_t *pconfig_param = get_board_config_param();

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_INTERNAL_TASK;
    reply.header.len = sizeof(reply);
    reply.TaskType = pdata[0];
    reply.RunState = TASK_CAN_RUN;

    memcpy(station_code, &pdata[1], 8);

//    for(int j = 0; j < pwless_param->station_num; j++)
//    {
//        if(strncmp(pconfig_param->dev_code, station_code, 8) == 0)
//        {
            memcpy(reply.StationParamTime, pwless_param->param_time, 7);
            // todo
            reply.DeviceState = 0x0;
//            break;
//        }
//    }

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

static int ready_shutdown_task(uint8_t *pdata, uint32_t len)
{
    int ret = 0;
    uart_cmd_0x20_ready_shutdown_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_INTERNAL_TASK;
    reply.header.len = sizeof(reply);
    reply.TaskType = pdata[0];
    reply.RunState = get_all_task_run_state();

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    if(reply.RunState == TASK_CAN_RUN)
        dt_system("halt");

    return ret;
}

static int save_sensor_config(char *pfilename, uart_cmd_ret_sensor_info_t *s)
{
    int fd = -1;
    fd = open(pfilename, O_WRONLY | O_CREAT);
    if (fd < 0) {
        return false;
    }
    write(fd, s, MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t));

    close(fd);
    return 0;
}

static int save_sensor_config_task(uint8_t *pdata, uint32_t len)
{
    int ret = 0;
    uart_cmd_ret_sensor_info_t *s = (uart_cmd_ret_sensor_info_t *)&pdata[9]; // skip gateway code
    DT_WLS_LOG("Got sensor info list from MCU:\r\n");
    for (int i = 0; i < MAX_SENSOR_CNT; i++) {
        if (s[i].Channel && strlen(s[i].Sn)) {
            DT_WLS_LOG("CH: %d, SN: %.10s, DevPos: %.32s, Bind: %d\r\n",
                    s[i].Channel, s[i].Sn, s[i].DevNumber, s[i].Bind);
        }
    }

    //get sensor config and save config
    char filename[256] = {0};
    snprintf(filename, sizeof(filename), "%s/%s", FW_SENSOR_CFG_DIR, WLESS_SENSOR_CONFIG);
    if (save_sensor_config(filename, s)) {
        DT_WLS_ERR("Failed to save sensor config: %s.\r\n", filename);
    }

    uart_cmd_0x20_save_sensor_config_ret_t reply;
    memset(&reply, 0, sizeof(reply));
    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_INTERNAL_TASK;
    reply.header.len = sizeof(uart_cmd_0x20_save_sensor_config_ret_t);
    reply.TaskType = pdata[0];
    reply.RunState = TASK_CAN_RUN;

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));

    return ret;
}

static inter_task_t inter_task[] = {
    {TASK_GET_SYSTEM_TIME, get_system_time_task},
    {TASK_GET_560G_PARAMETER, get_560G_parameter_task},
    {TASK_GET_UPGRADE_VERSION, get_upgrade_version_task},
    {TASK_GET_560_PARAMETER, get_560_param_task},
    {TASK_READY_SHUTDOWN, ready_shutdown_task},
    {TASK_SAVE_SENSOR_CONFIG, save_sensor_config_task}
};

int inter_task_handle(uint8_t *pdata, uint32_t data_len)
{
    for (uint32_t i = 0; i < sizeof(inter_task)/sizeof(inter_task_t); i++) {
        if (pdata[0] == inter_task[i].task_type) {
            if (inter_task[i].handle == NULL) {
                break;
            }
            return inter_task[i].handle(pdata, data_len);
        }
    }

    DT_WLS_ERR("inter task type: 0x%.x error !\r\n", pdata[0]);
    return -1;
}



/* 1) 文件类：全部改为服务器主动下发，文件存在采集站中;
 * 2） 上传类，全部改为采集站主动上传，不再请求；
 */

static wireless_task_t wireless_task[] = {
    // {TASK_WIFI_CHECK, NULL},
    // {TASK_4G_CHECK, NULL},
    // {TASK_RECV_DEBUG_FILE, NULL},
    // {TASK_WIFI_SIGNAL_DETECT, NULL},
    // {TASK_4G_SIGNAL_DETECT, NULL},
    // {TASK_GET_PARAMETER, },
    // {TASK_GET_UPGRADE_FILE, },
    // {TASK_RECV_STATION_DATA, },
    // {TASK_REQ_PUT_FILE, },
    // {TASK_UPLOAD_DATA, },
    // {TASK_UPLOAD_EIGEN, },
    // {TASK_CONNECT_ONLINE, },
};

int wireless_task_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x21_t *pwireless = (uart_cmd_0x21_t *)pdata;
    uart_cmd_0x21_ret_t reply;

    for (uint32_t i = 0; i < sizeof(wireless_task)/sizeof(wireless_task_t); i++) {
        if (pwireless->TaskType == wireless_task[i].task_type) {
            if (wireless_task[i].handle == NULL) {
                break;
            }
            ret = wireless_task[i].handle(pdata, data_len);
            break;
        }
    }

    if (ret) {
        return ret;
        DT_WLS_ERR("wireless_task task type: 0x%.x error !\r\n", pdata[0]);
    }

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_WIRELESS_TASK;
    reply.header.len = sizeof(reply);
    reply.TaskType = pwireless->TaskType;
    memcpy(reply.StationCode, pwireless->StationCode, 8);
    reply.TaskState = 0x1;

    set_cur_task_type(pwireless->TaskType); // set mpu state
    // if(get_task_run_state() == TASK_RUNNING)
	// {
	// 	DT_WLS_ERR("task is running, type: %d\r\n", pwireless->TaskType);
    //     reply.TaskState = 0x0;
	// }
	set_task_run_state(TASK_SUCCESS);

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));


    return ret;
}

int get_task_state_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x22_t *ptask_state = (uart_cmd_0x22_t *)pdata;
    uart_cmd_0x22_ret_t reply;
    memset(&reply, 0, sizeof(reply));
    unsigned char *left = NULL;

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_GET_TASK_STATE;
    reply.header.len = sizeof(reply);

    reply.TaskType = get_cur_task_type();
    reply.RunState = get_task_run_state();

    if (reply.TaskType != 0x04) { //0x4为MCU询问数据是否读取完成任务指令
        reply.RunState = TASK_SUCCESS;
    } else {
        reply.RunState = get_cur_task_state();
    }
    memcpy(reply.StationCode, ptask_state->StationCode, 8);

#ifdef SNESOR_DBG_PRINTF_ENABLE
    DT_WLS_LOG("TaskType: %d, RunState: %d\r\n", reply.TaskType, reply.RunState);
#endif
    // TODO, get wifi config
    if(reply.RunState == TASK_SUCCESS)
    {
        reply.SignalStrength = 0;
        reply.TransferRate = 0;
        reply.LossRate = 0;

        reply.costtime = 0;
        reply.wificonnectstate = 1;
        reply.tcpconnectstate = 1;
        reply.headconnectstate = 1;
    }

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));

    return ret;
}

int notify_mpu_upgrade_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x12_ret_t reply;
    char file_name[512];
    int file_size;
    char reset_cmd[256] = {0};
    int need_upg = false;


    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_NOTIFY_MPU_UPGRADE;
    reply.header.len = sizeof(reply);

    if(get_upgrade_file(FILE_UPGRADE_MPU, file_name, &file_size))
    {
        reply.UpgradeState = 0x01;
        need_upg = true;
    } else
    {
        reply.UpgradeState = 0x00;
    }

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));

    if(need_upg)
    {
        /* no handle, ms810 not have mpu */
    }

    return ret;
}

int notify_mpu_rs485_upgrade_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x13_ret_t reply;
	uart_cmd_0x13_t *cmd0x13;


    // string file_name;
	// int file_size;
    // int need_upg = false;
	// char file[256];

	// cmd0x13 = (uart_cmd_0x13_t *)left;
	// DT_WLS_LOG("FileType: %d", cmd0x13->FileType);

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_NOTIFY_MPU_RS485_UPGRADE;
    reply.header.len = sizeof(reply);

#if 0
	if (cmd0x13->FileType != 1) {
		DT_WLS_ERR("Only support RS485 upgrade now...");
		err = 1;
		goto ack;
	}

    if(get_upgrade_file(FILE_UPGRADE_MPU_RS485, file_name, &file_size))
    {
        reply.UpgradeState = 0x01;
        need_upg = true;
    }
    else
    {
        reply.UpgradeState = 0x00;
    }

    if(need_upg)
    {
		memset(file, 0, sizeof(file));
		sprintf(file, "%s%s", FW_WLESS_DIR, file_name.c_str());
        if (rs485_do_upgrade(file) >= 0)
			remove(file);
		else
			reply.UpgradeState = 0x00;
    }
#endif

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

static int load_sensor_config(char *pfilename, uart_cmd_ret_sensor_info_t *s)
{
    //这里面要做的是，将配置表中数据先读取出来，然后将每个通道的数据填充到s中
    int ret = DT_ERROR;
    int fd = -1;
    uint32_t len = MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t);
    sensor_config_info_t *pconfig = NULL;

    printf("DBG******************load_sensor pfilename:%s\n", pfilename);
    //文件参数配置表解析到内存
    if (DT_EOK != sensor_param_parse(pfilename)) {
        DT_WLS_ERR("parse failed.\n");
        goto _exit;
    }
    pconfig = sensor_info_get();

    memcpy(s, &pconfig->sensor[0], MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t));

    ret = DT_EOK;
_exit:
    return ret;
}

/**
 * @brief 这里只是告诉MCU有没有升级文件文件，升级文件多大
 *
 */
int req_get_file_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x31_t req_get_file_head;
    uart_proto_tail_t tail;

    uint8_t info[UART_ACK_BUF_LEN] = {0};

    char file_name[32];
    memset(file_name, 0, sizeof(file_name));
    int file_size = 0;
    char file_name_with_path[256];
    memset(file_name_with_path, 0, sizeof(file_name_with_path));
    uart_cmd_0x31_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_REQ_GET_FILE;
    reply.header.len = sizeof(reply);

    memset(&req_get_file_head, 0, sizeof(req_get_file_head));
    memset(&tail, 0, sizeof(tail));
    memcpy(&req_get_file_head, pdata, sizeof(req_get_file_head));

#ifdef SNESOR_DBG_PRINTF_ENABLE
    printf("-----req_get_file_head:\r\n");
    uint8_t *p = (uint8_t *)&req_get_file_head;
    for (uint32_t i = 0; i < sizeof(req_get_file_head); i++) {
        printf("%02x ", p[i]);
        if ((i + 1) % 32 == 0) {
            printf("\r\n");
        }
    }
    printf("\r\n");
#endif

    DT_WLS_LOG("req_get_file_head.FileType = %d\r\n", req_get_file_head.FileType);

    if(req_get_file_head.FileType == 0x1) {
        memset(file_name_with_path, 0, sizeof(file_name_with_path));
        snprintf(file_name, sizeof(file_name), "%s", WLESS_PARAM_FILE);
        snprintf(file_name_with_path, sizeof(file_name_with_path), "%s%s", WLESS_PARAM_PATH, file_name);

        if (0 != access(file_name_with_path, F_OK)) {
            return -1;
        }
        struct stat stat_info;
        if (stat(file_name_with_path, &stat_info) == 0) {
            file_size = stat_info.st_size;
            // printf("param file size = %d\r\n", file_size);
        }
    } else if(req_get_file_head.FileType == 0x2)
    {
        if(get_upgrade_file(FILE_UPGRADE_MPU_RS485, file_name, &file_size))
        {
            snprintf(file_name_with_path, sizeof(file_name_with_path), "%s/%s", FW_WLESS_DIR, file_name);
        }
    } else if(req_get_file_head.FileType == 0x3)
    {
        if(get_upgrade_file(FILE_UPGRADE_RH605, file_name, &file_size)) {
            snprintf(file_name_with_path, sizeof(file_name_with_path), "%s/%s", FW_WLESS_DIR, file_name);
        }
    } else if(req_get_file_head.FileType == 0x4)
    {
        if(get_upgrade_file(FILE_UPGRADE_MCU, file_name, &file_size)) {
            snprintf(file_name_with_path, sizeof(file_name_with_path), "%s/%s", FW_WLESS_DIR, file_name);
        }
    } else if(req_get_file_head.FileType == 0x5)
    {
        if(get_upgrade_file(FILE_SENSOR_CONFIG, file_name, &file_size)) {
            snprintf(file_name_with_path, sizeof(file_name_with_path), "%s/%s", FW_WLESS_DIR, file_name);
        }
    }

    // DT_WLS_LOG("DBG: file_name_with_path = %s, file_size=%d\n", file_name_with_path, file_size);
    memcpy(reply.StationCode, req_get_file_head.StationCode, 8);
    strncpy(reply.FileName, file_name, sizeof(file_name));
    if(req_get_file_head.FileType == 0x1)
    {
        //check weather need update param
        int read_value = DT_DISABLE;
        if (DT_EOK == wless_param_update_flag_get(&read_value))
            reply.FileName[31] = read_value;
        else
            reply.FileName[31] = DT_DISABLE;
    }
    if (req_get_file_head.FileType == 0x5) {
        uart_cmd_ret_sensor_info_t s[MAX_SENSOR_CNT] = {0};
        if (!load_sensor_config(file_name_with_path, s)) {
            DT_WLS_ERR("Failed to load %s\r\n", file_name_with_path);
            return false;
        }
        reply.FileCrc = ComputeCRC32((unsigned char *)s, sizeof(s));
        reply.FileSize = sizeof(s);
        file_size = sizeof(s);
    } else {
        uint8_t data[file_size+16];
        memset(data, 0 ,sizeof(data));
        int fd = open(file_name_with_path, O_RDONLY);
        if (fd < 0) {
            return false;
        }
        ret = read(fd, data, file_size);

        close(fd);
        if (ret != file_size) {
            return -1;
        }
        reply.FileCrc = ComputeCRC32(data, file_size);
        reply.FileSize = file_size;
    }
    reply.TotalPacket = ((file_size%UART_SIZE_PER_PKT == 0) ? (file_size/UART_SIZE_PER_PKT) : (file_size/UART_SIZE_PER_PKT + 1));

    DT_WLS_LOG("FileName = %s, FileCrc = 0x%x, FileSize = %d, TotalPacket = %d\r\n",
           reply.FileName, reply.FileCrc, reply.FileSize, reply.TotalPacket);

    ret = 0;
    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

/**
 * @brief 这里是真正传输升级文件的地方
 *        请求方式: 分包请求
 *
 */
int get_file_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    int is_delete = 0;
    uart_cmd_0x32_t get_file_head;
    uart_cmd_0x32_ret_t reply;
    memset(&reply, 0, sizeof(reply));
    wless_param_t *pwless_param = get_wless_param();

#if 1
    unsigned char info[UART_ACK_BUF_LEN];
    int file_fd = -1;
    unsigned char *buf = NULL;
    char file_name[128];
    int file_size = 0;
    unsigned char cs = 0;
    uart_proto_tail_t ret_tail;
    char file_name_with_path[256];
    unsigned char send_buf[UART_RECV_LEN];
    unsigned int send_len = 0;
    unsigned int FileTotalPacket;
    unsigned int head_len = 0;

    memset(info, 0, sizeof(info));
    memset(send_buf, 0, sizeof(send_buf));
    memset(file_name, 0, sizeof(file_name));
    memset(file_name_with_path, 0, sizeof(file_name_with_path));

    memcpy(&get_file_head, pdata, sizeof(get_file_head));
    //DT_WLS_LOG("file type:%d\n",get_file_head.FileType);
    if(get_file_head.FileType == 0x1)
    {
        memset(file_name_with_path, 0, sizeof(file_name_with_path));
        snprintf(file_name, sizeof(file_name), "%s", WLESS_PARAM_FILE);
        snprintf(file_name_with_path, sizeof(file_name_with_path), "%s%s", WLESS_PARAM_PATH, file_name);

        struct stat stat_info;
        if (stat(file_name_with_path, &stat_info) == 0) {
            if (S_ISDIR(stat_info.st_mode)) {
                goto ack;
            }
            file_size = stat_info.st_size;
        }

        buf = (unsigned char *)malloc(file_size);
        if(buf == NULL)
        {
            DT_WLS_ERR("malloc error...\r\n");
            goto ack;
        }

        file_fd = open(file_name_with_path, O_RDONLY);
        if(file_fd < 0)
        {
            DT_WLS_ERR("read file: %s error...\r\n", file_name);
            goto ack;
        }
        read(file_fd, buf, file_size);
        close(file_fd);

        //dt_mqtt_wls_collectparam_get(0)->is_update = false;
    }
    else if(get_file_head.FileType == 0x2)
    {
        get_upgrade_file(FILE_UPGRADE_MPU_RS485, file_name, &file_size);
        snprintf(file_name_with_path, sizeof(file_name_with_path), "%s/%s", FW_WLESS_DIR, file_name);

        buf = (unsigned char *)malloc(file_size);
        if(buf == NULL)
        {
            DT_WLS_ERR("malloc error...\r\n");
            goto ack;
        }

        file_fd = open(file_name_with_path, O_RDONLY);
        if(file_fd < 0)
        {
            DT_WLS_ERR("read file: %s error...\r\n", file_name);
            goto ack;
        }
        read(file_fd, buf, file_size);
        close(file_fd);
    }
    else if(get_file_head.FileType == 0x3)
    {
        get_upgrade_file(FILE_UPGRADE_RH605, file_name, &file_size);
        snprintf(file_name_with_path, sizeof(file_name_with_path), "%s/%s", FW_WLESS_DIR, file_name);

        buf = (unsigned char *)malloc(file_size);
        if(buf == NULL)
        {
            DT_WLS_ERR("malloc error...\r\n");
            goto ack;
        }

        file_fd = open(file_name_with_path, O_RDONLY);
        if(file_fd < 0)
        {
            DT_WLS_ERR("read file: %s error...\r\n", file_name);
            goto ack;
        }
        int size=read(file_fd, buf, file_size);
        //printf("filesize:%d,read:%d\n",file_size,size);
        close(file_fd);
    }
    else if(get_file_head.FileType == 0x4)
    {
        get_upgrade_file(FILE_UPGRADE_MCU, file_name, &file_size);
        snprintf(file_name_with_path, sizeof(file_name_with_path), "%s/%s", FW_WLESS_DIR, file_name);

        buf = (unsigned char *)malloc(file_size);
        if(buf == NULL)
        {
            DT_WLS_ERR("malloc error...\r\n");
            goto ack;
        }

        file_fd = open(file_name_with_path, O_RDONLY);
        if(file_fd < 0)
        {
            DT_WLS_ERR("read file: %s error...\r\n", file_name);
            goto ack;
        }
        read(file_fd, buf, file_size);
        close(file_fd);
    }
    else if(get_file_head.FileType == 0x5)
    {
        get_upgrade_file(FILE_SENSOR_CONFIG, file_name, &file_size);
        snprintf(file_name_with_path, sizeof(file_name_with_path), "%s/%s", FW_WLESS_DIR, file_name);

        uart_cmd_ret_sensor_info_t s[MAX_SENSOR_CNT] = {0};
        if (!load_sensor_config(file_name_with_path, s)) {
            DT_WLS_ERR("Failed to load %s\r\n", file_name_with_path);
        }
        file_size = sizeof(s);

        buf = (unsigned char *)malloc(file_size);
        if(buf == NULL)
        {
            DT_WLS_ERR("malloc error...\r\n");
            goto ack;
        }
        memcpy(buf, s, file_size);
        // is_delete = 1;
    }

    ret = 0;
ack:
    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_GET_FILE;
    head_len = sizeof(reply);
    FileTotalPacket = ((file_size%UART_SIZE_PER_PKT == 0) ? file_size/UART_SIZE_PER_PKT : (file_size/UART_SIZE_PER_PKT + 1));
    if(get_file_head.CurPacketNum < FileTotalPacket - 1)
    {
        head_len += UART_SIZE_PER_PKT;
    } else {
        head_len += (file_size - get_file_head.CurPacketNum*UART_SIZE_PER_PKT);
        if (get_file_head.FileType == 0x01)
            wless_param_update_flag_set(DT_DISABLE);  //采集参数不需要删除, 置位不需要更新即可
        else
            is_delete = 1;  //最后一包时，需要删除升级包
    }
    head_len += sizeof(ret_tail);
    reply.header.len = head_len;
    memcpy(reply.StationCode, get_file_head.StationCode, 8);
    memcpy(send_buf, &reply, sizeof(reply));
    send_len = sizeof(reply);

    if(get_file_head.CurPacketNum < FileTotalPacket - 1)
    {
        memcpy(send_buf + send_len, buf + get_file_head.CurPacketNum*UART_SIZE_PER_PKT, UART_SIZE_PER_PKT);
        send_len += UART_SIZE_PER_PKT;
    } else {
        memcpy(send_buf + send_len, buf + get_file_head.CurPacketNum*UART_SIZE_PER_PKT, file_size - get_file_head.CurPacketNum*UART_SIZE_PER_PKT);
        send_len += (file_size - get_file_head.CurPacketNum*UART_SIZE_PER_PKT);

        if(get_file_head.FileType == 0x1)
        {
           pwless_param->param_flag = 0;
        }
    }

    ret_tail.cs = dt_compute_cs(send_buf, send_len);
    memcpy(send_buf + send_len, &ret_tail, sizeof(ret_tail));
    send_len += sizeof(ret_tail);


    if(get_file_head.CurPacketNum < FileTotalPacket - 1)
    {
        uart_reply_mcu(send_buf, send_len);
    }
    else
    {
        unsigned char rx_buf[4096] = {0}; /* 接收缓冲区 */
        unsigned  int rx_len = 0;        /* 接收长度 */
        printf("update send last file !!!\n");
        if (DT_EOK != uart_send_with_recv(SERIAL_UART9, send_buf, send_len, rx_buf, &rx_len, 2000)) {
            ret = DT_ERROR;
        }
        else
        {
        #if 1
            uint8_t_data_printf(rx_buf, rx_len);
        #endif
            ZIGBEEParam * lparm = getSharedData();
            lparm->update = 1;
            printf("g_ZIGBEEParam->update:%d\n",lparm->update);
        }

    }
#ifdef SNESOR_DBG_PRINTF_ENABLE
    DT_WLS_LOG("send_len = %d, file_size = %d\r\n", send_len, file_size);
#endif
    if(buf)
    {
        free(buf);
        buf = NULL;
    }
#endif
    if (is_delete)
        unlink(file_name_with_path); // remove after send to uart.
    DT_WLS_LOG("get_file_head.CurPacketNum = %d, task_run_state = %d, is_delete = %d\r\n", get_file_head.CurPacketNum, get_task_run_state(), is_delete);

    return ret;
}

int upload_real_eigen_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_0x34_indicator_data_t *pind_data = (uart_0x34_indicator_data_t *)pdata;
    uart_cmd_0x34_ret_t reply;
    memset(&reply, 0, sizeof(reply));
    wless_data_t *pwless_data = get_wless_data();
    uint32_t ch_num = pind_data->ChannelNo;

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_UPLOAD_REAL_EIGEN;
    reply.header.len = sizeof(reply);
    if(pind_data->DataType!=0x01)
    {
        DT_PRINTF("INDEX type :%d, not save!!!!\n",pind_data->DataType);
        return ret;
    }
    memcpy(reply.StationCode, pind_data->StationCode, sizeof(reply.StationCode));

    if(ret == 0)
    {
        reply.RecvState = 1;
        memcpy(&pwless_data->real_eigen[ch_num-1], pind_data, sizeof(uart_0x34_indicator_data_t));
        //TODO save UPLOAD_DATA_TYPE_WLESS_REAL_EIGEN
//        dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_REAL_EIGEN, &pwless_data->real_eigen[ch_num], \
//                            sizeof(uart_0x34_indicator_data_t), (char *)pwless_data->real_eigen[ch_num].SampleTime);
        ZIGBEEParam * lparm = getSharedData();
#ifndef ZIGBEE_TEST_SWITCH
        storeIndexData(&pwless_data->real_eigen[ch_num-1],lparm->sensor_info[ch_num-1].Sn,lparm->count,ch_num-1);
#else
        storeIndexData(&pwless_data->real_eigen[ch_num-1],lparm->sensor_info[g_switch_flag].Sn,lparm->count,ch_num-1);
#endif
    } else {
        reply.RecvState = 0;
    }

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

int spi_flash_info_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x36_t flash_info;
    memset(&flash_info, 0, sizeof(flash_info));
    uart_cmd_new_0x36_t new_flash_info;
    memset(&new_flash_info, 0, sizeof(new_flash_info));
    uint32_t len = data_len + sizeof(uart_proto_head_t) + sizeof(uart_proto_tail_t);

    if (len == sizeof(flash_info)) {
        memcpy(&flash_info.StationCode, pdata, sizeof(flash_info.StationCode));
        memcpy(&flash_info.spi_flash_sector_size, pdata + sizeof(flash_info.StationCode), data_len - sizeof(flash_info.StationCode));
    } else if (len == sizeof(new_flash_info)) {
        memcpy(&new_flash_info.StationCode, pdata, sizeof(new_flash_info.StationCode));
        memcpy(&new_flash_info.spi_flash_sector_size, pdata + sizeof(new_flash_info.StationCode), data_len - sizeof(new_flash_info.StationCode));
    }

    /* 收到命令后立刻设置任务状态
    * 之前在线程里设置, 有可能会导致延迟2s左右才真正地去读,
    * 此时MCU以为MPU已经读完了, 就会让MPU上传数据, MCU发完上传数据命令后就会删除数据,
    * 而此时MPU还未开始读, 从而导致丢数据, 以及采集站日志为0的情况!
    * 这样修改以后, MCU让MPU上传数据前会查任务状态, 发现有running的任务, 就不会继续往下走,
    * 也就不会删除数据了
    * */
//    if(get_task_run_state() == TASK_RUNNING)
//    {
//        DT_WLS_ERR("spi read flash data task is running!\r\n");
//        return 0;
//    }
//    set_task_run_state(TASK_RUNNING);

    /* 在这了去读取spi数据前，需要确保spi-nor.ko加载成功 */
    usleep(100000);
    if (DT_EOK == dt_check_spi_nor_module()) {
        if(len == sizeof(flash_info)) {
            ret = spi_read_flash_data(0, flash_info, new_flash_info);
        } else {
            ret = spi_read_flash_data(1, flash_info, new_flash_info);
        }
    }

    uart_cmd_0x36_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_SPI_FLASH_INFO;
    reply.header.len = sizeof(reply);
    reply.ChannelNo = new_flash_info.ChannelNo;
    reply.WaveType = new_flash_info.DataType;
    reply.State = ret;
    DT_WLS_LOG("DataType:%d,wavetype:%d",new_flash_info.DataType,new_flash_info.WaveType);
    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

typedef enum selfcheck_type_def {
    EMMC_SELF_CHECK = 0x01,
    SPI_NOR_SELF_CHECK = 0x04,
} selfcheck_type_t;

int hw_self_check_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x37_t *phw_check = (uart_cmd_0x37_t *)pdata;

    uart_cmd_0x37_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_HW_SELF_CHECK;
    reply.header.len = sizeof(reply);

    if(phw_check->hw_check_type == EMMC_SELF_CHECK)
    {
        //TODO what check ?
    } else if(phw_check->hw_check_type == 0x02) {
        //TODO what check ?
    } else if(phw_check->hw_check_type == SPI_NOR_SELF_CHECK) {
        //TODO what check ?
    }

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

int hw_check_query_handle (uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x38_t *pcheck_query = (uart_cmd_0x38_t *)pdata;

    uart_cmd_0x38_ret_t reply;
    memset(&reply, 0, sizeof(reply));

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_HW_CHECK_QUERY;
    reply.header.len = sizeof(reply);
    reply.hw_check_type = pcheck_query->hw_check_type;
    reply.check_result = global_hw_check_result;

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

int debug_task_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;

#if 0
    unsigned char tasktype;
    unsigned char state;
    uart_gw_param *pGwParam;
    uart_cmd_ret_sensor_info_t *pSensorParam;
    unsigned char cnt;

    tasktype = pdata[0];

    switch (tasktype)
    {
        case TASK_GET_GW_CONFIG:
        {
            if (gtcm)
            {
                pGwParam = (uart_gw_param *)&left[1];
                gtcm->GetGwConfig(pGwParam);
            }
            break;
        }
        case TASK_SET_GW_CONFIG:
        {
            if (gtcm)
            {
                state = left[1];
                gtcm->SetGwConfig(&state);
            }
            break;
        }
        case TASK_GET_SENSOR_CONFIG:
        {
            if (gtcm)
            {
                cnt = 45;
                pSensorParam = (uart_cmd_ret_sensor_info_t *)&left[1];
                gtcm->GetSensorConfig(pSensorParam, &cnt);
            }
            break;
        }
        case TASK_SET_SENSOR_CONFIG:
        {
            if (gtcm)
            {
                state = left[1];
                gtcm->SetSensorConfig(&state);
            }
            break;
        }
        case TASK_TEMP_SAMPLE:
        {
            if (gtcm)
            {
                state = left[1];
                gtcm->SetSampleParam(&state);
            }
            break;
        }
        case TASK_TEMP_SAMPLE_QUERY:
        {
            if (gtcm)
            {
                state = left[1];
                gtcm->HandleSampleQuery(&state);
            }
            break;
        }
        case TASK_TREND_SAMPLE:
        {
            DT_WLS_LOG("TASK_TREND_SAMPLE, state: %d", left[1]);
            break;
        }
        case TASK_FW_UPGRADE_INFO:
        {
            DT_WLS_LOG("TASK_FW_UPGRADE_INFO, state: %d", left[1]);
            gFwUpgradePerPacketState = left[1];
            gUpgradeCondMutex.lock();
            gUpgradeCond.wakeOne();
            gUpgradeCondMutex.unlock();
            break;
        }
        case TASK_FW_UPGRADE_DATA:
        {
            DT_WLS_LOG("TASK_FW_UPGRADE_DATA, state: %d", left[1]);
            gFwUpgradePerPacketState = left[1];
            gUpgradeCondMutex.lock();
            gUpgradeCond.wakeOne();
            gUpgradeCondMutex.unlock();
            break;
        }
        default:
        {
            DT_WLS_ERR("Error: unkown debug task: %d...", tasktype);
            err = 1;
            goto ack;
        }
    }
#endif

    return ret;
}

int upload_trend_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x41_t *ptrend_data = (uart_cmd_0x41_t *)pdata;
    uart_cmd_0x41_mpu_ack reply;
    memset(&reply, 0, sizeof(reply));
    uint32_t ch_num = ptrend_data->SensorChannel;

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_UPLOAD_TREND;
    reply.header.len = sizeof(reply);
	reply.State = 1;

    wless_data_t *pwless_data = get_wless_data();
    memcpy(&pwless_data->trend_data[ch_num], ptrend_data, sizeof(uart_cmd_0x41_t));
    //TODO save UPLOAD_DATA_TYPE_WLESS_TREND
//    dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_TREND, &pwless_data->trend_data[ch_num], \
//                        sizeof(uart_cmd_0x41_t), (char *)pwless_data->trend_data[ch_num].SampleTime);

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

int upload_upgrade_state_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x42_t *pUpgradeState = (uart_cmd_0x42_t *)pdata;
    uart_cmd_0x42_mpu_ack reply;

    memset(&reply, 0, sizeof(reply));
    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_UPLOAD_UPGRADE_STATE;
    reply.header.len = sizeof(reply);
	reply.State = 1;
#if 0
   	if (gtcm) {
   			gtcm->SendUpgradeState(pUpgradeState);
   	}

   	gFwUpgradePerPacketState = pUpgradeState->State;
		gUpgradeCondMutex.lock();
		gUpgradeCond.wakeOne();
		gUpgradeCondMutex.unlock();
#endif

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

static int save_mcu_log(char *buf, int len)
{

    char *temp = strtok(buf, "\r\n");
    while (temp) {
        // DT_WLS_LOG("%s\r\n", temp);
        temp = strtok(NULL, "\r\n");
    }

	return true;
}

int upload_log_handle(uint8_t *pdata, uint32_t data_len)
{
    int ret = 0;
    uart_cmd_0x43_mpu_ack reply;
    memset(&reply, 0x00, sizeof(reply));

    reply.header.head = CMD_SOURCE_MPU;
    reply.header.cmd = UART_CMD_UPLOAD_LOG;
    reply.header.len = sizeof(reply);
    reply.State = 0;


	if (save_mcu_log((char *)pdata, data_len)) {
		reply.State = 1;
		dt_system("sync");
	}

    reply.tailer.cs = dt_compute_cs((unsigned char *)&reply, sizeof(reply)-1);
    ret = uart_reply_mcu((uint8_t *)&reply, sizeof(reply));
    return ret;
}

/**
 * @brief 系统状态设置
 */
FREQ_PUBLIC void sys_status_set(int status)
{
    pthread_mutex_lock(&sys_mutex);
    sys_status = status;
    pthread_mutex_unlock(&sys_mutex);
}

/**
 * @brief 系统状态设置
 */
FREQ_PUBLIC int sys_status_get(void)
{
    return sys_status;
}
