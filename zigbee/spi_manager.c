#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/time.h>
#include <string.h>

#include "spi_manager.h"
#include "common.h"
//#include "dt_mqtt.h"
#include "proto_common.h"
//#include "dt_upload_manager.h"
//#include "dt_mqtt_protocol.h"
#include "dt_malloc_list.h"
#include "proto_handle.h"
#include "wrapper.h"
extern unsigned char global_hw_check_result;
extern int g_switch_flag;
pthread_mutex_t storage_mutex = PTHREAD_MUTEX_INITIALIZER;
//extern ZIGBEEParam *g_ZIGBEEParam;
//#define SPI_REBOOT_ON_ERR
#ifdef SPI_REBOOT_ON_ERR
#define SPI_ERR_FILE	"/usrdata/spi_err"
static volatile int g_spi_err = false;
static int spi_need_reboot()
{
    int fd;
    int count = 0;

	if (g_spi_err) {
		DT_WARN("");
		return true;
	}

    fd = open(SPI_ERR_FILE, O_RDWR);
    if (fd < 0) {
        fd = open(SPI_ERR_FILE, O_RDWR | O_CREAT);
		if (fd < 0) {
			DT_WLS_ERR("failed to create %s.", SPI_ERR_FILE);
			return false;
		}
		goto out;
    }

    read(fd, &count, sizeof(int));
out:
    count++;
    lseek(fd, 0, SEEK_SET);
    write(fd, &count, sizeof(int));
	fsync(fd);
    close(fd);
	DT_WLS_LOG("count: %d", count);

	if (count > 3) {
		g_spi_err = true;
		return true;
	} else {
		return false;
	}
}
#endif

#define SPI_SEEK(fd, offset, whence) \
do{\
	if (lseek(fd, offset, whence) == -1) {\
		DT_WLS_ERR("offset: %d, whence: %d, errno: %d: %s", offset, whence, errno, strerror(errno));\
		/*if (spi_need_reboot()) reboot_notify_mcu();*/\
	}\
} while(0)

#define EC20_SPI_FLASH	"/dev/mtdblock20"
#define EG25_SPI_FLASH	"/dev/mtdblock18"

#define SPI_FLASH	"/dev/mtd0"

static const char *device = NULL;

//#define SPI_NOR_FLASH_MODULE "/lib/modules/`uname -r`/kernel/drivers/mtd/devices/m25p80.ko"
#define SPI_NOR_FLASH_MODULE "/oem/dma/qt/ko/m25p80.ko"

#define MAX_SENSOR_NUM   32

static int spi_fd = -1;

const uint8_t longwave_index_header[4] = {0x68, 0x01, 0x55, 0x68};
const uint8_t wave_index_header[4] = {0x68, 0x01, 0x57, 0x68};
const uint8_t wave_index_tailer[6] = {0xFE, 0xEF, 0xFD, 0xDF, 0xFC, 0xCF};
const uint8_t battery_index_header[4] = {0x68, 0x01, 0x6A, 0x68};

static unsigned char *pglobal_wave_data_buffer = NULL;
unsigned char *get_wave_data_buffer(void)
{
    if (NULL == pglobal_wave_data_buffer) {
        pglobal_wave_data_buffer = (unsigned char *)malloc(4*1024*1024);
    }
    return pglobal_wave_data_buffer;
}


#define START_YEAR 12  //起始年开始时间2012
//year%400=0为闰年，year%100!=0且year%4=0为闰年
uint16_t YEAR_TABLE[] = {366*1+365*0,  //2008, 闰-366
                         366*1+365*1,  //2009, 平-365
                         366*1+365*2,  //2010, 平-365
                         366*1+365*3,  //2011, 平-365
             366*2+365*3,  //2012, 闰-366
                         366*2+365*4,  //2013, 平-365
                         366*2+365*5,  //2014, 平-365
                         366*2+365*6,  //2015, 平-365
             366*3+365*6,  //2016, 闰-366
                         366*3+365*7,  //2017, 平-365
                         366*3+365*8,  //2018, 平-365
                         366*3+365*9,  //2019, 平-365
                         366*4+365*9,  //2020, 闰-366
                         366*4+365*10, //2021, 平-365
                         366*4+365*11, //2022, 平-365
                         366*4+365*12, //2023, 平-365
             366*5+365*12, //2024, 闰-366
                         366*5+365*13, //2025, 平-365
                         366*5+365*14, //2026, 平-365
                         366*5+365*15, //2027, 平-365
             366*6+365*15, //2028, 闰-365
                         366*6+365*16, //2029, 平-365
                         366*6+365*17, //2030, 平-365
                         366*6+365*18};//2031, 平-365

//平年每个月多少天
uint16_t DAY_TABLE1[] = {31,
                         31+28,
             31+28+31,
             31+28+31+30,
             31+28+31+30+31,
             31+28+31+30+31+30,
             31+28+31+30+31+30+31,
             31+28+31+30+31+30+31+31,
             31+28+31+30+31+30+31+31+30,
             31+28+31+30+31+30+31+31+30+31,
             31+28+31+30+31+30+31+31+30+31+30,
             31+28+31+30+31+30+31+31+30+31+30+31};

//闰年每个月多少天
uint16_t DAY_TABLE2[] = {31,
                         31+29,
                         31+29+31,
                         31+29+31+30,
                         31+29+31+30+31,
                         31+29+31+30+31+30,
                         31+29+31+30+31+30+31,
                         31+29+31+30+31+30+31+31,
                         31+29+31+30+31+30+31+31+30,
                         31+29+31+30+31+30+31+31+30+31,
                         31+29+31+30+31+30+31+31+30+31+30,
                         31+29+31+30+31+30+31+31+30+31+30+31};

void rtc_seconds_to_time(uint32_t seconds, unsigned char *convertime)
{
    uint8_t i;
    uint8_t YEAR;
    uint8_t MONTH;
    uint8_t DAY;
    uint8_t HOUR;
    uint8_t MINUTE;
    uint8_t SECOND;

    SECOND = seconds%60;
    seconds/=60;
    MINUTE = seconds%60;
    seconds/=60;
    HOUR   = seconds%24;
    seconds/=24;

    for(i=0;i<20;i++)
    {
        if(seconds < YEAR_TABLE[i])
        {
            break;
        }
    }
    YEAR = START_YEAR+i;
    if(YEAR > START_YEAR)
    {
        seconds = seconds-YEAR_TABLE[i-1];
    }

    if(YEAR/4*4 == YEAR)
    {
        for(i=0;i<12;i++)
        {
            if(seconds < DAY_TABLE2[i])
            {
                break;
            }
        }
        MONTH = i+1;
        if(i != 0)
        {
            DAY = seconds-DAY_TABLE2[i-1]+1;
        }
        else
        {
            DAY = seconds+1;
        }
    }
    else
    {
        for(i=0;i<12;i++)
        {
            if(seconds < DAY_TABLE1[i])
            {
                break;
            }
        }
        MONTH = i+1;
        if(i != 0)
        {
            DAY = seconds-DAY_TABLE1[i-1]+1;
        }
        else
        {
            DAY = seconds+1;
        }
    }

    convertime[0] = 20;
    convertime[1] = YEAR;
    convertime[2] = MONTH;
    convertime[3] = DAY;
    convertime[4] = HOUR;
    convertime[5] = MINUTE;
    convertime[6] = SECOND;
}

int spi_init(void)
{
	// if (!device) {
	// 	if (get_platform() == PLATFORM_EG25)
	// 		device = EG25_SPI_FLASH;
	// 	else
	// 		device = EC20_SPI_FLASH;
	// }

    device = SPI_FLASH;
    if(access(device, F_OK) == 0)
    {
        return true;
    }
#if 0
    if(access(SPI_NOR_FLASH_MODULE, F_OK) != 0)
    {
        DT_WLS_LOG("<%s,%d>module %s not exist!", __FUNCTION__, __LINE__, SPI_NOR_FLASH_MODULE);
        return false;
    }
#endif
     char cmd[512] = {0};
     snprintf(cmd, sizeof(cmd), "rmmod %s;usleep 500000;insmod -f %s;usleep 500000;", SPI_NOR_FLASH_MODULE);
     dt_system(cmd);

    for(int i=0;i<10;i++)
    {
        if(access(device, F_OK) == 0)
        {
            return true;
        }

        usleep(500*1000);
    }

    DT_WLS_LOG("spi_init fail!\r\n");
    return false;

}

int spi_open(void)
{
    int fd = -1;

    if(spi_init() != true)
    {
        return false;
    }

    if(spi_fd > 0)
    {
        return true;
    }

    fd = open(device, O_RDONLY | O_SYNC);
    if (fd < 0) {
        DT_WLS_LOG("can't open spi device\r\n");
        return false;
    }

    spi_fd = fd;
    DT_WLS_LOG("SPI - Open Succeed. fd=%d!\r\n", spi_fd);

    return true;
}

void spi_close(void)
{
    if(spi_fd > 0)
    {
        close(spi_fd);
        spi_fd = -1;
    }
}

#if 0
int SaveDataIndex(char *code, wavedata_index_t data)
{
    int fd = -1;
    int flag;
    int ret;

    wavedata_index_t waveindex;

    char dir[256];
    memset(dir, 0, sizeof(dir));
    strcpy(dir, strcat(strcat(WLESS_DATA_DIR,code), WAVEDATA_FILE));
    char file[128];
    memset(file, 0, sizeof(file));

    strcpy(file, strcat(dir, DATA_INDEX));

    if(access(dir, F_OK) != 0)
    {
        CreateDir(dir);
    }

    fd = open(file, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        DT_WLS_ERR("Open file: %s failed...", file);
        return false;
    }

    while(read(fd, &waveindex, sizeof(waveindex)) > 0)
    {
        if(waveindex.ChannelNo == data.ChannelNo && waveindex.MeasPointID == data.MeasPointID
                && waveindex.MeasDefID == data.MeasDefID && waveindex.DataType == data.DataType
                && waveindex.SampleTime[0] == data.SampleTime[0] && waveindex.SampleTime[1] == data.SampleTime[1]
                && waveindex.SampleTime[2] == data.SampleTime[2] && waveindex.SampleTime[3] == data.SampleTime[3]
                && waveindex.SampleTime[4] == data.SampleTime[4] && waveindex.SampleTime[5] == data.SampleTime[5]
                && waveindex.SampleTime[6] == data.SampleTime[6])
        {
            DT_WLS_ERR("wave index has exists!\n");
            close(fd);
            return true;
        }
    }

    lseek(fd, 0, SEEK_END);
    ret = dt_write(fd, (char *)&data, sizeof(wavedata_index_t));
    if (ret != sizeof(wavedata_index_t))
    {
        flag = false;
        DT_WLS_ERR("Write dataindex error, ret=%d...", ret);
    }
    else
        flag = true;

    close(fd);
    return flag;
}

