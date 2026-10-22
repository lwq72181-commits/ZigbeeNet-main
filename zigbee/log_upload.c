#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "log_upload.h"
#include "common.h"
//#include "dt_mqtt_protocol.h"
//#include "dt_network.h"
#include "ringbuffer.h"
#include "zigbeetype.h"
//#include "dt_mqtt.h"

static pthread_mutex_t log_save_mutex = PTHREAD_MUTEX_INITIALIZER;/* 用于日志存储 */
static log_info_t log_mgr;
char compress_format[ZIP_TYPE_MAX][10] = {COMPRESS_FORMAT_BZ2, COMPRESS_FORMAT_7Z};


/**
 * @brief 用于获取日志信息结构体
 *
 * @return 指向日志信息的结构体地址
 */
FREQ_PUBLIC log_info_t *log_mgr_get(void)
{
    return &log_mgr;
}

/**
 * @brief 检查日志索引是否有效
 * @return DT_EOK or DT_ERROR
 */
FREQ_PRIVATE int _log_index_valid(upload_log_index_t *param)
{
    if ((OFFLINE_INDEX_LABEL_START != param->body.label_start) || (OFFLINE_INDEX_LABEL_END != param->body.label_end)) {
        DT_ERR("index param label value err.");
        DT_LOG("param->label_start 0x%04x.",param->body.label_start);
        DT_LOG("param->label_end   0x%04x.",param->body.label_end);
        return DT_ERROR;
    }

    return DT_EOK;
}

/**
 * @brief 日志文件上传接口
 * @param file_path 日志文件路径名
 * @param p 日志索引
 * @param is_del_file 是否需要删除上传文件
 * @return DT_EOK or DT_ERROR
 */
FREQ_PRIVATE int _log_upload_from_file(char *file_path, upload_log_index_t *p, int * is_del_file)
{
    int fd = -1;
    int iret = DT_ERROR;
    char FileName[64] = {0};
    unsigned read_crc = 0;
    char *log_buff    = NULL;
    upload_log_index_t  *plog = NULL;
    char (*pformat)[10] = compress_format;

    /* step1: 读出数据*/
    log_buff = (char *)dt_malloc(p->body.datasize);
    if (!log_buff) {
        DT_PRINTF("malloc failed.\r\n");
        goto err;
    }
    memset(log_buff, 0, p->body.datasize);

    fd = open(file_path, O_RDONLY);
    if(fd < 0 ) {
        DT_PRINTF(" open file %s failed\r\n", file_path);
        goto err;
    }

    if(dt_read(fd, log_buff, p->body.datasize) < 0) {
        DT_PRINTF("read file %s failed\r\n", file_path);
        goto err;
    }

    /* step2: 计算crc */
    read_crc = dt_crc32(log_buff, p->body.datasize);
    if (p->body.crc != read_crc) {
        DT_PRINTF("read crc 0x%x != calc crc 0x%x ", read_crc, p->body.crc);
        *is_del_file = true;
        goto err;
    }
    //DT_PRINTF("src=0x%x, read=0x%x\r\n", p->body.crc, read_crc);

    /* step3: 构造上传文件名 */
    if (p->body.compress_type >= BZIP_TYPE && p->body.compress_type < ZIP_TYPE_MAX ) {
        snprintf(FileName, sizeof(FileName),"dtlog_%02d%02d%02d%02d%02d%02d%02d.mpulog%s",
            p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6], pformat[p->body.compress_type]);
    } else {
        snprintf(FileName, sizeof(FileName),"dtlog_%02d%02d%02d%02d%02d%02d%02d.mpulog.bz2", //为了兼容新旧方式，这里进行适配
            p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6]);
    }

    /* step4: 上传 */
    /* 发送数据,上位机不校验MD5码，2022-7-7 */
    //TODO UPLOAD log file
//    if (DT_MQTT_OK != dt_mqtt_upload_log(FileName, NULL, log_buff, p->body.datasize, DT_MQTT_NO_ACK_TIMEOUT)) {
//       DT_PRINTF("log file UL failed\r\n");
//       iret = DT_ETIMEOUT;
//       goto err;
//    }

    iret = DT_EOK;
err:
    if (log_buff) {
        dt_free(log_buff);
        log_buff = NULL;
    }
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    return iret;
}

