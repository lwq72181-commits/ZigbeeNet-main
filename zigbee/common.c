#include "common.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include "log_upload.h"
#include "zigbeetype.h"
#include "vpopen.h"
#include <linux/rtc.h>
#include <stdbool.h>
#include "proto_handle.h"
/* 存放一些公共的函数 */

/**
 * @brief 用于将线程绑定到指定的核心
 *
 * @param thread    (pthread_t id)  线程id
 * @param core      (0/1)           指定的核心
 * @return 成功DT_EOK/失败DT_ERROR
 */
int bind_thread_to_core(pthread_t thread, int core)
{
//    cpu_set_t cpuset;
//    CPU_ZERO(&cpuset);
//    CPU_SET(core, &cpuset);

//    if (0 != sched_setaffinity(0, sizeof(cpuset), &cpuset)) {
//        return DT_ERROR;
//    }

    return DT_EOK;
}

/**
 * @brief  用于创建一个 epoll 实例
 * @note
 * @param  size: 整数参数 size，表示 epoll 实例的大小
 * @retval 函数返回一个整数，表示 epoll 实例的文件描述符
 */
int dt_epoll_create(int size)
{
    int epoll_fd = -1;

    epoll_fd  = epoll_create(size);
    if (epoll_fd < 0)
        DT_ERR("epoll create fail!\r\n");
    return epoll_fd;
}

/**
 * @brief  用于向 epoll 实例中添加一个文件描述符，并指定需要监听的事件类型
 * @note
 * @param  epoll_fd:   epoll 实例的文件描述符
 * @param  target_fd: 需要添加的文件描述符
 * @param  events:    需要监听的事件类型
 * @retval 返回 epoll_ctl 函数的返回值，表示添加操作的结果。
 */
int dt_epoll_add(int epoll_fd, int target_fd, unsigned int events)
{
    struct epoll_event ev;

    ev.data.fd = target_fd;
    ev.events = events;
    int n = -1;
    n = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_fd, &ev);/*  */
    return n;
}

/**
 * @brief 时间正确性检查
 * @param 时间数组指针 时间格式char time[7];
 *
 * @return 正确DT_EOK/失败DT_ERROR
 */
int current_time_check(char *t)
{
    /* check year */
    if (20 != t[0] || (t[1] < 23 || t[1] > 99)) {
        DT_ERR("year err.\r\n");
        return DT_ERROR;
    }

    /* check month */
    if (t[2] < 1 || t[2] > 12) {
        DT_ERR("month err.\r\n");
        return DT_ERROR;
    }

    /* check day */
    if (t[3] < 1 || t[3] > 31) {
        DT_ERR("day err.\r\n");
        return DT_ERROR;
    }

    /* check hour */
    if (t[4] < 0 || t[4] > 23) {
        DT_ERR("hour err.\r\n");
        return DT_ERROR;
    }

    /* check minute */
    if (t[5] < 0 || t[5] > 59) {
        DT_ERR("minute err.\r\n");
        return DT_ERROR;
    }

    /* check second */
    if (t[6] < 0 || t[6] > 59) {
        DT_ERR("second err.\r\n");
        return DT_ERROR;
    }

    return DT_EOK;
}

/**
 * @brief 时间数组获取接口
 * @param 时间数组指针 时间格式char time[7];
 */
void dt_current_time_get(char *t)
{
    time_t sys_time;
    struct tm *trans_time = NULL;
    int year  = 0;
    int yearh = 0;
    int yearl = 0;

    time(&(sys_time));
    trans_time = localtime(&sys_time);
    year  = 1900 + trans_time->tm_year;
    yearh = (year-year % 100) / 100;
    yearl = year % 100;

    t[0] = yearh;
    t[1] = yearl;
    t[2] = 1 + (trans_time->tm_mon);
    t[3] = trans_time->tm_mday;
    t[4] = trans_time->tm_hour;
    t[5] = trans_time->tm_min;
    t[6] = trans_time->tm_sec;
}

/**
 * @brief 时间字符串获取接口
 * @param "%d-%02d-%02d %02d:%02d:%02d"该格式类型的时间字符串
 */