int save_battery_info_index(char *stationcode, battery_info_index_t *battery_index, int battery_type)
{
    int ret = false;
    int fd = -1;

    char battery_info_dir[256];
    memset(battery_info_dir, 0, sizeof(battery_info_dir));
    strcpy(battery_info_dir, strcat(strcat(WLESS_DATA_DIR,stationcode), BATTERY_INDEX_FILE));

    char now_time[7];
    const char now_string[128] = {"yyyyMMddhhmmss"};

    char battery_file[256];
    memset(battery_file, 0, sizeof(battery_file));
    if(battery_type == 1)
    {
        strcpy(battery_file,
        strcat(strcat(battery_info_dir, "vibsensor_battery_"), strcat(now_string, ".dat")));
    }
    else if(battery_type == 2)
    {
        strcpy(battery_file,
        strcat(strcat(battery_info_dir, "station_battery_"), strcat(now_string, ".dat")));
    }

    if(access(battery_info_dir, F_OK) != 0)
    {
        CreateDir(battery_info_dir);
    }

    if(battery_index == NULL)
    {
        return true;
    }

    fd = open(battery_file, O_RDWR|O_CREAT|O_TRUNC,  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(fd < 0)
    {
        DT_WLS_ERR("open sensor battery info file %s error\n", battery_file);
        goto end;
    }

    for(unsigned int i = 0;i<strlen(battery_index);i++)
    {
        if(dt_write(fd, &(battery_index[i]), sizeof(battery_info_index_t)) != sizeof(battery_info_index_t))
        {
            goto end;
        }
    }
    ret = true;
end:
    if(fd > 0)
    {
        close(fd);
        fd = -1;
    }
    if(ret == false)
    {
        deleteDir(battery_file);
    }

    return ret;
}
#endif

//int test_read_flash_data()
//{
//    int block[20]={};
//}

int spi_read_wave_data(int sector_start, int sector_end, int sector_size,uart_cmd_new_0x36_t new_info,\
                       char *spi_stationcode,unsigned int sensor_byte, unsigned char sensor_num, unsigned int hight_sensor_byte)
{
    int wavfilesize = new_info.WaveFileSize;
    unsigned char DataType = new_info.DataType;
    int ch_num = new_info.ChannelNo;
    int start_pos=0, end_pos=0, cur_pos=0;
    int sector_per_file = (wavfilesize+sector_size-1)/sector_size;
    int count = (sector_end-sector_start+1)/sector_per_file;
    int ret =0;
    int size = 0;
    char buf[10] = {0};
    DT_WLS_LOG("spi read wave start: %d, end: %d, sector_size: %d, sector_per_file:%d,wavfilesize:%d,count:%d\r\n", sector_start, sector_end,\
               sector_size, sector_per_file,wavfilesize,count);

    uint8_t *pwave_buf = get_wave_data_buffer();
    if (pwave_buf == NULL) {
        DT_WLS_ERR("wave_buf malloc err");
        return false;
    }
    wavedata_index_t temp_wave_index;
    memset(&temp_wave_index, 0, sizeof(temp_wave_index));

    spi_storage_wave_head_t wave_heads[128*1024];
    uint32_t wave_count = 0;
    memset(wave_heads, 0, sizeof(wave_heads));

    //SPI_SEEK(spi_fd, sector_start*sector_size, SEEK_SET);
//    SPI_SEEK(spi_fd, 0, SEEK_SET);
//    memset(buf,0,10);
//    if(read(spi_fd, buf, 5)!=5)
//        return 0;
//    DT_PRINTF("read spi flash check before :%s\n",buf);
    for(int i=sector_start;i<=sector_end;i+=sector_per_file)
    {
        SPI_SEEK(spi_fd, i*sector_size, SEEK_SET);
        if((size=read(spi_fd, pwave_buf, wavfilesize)) != wavfilesize)
        {
            DT_PRINTF("read wave file size error len:%d,need:%d\n",size,wavfilesize);
            return 0;
        }
        else
        {
            SPI_SEEK(spi_fd, 0, SEEK_SET);
            memset(buf,0,10);
            if(read(spi_fd, buf, 5)!=5)
                return 0;
            DT_PRINTF("read spi flash check:%s\n",buf);
            if(strcmp(buf,"FREQX")==0)
            {
                for(int i =0;i<255;i++)
                {
                    printf("%x ",pwave_buf[i]);
                }
                printf("\n");
                ZIGBEEParam * lparm = getSharedData();
                printf("type:%d,sn:%s,size:%d\n",DataType,lparm->sensor_info[ch_num-1].Sn,wavfilesize);
                if(pwave_buf[0]==0x68&&pwave_buf[1]==0x01&&pwave_buf[3]==0x68)
                {
#ifndef ZIGBEE_TEST_SWITCH
                    storeWaveData(pwave_buf,DataType,lparm->sensor_info[ch_num-1].Sn,wavfilesize);
#else
                    storeWaveData(pwave_buf,DataType,lparm->sensor_info[g_switch_flag].Sn,wavfilesize);
#endif
                }
                else
                    DT_PRINTF("read spi flash check wave head error");
            }
            else
            {
                DT_PRINTF("read spi flash check flash head error");
                return 0;
            }
            ret++;
        }
    }
    if(ret == count)
    {
        DT_PRINTF("read spi flash ret:%d,count:%d",ret,count);
        return 1;
    }
    else
    {
        DT_PRINTF("read spi flash ret:%d,count:%d",ret,count);
        return 0;
    }

    // char wave_data_dir[1024];
    // memset(wave_data_dir, 0, sizeof(wave_data_dir));

    uint32_t chl_count = 0; //有效传感器通道数
    unsigned char sensor_channels[128];
    memset(sensor_channels, 0, sizeof(sensor_channels));

    int sector_num_per_sensor = 0;

    int battery_count = 0;
    battery_info_index_t battery_index_v[1024];  //存放电池信息

	int n;

    for(unsigned char i=0;i<MAX_SENSOR_NUM;i++)
    {
        if( ((sensor_byte>>i)&0x0001) == 1)
        {
            sensor_channels[chl_count] = i+1;
            chl_count++;
        }
    }

    if(sensor_num > 32)
    {
        for(unsigned char i=0;i<MAX_SENSOR_NUM;i++)
        {
            if( ((hight_sensor_byte>>i)&0x0001) == 1)
            {
                sensor_channels[chl_count] = 32+i+1;
                chl_count++;
            }
        }
    }
    if(chl_count == 0)
    {
        DT_WLS_LOG("have no vaild sensor bit!\r\n");
        return false;
    }

    sector_num_per_sensor = (sector_end-sector_start+1)/chl_count;
    if( sector_num_per_sensor < 1)
    {
        DT_WLS_LOG("spi wavedata invaild sector!\r\n");
        return false;
    }

    DT_WLS_LOG("start scan wave header! %ld, sector_size: 0x%x\r\n", time(NULL), sector_size);
    for(unsigned int j=0;j<chl_count;j++)
    {
        int temp_start_sector = sector_start + j*sector_num_per_sensor;
        int temp_end_sector = temp_start_sector + sector_num_per_sensor-1;

        for(int i=temp_start_sector;i<=temp_end_sector;i++)
        {
            spi_storage_wave_head_t temp_head;

            SPI_SEEK(spi_fd, i*sector_size, SEEK_SET);
            if((n = read(spi_fd, &temp_head, sizeof(temp_head))) == sizeof(temp_head))
            {
#ifdef SNESOR_DBG_PRINTF_ENABLE
                uint8_t *pdata = (uint8_t *)&temp_head;
                for (uint32_t i = 0; i < sizeof(temp_head); i++) {
                    printf("%02x ", pdata[i]);
                    if (((i+1)%32) == 0) {
                        printf("\n");
                    }
                }
                printf("\n");
                printf("wave_head: 0x%.x, dataSaveOk = %d\r\n", temp_head.head, temp_head.dataSaveOk);
#endif

                if(temp_head.head == SPI_WAVE_INFO_HEAD)
                {
                    if(temp_head.dataSaveOk == 1)
                    {
                        memcpy(&wave_heads[wave_count], &temp_head, sizeof(spi_storage_wave_head_t));
                        wave_count++;
                        DT_WLS_LOG("spi read head ok,sector=%d!\r\n", i);
                    }

                    if(temp_head.endBlk >= i)  //防止i减小，陷入死循环
                    {
                        i = temp_head.endBlk;
                    }
                    else
                    {
                        DT_WLS_LOG("spi wave bug index sec error,%d/%d, ignore!\r\n", i, temp_head.endBlk);
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
				DT_WLS_ERR("i: %d, sector_size: %d. sector_start: %d, sector_num_per_sensor: %d\r\n",
					i, sector_size, sector_start, sector_num_per_sensor);
                DT_WLS_ERR("spi read error! n: %d, expect: %d, err: %d, %s\r\n", n, sizeof(temp_head), errno, strerror(errno));
				/* 可能系统出问题了, 这里先加上reboot测试看看是否能恢复 */
#ifdef SPI_REBOOT_ON_ERR
				if (spi_need_reboot()) reboot_notify_mcu();
#endif
            }
        }

    }
    DT_WLS_LOG("end scan wave header! %ld\r\n", time(NULL));

    char file_name[512];
    memset(file_name, 0, sizeof(file_name));

    for(unsigned int i=0; i<wave_count; i++)
    {
        if(wave_heads[i].startBlk > wave_heads[i].endBlk)
        {
            DT_WLS_LOG("spi read wave head blkerror,%d/%d!\r\n", wave_heads[i].startBlk, wave_heads[i].endBlk);
            continue;
        }

        // strcpy(file_name, WLESS_DATA_DIR);
        // strcat(file_name, spi_stationcode);
        // strcat(file_name, WAVEDATA_FILE);
        // strcpy(wave_data_dir, file_name);

        start_pos = ( (wave_heads[i].startBlk+1) * sector_size);
        end_pos = ( (wave_heads[i].endBlk+1) * sector_size);
        cur_pos = start_pos;
        SPI_SEEK(spi_fd, start_pos, SEEK_SET);

		DT_WLS_LOG("%d ~ %d (0x%x ~ 0x%x), wave_heads[%d].dataType: %d\r\n", wave_heads[i].startBlk, wave_heads[i].endBlk, start_pos, end_pos, i, wave_heads[i].dataType);
        while(cur_pos < end_pos)
        {
            spi_storage_measdef_index_t temp_measdef_index;

            spi_storage_longwave_index_t temp_longwave_index;

            uint16_t wave_cs = 0;
            uint32_t temp_wave_data_len = 0;

            memset(&temp_wave_index, 0, sizeof(temp_wave_index));

            if(wave_heads[i].dataType == NORMAL_SYNC_MEA || wave_heads[i].dataType == NORMAL_ASYNC_MEA ||
               wave_heads[i].dataType == START_MEA || wave_heads[i].dataType == TEMP_ALARM_MEA ||
               wave_heads[i].dataType == IND_ALARM_MEA)
            {
                if(read(spi_fd, &temp_measdef_index, sizeof(temp_measdef_index)) != sizeof(temp_measdef_index))
                {
                    DT_WLS_LOG("<%s,%d> spi read index error!\r\n", __FUNCTION__, __LINE__);
                    break;
                }
                if(memcmp(temp_measdef_index.head, wave_index_header, sizeof(temp_measdef_index.head)) != 0)
                {
                    DT_WLS_ERR("wave index head error,%x,%x,%x,%x! cur_pos: 0x%x\r\n", temp_measdef_index.head[0], temp_measdef_index.head[1],
						temp_measdef_index.head[2], temp_measdef_index.head[3], cur_pos);
					/* 测试发现问题: 程序读不到数据, 怀疑是此处导致退出, 加一些打印来证实是否是这个原因, 如果是直接重启 */
#ifdef SPI_REBOOT_ON_ERR
					if (spi_need_reboot()) reboot_notify_mcu();
#endif
                    break;
                }
                if(memcmp(temp_measdef_index.meaEnd, wave_index_tailer, sizeof(temp_measdef_index.meaEnd)) != 0)
                {
                    DT_WLS_ERR("wave index tail error,%x,%x,%x,%x,%x,%x!, cur_pos: 0x%x\r\n",
						temp_measdef_index.meaEnd[0], temp_measdef_index.meaEnd[1], temp_measdef_index.meaEnd[2],
                        temp_measdef_index.meaEnd[3], temp_measdef_index.meaEnd[4], temp_measdef_index.meaEnd[5], cur_pos);
					/* 测试发现问题: 程序读不到数据, 怀疑是此处导致退出, 加一些打印来证实是否是这个原因, 如果是直接重启 */
#ifdef SPI_REBOOT_ON_ERR
                    if (spi_need_reboot()) reboot_notify_mcu();
#endif
					break;
                }

                temp_wave_data_len = temp_measdef_index.sampleLen*2;

                if(read(spi_fd, pwave_buf, temp_wave_data_len) != (int)temp_wave_data_len)
                {
                    DT_WLS_LOG("<%s,%d> spi read wave len error,%d!\r\n", __FUNCTION__, __LINE__, temp_wave_data_len);
                    break;
                }

                for(unsigned int j = 0;j<temp_wave_data_len;j++)
                {
                    wave_cs += pwave_buf[j];
                }
                if(wave_cs != temp_measdef_index.chkSum)
                {
                    DT_WLS_LOG("ChNo=%d, zhouxiang=%d, time:%02d%02d-%02d-%02d %02d:%02d:%02d, spi wave data cs error,%x/%x!\r\n",
                           wave_heads[i].channel, temp_measdef_index.signalType,
                           temp_measdef_index.sampleTime[0], temp_measdef_index.sampleTime[1], temp_measdef_index.sampleTime[2],
                           temp_measdef_index.sampleTime[3], temp_measdef_index.sampleTime[4], temp_measdef_index.sampleTime[5],
                           temp_measdef_index.sampleTime[6], wave_cs, temp_measdef_index.chkSum);

//                    cur_pos += (sizeof(temp_measdef_index) + temp_wave_data_len); //位置变换，继续下一组
//                    continue;
                }

//				struct VibPara *pVp = NULL;
//                struct Wireless_CollectPara *pWireless_CollectPara = dt_mqtt_wls_collectparam_get(0);
//				for (int j = 0; j < MAX_WLS_VIB_CH_NUM; j++) {
//					struct VibChannalPara *vcp = &pWireless_CollectPara->VibChannelList[j];
//					if (vcp->ChNo == 0)
//						continue;
//					// DT_WLS_LOG("channel = %d, vcp->ChNo:%d\n", wave_heads[i].channel, vcp->ChNo);
//					if (wave_heads[i].channel == (vcp->ChNo -100)) {
//						for (int k = 0; k < vcp->VibParaCnt; k++) {
//							struct VibPara *vp = &vcp->VibParaList[k];
//                                // printf("j = %d, k = %d, vp->Dir = %d, vp->HighFreq = %d, vp->Length = %d, vp->PlusLength = %d,"
//                                //         "temp_measdef_index.sampleFreq = %f, temp_measdef_index.sampleLen = %d, temp_measdef_index.signalType = %d\r\n", j, k,
//                                //         vp->Dir, vp->HighFreq, vp->Length, vp->PlusLength, temp_measdef_index.sampleFreq, temp_measdef_index.sampleLen, temp_measdef_index.signalType);
//							if (temp_measdef_index.signalType == 1) {
//								if (vp->Dir == VIB_DIR_NONE || vp->Dir == VIB_DIR_Z) {
//									if(((int)(vp->HighFreq*2.56) == (int)(temp_measdef_index.sampleFreq)) &&
//                                 	   ((vp->Length + vp->PlusLength) == (int)(temp_measdef_index.sampleLen)))
//	                                {
//	                                	pVp = vp;
//	                                    break;
//	                                }
//								}
//							} else if (temp_measdef_index.signalType == 2) {
//								if (vp->Dir == VIB_DIR_X) {
//									if(((int)(vp->HighFreq*2.56) == (int)(temp_measdef_index.sampleFreq)) &&
//                                 	   ((vp->Length + vp->PlusLength) == (int)(temp_measdef_index.sampleLen)))
//	                                {
//	                                	pVp = vp;
//	                                    break;
//	                                }
//								}
//							} else if (temp_measdef_index.signalType == 3) {
//								if (vp->Dir == VIB_DIR_Y) {
//									if(((int)(vp->HighFreq*2.56) == (int)(temp_measdef_index.sampleFreq)) &&
//                                 	   ((vp->Length + vp->PlusLength) == (int)(temp_measdef_index.sampleLen)))
//	                                {
//	                                	pVp = vp;
//	                                    break;
//	                                }
//								}
//							}
//						}
//					}
//				}

//				if(!pVp)
//                {
//                    DT_WLS_ERR("<measdef>Can not find the measdef info! Dir:%d,samplelen:%d,freq:%f,"
//                           "channelno:%d,time:%02d%02d-%02d-%02d %02d:%02d:%02d\r\n",
//                           temp_measdef_index.signalType, temp_measdef_index.sampleLen, temp_measdef_index.sampleFreq,
//                           wave_heads[i].channel, temp_measdef_index.sampleTime[0], temp_measdef_index.sampleTime[1],
//                           temp_measdef_index.sampleTime[2], temp_measdef_index.sampleTime[3], temp_measdef_index.sampleTime[4],
//                           temp_measdef_index.sampleTime[5], temp_measdef_index.sampleTime[6]);

//                    cur_pos += (sizeof(temp_measdef_index) + temp_wave_data_len); //位置变换，继续下一组
//                    continue;
//                }

                temp_wave_index.ChannelNo = wave_heads[i].channel;
                temp_wave_index.MeasPointID = 0;//temp_wave_index.ChannelNo | (pVp->Dir << 8);
                temp_wave_index.MeasDefID = 0;//pVp->Code;

                memcpy(temp_wave_index.SampleTime, temp_measdef_index.sampleTime, sizeof(temp_wave_index.SampleTime));
                temp_wave_index.ConverCoef = temp_measdef_index.convCoe;
                temp_wave_index.RPM = temp_measdef_index.runRPM;
                if(wave_heads[i].dataType == NORMAL_SYNC_MEA || wave_heads[i].dataType == NORMAL_ASYNC_MEA)
                {
                    temp_wave_index.DataSource = 0;
                }
                else if(wave_heads[i].dataType == START_MEA)
                {
                    temp_wave_index.DataSource = 8;
                }
                else if(wave_heads[i].dataType == TEMP_ALARM_MEA)
                {
                    temp_wave_index.DataSource = 9;
                }
                else if(wave_heads[i].dataType == IND_ALARM_MEA)
                {
                    temp_wave_index.DataSource = 2;
                }
                else
                {
                    temp_wave_index.DataSource = 0;
                }

                temp_wave_index.WaveType = 0;
                temp_wave_index.UpTag = 0;

                temp_wave_index.DataType = 0;

                temp_wave_index.SampleValue = temp_measdef_index.sampleVal;
                temp_wave_index.SensorSN = atoi(temp_measdef_index.senSn);
                temp_wave_index.WaveFileSize = temp_wave_data_len;

                cur_pos += (sizeof(temp_measdef_index) + temp_wave_data_len); //位置变换，继续下一组
//                DT_WLS_LOG("<measdef wave>zhouxiang:%d,samplelen:%d,freq:%f, mos datalen:%d,pluslen:%d,upfreq:%d",
//                       temp_measdef_index.signalType, temp_measdef_index.sampleLen, temp_measdef_index.sampleFreq,
//                       para_config.measdeflist(temp_i).datalength(), para_config.measdeflist(temp_i).pluslength(),
//                       para_config.measdeflist(temp_i).upperfreq());
            }
            else if(wave_heads[i].dataType == NORMAL_LW || wave_heads[i].dataType == IND_ALARM_LW)
            {
                if(read(spi_fd, &temp_longwave_index, sizeof(temp_longwave_index)) != sizeof(temp_longwave_index))
                {
                    DT_WLS_LOG("spi read long wave index error!");
                    break;
                }
                if(memcmp(temp_longwave_index.head, longwave_index_header, sizeof(temp_longwave_index.head)) != 0)
                {
//                    DT_WLS_LOG("<%s,%d> spi long wave index head error,%x,%x,%x,%x!", __FUNCTION__, __LINE__,
//                           temp_longwave_index.head[0], temp_longwave_index.head[1],
//                            temp_longwave_index.head[2], temp_longwave_index.head[3]);
                    break;
                }
                if(memcmp(temp_longwave_index.longEnd, wave_index_tailer, sizeof(temp_longwave_index.longEnd)) != 0)
                {
//                    DT_WLS_LOG("<%s,%d> spi long wave index tail error,%x,%x,%x,%x,%x,%x!", __FUNCTION__, __LINE__,
//                           temp_longwave_index.longEnd[0], temp_longwave_index.longEnd[1], temp_longwave_index.longEnd[2],
//                            temp_longwave_index.longEnd[3], temp_longwave_index.longEnd[4], temp_longwave_index.longEnd[5]);
                    break;
                }

                temp_wave_data_len = temp_longwave_index.dataLength*2;

                if(read(spi_fd, pwave_buf, temp_wave_data_len) != (int)temp_wave_data_len)
                {
                    DT_WLS_LOG("spi read wave len error,%d!\r\n", temp_wave_data_len);
                    break;
                }

                for(unsigned int j = 0;j<temp_wave_data_len;j++)
                {
                    wave_cs += pwave_buf[j];
                }
                if(wave_cs != temp_longwave_index.chkSum)
                {
                    DT_WLS_LOG("ChNo=%d, time:%02d%02d-%02d-%02d %02d:%02d:%02d, spi long wave data cs error,%x/%x!\r\n",
                           wave_heads[i].channel,
                           temp_longwave_index.sampleTime[0], temp_longwave_index.sampleTime[1], temp_longwave_index.sampleTime[2],
                           temp_longwave_index.sampleTime[3], temp_longwave_index.sampleTime[4], temp_longwave_index.sampleTime[5],
                           temp_longwave_index.sampleTime[6], wave_cs, temp_longwave_index.chkSum);

//                    cur_pos += (sizeof(temp_longwave_index) + temp_wave_data_len); //位置变换，继续下一组
//                    continue;
                }

//				struct VibPara *pVp = NULL;
//                struct Wireless_CollectPara *pWireless_CollectPara = dt_mqtt_wls_collectparam_get(0);
//				for (int j = 0; j < MAX_WLS_VIB_CH_NUM; j++) {
//					struct VibChannalPara *vcp = &pWireless_CollectPara->VibChannelList[j];
//					if (vcp->ChNo == 0)
//						continue;
//					if (wave_heads[i].channel == (vcp->ChNo - 100)) {
//						for (int k = 0; k < vcp->VibParaCnt; k++) {
//							struct VibPara *vp = &vcp->VibParaList[k];
//							if (vp->Dir == VIB_DIR_NONE || vp->Dir == VIB_DIR_Z) {
//								pVp = vp;
//								break;
//							}
//						}
//					}
//				}

//				if(!pVp)
//                {
//                    DT_WLS_ERR("<longwave>Can not find the measdef info!freq type:%d,data len:%d,freq:%d,"
//                           "channelno:%d,time:%02d%02d-%02d-%02d %02d:%02d:%02d\r\n",
//                           temp_longwave_index.freqType, temp_longwave_index.dataLength, temp_longwave_index.freq,
//                           wave_heads[i].channel, temp_longwave_index.sampleTime[0], temp_longwave_index.sampleTime[1],
//                           temp_longwave_index.sampleTime[2], temp_longwave_index.sampleTime[3], temp_longwave_index.sampleTime[4],
//                           temp_longwave_index.sampleTime[5], temp_longwave_index.sampleTime[6]);

//                    cur_pos += (sizeof(temp_longwave_index) + temp_wave_data_len); //位置变换，继续下一组
//                    continue;
//                }

				temp_wave_index.ChannelNo = wave_heads[i].channel;
                temp_wave_index.MeasPointID = 0;//temp_wave_index.ChannelNo | (pVp->Dir << 8);
                temp_wave_index.MeasDefID = 0;//pVp->Code;

                memcpy(temp_wave_index.SampleTime, temp_longwave_index.sampleTime, sizeof(temp_wave_index.SampleTime));
                temp_wave_index.ConverCoef = temp_longwave_index.convCoe;
                temp_wave_index.RPM = temp_longwave_index.runRPM;
#if USE_MQTT
                temp_wave_index.WaveType = 0;
#else
                temp_wave_index.WaveType = TimeDomainWave;
#endif

                if(wave_heads[i].dataType == NORMAL_LW)
                {
                    temp_wave_index.DataSource = 0;
                }
                else if(wave_heads[i].dataType == IND_ALARM_LW)
                {
                    temp_wave_index.DataSource = 1;
                }
                else
                {
                    temp_wave_index.DataSource = 0;
                }

                temp_wave_index.UpTag = 0;

				if (temp_longwave_index.signalType == 4)
					/* audio */
					temp_wave_index.DataType = 2;
				else
					temp_wave_index.DataType = 1;
				temp_wave_index.Freq = temp_longwave_index.freq;

                temp_wave_index.SampleValue = temp_longwave_index.sampleVal;
                temp_wave_index.WaveFileSize = temp_wave_data_len;

                cur_pos += (sizeof(temp_longwave_index) + temp_wave_data_len); //位置变换，继续下一组
            }
            else
            {
                DT_WLS_LOG("unknown wave data type:%d!\r\n", wave_heads[i].dataType);
                break;
            }

#ifdef SNESOR_DBG_PRINTF_ENABLE
            DT_WLS_LOG("%02d%02d-%02d-%02d %02d:%02d:%02d CH: %d, L: 0x%x, MPID: 0x%x, Coe: %f, "
                   "Val: %f, SN: %d, DT: %d\r\n",
                   temp_wave_index.SampleTime[0],temp_wave_index.SampleTime[1],temp_wave_index.SampleTime[2],
                    temp_wave_index.SampleTime[3],temp_wave_index.SampleTime[4],temp_wave_index.SampleTime[5],
                    temp_wave_index.SampleTime[6],
                    temp_wave_index.ChannelNo, temp_wave_index.WaveFileSize, temp_wave_index.MeasPointID, temp_wave_index.ConverCoef,
					temp_wave_index.SampleValue, temp_wave_index.SensorSN, temp_wave_index.DataType);
#endif
            /* save wave index+data */
            uint32_t size;
            void *pdata_buf = dt_malloc(WLESS_WAVE_BUFF_SIZE);
            if (pdata_buf == NULL) {
                DT_WLS_ERR("malloc err");
                return false;
            }
            memcpy(pdata_buf, &temp_wave_index, sizeof(wavedata_index_t));
            memcpy(pdata_buf + sizeof(wavedata_index_t), pwave_buf, temp_wave_data_len);
            size = temp_wave_data_len + sizeof(wavedata_index_t);
            //TODO save UPLOAD_DATA_TYPE_WLESS_WAVE
//            dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_WAVE, pdata_buf, size, (char *)temp_wave_index.SampleTime);
            if (pdata_buf) {
                dt_free(pdata_buf);
                pdata_buf = NULL;
            }
            // 电池信息, 保存在波形最后
            if(wave_heads[i].dataType == NORMAL_SYNC_MEA || wave_heads[i].dataType == NORMAL_ASYNC_MEA ||
               wave_heads[i].dataType == START_MEA || wave_heads[i].dataType == TEMP_ALARM_MEA ||
               wave_heads[i].dataType == IND_ALARM_MEA)
            {
                unsigned int battery_read_len = 0;
                battery_info_head_t battery_head;
                unsigned char battery_tail[6] = {0};

                battery_info_index_t temp_battery_info;

                if(read(spi_fd, &battery_head, sizeof(battery_head)) != sizeof(battery_head))
                {
                    DT_WLS_LOG("spi read battery index header error!\r\n");
                    break;
                }
                if(memcmp(battery_head.head, battery_index_header, sizeof(battery_head.head)) != 0)
                {
                    SPI_SEEK(spi_fd, 0-sizeof(battery_head), SEEK_CUR);
                    continue;
                }
                battery_read_len += sizeof(battery_head);

                for(int i = 0;i<battery_head.battery_num;i++)
                {
                    if(read(spi_fd, &temp_battery_info, sizeof(temp_battery_info)) != sizeof(temp_battery_info))
                    {
                        DT_WLS_LOG("spi read battery info error!\r\n");
                        break;
                    }
                    memcpy(&battery_index_v[battery_count], &temp_battery_info, sizeof(temp_battery_info));
                    battery_count++;

                    battery_read_len += sizeof(temp_battery_info);
                    DT_WLS_LOG("battery info, state=%d, chno=%d, time=%02d%02d-%02d-%02d %02d:%02d:%02d!\r\n",
                           temp_battery_info.State, temp_battery_info.ChannelNo,
                           temp_battery_info.SampleTime[0], temp_battery_info.SampleTime[1],
                           temp_battery_info.SampleTime[2], temp_battery_info.SampleTime[3],
                           temp_battery_info.SampleTime[4], temp_battery_info.SampleTime[5],
                           temp_battery_info.SampleTime[6]);
                }

                if(read(spi_fd, battery_tail, sizeof(battery_tail)) != sizeof(battery_tail))
                {
                    DT_WLS_LOG("spi read battery tail error!\r\n");
                    break;
                }
                battery_read_len += sizeof(battery_tail);
                if(memcmp(battery_tail, wave_index_tailer, sizeof(battery_tail)) != 0)
                {
                    DT_WLS_LOG("battery info tail error!\r\n");
                    // 电池信息尾不对，回滚
                    for(int i = 0;i<battery_head.battery_num;i++)
                    {
                        battery_count--;
                        if (battery_count >= 0) {
                            memset(&battery_index_v[battery_count], 0, sizeof(temp_battery_info));
                        } else {
                            battery_count = 0;
                        }
                    }
                    SPI_SEEK(spi_fd, 0-battery_read_len, SEEK_CUR);
                    continue;
                }

                // 电池信息放在波形最后， 没有出错，表示成功，直接跳出
                break;
            }
        }
    }

    if(battery_count > 0)
    {
        // for (uint32_t i = 0; i < battery_count; i++) {
        //     dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_BATTERY, &battery_index_v[i],
        //     sizeof(battery_info_index_t), (char *)battery_index_v[i].SampleTime);
        // }
        //TODO save UPLOAD_DATA_TYPE_WLESS_BATTERY
//        dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_BATTERY, battery_index_v,
//            battery_count * sizeof(battery_info_index_t), (char *)battery_index_v[0].SampleTime);
    }

    return true;
}


int AEigenDataIndex_convert(eigendata_index_t *peigen, unsigned char chno, spi_storage_a_ind_data_t *a_data)
{
#if 0
    unsigned int x_measpoint=0, y_measpoint=0, z_measpoint=0;
    struct Wireless_CollectPara *pWireless_CollectPara = dt_mqtt_wls_collectparam_get(0);

	for (int i = 0; i < MAX_SENSOR_CNT; i++) {
		struct VibChannalPara *vcp = &pWireless_CollectPara->VibChannelList[i];
		if (chno == (vcp->ChNo - 100)) {
			for (int j = 0; j < vcp->VibParaCnt; j++) {
				struct VibPara *vp = &vcp->VibParaList[j];
				if (vp->Dir == VIB_DIR_NONE) {
					z_measpoint = chno | (vp->Dir << 8);
					break;
				} else {
					if (vp->Dir == VIB_DIR_Z)
						z_measpoint = chno | (vp->Dir << 8);
					if (vp->Dir == VIB_DIR_X)
						x_measpoint = chno | (vp->Dir << 8);
					if (vp->Dir == VIB_DIR_Y)
						y_measpoint = chno | (vp->Dir << 8);
				}
			}
		}
	}

	if((x_measpoint == 0) && (y_measpoint == 0) && (z_measpoint == 0))
    {
        DT_WLS_ERR("Can not find the measdef info. ChannelNo = %d\r\n", chno);
        return false;
    }

    peigen->ChannelNo = chno;
    rtc_seconds_to_time(a_data->indTime, peigen->SampleTime);
    peigen->UpTag = 0;

    if(z_measpoint > 0)
    {
        peigen->MeasPointID = z_measpoint;
        peigen->stIndData[0].isValue = 1;
        peigen->stIndData[0].dataVal = a_data->piez_high_rms;
        peigen->stIndData[0].isWave = a_data->piez_high_rms_alarm;
        peigen->stIndData[12].isValue = 1;
        peigen->stIndData[12].dataVal = a_data->piez_ver;
        peigen->stIndData[12].isWave = a_data->piez_ver_alarm;
    }

    if(x_measpoint > 0)
    {
        memset(peigen->stIndData, 0, sizeof(peigen->stIndData));
        peigen->MeasPointID = x_measpoint;
        peigen->stIndData[12].isValue = 1;
        peigen->stIndData[12].dataVal = a_data->memsx_ver;
        peigen->stIndData[12].isWave = a_data->memsx_ver_alarm;
    }

    if(y_measpoint > 0)
    {
        memset(peigen->stIndData, 0, sizeof(peigen->stIndData));
        peigen->MeasPointID = y_measpoint;
        peigen->stIndData[12].isValue = 1;
        peigen->stIndData[12].dataVal = a_data->memsy_ver;
        peigen->stIndData[12].isWave = a_data->memsy_ver_alarm;
    }

#endif
    return true;
}

int BEigenDataIndex_convert(eigendata_index_t *peigen, unsigned char chno, spi_storage_b_ind_data_t *b_data)
{
#if 0
    unsigned int x_measpoint=0, y_measpoint=0, z_measpoint=0;
    struct Wireless_CollectPara *pWireless_CollectPara = dt_mqtt_wls_collectparam_get(0);

	for (int i = 0; i < MAX_SENSOR_CNT; i++) {
		struct VibChannalPara *vcp = &pWireless_CollectPara->VibChannelList[i];
		if (chno == (vcp->ChNo - 100)) {
			for (int j = 0; j < vcp->VibParaCnt; j++) {
				struct VibPara *vp = &vcp->VibParaList[j];
				if (vp->Dir == VIB_DIR_NONE) {
					z_measpoint = chno | (vp->Dir << 8);
					break;
				} else {
					if (vp->Dir == VIB_DIR_Z)
						z_measpoint = chno | (vp->Dir << 8);
					if (vp->Dir == VIB_DIR_X)
						x_measpoint = chno | (vp->Dir << 8);
					if (vp->Dir == VIB_DIR_Y)
						y_measpoint = chno | (vp->Dir << 8);
				}
			}
		}
	}

	if((x_measpoint == 0) && (y_measpoint == 0) && (z_measpoint == 0))
    {
        DT_WLS_ERR("Can not find the measdef info. ChannelNo = %d\r\n", chno);
        return false;
    }

    peigen->ChannelNo = chno;
    rtc_seconds_to_time(b_data->indTime, peigen->SampleTime);
    peigen->UpTag = 0;

    if(z_measpoint > 0)
    {
        peigen->MeasPointID = z_measpoint;
        peigen->stIndData[1].isValue = 1;
        peigen->stIndData[1].dataVal = b_data->piez_high_kurt;
        peigen->stIndData[1].isWave = b_data->piez_high_kurt_alarm;
        peigen->stIndData[2].isValue = 1;
        peigen->stIndData[2].dataVal = b_data->piez_high_e1;
        peigen->stIndData[2].isWave = b_data->piez_high_e1_alarm;
        peigen->stIndData[3].isValue = 1;
        peigen->stIndData[3].dataVal = b_data->piez_high_e2;
        peigen->stIndData[3].isWave = b_data->piez_high_e2_alarm;
        peigen->stIndData[4].isValue = 1;
        peigen->stIndData[4].dataVal = b_data->piez_high_e3;
        peigen->stIndData[4].isWave = b_data->piez_high_e3_alarm;
        peigen->stIndData[5].isValue = 1;
        peigen->stIndData[5].dataVal = b_data->piez_high_e4;
        peigen->stIndData[5].isWave = b_data->piez_high_e4_alarm;
        peigen->stIndData[6].isValue = 1;
        peigen->stIndData[6].dataVal = b_data->piez_low_rms;
        peigen->stIndData[6].isWave = b_data->piez_low_rms_alarm;
        peigen->stIndData[8].isValue = 1;
        peigen->stIndData[8].dataVal = b_data->piez_low_e1;
        peigen->stIndData[8].isWave = b_data->piez_low_e1_alarm;
        peigen->stIndData[9].isValue = 1;
        peigen->stIndData[9].dataVal = b_data->piez_low_e2;
        peigen->stIndData[9].isWave = b_data->piez_low_e2_alarm;

        peigen->temp.dataVal = b_data->temp;
    }

    if(x_measpoint > 0)
    {
        memset(peigen->stIndData, 0, sizeof(peigen->stIndData));
        peigen->MeasPointID = x_measpoint;
        peigen->stIndData[6].isValue = 1;
        peigen->stIndData[6].dataVal = b_data->memsx_rms;
        peigen->stIndData[6].isWave = b_data->memsx_rms_alarm;
        peigen->stIndData[8].isValue = 1;
        peigen->stIndData[8].dataVal = b_data->memsx_e1;
        peigen->stIndData[8].isWave = b_data->memsx_e1_alarm;
        peigen->stIndData[9].isValue = 1;
        peigen->stIndData[9].dataVal = b_data->memsx_e2;
        peigen->stIndData[9].isWave = b_data->memsx_e2_alarm;
    }

    if(y_measpoint > 0)
    {
        memset(peigen->stIndData, 0, sizeof(peigen->stIndData));
        peigen->MeasPointID = y_measpoint;
        peigen->stIndData[6].isValue = 1;
        peigen->stIndData[6].dataVal = b_data->memsy_rms;
        peigen->stIndData[6].isWave = b_data->memsy_rms_alarm;
        peigen->stIndData[8].isValue = 1;
        peigen->stIndData[8].dataVal = b_data->memsy_e1;
        peigen->stIndData[8].isWave = b_data->memsy_e1_alarm;
        peigen->stIndData[9].isValue = 1;
        peigen->stIndData[9].dataVal = b_data->memsy_e2;
        peigen->stIndData[9].isWave = b_data->memsy_e2_alarm;
    }
#endif
    return true;
}



void init_sample_parameter()
{

}

int spi_read_eigen_data(int sector_start, int sector_end, int sector_size, char *spi_stationcode,
                         int eigen_type, unsigned int sensor_byte, unsigned char sensor_num, unsigned int hight_sensor_byte)
{
    int start_pos=0, end_pos=0, cur_pos=0;

    DT_WLS_LOG("spi read eigen start: %d, end: %d, sector_size: %d\r\n", sector_start, sector_end, sector_size);

    uint32_t chl_count = 0; //有效传感器通道数
    unsigned char sensor_channels[128];
    memset(sensor_channels, 0, sizeof(sensor_channels));
    int sector_num_per_sensor = 0;

    uint32_t a_eigens_count = 0;

    for(unsigned char i=0;i<MAX_SENSOR_NUM;i++)
    {
        if( ((sensor_byte>>i)&0x0001) == 1)
        {
            sensor_channels[chl_count] = i+1;
            chl_count++;
        }
    }

    if(sensor_num > 32)
    {
        for(unsigned char i=0;i<MAX_SENSOR_NUM;i++)
        {
            if( ((hight_sensor_byte>>i)&0x0001) == 1)
            {
                sensor_channels[chl_count] = 32+i+1;
                chl_count++;
            }
        }
    }
    if(chl_count == 0)
    {
        DT_WLS_LOG("have no vaild sensor bit!\r\n");
        return false;
    }

    sector_num_per_sensor = (sector_end-sector_start+1)/chl_count;
    if( sector_num_per_sensor < 1)
    {
        DT_WLS_LOG("spi eigen invaild sector!\r\n");
        return false;
    }

    spi_storage_a_ind_data_t *a_eigens = (spi_storage_a_ind_data_t *)wless_eigen_buf_get(0, WLESS_EIGEN_BUFF_SIZE);
    if (!a_eigens) {
        DT_WLS_ERR("malloc a_eigens err\r\n");
        return false;
    }
    uint32_t b_eigens_count = 0;
    spi_storage_b_ind_data_t *b_eigens = (spi_storage_b_ind_data_t *)wless_eigen_buf_get(1, WLESS_EIGEN_BUFF_SIZE);
    if (!b_eigens) {
        DT_WLS_ERR("malloc b_eigens err\r\n");
        return false;
    }

#ifdef SNESOR_DBG_PRINTF_ENABLE
    static int first_printf = true;
    first_printf = true;
#endif

    uint32_t eigen_num = 0;
    eigendata_index_t *peigen = NULL;

    for(unsigned int i=0;i<chl_count;i++)
    {
        start_pos = ( (sector_start+ i*sector_num_per_sensor )* sector_size);
        end_pos = ( (start_pos/sector_size) + sector_num_per_sensor) * sector_size;
        cur_pos = start_pos;
        SPI_SEEK(spi_fd, start_pos, SEEK_SET);

//        DT_WLS_LOG("i=%d,chno:%d,startpos:%d(%d),endpos:%d(%d)!", i, sensor_channels[i],
//               start_pos, start_pos/sector_size, end_pos, end_pos/sector_size);

        while(cur_pos < end_pos)
        {
            spi_storage_a_ind_t temp_a_eigen_index;
            spi_storage_b_ind_t temp_b_eigen_index;

            if(eigen_type == 0)
            {
                if(read(spi_fd, &temp_a_eigen_index, sizeof(temp_a_eigen_index)) != sizeof(temp_a_eigen_index))
                {
                    DT_WLS_LOG("spi read a eigen error!\r\n");
                    break;
                }

#ifdef SNESOR_DBG_PRINTF_ENABLE
                uint8_t *pdata = (uint8_t *)&temp_a_eigen_index;
                if (first_printf) {
                    for (uint32_t i = 0; i < sizeof(temp_a_eigen_index); i++) {
                        printf("%02x ", pdata[i]);
                        if (((i+1)%32) == 0) {
                            printf("\n");
                        }
                    }
                    printf("\n");
                    printf("temp_a_eigen_index.head = 0x%x\r\n", temp_a_eigen_index.head);
                }
                first_printf = false;
#endif

                if(temp_a_eigen_index.head != SPI_AIND_DATA_HEAD)
                {
//                    DT_WLS_LOG("<%s,%d> spi eigen a index head error,%x, %d,%d!", __FUNCTION__, __LINE__,
//                           temp_a_eigen_index.head, cur_pos, cur_pos%sector_size);
                    if( (cur_pos%sector_size) == 0)
                    {
                        break;
                    }
                    else
                    {
                        // 如果不是页的开头，则跳出下一页的开头
                        cur_pos += (sector_size - cur_pos%sector_size);
                        SPI_SEEK(spi_fd, cur_pos, SEEK_SET);

                        continue;
                    }
                }
                memcpy(&a_eigens[a_eigens_count], &temp_a_eigen_index.aindData, sizeof(spi_storage_a_ind_data_t));
                a_eigens_count++;
                cur_pos += sizeof(temp_a_eigen_index); //位置变换，继续下一组
            }
            else if(eigen_type == 1)
            {
                if(read(spi_fd, &temp_b_eigen_index, sizeof(temp_b_eigen_index)) != sizeof(temp_b_eigen_index))
                {
                    DT_WLS_LOG("spi read a eigen error!\r\n");
                    break;
                }
                if(temp_b_eigen_index.head != SPI_BIND_DATA_HEAD)
                {
//                    DT_WLS_LOG("<%s,%d> spi eigen b index head error,%x, %d!", __FUNCTION__, __LINE__,
//                           temp_b_eigen_index.head, cur_pos%sector_size);

                    if( (cur_pos%sector_size) == 0)
                    {
                        break;
                    }
                    else
                    {
                        // 如果不是页的开头，则跳出下一页的开头
                        cur_pos += (sector_size - cur_pos%sector_size);
                        SPI_SEEK(spi_fd, cur_pos, SEEK_SET);
                        continue;
                    }
                }

                memcpy(&b_eigens[b_eigens_count], &temp_b_eigen_index.bindData, sizeof(spi_storage_b_ind_data_t));
                b_eigens_count++;
                cur_pos += sizeof(temp_b_eigen_index); //位置变换，继续下一组
            }
            else
            {
                DT_WLS_LOG("unknown eigen type:%d!\r\n", eigen_type);
                break;
            }
        }

        eigen_num = ((a_eigens_count > b_eigens_count)? a_eigens_count:b_eigens_count) + 16;
        peigen = (eigendata_index_t *)wless_eigen_buf_get(2, eigen_num*sizeof(eigendata_index_t));
        if (NULL == peigen) {
            DT_ERR("peigen get failed\r\n");
            continue;
        }

#ifdef SNESOR_DBG_PRINTF_ENABLE
        DT_WLS_LOG("i = %d, a_eigens_count = %d, b_eigens_count = %d, sensor_channels[i] = %d\r\n", i, a_eigens_count, b_eigens_count, sensor_channels[i]);
#endif
        memset(peigen, 0, eigen_num*sizeof(eigendata_index_t));
        if(a_eigens_count > 0)
        {
            for (uint32_t k = 0; k < a_eigens_count; k++) {
                if (!AEigenDataIndex_convert(&peigen[k], sensor_channels[i], &a_eigens[k])) {
                    continue;
                }
            }
            //TODO save UPLOAD_DATA_TYPE_WLESS_EIGEN
//            dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_EIGEN, peigen,
//                a_eigens_count*sizeof(eigendata_index_t), (char *)peigen[0].SampleTime);
        }

        memset(peigen, 0, eigen_num*sizeof(eigendata_index_t));
        if(b_eigens_count > 0)
        {
            for (uint32_t k = 0; k < b_eigens_count; k++) {
                if (!BEigenDataIndex_convert(&peigen[k], sensor_channels[i], &b_eigens[k])) {
                    continue;
                }
            }
            //TODO save UPLOAD_DATA_TYPE_WLESS_EIGEN
//            dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_EIGEN, peigen,
//                b_eigens_count*sizeof(eigendata_index_t), (char *)peigen[0].SampleTime);
        }
    }

    return true;
}

int spi_read_station_log_data(int sector_start, int sector_end, int sector_size, char *spi_stationcode)
{
    DT_WLS_LOG("spi read station log: start: %d, end: %d, sector_size: %d\r\n", sector_start, sector_end, sector_size);
    int start_pos=0, end_pos=0, cur_pos=0;
	static int zero_station_log_cnt = 0;

    spi_storage_station_log_t temp_sta_log;

    uint32_t log_count = 0;
    mcu_log_index_t *sta_logs = (mcu_log_index_t *)dt_malloc(WLESS_MCU_LOG_BUFF_SIZE); //一个日志文件16个字节
    if (!sta_logs) {
        DT_WLS_ERR("malloc sta_logs erroe\r\n");
        return false;
    }
    memset(sta_logs, 0, WLESS_MCU_LOG_BUFF_SIZE);

    start_pos = sector_start * sector_size;
    end_pos = ( sector_end+1) * sector_size;
    cur_pos = start_pos;
    SPI_SEEK(spi_fd, start_pos, SEEK_SET);

#ifdef SNESOR_DBG_PRINTF_ENABLE
    static int first_printf = true;
    first_printf = true;
#endif

    while(cur_pos < end_pos)
    {
        if(read(spi_fd, &temp_sta_log, sizeof(temp_sta_log)) != sizeof(temp_sta_log))
        {
            DT_WLS_LOG("spi read station log error!\r\n");
            break;
        }

#ifdef SNESOR_DBG_PRINTF_ENABLE
        uint8_t *pdata = (uint8_t *)&temp_sta_log;
        if (first_printf) {
            for (uint32_t i = 0; i < sizeof(temp_sta_log); i++) {
                printf("%02x ", pdata[i]);
                if (((i+1)%32) == 0) {
                    printf("\n");
                }
            }
            printf("\n");
            printf("temp_sta_log.head = 0x%x, cur_pos = %d, end_pos = %d\n", temp_sta_log.head, cur_pos, end_pos);
        }
        first_printf = false;
#endif

        if(temp_sta_log.head != SPI_STATION_LOG_HEAD)
        {
//            DT_WLS_LOG("<%s,%d> spi station log head error,%x, %d,%d!", __FUNCTION__, __LINE__,
//                   temp_sta_log.head, cur_pos, cur_pos%sector_size);
            if( (cur_pos%sector_size) == 0)
            {
                break;
            }
            else
            {
                // 如果不是页的开头，则跳出下一页的开头
                cur_pos += (sector_size - cur_pos%sector_size);
                SPI_SEEK(spi_fd, cur_pos, SEEK_SET);

                continue;
            }
        }
        else
        {
            memcpy(&sta_logs[log_count], &temp_sta_log.stationlog, sizeof(mcu_log_index_t));
            log_count++;

        //    DT_WLS_LOG("<station line:%d,pos:%d>time:%02d%02d-%02d-%02d %02d:%02d:%02d\r\n",
        //           __LINE__, cur_pos, temp_sta_log.stationlog.logTime[0],
        //             temp_sta_log.stationlog.logTime[1], temp_sta_log.stationlog.logTime[2],
        //             temp_sta_log.stationlog.logTime[3], temp_sta_log.stationlog.logTime[4],
        //             temp_sta_log.stationlog.logTime[5], temp_sta_log.stationlog.logTime[6] );
        }

        cur_pos += sizeof(temp_sta_log); //位置变换，继续下一组
    }

    DT_WLS_LOG("scan station log num: %d!\r\n", log_count);
	if (!log_count) {
		zero_station_log_cnt++;
		DT_WLS_ERR("GOT 0 station log, %d\r\n", zero_station_log_cnt);
		if (zero_station_log_cnt >= 3) {
			DT_WLS_ERR("mcu log can't be 0, maybe some thing wrong with spi flash read, reboot to recovery!\r\n");
			// reboot_notify_mcu(false);
		}
		/* 现场出现问题: 软重启后有可能mcu日志读不到
		 * 之前读不到就返回false, 这样会导致mcu不删除数据, 如果不删除数据这个问题就恢复不了
		 * 所以, 现在修改为返回true, mcu会删除数据, 可能会导致丢一些日志
		*/
        if (sta_logs) {
            dt_free(sta_logs);
            sta_logs = NULL;
        }
        return true;
	} else
		zero_station_log_cnt = 0;

    if(log_count > 0)
    {
    //     for (uint32_t i = 0; i < log_count; i++) {
    //         dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_MCU_LOG, &sta_logs[i],
    //         sizeof(mcu_log_index_t), sta_logs[i].logTime);
    //     }
        //TODO save UPLOAD_DATA_TYPE_WLESS_MCU_LOG
//        dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_MCU_LOG, sta_logs,
//            log_count * sizeof(mcu_log_index_t), sta_logs[0].logTime);
    }

    if (sta_logs) {
        dt_free(sta_logs);
        sta_logs = NULL;
    }
    return true;
}

int spi_read_sensor_log_data(int sector_start, int sector_end, int sector_size, char *spi_stationcode)
{
    DT_WLS_LOG("spi read sensor log start: %d, end: %d, sector_size: %d\r\n", sector_start, sector_end, sector_size);
    int start_pos=0, end_pos=0, cur_pos=0;

    spi_storage_sensor_log_t temp_sensor_log;

    uint32_t sensor_log_cont = 0;
    sensor_info_index_t *sensor_logs = (sensor_info_index_t *)dt_malloc(WLESS_SENSOR_LOG_BUFF_SIZE);
    if (!sensor_logs) {
        DT_WLS_ERR("malloc sta_logs erroe\r\n");
        return false;
    }
    memset(sensor_logs, 0, WLESS_SENSOR_LOG_BUFF_SIZE);

    start_pos = sector_start * sector_size;
    end_pos = ( sector_end+1) * sector_size;
    cur_pos = start_pos;
    SPI_SEEK(spi_fd, start_pos, SEEK_SET);

#ifdef SNESOR_DBG_PRINTF_ENABLE
    static int first_printf = true;
    first_printf = true;
#endif

    while(cur_pos < end_pos)
    {
        if(read(spi_fd, &temp_sensor_log, sizeof(temp_sensor_log)) != sizeof(temp_sensor_log))
        {
            DT_WLS_LOG("spi read sensor log error!\r\n");
            break;
        }
#ifdef SNESOR_DBG_PRINTF_ENABLE
        uint8_t *pdata = (uint8_t *)&temp_sensor_log;
        if (first_printf) {
            for (uint32_t i = 0; i < sizeof(temp_sensor_log); i++) {
                printf("%02x ", pdata[i]);
                if (((i+1)%32) == 0) {
                    printf("\n");
                }
            }
            printf("\n");
        }
        first_printf = false;
#endif

        if(temp_sensor_log.head != SPI_SENSOR_LOG_HEAD)
        {
//            DT_WLS_LOG("<%s,%d> spi sensor log head error,%x, %d,%d!", __FUNCTION__, __LINE__,
//                   temp_sensor_log.head, cur_pos, cur_pos%sector_size);
            if( (cur_pos%sector_size) == 0)
            {
                break;
            }
            else
            {
                // 如果不是页的开头，则跳出下一页的开头
                cur_pos += (sector_size - cur_pos%sector_size);
                SPI_SEEK(spi_fd, cur_pos, SEEK_SET);

                continue;
            }
        }
        else
        {
			DT_WLS_LOG("CH: %d, SN: %d, coe: %.5f.\r\n", temp_sensor_log.sensorlog.ChannelNo, temp_sensor_log.sensorlog.SensorSN, temp_sensor_log.sensorlog.AccCoef);
            memcpy(&sensor_logs[sensor_log_cont], &temp_sensor_log.sensorlog, sizeof(sensor_info_index_t));
            sensor_log_cont++;

//            DT_WLS_LOG("<sensor line:%d,pos:%d>, measpointid:%d, time:%02d%02d-%02d-%02d %02d:%02d:%02d",
//                   __LINE__, cur_pos, temp_sensor_log.sensorlog.MeasPointID,
//                     temp_sensor_log.sensorlog.SensorTime[0],
//                     temp_sensor_log.sensorlog.SensorTime[1], temp_sensor_log.sensorlog.SensorTime[2],
//                     temp_sensor_log.sensorlog.SensorTime[3], temp_sensor_log.sensorlog.SensorTime[4],
//                     temp_sensor_log.sensorlog.SensorTime[5], temp_sensor_log.sensorlog.SensorTime[6] );
        }

        cur_pos += sizeof(temp_sensor_log); //位置变换，继续下一组
    }

    DT_WLS_LOG("scan sensor log num: %d!\r\n", sensor_log_cont);
    if(sensor_log_cont > 0)
    {
        // for (uint32_t i = 0; i < sensor_log_cont; i++) {
        //     dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_SENSOR_LOG, &sensor_logs[i],
        //     sizeof(sensor_info_index_t), (char *)sensor_logs[i].SensorTime);
        // }
    //TODO save UPLOAD_DATA_TYPE_WLESS_SENSOR_LOG
//        dt_upload_data_save(UPLOAD_DATA_TYPE_WLESS_SENSOR_LOG, sensor_logs,
//            sensor_log_cont*sizeof(sensor_info_index_t), (char *)sensor_logs[0].SensorTime);
    }

    if (sensor_logs) {
        dt_free(sensor_logs);
        sensor_logs = NULL;
    }
    return true;
}

#if 0
int spi_read_sta_battery_info(int sector_start, int sector_end, int sector_size, char *spi_stationcode)
{
    DT_WLS_LOG("spi_read_sta_battery_info start");
    int start_pos=0, end_pos=0, cur_pos=0;

    int battery_count = 0;
    battery_info_index_t battery_infos[1024];
    memset(battery_infos, 0, sizeof(battery_infos));

    start_pos = sector_start * sector_size;
    end_pos = ( sector_end+1) * sector_size;
    cur_pos = start_pos;
    SPI_SEEK(spi_fd, start_pos, SEEK_SET);

    unsigned int battery_read_len = 0;
    unsigned char battery_tail[6] = {0};
    battery_info_head_t battery_head;
    battery_info_index_t temp_battery_info;

    while(cur_pos < end_pos)
    {
        battery_read_len = 0;
        if(read(spi_fd, &battery_head, sizeof(battery_head)) != sizeof(battery_head))
        {
            DT_WLS_LOG("<%s,%d> spi read battery index header error!", __FUNCTION__, __LINE__);
            break;
        }
        if(memcmp(battery_head.head, battery_index_header, sizeof(battery_head.head)) != 0)
        {
            if( (cur_pos%sector_size) == 0)
            {
                break;
            }
            else
            {
                // 如果不是页的开头，则跳出下一页的开头
                cur_pos += (sector_size - cur_pos%sector_size);
                SPI_SEEK(spi_fd, cur_pos, SEEK_SET);

                continue;
            }
        }
        battery_read_len += sizeof(battery_head);

        for(int i = 0;i<battery_head.battery_num;i++)
        {
            if(read(spi_fd, &temp_battery_info, sizeof(temp_battery_info)) != sizeof(temp_battery_info))
            {
                DT_WLS_LOG("<%s,%d> spi read battery info error!", __FUNCTION__, __LINE__);
                break;
            }
            memcpy(&battery_infos[battery_count], &temp_battery_info, sizeof(temp_battery_info));
            battery_count++;

            battery_read_len += sizeof(temp_battery_info);
            DT_WLS_LOG("station battery info, state=%d, chno=%d, time=%02d%02d-%02d-%02d %02d:%02d:%02d!",
                   temp_battery_info.State, temp_battery_info.ChannelNo,
                   temp_battery_info.SampleTime[0], temp_battery_info.SampleTime[1],
                   temp_battery_info.SampleTime[2], temp_battery_info.SampleTime[3],
                   temp_battery_info.SampleTime[4], temp_battery_info.SampleTime[5],
                   temp_battery_info.SampleTime[6]);
        }

        if(read(spi_fd, battery_tail, sizeof(battery_tail)) != sizeof(battery_tail))
        {
            DT_WLS_LOG("<%s,%d> spi read battery tail error!", __FUNCTION__, __LINE__);
            break;
        }
        battery_read_len += sizeof(battery_tail);
        if(memcmp(battery_tail, wave_index_tailer, sizeof(battery_tail)) != 0)
        {
            DT_WLS_LOG("battery info tail error!");
            // 电池信息尾不对，回滚
            for(int i = 0;i<battery_head.battery_num;i++)
            {
                battery_count--;
                if (battery_count >=0 ) {
                    memset(&battery_infos[battery_count], 0, sizeof(temp_battery_info));
                } else {
                    battery_count = 0;
                }
            }

            if( (cur_pos%sector_size) == 0)
            {
                break;
            }
            else
            {
                // 如果不是页的开头，则跳出下一页的开头
                cur_pos += (sector_size - cur_pos%sector_size);
                SPI_SEEK(spi_fd, cur_pos, SEEK_SET);
                continue;
            }
        }
        cur_pos += battery_read_len;
    }

    DT_WLS_LOG("station battery info num:%d!", battery_count);
    if(battery_count > 0)
    {
        if(check_used_capacity() != true)
        {
            DT_WLS_LOG("<%s,%d> have not enough stroage space!", __FUNCTION__, __LINE__);
            return false;
        }

        if(save_battery_info_index(spi_stationcode, battery_infos, 2) != true)
        {
            DT_WLS_LOG("<%s,%d>save_battery_info_index failed!", __FUNCTION__, __LINE__);
            return false;
        }
    }

    return true;
}
#endif

int spi_read_flash_data(unsigned char type, uart_cmd_0x36_t info, uart_cmd_new_0x36_t new_info)
{
    int ret = 0;
    unsigned short sector_size = 4096;
    unsigned short max_sector_num = 0;
    unsigned char max_sensor_num = 45;
    char TempStationCode[128];

    if(type == 0)
    {
        snprintf(TempStationCode, sizeof(TempStationCode), "%s", info.StationCode);

        DT_WLS_LOG("code:%s,sectorsize:%d,sensornum:%d,sensorbit:%x, iswave:%d,%d~%d, aeigen:%d,%d~%d, begien:%d,%d~%d,"
           "stationlog:%d,%d~%d, sensorlog:%d,%d~%d, station battery:%d,%d~%d\r\n",TempStationCode,
           info.spi_flash_sector_size, info.save_sensor_info_num, info.vaild_sensor_bit,
           info.is_read_wave, info.wave_data_start_sector, info.wave_data_end_sector,
           info.is_read_a_eigen, info.a_eigen_start_sector, info.a_eigen_end_sector,
           info.is_read_b_eigen, info.b_eigen_start_sector, info.b_eigen_end_sector,
           info.is_read_station_log, info.station_log_start_sector, info.station_log_end_sector,
           info.is_read_sensor_log, info.sensor_log_start_sector, info.sensor_log_end_sector,
           info.is_read_sta_battery, info.sta_battery_start_sector, info.sta_battery_end_sector);

        if(info.spi_flash_sector_size > 0)
        {
            sector_size = info.spi_flash_sector_size;
        }
        if((info.save_sensor_info_num > 0))
        {
            max_sensor_num = info.save_sensor_info_num;
        }
    }
    else
    {
        snprintf(TempStationCode, sizeof(TempStationCode), "%s", new_info.StationCode);

        DT_WLS_LOG("code:%s,sectorsize:%d,sensornum:%d,sensorbit:%x, iswave:%d,%d~%d, aeigen:%d,%d~%d, begien:%d,%d~%d,"
           "stationlog:%d,%d~%d, sensorlog:%d,%d~%d, station battery:%d,%d~%d, high sensor bit:%x\r\n",TempStationCode,
           new_info.spi_flash_sector_size, new_info.save_sensor_info_num, new_info.vaild_sensor_bit,
           new_info.is_read_wave, new_info.wave_data_start_sector, new_info.wave_data_end_sector,
           new_info.is_read_a_eigen, new_info.a_eigen_start_sector, new_info.a_eigen_end_sector,
           new_info.is_read_b_eigen, new_info.b_eigen_start_sector, new_info.b_eigen_end_sector,
           new_info.is_read_station_log, new_info.station_log_start_sector, new_info.station_log_end_sector,
           new_info.is_read_sensor_log, new_info.sensor_log_start_sector, new_info.sensor_log_end_sector,
           new_info.is_read_sta_battery, new_info.sta_battery_start_sector, new_info.sta_battery_end_sector, new_info.high_vaild_sensor_bit);

        if(new_info.spi_flash_sector_size > 0)
        {
            sector_size = new_info.spi_flash_sector_size;
        }
        if((new_info.save_sensor_info_num > 0))
        {
            max_sensor_num = new_info.save_sensor_info_num;
        }
    }

    if(spi_open() != true)
    {
        goto end;
    }
    // init_sample_parameter();


    max_sector_num = SPI_NOR_FLASH_SIZE/sector_size;

    pthread_mutex_lock(&storage_mutex);

    if(type == 0)
    {
        if(info.is_read_wave == 1)
        {
            if( (info.wave_data_start_sector<max_sector_num) &&
                (info.wave_data_end_sector<max_sector_num) &&
                (info.wave_data_start_sector < info.wave_data_end_sector) )
            {
                if((ret = spi_read_wave_data(info.wave_data_start_sector, info.wave_data_end_sector, sector_size,
                                new_info,TempStationCode, info.vaild_sensor_bit, max_sensor_num, 0)) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }
    else
    {
    if(new_info.is_read_wave == 1)
        {
            if( (new_info.wave_data_start_sector<max_sector_num) &&
                (new_info.wave_data_end_sector<max_sector_num) &&
                (new_info.wave_data_start_sector < new_info.wave_data_end_sector) )
            {
                if((ret = spi_read_wave_data(new_info.wave_data_start_sector, new_info.wave_data_end_sector, sector_size,
                                new_info,TempStationCode, new_info.vaild_sensor_bit, max_sensor_num,
                                new_info.high_vaild_sensor_bit)) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }

    if(type == 0)
    {
    if(info.is_read_a_eigen == 1)
        {
            if( (info.a_eigen_start_sector<max_sector_num) &&
                (info.a_eigen_end_sector<max_sector_num) &&
                (info.a_eigen_start_sector <= info.a_eigen_end_sector) )
            {
                if(spi_read_eigen_data(info.a_eigen_start_sector, info.a_eigen_end_sector, sector_size,
                                    TempStationCode, 0, info.vaild_sensor_bit, max_sensor_num, 0) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }
    else
    {
        if(new_info.is_read_a_eigen == 1)
        {
            if( (new_info.a_eigen_start_sector<max_sector_num) &&
                (new_info.a_eigen_end_sector<max_sector_num) &&
                (new_info.a_eigen_start_sector <= new_info.a_eigen_end_sector) )
            {
                if(spi_read_eigen_data(new_info.a_eigen_start_sector, new_info.a_eigen_end_sector, sector_size,
                                    TempStationCode, 0, new_info.vaild_sensor_bit, max_sensor_num,
                                    new_info.high_vaild_sensor_bit) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }

    if(type == 0)
    {
        if(info.is_read_b_eigen == 1)
        {
            if( (info.b_eigen_start_sector<max_sector_num) &&
                (info.b_eigen_end_sector<max_sector_num) &&
                (info.b_eigen_start_sector <= info.b_eigen_end_sector) )
            {
                if(spi_read_eigen_data(info.b_eigen_start_sector, info.b_eigen_end_sector, sector_size,
                                    TempStationCode, 1, info.vaild_sensor_bit, max_sensor_num, 0) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }
    else
    {
        if(new_info.is_read_b_eigen == 1)
        {
            if( (new_info.b_eigen_start_sector<max_sector_num) &&
                (new_info.b_eigen_end_sector<max_sector_num) &&
                (new_info.b_eigen_start_sector <= new_info.b_eigen_end_sector) )
            {
                if(spi_read_eigen_data(new_info.b_eigen_start_sector, new_info.b_eigen_end_sector, sector_size,
                                    TempStationCode, 1, new_info.vaild_sensor_bit, max_sensor_num,
                                    new_info.high_vaild_sensor_bit) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }

    if(type == 0)
    {
        if(info.is_read_station_log == 1)
        {
            if( (info.station_log_start_sector<max_sector_num) &&
                (info.station_log_end_sector<max_sector_num) &&
                (info.station_log_start_sector <= info.station_log_end_sector) )
            {
                if(spi_read_station_log_data(info.station_log_start_sector, info.station_log_end_sector, sector_size,
                                        TempStationCode) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }
    else
    {
        if(new_info.is_read_station_log == 1)
        {
            if( (new_info.station_log_start_sector<max_sector_num) &&
                (new_info.station_log_end_sector<max_sector_num) &&
                (new_info.station_log_start_sector <= new_info.station_log_end_sector) )
            {
                if(spi_read_station_log_data(new_info.station_log_start_sector, new_info.station_log_end_sector, sector_size,
                                        TempStationCode) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }

    if(type == 0)
    {
        if(info.is_read_sensor_log == 1)
        {
            if( (info.sensor_log_start_sector<max_sector_num) &&
                (info.sensor_log_end_sector<max_sector_num) &&
                (info.sensor_log_start_sector <= info.sensor_log_end_sector) )
            {
                if(spi_read_sensor_log_data(info.sensor_log_start_sector, info.sensor_log_end_sector, sector_size,
                                        TempStationCode) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }
    else
    {
        if(new_info.is_read_sensor_log == 1)
        {
            if( (new_info.sensor_log_start_sector<max_sector_num) &&
                (new_info.sensor_log_end_sector<max_sector_num) &&
                (new_info.sensor_log_start_sector <= new_info.sensor_log_end_sector) )
            {
                if(spi_read_sensor_log_data(new_info.sensor_log_start_sector, new_info.sensor_log_end_sector, sector_size,
                                        TempStationCode) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }

#if 0
    if(type == 0)
    {
    if(info.is_read_sta_battery == 1)
        {
            if( (info.sta_battery_start_sector<max_sector_num) &&
                (info.sta_battery_end_sector<max_sector_num) &&
                (info.sta_battery_start_sector <= info.sta_battery_end_sector) )
            {
                if(spi_read_sta_battery_info(info.sta_battery_start_sector, info.sta_battery_end_sector, sector_size,
                                        TempStationCode) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }
    else
    {
    if(new_info.is_read_sta_battery == 1)
        {
            if( (new_info.sta_battery_start_sector<max_sector_num) &&
                (new_info.sta_battery_end_sector<max_sector_num) &&
                (new_info.sta_battery_start_sector <= new_info.sta_battery_end_sector) )
            {
                if(spi_read_sta_battery_info(new_info.sta_battery_start_sector, new_info.sta_battery_end_sector, sector_size,
                                        TempStationCode) != true)
                {
                    pthread_mutex_unlock(&storage_mutex);
                    goto end;
                }
            }
        }
    }
#endif

    //ret = true;

    pthread_mutex_unlock(&storage_mutex);
end:

    spi_close();
#ifdef SPI_REBOOT_ON_ERR
	if (!g_spi_err && access(SPI_ERR_FILE, F_OK) == 0)
		remove(SPI_ERR_FILE);
#endif
    dt_system("sync");
    printf("spi read wave ret:%d!!!",ret);
    if(ret)
    {
        set_task_run_state(TASK_SUCCESS);
    }
    else
    {
        set_task_run_state(TASK_STORE_DATA_FAIL);
    }
    return ret;
}

int spi_nor_flash_check(void)
{
    int ret = false;
    unsigned char test_buf[16] = {0};
    if(global_hw_check_result == 0x02)
    {
        return true;
    }

    if(global_hw_check_result != 0x02)
    {
        global_hw_check_result = 0x02;
    }

    if(spi_open() != true)
    {
        global_hw_check_result = 0x0;
        return false;
    }

    if(read(spi_fd, test_buf, sizeof(test_buf)) != sizeof(test_buf))
    {
        goto end;
    }
    for(unsigned char i=0;i<16;i++)
    {
        if(test_buf[i] != i)
        {
            goto end;
        }
    }
    ret = true;
end:
    spi_close();

    if(ret)
    {
        global_hw_check_result = 0x1;
    }
    else
    {
        global_hw_check_result = 0x0;
    }
    return true;
}