/**
 * @brief 文件名构造函数
 * @param ppath_file 构造文件名存储地址
 * @param type 构造文件类型
 * @param param 参数信息
 * @return DT_EOK or DT_ERROR
 */
FREQ_PUBLIC int dt_file_name_construct(char *ppath_file, unsigned int length, dt_upload_data_type_t type, void *param)
{
    int iret = DT_ERROR;

    if (!ppath_file) {
        DT_ERR("ppath_file param is NULL");
        goto __exit;
    }

    switch (type) {
        case UPLOAD_DATA_TYPE_LOG:
        {
            upload_log_index_t *p  = (upload_log_index_t*)param;
            snprintf(ppath_file, length, "%s/dtlog_%02d%02d%02d%02d%02d%02d%02d.mpulog", LOG_FORMAL_DIR,
                p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6]);
        }
        break;
//        case UPLOAD_DATA_TYPE_RPM:
//        {
//            upload_rpm_index_t *p  = (upload_rpm_index_t *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d_ch%d%s", RPM_DATA_DIR,
//                p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6], p->body.ch_no, RPM_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_WAVE:
//        {
//            upload_wave_index_t *p  = (upload_wave_index_t *)param;
//            if (DT_HIGH_FREQ == p->body.acc_way) {
//                snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d_h_ch%d%s", WAVE_DATA_DIR,
//                    p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6],p->body.ch_no, WAVE_TYPE);
//            } else if (DT_LOW_FREQ == p->body.acc_way) {
//                snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d_l_ch%d%s", WAVE_DATA_DIR,
//                    p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6],p->body.ch_no, WAVE_TYPE);
//            } else {
//                /* nothing */
//            }
//        }
//        break;
//        case UPLOAD_DATA_TYPE_LWAVE:
//        {
//            upload_lw_index_t *p  = (upload_lw_index_t *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d_ch%d%s", LW_DATA_DIR,
//                p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6],p->body.ch_no, LW_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_EIGEN:
//        {
//            char *ptime = (char *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d%s", EIGEN_DATA_DIR,
//                ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5], ptime[6], EIGEN_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_ALARM_WAVE:
//        {
//            upload_wave_index_t *p  = (upload_wave_index_t *)param;
//            if (DT_HIGH_FREQ == p->body.acc_way) {
//                snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d_h_ch%d%s", ALARM_WAVE_DATA_DIR,
//                    p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6],p->body.ch_no, ALARM_WAVE_TYPE);
//            } else if (DT_LOW_FREQ == p->body.acc_way) {
//                snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d_l_ch%d%s", ALARM_WAVE_DATA_DIR,
//                    p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6],p->body.ch_no, ALARM_WAVE_TYPE);
//            } else {
//                /* nothing */
//            }
//        }
//        break;
//        case UPLOAD_DATA_TYPE_RTU:
//        {
//            char *ptime = (char *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d%s", RTU_DATA_DIR,
//                ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5], ptime[6], RTU_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_TEMP:
//        {
//            char *ptime = (char *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d%s", TEMPERATURE_DATA_DIR,
//                ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5], ptime[6], TEMPERATURE_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_TECH:
//        {
//            char *ptime = (char *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d%s", TECH_DATA_DIR,
//                ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5], ptime[6], TECH_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_TAG:
//        {
//            upload_tag_index_t *p = (upload_tag_index_t *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d_%d%s", TAG_DATA_DIR,
//                p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6], p->body.data_type, TAG_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_AUDIO:
//        {
//            upload_audio_index_t *p  = (upload_audio_index_t *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d_ch%d%s", AUDIO_DATA_DIR,
//                p->body.time[0], p->body.time[1], p->body.time[2], p->body.time[3], p->body.time[4], p->body.time[5], p->body.time[6], p->body.ch_no, AUDIO_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_WLESS_REAL_EIGEN:
//        {
//            char *pfile_name = (char *)param;
//            strcpy(ppath_file, pfile_name);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_WLESS_TREND:
//        {
//            char *ptime = (char *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d%s", WLESS_TREND_DATA_DIR,
//                ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5], ptime[6], WLESS_TREND_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_WLESS_WAVE:
//        {
//            char *ptime = (char *)param;
//            snprintf(ppath_file, length, "%s/%02d%02d-%02d-%02dT%02d:%02d:%02d%s", WLESS_WAVE_DATA_DIR,
//                ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5], ptime[6], WLESS_WAVE_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_WLESS_EIGEN:
//        {
//            char *pfile_name = (char *)param;
//            strcpy(ppath_file, pfile_name);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_WLESS_MCU_LOG:
//        {
//            char *ptime = (char *)param;
//            snprintf(ppath_file, length, "%s/a_%02d%02d-%02d-%02dT%02d:%02d:%02d%s", WLESS_MCU_LOG_DATA_DIR,
//                ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5], ptime[6], WLESS_MCU_LOG_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_WLESS_SENSOR_LOG:
//        {
//            char *ptime = (char *)param;
//            snprintf(ppath_file, length, "%s/a_%02d%02d-%02d-%02dT%02d:%02d:%02d%s", WLESS_SENSOR_LOG_DATA_DIR,
//                ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5], ptime[6], WLESS_SENSOR_LOG_TYPE);
//        }
//        break;
//        case UPLOAD_DATA_TYPE_WLESS_BATTERY:
//        {
//            char *pfile_name = (char *)param;
//            strcpy(ppath_file, pfile_name);
//        }
//        break;
        default :
        {
            DT_ERR("input type param err.\r\n");
        }
        break;
    }

     iret = DT_EOK;