void dt_get_sys_time(char *t)
{
		time_t timep;
		struct tm *p;
		time(&timep);
		/* 取得当地时间 */
		p = localtime(&timep);
		sprintf(t, "%d-%02d-%02d %02d:%02d:%02d",
			1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
			p->tm_hour, p->tm_min, p->tm_sec);
}

/**
 * @brief 同步系统时间到RTC
 *        实现方式: 1、先获取系统时间，并打开RTC设备
 *                 2、将系统时间转换为RTC时间格式
 *                 3、设置RTC时间
 *
 * @return DT_EOK/DT_ERROR
 */
FREQ_PUBLIC int dt_sys_sync_to_rtc(void)
{
#define SYS_RTC_DEVICE  "/dev/rtc0"
    struct rtc_time rtc_tm;
    struct tm* cur_time = NULL;
    int rtc_fd = -1;
    int ret = DT_ERROR;

    /* 获取系统时间 */
    time_t sys_time = time(NULL);
    if (sys_time == (time_t) - 1) {
        DT_ERR("Failed to get current time.\r\n");
        goto _exit;
    }
    cur_time = localtime(&sys_time);

    /* 打开rtc设备 */
    rtc_fd = open(SYS_RTC_DEVICE, O_WRONLY);
    if (rtc_fd < 0) {
        DT_ERR("Failed to open RTC device.\r\n");
        goto _exit;
    }
    rtc_tm.tm_sec   = cur_time->tm_sec;
    rtc_tm.tm_min   = cur_time->tm_min;
    rtc_tm.tm_hour  = cur_time->tm_hour;
    rtc_tm.tm_mday  = cur_time->tm_mday;
    rtc_tm.tm_mon   = cur_time->tm_mon;
    rtc_tm.tm_year  = cur_time->tm_year;
    rtc_tm.tm_wday  = cur_time->tm_wday;
    rtc_tm.tm_yday  = cur_time->tm_yday;
    rtc_tm.tm_isdst = cur_time->tm_isdst;

    /* 设置rtc时间 */
    if (-1 == ioctl(rtc_fd, RTC_SET_TIME, &rtc_tm)) {
        DT_ERR("Failed to set RTC time.\r\n");
        goto _exit;
    }

    ret = DT_EOK;
_exit:
    if (rtc_fd > 0) {
        close(rtc_fd);
        rtc_fd = -1;
    }
    DT_LOG("sys_sync_to_rtc %s.\r\n", (DT_EOK == ret) ? "ok" : "fail");

    return ret;
}

/**
 * @brief 同步RTC时间到系统
 *        实现方式: 1、先打开RTC设备读取时间，
 *                 2、将RTC时间转换为系统时间格式
 *                 3、设置系统时间
 *
 * @return DT_EOK/DT_ERROR
 */
FREQ_PUBLIC int dt_rtc_sync_to_sys(void)
{
    int rtc_fd = -1;
    int ret = DT_ERROR;

    /* 打开系统rtc设备 */
    rtc_fd = open(SYS_RTC_DEVICE, O_RDONLY);
    if (rtc_fd < 0) {
        DT_ERR("Failed to open RTC device.\r\n");
        goto _exit;
    }

    /* 读取RTC时间 */
    struct rtc_time rtc_tm;
    if (-1 == ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm)) {
        DT_ERR("Failed to read RTC time.\r\n");
        goto _exit;
    }

    /* 将RTC时间转换为struct tm结构 */
    struct tm sys_tm;
    sys_tm.tm_sec   = rtc_tm.tm_sec;
    sys_tm.tm_min   = rtc_tm.tm_min;
    sys_tm.tm_hour  = rtc_tm.tm_hour;
    sys_tm.tm_mday  = rtc_tm.tm_mday;
    sys_tm.tm_mon   = rtc_tm.tm_mon;
    sys_tm.tm_year  = rtc_tm.tm_year;
    sys_tm.tm_wday  = rtc_tm.tm_wday;
    sys_tm.tm_yday  = rtc_tm.tm_yday;
    sys_tm.tm_isdst = rtc_tm.tm_isdst;

    /* 将系统时间转换为timeval结构 */
    struct timeval tv;
    tv.tv_sec = mktime(&sys_tm);
    tv.tv_usec = 0;

    /* 设置系统时间 */
    if (-1 == settimeofday(&tv, NULL)) {
        DT_ERR("Failed to set system time.\r\n");
        goto _exit;
    }


    ret = DT_EOK;
