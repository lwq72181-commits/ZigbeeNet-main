#ifndef __LOG_UPLOAD_H
#define __LOG_UPLOAD_H
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
//#include "param_manager.h"
//#include "dt_upload_manager.h"
#include "zigbeetype.h"
#define LOG_INDEX_SIZE_LEN_MAX (128)
#define LOG_DATA_SAVE_LEN       (32 * 1024)
#define LOG_RINGBUFFER_SIZE     (48 * 1024)  /* 暂存日志ringbuffer大小 */

#pragma pack(1)
typedef struct _log_info_def {
    struct rt_ringbuffer* log_rb;
    unsigned char read_buff[LOG_RINGBUFFER_SIZE];
    sem_t sem;
}log_info_t;

/* 日志存储数据结构 */
typedef struct _log_index_def {
    unsigned int  label_start;                /* 0x5a5a5a5a */
    unsigned char time[7];                    /* 时间 */
    unsigned int crc;                         /* 数据crc */
    unsigned int datasize;                    /* 数据大小 */
    unsigned int compress_type;               /* 压缩类型 */
    unsigned int label_end;                   /* 0xa5a5a5a5*/
}log_index_t;

typedef struct _upload_log_index_def {
    log_index_t body;
    char rsvd[LOG_INDEX_SIZE_LEN_MAX - sizeof(log_index_t)];
}upload_log_index_t;
#pragma pack()

/* 兼容新旧压缩类型 */
typedef enum _compress_type_def {
    BZIP_TYPE = 0X00,
    P7ZIP_TYPE,
    ZIP_TYPE_MAX,
}compress_type_def;

/* 日志存储位置 临时文件目录*/
#define LOG_DIR            LOG_DATA_DIR"/temp"              /* 这里考虑实际的存放目录 */
#define LOG_TEMP_FILE      LOG_DIR"/temp.log"
#define LOG_TEMP_COPY_FILE LOG_DIR"/temp_copy.log"

/* 日志压缩格式 */
#define COMPRESS_FORMAT_BZ2 ".bz2"
#define COMPRESS_FORMAT_7Z  ".7z"

/* 日志存储位置 临时文件目录*/
#define LOG_FORMAL_DIR    LOG_DATA_DIR"/formal"

#define MIN_SPACE_SIZE      (165* 1024 * 1024)       /* 最小文件存储空间，除了日志外 */
#define LOG_MIN_SPACE_SIZE  (140 * 1024 * 1024)      /* 最小日志存储空间 */
#define UPG_MIN_SAPCE_SIZE  (50 * 1024 * 1024)       /* 最小固件升级存储空间剩余 */
FREQ_PUBLIC int do_file_save_check(int size);
#define DO_FILE_SAVE_CHECK()     do_file_save_check(MIN_SPACE_SIZE)
#define DO_LOG_FILE_SAVE_CHECK() do_file_save_check(LOG_MIN_SPACE_SIZE)
#define DO_FIRMWARE_UPG_CHECK()  do_file_save_check(UPG_MIN_SAPCE_SIZE)

FREQ_PUBLIC log_info_t *log_mgr_get(void);

FREQ_PUBLIC void dt_save_log(void *buf, unsigned int len);
FREQ_PUBLIC void log_save_init(void);
FREQ_PUBLIC void dt_log_upload_init(void);
#ifdef __cplusplus
}
#endif
#endif /* __LOG_UPLOAD_H */