__exit:
    return iret;
}


/**
 * brief 日志检索上传
 */
FREQ_PRIVATE int _log_data_upload(void)
{
    int fd = -1;
    int ret = DT_ERROR;
    int file_exist = false;
    int fd_index      = -1;
    int fd_index_next = -1;
    char ppath[FILE_NAME_LEN]       = {0};
    char index_ppath[FILE_NAME_LEN] = {0};
    char *ppath_file      = NULL;
    char *pbuf_index      = NULL;
    char *pbuf_index_next = NULL;
    upload_log_index_t *pindex = NULL;
    unsigned int dir_name_len = 0;
    unsigned int file_index_len = 0;
    unsigned int index_len = 0;

    /* step1: 确认索引名称 */
    snprintf(ppath, sizeof(ppath), "%s/", LOG_FORMAL_DIR);
    dir_name_len = strlen(ppath);
    ppath_file   = ppath + dir_name_len;

    memset(ppath_file, 0, FILE_NAME_LEN - dir_name_len);
    strcpy(ppath_file, INDEX_FILE);
    //检查文件是否存在及访问权限
    if (0 != access(ppath, F_OK)) {
        goto end;
    }
    DT_PRINTF("log start upload... \r\n");

    fd_index = open(ppath, O_RDWR | O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    //printf("fd_index==========%d\r\n",fd_index);
    if (fd_index < 0) {
        //index 为空, 删除该目录下所有文件
        memset(ppath_file, 0, FILE_NAME_LEN - dir_name_len);
        dt_delete_dirfile(ppath);
        goto end;
    }
    memcpy(index_ppath, ppath, sizeof(ppath));/* 拷贝复制索引文件名称，不然后面路径会被改变 */
    // DT_PRINTF("upload---data index name : %s\r\n", index_ppath);

    /* step2: 读出索引内容到内存中 */
    file_index_len = dt_get_file_size(ppath);
    // DT_PRINTF("upload---data index len : %d\r\n", file_index_len);
    if((file_index_len % sizeof(upload_log_index_t)) != 0) {/* 检查索引是否正确 */
        DT_PRINTF("-----log index file len = %d, error, please check(or delete it)\r\n", file_index_len);
        dt_delete_dirfile(ppath);
        goto end;
    }

    pbuf_index = (char *)dt_malloc(file_index_len);
    if(!pbuf_index) {
        DT_PRINTF("-----log index malloc err\r\n");
        goto end;
    }
    memset(pbuf_index, 0, file_index_len);

    if(file_index_len != dt_read(fd_index, pbuf_index, file_index_len)) {/* 将索引读取到内存 */
        DT_PRINTF("-----log index file read error, please check(or delete it)\r\n");
        dt_delete_dirfile(ppath);
        goto end;
    }

    /* step3: 取出第一个index */
    pindex  = (upload_log_index_t*)pbuf_index;
    if (!_log_index_valid(pindex)) {
        DT_PRINTF("-----log index file err, will be delete\r\n");
        memset(ppath_file, 0, FILE_NAME_LEN - dir_name_len);
        dt_delete_dirfile(ppath);
        goto end;
    }

    /* step4: 根据索引时间，构造待上传文件名*/
    memset(ppath_file, 0, FILE_NAME_LEN - dir_name_len);
    if (!dt_file_name_construct(ppath, FILE_NAME_LEN, UPLOAD_DATA_TYPE_LOG, pindex)) {
        DT_PRINTF("-----file_name_construct err failed\r\n");
        goto end; /* 上传失败，不删除文件 */
    }

    /* step5: 检验数据信息是否正确*/
    if (pindex->body.datasize == dt_get_file_size(ppath)) {
        file_exist = true;
        DT_PRINTF("log file(%s) will be uploaded, size:%d \r\n", ppath, pindex->body.datasize);
    } else {
        file_exist = false;
        DT_PRINTF("log file(%s) is not exist, upload skip\r\n", ppath);
    }

    /* step6: 如果存在，则上传 */
    if (file_exist) {
        int is_del_file = DT_ERROR;
        if (DT_EOK != (ret = _log_upload_from_file(ppath, pindex, &is_del_file))) {
            DT_PRINTF("-----upload log failed.\r\n");
            if (is_del_file)
                DT_PRINTF("CRC err! wait delete file.\r\n");
            else {
                DT_PRINTF("upload timeout.\r\n");
                goto end;/* 因网络原因，则不删除 */
            }
        } else {
            DT_PRINTF("-----upload log success.\r\n");
        }
        unlink(ppath);/* 删除文件 */
    }

    /* step7: 更新索引 */
    if (fd_index > 0) {/* 先关闭打开索引文件 */
        close(fd_index);
        fd_index = -1;
    }
    index_len = file_index_len - sizeof(upload_log_index_t);
    if (index_len) {
        pbuf_index_next = (char *)(pbuf_index + sizeof(upload_log_index_t));/* 将数据指针向后偏移一个索引大小 */
        fd_index_next  = open (index_ppath, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);/* 以覆盖方式重新打开索引文件 */
        dt_write(fd_index_next, pbuf_index_next, file_index_len - sizeof(upload_log_index_t));
    } else {
        dt_delete_dirfile(index_ppath);
    }
    pbuf_index_next = NULL;
    ppath_file = NULL;

    sync();
    ret = DT_EOK;
end:
    if (fd_index_next > 0) {
        close(fd_index_next);
        fd_index_next = -1;
    }
    if (fd_index > 0) {
        close(fd_index);
        fd_index_next = -1;
    }
    if (pbuf_index) {
        dt_free(pbuf_index);
        pbuf_index = NULL;
    }
    return ret;
}

/**
 * @brief 日志临时文件=>正式文件
 *        用于将临时文件，压缩，计算crc，填写上传索引
 */
FREQ_PRIVATE int _log_data_save(dt_upload_data_type_t type,  void *data, int size, char *ptime)
{
    int ret  = DT_ERROR;
    int fd_file   = -1;
    int fd_index  = -1;
    char path[2 * FILE_NAME_LEN]    = {0};
    char upload_name[FILE_NAME_LEN] = {0};
    char *cmd       = path;
    char time[7] = {0};
    upload_log_index_t log;
    unsigned int log_crc = 0;
    char *log_buff        = NULL;
    unsigned int log_size = 0;
    char (*pformat)[10] = compress_format;
    int format          = BZIP_TYPE;    /* 0:bz2; 1:7z */
    DT_PRINTF("log start save... \r\n");

    /* 初始化参数 */
    dt_current_time_get(time);

    /* 确定压缩类型 */
    (0 == access(COMPRESS_TOOL_PATH, F_OK)) ? (format = P7ZIP_TYPE) : (format = BZIP_TYPE);
    if (format < BZIP_TYPE || format >= ZIP_TYPE_MAX)
        format = BZIP_TYPE;

    /* 删除遗留文件 */
    memset(path, 0x00, sizeof(path));
    snprintf(path, sizeof(path), "%s%s", LOG_TEMP_COPY_FILE, pformat[format]);
    if (0 == access(path, F_OK)) /* 判断是否存在上次遗留文件 */
        unlink(path);

    /* 压缩 */
    if (format) {
        encryption_compress(LOG_TEMP_COPY_FILE);
        unlink(LOG_TEMP_COPY_FILE);
    } else {
        bzip2_compress(LOG_TEMP_COPY_FILE);
    }

    /* 计算源文件crc */
    memset(upload_name, 0x00, FILE_NAME_LEN);
    snprintf(upload_name, FILE_NAME_LEN, "%s%s", LOG_TEMP_COPY_FILE, pformat[format]);
    log_size = dt_get_file_size(upload_name);

    fd_file = open(upload_name, O_RDONLY);
    if(fd_file < 0 ) {
        DT_PRINTF(" open file %s failed\r\n", upload_name);
        goto end;
    }

    log_buff = (char *)dt_malloc(log_size);
    if (!log_buff) {
        DT_PRINTF("malloc failed.\r\n");
        goto end;
    }

    if(read(fd_file, log_buff, log_size) < 0) {
        DT_PRINTF("read file %s failed\r\n");
        goto end;
    }
    log_crc = dt_crc32(log_buff, log_size);

    /* 转存正式区 */
    memset(cmd, 0x00, 2 * FILE_NAME_LEN);
    memset(upload_name, 0x00, sizeof(upload_name));

    snprintf(upload_name, FILE_NAME_LEN, "%s/dtlog_%0d%0d%02d%02d%02d%02d%02d.mpulog",
        LOG_FORMAL_DIR, time[0], time[1], time[2], time[3], time[4], time[5], time[6]);
    snprintf(cmd, 2 * FILE_NAME_LEN, "mv %s%s %s", LOG_TEMP_COPY_FILE, pformat[format], upload_name);
    dt_system(cmd);

    /* 索引信息填充 */
    memset(&log, 0x00, sizeof(upload_log_index_t));

    log.body.label_start = OFFLINE_INDEX_LABEL_START;
    memcpy(log.body.time, time, sizeof(log.body.time));
    log.body.datasize      = log_size;
    log.body.crc           = log_crc;
    log.body.compress_type = format;
    log.body.label_end = OFFLINE_INDEX_LABEL_END;

    /* 索引信息写入 */
    memset(path, 0x00, sizeof(path));

    snprintf(path, sizeof(path), "%s/%s", LOG_FORMAL_DIR, INDEX_FILE);
    fd_index = open(path, O_CREAT | O_RDWR | O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); /*追加读写方式打开或建立一个二进制文件*/
    if (fd_index < 0 ) {
        DT_PRINTF("open %s failed \r\n", path);
        goto end;
    }
    if (sizeof(upload_log_index_t) != write(fd_index, &log, sizeof(upload_log_index_t))) {
        DT_PRINTF("log write failed \r\n");
        goto end;
    }
    DT_PRINTF("save success---index file name : %s, size %d\r\n", path, sizeof(upload_log_index_t));

    ret = DT_EOK;
end:
    if (fd_index >= 0) {
        close(fd_index);
        fd_index = -1;
    }
    if (fd_file >= 0) {
        close(fd_file);
        fd_file = -1;
    }
    if (log_buff) {
        dt_free(log_buff);
        log_buff = NULL;
    }

    return ret;
}


/**
 * @brief 日志内存=>临时文件
 *        当ringbuffer中数据，达到指定长度会调用该接口
 *
 */
FREQ_PUBLIC int _log_memory_to_tempfile(void)
{
#define SINGLE_FILE_SIZE_MAX (1024 * 1024) /* 单个文件支持最大1M */
    int fd   = -1;
    int iret = DT_ERROR;
    char cmd[256] = {0};
    int read_len  = 0;
    int file_size = 0;

    /* 读出全部 */
    memset(log_mgr.read_buff, 0, sizeof(log_mgr.read_buff));
    read_len = rt_ringbuffer_get(log_mgr.log_rb, log_mgr.read_buff, LOG_RINGBUFFER_SIZE);

    fd = open(LOG_TEMP_FILE, O_CREAT | O_RDWR | O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        DT_PRINTF("open file:%s failed!\r\n", LOG_DIR);/* 存log前，不能有打印 */
        goto err;
    }
    if (read_len != write(fd, (char*)log_mgr.read_buff, read_len)) {
        DT_PRINTF("log write failed.\r\n");
        goto err;
    }
    file_size = dt_get_file_size(LOG_TEMP_FILE);

    /* 当文件达到需要上传长度时 */
    if (file_size < SINGLE_FILE_SIZE_MAX)
        goto _normal_process;

    snprintf(cmd, sizeof(cmd), "cp %s %s", LOG_TEMP_FILE, LOG_TEMP_COPY_FILE);
    dt_system(cmd);
    if (fd >= 0) {/* linux下暂时先关闭再删除，不然会出现文件被占用情况 */
        close(fd);
        fd = -1;
    }

    dt_delete_dirfile(LOG_TEMP_FILE);
    dt_delete_dirfile(LOG_TEMP_COPY_FILE);
    //TODO save log file
//    dt_upload_data_save(UPLOAD_DATA_TYPE_LOG, NULL, SINGLE_FILE_SIZE_MAX, NULL);
_normal_process:
    iret = DT_EOK;
err:
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    return iret;
}

/**
 * @brief 日志处理线程
 *        当ringbuffer中日志达到指定长都，会给此线程发信号，将日志进行文件存储
 *        当reboot前，将日志进行文件存储
 *
 * @param arg 预留形参
 */
FREQ_PRIVATE void *log_save_thread_enter(void * arg)
{
    pthread_detach(pthread_self());

    while (1) {
        DT_PRINTF("log_save_thread_enter.\r\n");
        sem_wait(&log_mgr.sem);
        DT_PRINTF("log_save_thread_enter1.\r\n");
        _log_memory_to_tempfile();
        DT_PRINTF("log_save_thread_enter2.\r\n");
    }
    pthread_exit(NULL);
}

FREQ_PUBLIC int do_file_save_check(int size)
{
    long long free_space = 0;
     /* disk space check */
    if (DT_EOK != dt_df_check(DISK_SPACE_PATH, &free_space)) {
        // DT_PRINTF("file system error(only read).\r\n");
        return DT_ERROR;
    }

    if (free_space <= size) {
        DT_PRINTF("not enough space (%d).\r\n", free_space);
        return DT_ERROR;
    }

    return DT_EOK;
}

/**
 * brief 日志存储接口
 */
FREQ_PUBLIC void dt_save_log(void *buf, unsigned int len)
{
    pthread_mutex_lock(&log_save_mutex);
   /* step1: 先写ringbuffer */
    if (!log_mgr.log_rb)
        goto _exit;
#ifdef DT_FILE_SPACE_CHECK  /* 针对日志保存，检查剩余空间*/
    if (DT_EOK != DO_LOG_FILE_SAVE_CHECK())
        goto _exit;
#endif

    if (0 == rt_ringbuffer_put(log_mgr.log_rb, (unsigned char *)buf, len)) {
        sem_post(&log_mgr.sem);
    }

    /* step2: 判断ringbuffer长度 */
    if (rt_ringbuffer_data_len(log_mgr.log_rb) >= LOG_DATA_SAVE_LEN) {
        sem_post(&log_mgr.sem);
    }

_exit:
    pthread_mutex_unlock(&log_save_mutex);
}

FREQ_PRIVATE dt_upload_data_t upload_log_obj = {
    .list = {0},
    .mutex           = PTHREAD_MUTEX_INITIALIZER,
    .is_mutex        = 0,
    .data_type       = UPLOAD_DATA_TYPE_LOG,
    .protocol        = PROTOCOL_TYPE_DOTON,
    .online          = _log_data_upload,
    .offline         = NULL,
    .save            = _log_data_save,
};

/**
 * @brief 日志存储初始化
 *        初始化ringbuffer,以及日志存储处理线程
 *
 */
FREQ_PUBLIC void log_save_init(void)
{
    pthread_t log_pid;

    sem_init(&log_mgr.sem, 0, 0);

    /* 初始化ringbuffer空间 */
    log_mgr.log_rb = rt_ringbuffer_create(LOG_RINGBUFFER_SIZE);
    if (!log_mgr.log_rb) {
        DT_ERR("log_rb create failed.\r\n");
    }

    if (0 != pthread_create(&log_pid, NULL, &log_save_thread_enter, NULL)) {
        DT_ERR("create log save thread error!\n");
    }
}

/**
 * @brief 日志上传初始化
 */
FREQ_PUBLIC void dt_log_upload_init(void)
{
    INIT_LIST_HEAD(&upload_log_obj.list);

    //检查文件是否存在及访问权限
    if (0 != access(LOG_DATA_DIR, F_OK)) {
        dt_create_dir(LOG_DATA_DIR);
    }

//    if (!upload_data_type_register(&upload_log_obj)) {
//        DT_ERR("log data upload register failed!\r\n");
//    }
}