_exit:
    if (rtc_fd > 0) {
        close(rtc_fd);
        rtc_fd = -1;
    }
    DT_LOG("rtc_sync_to_sys %s.\r\n", (DT_EOK == ret) ? "ok" : "fail");

    return ret;
}

/**
 * @brief 设置系统软件时间
 *
 * @param 时间数组
 */
FREQ_PUBLIC void dt_set_sys_time(int *time)
{
#define DATE_CHECK_YEAR    (2023)
    char cmd[256] = {0};
    struct tm sys_tm;
    struct timeval tv;

    if (time[0] < DATE_CHECK_YEAR) {
        DT_ERR("SysTime param err. year(%d).\r\n", time[0]);
        return;
    }

    /* 将网络时间转换为struct tm结构 */
    sys_tm.tm_sec   = time[5];
    sys_tm.tm_min   = time[4];
    sys_tm.tm_hour  = time[3];
    sys_tm.tm_mday  = time[2];
    sys_tm.tm_mon   = time[1] - 1;  /* 月份从0开始计算 */
    sys_tm.tm_year  = time[0] - 1900; /* 年份从1900年开始计算 */
    sys_tm.tm_wday = 0;     /* 不使用此字段 */
    sys_tm.tm_yday = 0;     /* 不使用此字段 */
    sys_tm.tm_isdst = -1;   /* 自动检测夏令时状态 */

    /* 将系统时间转换为timeval结构 */
    tv.tv_sec = mktime(&sys_tm);
    tv.tv_usec = 0;

    /* 设置系统时间 */
    if (-1 == settimeofday(&tv, NULL)) {
        DT_ERR("Failed to set system time.\r\n");
    }

    /* 同步系统时间到RTC */
    dt_sys_sync_to_rtc();
}

void dt_log(const char *opt, const char *func, int line, const char *fmt, ...)
{
#define PRINTF_MAX_LEN  (4*1024)   /* 最大支持打印长度，以防特殊情况导致地址越界 */
		char msg_buf[PRINTF_MAX_LEN] = {0};
		char t[32] = {0};
		va_list ap;
		va_start(ap, fmt);
		dt_get_sys_time(t);
		int required_len = vsnprintf(NULL, 0, fmt, ap) + strlen(opt) + strlen(t) + strlen(func) + 128;
		/* 判断是否超出最大长度 */
		if (required_len < sizeof(msg_buf)) {
			snprintf(msg_buf, sizeof(msg_buf), "<%s><%s><%s %d> ",opt, t, func, line);
			vsprintf(msg_buf + strlen(msg_buf), fmt, ap);
		} else {
			snprintf(msg_buf, sizeof(msg_buf), "<%s><%s><%s %d> [Output data exceeds the max size, cur_len(%d)]\r\n", opt, t, func, line, required_len);
		}
		/* 输出到标准输出, 后面扩展一下输出到文件保存下来 */
		fprintf(stderr,"%s",msg_buf);
		va_end(ap);
		if (0 != strncmp(opt, "PRI", 3))
			dt_save_log((char*)msg_buf, strlen(msg_buf));/* 保存为日志文件 */
}

/* 系统命令 */
int dt_system(const char *cmd_line)
{
		int ret = -1;
		__sighandler_t old_handler;

		old_handler = signal(SIGCHLD, SIG_DFL);
		ret = system(cmd_line);
		signal(SIGCHLD, old_handler);

//		DT_DBG("cmd: %s\r\n", cmd_line);
		if (ret != -1 && WIFEXITED(ret) && !WEXITSTATUS(ret))
			ret = 0;
		else {
			DT_PRINTF("cmd: %s, ret: %d, err: %d(%s)\r\n",
				cmd_line, ret, errno, strerror(errno));
			ret = -1;
		}

		return ret;
}

/* 文件读写相关 */
int dt_write(int fd, const void *buf, unsigned int len)
{
    int n = -1;
    if (fd < 0 || !buf || !len) {
    	DT_ERR("invalid param.\r\n");
    	return -1;
    }

		n = write(fd, buf, len);
		if (n != len)
			DT_WARN("fd: %d, write %d, return %d.\r\n", fd, len, n);
    return n;
}
/**
 * @brief 写入检查函数
 *
 * @param fd 文件描述符
 * @param buf 缓冲区指针
 * @param len 数据长度
 * @return int 返回状态，如果成功写入返回 DT_EOK，否则返回 DT_ERROR
 */
int dt_write_check(int fd, const void *buf, unsigned int len)
{
    int n = -1;
    if (fd < 0 || !buf || !len) {
        DT_ERR("invalid param.\r\n");
        return DT_ERROR;
    }

    n = write(fd, buf, len);
    if (n != len) {
        DT_WARN("fd: %d, write %d, return %d.\r\n", fd, len, n);
        return DT_ERROR;
    }
    return DT_EOK;
}


int dt_read(int fd, void *buf, unsigned int len)
{
    int n = -1;
    if (fd < 0 || !buf || !len) {
        DT_ERR("invalid param.\r\n");
        return -1;
    }
    n = read(fd, buf, len);
    if (n != len)
        DT_WARN("fd: %d, read %d, return %d.\r\n", fd, len, n);
    return n;
}

/* 判断一个路径是否是目录 */
int testdir(const char *path)
{
    struct stat info;
    if (lstat(path,&info) < 0)
        return -1;

    if (S_ISDIR(info.st_mode))
        return 1; //directory
    return 0;
}

int dt_create_dir(const char *dirname)
{
#if 0
    int ret = -1;
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", dirname);
    ret = system(cmd);
    return ret;
#else
    int begincmp;
    int endcmp;
    char cur_path[256] = {0};

    //绝对路径
    int pathlen = strlen(dirname);
    //strncpy(cur_path, dirname, sizeof(cur_path));
    if (pathlen >= sizeof(cur_path))
        return -1;
    snprintf(cur_path, sizeof(cur_path), "%s", dirname);
    if (dirname[pathlen] != '/')
        strcat(cur_path, "/");

    begincmp = 1;
    endcmp = strlen(cur_path);

    //创建各级目录
    for (int i = begincmp; i < endcmp ; i++ ) {
        if ('/' == cur_path[i]) {
            cur_path[i] = '\0';
            if (access(cur_path, F_OK) != 0) {
                if (mkdir(cur_path, 0755) == -1) {
                    perror("mkdir error %s\n");
                    return -1;
                }
            }
            cur_path[i] = '/';
        }
    }
    return 0;

#endif
}

int dt_delete_dirfile(const char *filename)
{
#if 0
    int ret = -1;
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "rm -rf %s", filename);
    ret = system(cmd);
    return ret;
#else
    /* 如果不是目录，直接删除 */
    if (1 != testdir(filename)) {
        unlink(filename);
        return 0;
    }
    /* 如果是目录，需要循环删除 */
    char cur_path[256] = {0};
    int pathlen = strlen(filename);
    if (pathlen >= sizeof(cur_path))
        return -1;
    snprintf(cur_path, sizeof(cur_path), "%s", filename);
    if (filename[pathlen] != '/')
        strcat(cur_path, "/");

    struct dirent *ptr = NULL;
    DIR *dir = opendir(cur_path);
    if (dir == NULL){
        return -1;
    }

    while ((ptr = readdir(dir)) != NULL) {
        char subpath[512] = {0};
        if ((strcmp(ptr->d_name,".")==0) || (strcmp(ptr->d_name,"..")==0))
            continue;
        snprintf(subpath, sizeof(subpath), "%s%s", cur_path, ptr->d_name);
        if (1 == testdir(filename)) {
            dt_delete_dirfile(subpath);
            rmdir(subpath);
        } else {
            unlink(subpath);
        }
    }
    closedir(dir);
    dir = NULL;

    return 0;
#endif
}

int dt_get_file_size(const char *file)
{
    int length = -1;
    struct stat statbuff;
    if(stat(file, &statbuff) < 0){
        return length;
    }else{
        length = statbuff.st_size;
    }
    return length;
}

/**
 * @brief 万能数据打印接口
 * @param data 打印的数据
 * @param len 数据长度
 */
void uint8_t_data_printf(unsigned char *data, int len)
{
    static int print_cnt = 0;
    printf("uint8_t print_packet[%d]= {\r\n", len);
    for(int i=0; i<len; i++) {
        printf("0x%x,",data[i]);
        if((print_cnt+1) % 16 == 0) {
            printf("\r\n");
        }
        print_cnt++;
    }
    print_cnt = 0;
    printf("\r\n};\r\n");
}

/**
 * @brief 时间获取接口
 * @return 当前时间，单位：s
 */
unsigned int dt_second_tick_get(void)
{
    time_t sys_time = 0;
    sys_time = time(NULL);
    return sys_time;
}

/**
 * @brief  计算数据的校验和
 * @note
 * @param  *pdata: 指向数据的指针
 * @param  len:    数据的长度
 * @retval 校验和返回一个无符号字符，
 */
unsigned char dt_compute_cs(unsigned char *pdata, unsigned int len)
{
    unsigned int i = 0;
    unsigned char check_sum = 0;

    for (i = 0; i < len; i++) {
        check_sum += pdata[i];
    }
    check_sum = 0x100 - check_sum;
    return check_sum;
}

/**
 * @brief  计算一个buffer累加和
 * @param  *pdata: 指向数据的指针
 * @param  len:    数据的长度
 * @retval 返回和值
 */
unsigned char dt_cusum(unsigned char *pdata, unsigned long len)
{
    unsigned long i = 0;
    unsigned char check_sum = 0;

    for (int i = 0; i<len; i++) {
        check_sum += pdata[i];
    }

    return check_sum;
}

/**
 * @brief  数据大小端转换
 * @param  *pdata: 指向数据的指针
 * @param  len:    数据的长度
 */
FREQ_PUBLIC void s16_swap(short *pdata, unsigned int len)
{
    for (int i = 0; i < len; i++)
        pdata[i] = ((pdata[i] << 8) & 0xFF00) | ((pdata[i] >> 8) & 0xFF);
}

void short_to_float_t_data_printf(short *data, int len, float coef)
{
    static int print_cnt = 0;
    printf("float print_packet[%d]= {\r\n", len);
    for(int i=0; i<len; i++) {
        printf("%5.3f,",data[i]*coef);
        if((print_cnt+1) % 16 == 0) {
            printf("\r\n");
        }
        print_cnt++;
    }
    print_cnt = 0;
    printf("\r\n};\r\n");
}
void int_to_float_t_data_printf(int *data, int len, float coef)
{
    static int print_cnt = 0;
    printf("float print_packet[%d]= {\r\n", len);
    for(int i=0; i<len; i++) {
        printf("%5.3f,",data[i]*coef);
        if((print_cnt+1) % 16 == 0) {
            printf("\r\n");
        }
        print_cnt++;
    }
    print_cnt = 0;
    printf("\r\n};\r\n");
}
void float_to_float_t_data_printf(float *data, int len, float coef)
{
    static int print_cnt = 0;
    printf("float print_packet[%d]= {\r\n", len);
    for(int i=0; i<len; i++) {
        printf("%5.3f,",data[i]*coef);
        if((print_cnt+1) % 16 == 0) {
            printf("\r\n");
        }
        print_cnt++;
    }
    print_cnt = 0;
    printf("\r\n};\r\n");
}


int toInt(char *str)
{
	return atoi(str);
}

int toString(int x, char *buf, int len)
{
	memset(buf, 0, len);
	sprintf(buf, "%d", x);
	return 0;
}

int toIp(uint8_t *ip, char *buf, int len)
{
	memset(buf, 0, len);
	snprintf(buf, len, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return 0;
}

int fromIp(uint8_t *ip, char *buf)
{
	int d[4];
	int i;
	sscanf(buf, "%d.%d.%d.%d", &d[0], &d[1], &d[2], &d[3]);
	for (i = 0; i < 4; i++)
		ip[i] = d[i];
	return 0;
}

int toMac(uint8_t *mac, char *buf, int len)
{
	memset(buf, 0, len);
	sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return 0;
}

int fromMac(uint8_t *mac, char *buf)
{
	int m[6];
	int i;
	sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]);

	for (i = 0; i < 6; i++)
		mac[i] = m[i];
	return 0;
}

/**
 * @brief 调用系统命令，匹配parttern里的字符串，
 * @param parttern,想要匹配的字符
 * @return 成功返回DT_EOK
 */
FREQ_PUBLIC int dt_system_cmd_match(const char *cmd, const char *parttern)
{
    FILE* fp = NULL;
    char line[1024] = {0};
    char* match = NULL;
    int result  = DT_ERROR;

    fp = popen(cmd, "r");
    if (fp == NULL) {
        DT_ERR("popen failed.\r\n");
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        /* 按行查找 */
        match = strstr(line, parttern);
        if (match != NULL) {
            result = DT_EOK;
            goto out;
        }
    }

out:
    // 关闭文件指针
    if (fp) {
        pclose(fp);
        fp = NULL;
    }

    return result;
}

/**
 * @brief 波形压缩接口
 *
 * @return 压缩失败-1/成功返回0
 */
pthread_mutex_t compress_mutex = PTHREAD_MUTEX_INITIALIZER;
FREQ_PUBLIC int bzip2_compress(const char*filename)
{
    pthread_mutex_lock(&compress_mutex);
    int ret = -1;
    char cmd[1024] = {0};
    snprintf(cmd, sizeof(cmd), "bzip2 -9 %s", filename);
    ret = dt_system(cmd);
    pthread_mutex_unlock(&compress_mutex);
    return ret;
}

/**
 * @brief 磁盘剩余空间查看
 *
 * @param path 需要查看的分区名
 * @param size 返回的空间大小存放地址
 * @return FREQ_PUBLIC
 */
FREQ_PUBLIC int dt_df_check(const char *path, long long *size)
{
    struct statfs diskInfo;

    statfs(path ? path : NULL, &diskInfo);

    if (FILE_ONLY_READ == diskInfo.f_flags)
        return DT_ERROR;

#if 1
    *size = ((long long)diskInfo.f_bsize) * ((long long)diskInfo.f_bfree);
#else       /* for test */
    long long blocksize;
    long long totalsize;
    long long freesize;
    long long availablesize;
    blocksize = diskInfo.f_bsize;                                   //每个block包含的字节数
    totalsize = blocksize * diskInfo.f_blocks;                      //总字节数
    freesize  = blocksize * diskInfo.f_bfree;                       //剩余空间
    availablesize = diskInfo.f_bavail * blocksize;                  //可用空间
    *size = freesize;
    DT_LOG("<path:%s> ,totalsize = %lld MB ,freesize = %lld MB, availablesize = %lld MB\n\r", path, totalsize/1024/1024, freesize/1024/1024, availablesize/1021/1024);
#endif

    return DT_EOK;
}

/* 获取一个shell命令的返回结果 */
int get_cmd_result(const char *cmd,  char *results)
{
    FILE *fp = NULL;
    int flag = false;
    char buf[16384] = {0};

    if (0 != strlen(results)) {
        memset(results, 0, strlen(results));
    }

    fp = vpopen(cmd, "r");
    if(NULL == fp) {
       printf("popen %s error!\n", cmd);
       return false;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        int len = strlen(buf);
        len = (len > 16383 ? 163843 : len);
        strcpy(results, strcat(results, buf));
        memset(buf, 0, sizeof(buf));
        flag = true;
    }
    vpclose(fp);
    fp = NULL;
    return flag;
}


/**
 * @brief 7z加密压缩
 *
 * @return 压缩失败-1/成功返回0
 */
pthread_mutex_t encry_mutex = PTHREAD_MUTEX_INITIALIZER;
FREQ_PUBLIC int encryption_compress(const char *filename)
{
#define COMPRESS_PASSWORD   "dotonlink"

    int ret = DT_ERROR;
    pthread_mutex_lock(&encry_mutex);
    char cmd[1024]    = {0};
    char result[1024] = {0};
    if (0 != access(COMPRESS_TOOL_PATH, F_OK)) {
        goto _exit;
    }

    snprintf(cmd, sizeof(cmd), "7za a -p%s %s.7z %s", COMPRESS_PASSWORD, filename, filename);
    get_cmd_result(cmd, result);
    if (strstr(result, "Everything is Ok")) {
        ret = DT_EOK;
    }

_exit:
    pthread_mutex_unlock(&encry_mutex);
    return ret;
}

FREQ_PUBLIC int dt_param_content_sync(const char *src, const char *dst)
{
    int fd_src   = -1;
    int fd_dst   = -1;
    int src_size = 0;
    int dst_size = 0;
    int iret     = DT_PARAM_GENERIC_FAILURE;
    char *src_buf = NULL;
    char *dst_buf = NULL;
    int n;
    dt_file_head_t *head = NULL;

    DT_LOG("%s -> %s \r\n", src, dst);
    /* 先校验源文件的正确性 */
    fd_src = open(src, O_RDONLY);
    if (fd_src < 0) {
        DT_ERR("fatal error, can't open src: %s \r\n", src);
        goto err;
    }
    src_size = dt_get_file_size(src);
    src_buf  = dt_malloc(src_size);
    if (!src_buf) {
        DT_ERR("failed to malloc for src buf. \r\n");
        goto err;
    }
    n = dt_read(fd_src, src_buf, src_size);
    if (n != src_size) {
        DT_ERR("failed to read src, expect %d but got %d.\r\n", src_size, n);
        goto err;
    }
    head = (dt_file_head_t *)src_buf;
    if (src_size != (sizeof(dt_file_head_t) + head->len)) {
        DT_ERR("size mismatch, file size: %d, %d.\r\n",
            src_size, head->len + sizeof(dt_file_head_t));
        goto err;
    }
    if (head->crc != dt_crc32(src_buf + sizeof(dt_file_head_t), head->len)) {
        DT_ERR("src crc mismatch.\r\n");
        goto err;
    }
    /* 源文件一切校验成功 */
    /* 再校验目的文件 */
    fd_dst = open(dst, O_RDONLY);
    if (fd_dst < 0) {
        DT_LOG("can't open dst, sync directly. \r\n", dst);
        /* 目的文件不存在? 直接同步过去 */
        goto sync;
    }
    dst_size = dt_get_file_size(dst);
    if (src_size != dst_size) {
        DT_LOG("size mismatch, src: %d, dst: %d, sync directly.\r\n", src_size, dst_size);
        /* 大小都不一样, 直接同步过去 */
        if (fd_dst) {
            close(fd_dst);
            fd_dst = -1;
        }
        goto sync;
    }
    dst_buf = dt_malloc(dst_size);
    if (!dst_buf) {
        DT_ERR("failed to malloc for dst buf. \r\n");
        goto err;
    }
    n = dt_read(fd_dst, dst_buf, dst_size);
    if (n != dst_size) {
        DT_ERR("failed to read dst, expect %d but got %d.\r\n", dst_size, n);
        goto err;
    }
    if (0 == memcmp(src_buf, dst_buf, src_size)) {
        DT_LOG("no need to sync.\r\n");
        iret = DT_PARAM_SUCCESS;
        goto err;
    }
    DT_LOG("need to sync.\r\n");
sync:
    if (fd_dst >= 0) {
        close(fd_dst);
        fd_dst = -1;
    }
    fd_dst = open(dst, O_WRONLY|O_CREAT|O_TRUNC , S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd_dst < 0) {
        DT_ERR("failed to create dst %s\r\n", dst);
        goto err;
    }
    n = dt_write(fd_dst, src_buf, src_size);
    if (n != src_size) {
        DT_ERR("failed to write dst, expect %d but got %d.\r\n", src_size, n);
        goto err;
    }
    /* 到此可以认为src和dst已经同步, 是否还需要把dst再读出来一次? */
    iret = DT_PARAM_SUCCESS;
err:
    if (src_buf) {
        dt_free(src_buf);
        src_buf = NULL;
    }
    if (dst_buf) {
        dt_free(dst_buf);
        dst_buf = NULL;
    }
    if (fd_src >= 0) {
        close(fd_src);
        fd_src = -1;
    }
    if (fd_dst >= 0) {
        close(fd_dst);
        fd_dst = -1;
    }
    DT_LOG("%s -> %s, %s.\r\n", src, dst, DT_PARAM_SUCCESS == iret ? "OK" : "Failed");
    return iret;
}

/**
 * @brief 根据文件夹列表，依次创建文件
 *
 */
void all_dir_init(void)
{
    char *dir_list[] = {
        CONFIG_DIR,
        BACK_CONFIG_DIR,
        STA_OFFLINE_DIR,
        LOG_DIR,
        LOG_FORMAL_DIR,
        FW_FPGA_DIR,/* 升级区目录 */
        FW_TECH_DIR,
        FW_MODBUS_DIR,
        FW_APP_DIR,
        FW_AD_DIR,
        FW_OTA_DIR,   /* 创建OTA临时数据区 */
        FW_SENSOR_CFG_DIR,
        FW_WLESS_DIR,
        TEMP_DATA_DIR,/* 创建临时数据区域 */
        RPM_DATA_DIR,/* 创建转速文件目录 */
        RPM_TEMP_DIR,
        WAVE_DATA_DIR,/* 创建波形文件目录 */
        ALARM_WAVE_DATA_DIR,/* 创建报警波形文件目录 */
        WAVE_TEMP_DIR,
        EIGEN_DATA_DIR,/* 创建指标文件目录 */
        EIGEN_TEMP_DIR,
        RTU_DATA_DIR,/* 创建RTU文件目录 */
        RTU_TEMP_DIR,
        //TEMPERATURE_DATA_DIR,
        //TEMPERATURE_TEMP_DIR,
        //TECH_DATA_DIR,
        //TECH_TEMP_DIR,
        TAG_DATA_DIR,
        TAG_TEMP_DIR,
        //AUDIO_DATA_DIR,
        //AUDIO_TEMP_DIR,
    };

    for (int i = 0; i < ArrayLength(dir_list); i++) {
        if(dir_list[i]) {
            // DT_DBG("dir_list[%d]:%s\r\n", i, dir_list[i]);
            dt_create_dir(dir_list[i]); /* 该接口会判断文件夹是否存在，不存在时，则创建该目录 */
        }
    };
}


/**
 * @brief 文件夹初始化
 * @param void
 * @return int
 */
FREQ_PUBLIC void dt_file_dir_init(void)
{
    sys_status_set(SYS_RUN);/* 设置系统为运行态 */
    log_save_init();        /*  日志存储初始化 */

    /* step1.如果存在临时文件夹，需要删除临时文件 */
//    if (0 == access(TEMP_DATA_DIR, F_OK)) {
//        //clean_temp_data();
//    }

//    /*如果标记存在，则删除历史文件*/
//    if (0 == access(DELETE_FILE, F_OK)) {
//        dt_delete_dirfile(DELETE_FILE);
//        offline_data_dele();
//    }

    /* 所需文件夹初始化 */
    all_dir_init();

//    /* 获取设备上电类型 */
//    sys_reset_type_get();
//    user_data_state_check();

//    /* 系统信息检查，NAND品牌，固件版本 */
//    sys_firmware_verison_check();

//    /* 打开eth0网口, 用于是否dbg检查 */
//    dbg_eth0_enable();

//#ifdef DT_OPEN_CHECK_ENABLE
//        open_list_init_info();/* open操作链表初始化 */
//#endif
}
