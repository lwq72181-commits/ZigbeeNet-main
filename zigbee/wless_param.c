#include <stdbool.h>
#include "sys/stat.h"
//#include "dt_mqtt_protocol.h"
#include "wless_param.h"
#include "proto_common.h"
//#include "dt_fram.h"

// static int is_update = DT_DISABLE; //告诉无线传感器，是否需要更新采集参数

int _wless_update_flag_file_read(char *pdata)
{
    int fd = -1;
    int ret = DT_ERROR;
    char fielname[128] = {0};

    snprintf(fielname, sizeof(fielname), "%s%s", WLESS_PARAM_PATH, WLESS_PARAM_FLAG_FILE);
    fd = open(fielname, O_RDONLY);
    if (fd < 0) {
        DT_ERR("open failed.\r\n");
        goto _exit;
    }
    if (sizeof(wless_param_flag_t) != read(fd, pdata, sizeof(wless_param_flag_t))) {
        DT_ERR("read failed.\r\n");
        goto _exit;
    }

    ret = DT_EOK;
_exit:
    if (fd > 0 ) {
        close(fd);
        fd = -1;
    }

    return ret;
}

int _wless_update_flag_file_write(char *pdata)
{
    int fd = -1;
    int ret = DT_ERROR;
    char fielname[128]    = {0};
    char write_data[2048] = {0};

    snprintf(fielname, sizeof(fielname), "%s%s", WLESS_PARAM_PATH, WLESS_PARAM_FLAG_FILE);
    fd = open(fielname, O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        DT_ERR("open failed.\r\n");
        goto _exit;
    }

    //防止文件过小
    memcpy(write_data, pdata, sizeof(wless_param_flag_t));
    if (sizeof(write_data) != write(fd, write_data, sizeof(write_data))) {
        DT_ERR("write failed.\r\n");
        goto _exit;
    }

    ret = DT_EOK;
_exit:
    if (fd > 0 ) {
        close(fd);
        fd = -1;
    }

    return ret;
}

/**
 * @brief 用于获取无线参数是否需要更新更新状态
 * @param *value 指向状态结果的指针
 *
 * @return DT_EOK/DT_ERROR
 */
FREQ_PUBLIC int wless_param_update_flag_get(int *value)
{
    wless_param_flag_t data;
    memset(&data, 0x00, sizeof(wless_param_flag_t));

    *value = DT_DISABLE;
//    if (DT_EOK != dt_fram_read(FRAM_WLESS_PARAM_FLAG_ADDR, (char *)&data, sizeof(wless_param_flag_t))) {
//        DT_ERR("fram read failed.\r\n");
//        //fram error, start read file
//        if (DT_EOK != _wless_update_flag_file_read((char *)&data)) {
//            DT_ERR("file read failed.\r\n");
//            return DT_ERROR;
//        }
//    }

    if (DT_EOK != _wless_update_flag_file_read((char *)&data)) {
        DT_ERR("file read failed.\r\n");
        return DT_ERROR;
    }
    if (data.head == FRAM_WLESS_PARAM_FLAG_HEAD && data.tail == FRAM_WLESS_PARAM_FLAG_TAIL)
        *value = data.flag;

    return DT_EOK;
}

/**
 * @brief 用于设置无线参数是否需要更新更新状态
 * @param value 0/1
 *
 * @return DT_EOK(成功)/DT_ERROR(失败)
 */
FREQ_PUBLIC void wless_param_update_flag_set(int value)
{
    int read_value = 0;
    wless_param_flag_t data;
    memset(&data, 0x00, sizeof(wless_param_flag_t));

    if (DT_EOK == wless_param_update_flag_get(&read_value)) {
        if (read_value == value)
            return;
    }

    data.head = FRAM_WLESS_PARAM_FLAG_HEAD;
    data.flag = value;
    data.tail = FRAM_WLESS_PARAM_FLAG_TAIL;
//    if (DT_EOK != dt_fram_write(FRAM_WLESS_PARAM_FLAG_ADDR, (char *)&data, sizeof(wless_param_flag_t))) {
//        DT_ERR("fram write failed.\r\n");
//        //fram error, start write file
//        _wless_update_flag_file_write((char *)&data);
//    }

    _wless_update_flag_file_write((char *)&data);
}

static int map_eigenindex_to_mqtt_code(int EigenIndex)
{
	int code = 0;

	switch (EigenIndex) {
	case LOW_FREQ_RMS:
		code = INDEX_TYPE_LOW_RMS;
		break;
	case LOW_FREQ_KURT:
		break;
	case LOW_ENERGY_1:
		break;
	case LOW_ENERGY_2:
		break;
	case LOW_ENERGY_3:
		break;
	case LOW_ENERGY_4:
		break;
	case VEL_RMS:
		code = INDEX_TYPE_VEL_RMS;
		break;
	case HIGH_FREQ_RMS:
		code = INDEX_TYPE_HIGH_RMS;
		break;
	case HIGH_FREQ_KURT:
		break;
	case HIGH_ENERGY_1:
		break;
	case HIGH_ENERGY_2:
		break;
	case HIGH_ENERGY_3:
		break;
	case HIGH_ENERGY_4:
		break;
	}

	return code;
}

static void insert_index_alarm(int fd, int direction, int EigenIndex, struct VibChannalPara *vcp)
{
    float temp_float;
	struct LimitPara *lp = NULL;
	int code = map_eigenindex_to_mqtt_code(EigenIndex);

	if (code) {
		for (int i = 0; i < vcp->LimitParaCnt; i++) {
			lp = &vcp->LimitParaList[i];
			if (direction == lp->Dir && code == lp->Code) {
				// 指标门限, 当前传感器只用了指标门限
				dt_write(fd, &lp->Limit, 4);
				DT_WLS_LOG("CHNo:%d, Dir: %d, Code: %d, Limit: %f\r\n", vcp->ChNo, lp->Dir, lp->Code, lp->Limit);
				goto next;
			}
		}
	}

	// 没有找到, 指标门限赋值为0
    temp_float = 0.0f;
	dt_write(fd, &temp_float, 4);
next:
	temp_float = 0.0f;
	// 指标最小值
	dt_write(fd, &temp_float, 4);
	temp_float = 0.0f;
	// 指标最大值
	dt_write(fd, &temp_float, 4);
	temp_float = 0.0f;
	// 指标幅度
	dt_write(fd, &temp_float, 4);
}

static float retrive_stoplimit(int ch, int code, struct Wireless_CollectPara *para)
{
	struct StopLimitInfo *stop = para->StopLimitList;
	for (int i = 0; i < para->StopLimitListCnt; i++) {
		stop = &para->StopLimitList[i];
		if (ch == stop->ChNo && code == stop->Code) {
			DT_WLS_LOG("found ch: %d, code: %d, limit: %f.\r\n", ch, code, stop->StopLimit);
			return stop->StopLimit;
		}
	}
	DT_WARN("NOT FOUND ch: %d, code: %d.\r\n", ch, code);
	return 0.0;
}

static float retrive_temperature_limit(int level, struct VibChannalPara *vcp)
{
	struct LimitPara *lp;
	float limit = 0.0;

	for (int i = 0; i < vcp->LimitParaCnt; i++) {
		lp = &vcp->LimitParaList[i];
		if (lp->Code == INDEX_TYPE_TEMP && lp->Level == level) {
			limit = lp->Limit;
			DT_WLS_LOG("Level: %d, Limit: %f\r\n", lp->Level, lp->Limit);
			break;
		}
	}
	return limit;
}

static void insert_measdef(int fd, int direction, struct VibChannalPara *vcp)
{
    unsigned char mdf_num = 0;
    unsigned int temp_uint32;
    int temp_point = 0;
    char unit[UNIT_LENGTH] = {0};

    for(int i = 0; i < vcp->VibParaCnt; i++) {
    	struct VibPara *vp = &vcp->VibParaList[i];
		/* 长波形不是测量定义 */
		if (vp->Code == VIB_CODE_LONGWAVE)
			continue;
		if ((vp->Dir == direction) || ((direction == VIB_DIR_Z) && (vp->Dir == VIB_DIR_NONE)))
			mdf_num++;
    }

	temp_point = vcp->ChNo | (direction << 8);
    DT_WLS_LOG("ch=%d, point_id = 0x%x, dir: %d, mdf_num=%d\r\n", vcp->ChNo, temp_point, direction, mdf_num);
    dt_write(fd, &temp_point, 4);

    dt_write(fd, &mdf_num, 1);
	for(int i = 0; i < vcp->VibParaCnt; i++) {
    	struct VibPara *vp = &vcp->VibParaList[i];
		/* 长波形不是测量定义 */
		if (vp->Code == VIB_CODE_LONGWAVE)
			continue;

		if ((vp->Dir == direction) || ((direction == VIB_DIR_Z) && (vp->Dir == VIB_DIR_NONE)))
		{
			DT_WLS_LOG("DIR: %d, direction: %d\r\n", vp->Dir, direction);
			temp_uint32 = vp->Code; // measdef id
            dt_write(fd, &temp_uint32, 4);
            memset(unit, 0, UNIT_LENGTH);
            dt_write(fd, unit, UNIT_LENGTH); // no unit?
            temp_uint32 = vp->HighFreq;
            dt_write(fd, &temp_uint32, 4);
            temp_uint32 = vp->Length;
            dt_write(fd, &temp_uint32, 4);
            temp_uint32 = vp->PlusLength;
            dt_write(fd, &temp_uint32, 4);
		}
    }
}

static void insert_index_freq_band(int fd, int direction, int EigenIndex, struct VibChannalPara *vcp)
{
	unsigned short temp_short;
	temp_short = 0;
    dt_write(fd, &temp_short, 2);
    temp_short = 0;
    dt_write(fd, &temp_short, 2);
}

int wless_param_convert_to_file(struct SysPara *pSysPara, struct Wireless_CollectPara *pCollectPara)
{
    char param_path[128] = {0};
    char param_file[256] = {0};

    int ret = true;
    wless_common_param_t wless_common_param;
    float temp_float;
    unsigned char temp_char;

    if (0 == pCollectPara->VibChannelCnt) {
        DT_WLS_ERR("wless VibChannelCnt is 0.\r\n");
        return false;
    }

#if 0
    if(access(param_path, F_OK) != 0)
    {
        CreateDir(param_path);
    }

    /* selfmake sys para for test */
    dt_current_time_get(pSysPara->SysTime);
    dt_current_time_get(pSysPara->ParaTime);

    // pSysPara->IndexPeriod = 300;
    // pSysPara->WavePeriod = 600;
    // pSysPara->LongWavePeriod = 3600;
    // pSysPara->CollectType = 0;
#endif
    strcpy(param_path, WLESS_PARAM_PATH);
	strcat(param_path, WLESS_PARAM_FILE);
    strcpy(param_file, param_path);
	DT_WLS_LOG("open file %s\r\n", param_file);
    int fd = open(param_file, O_RDWR|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(fd < 0)
    {
        DT_WLS_ERR("open file: %s error!\r\n", param_file);
        return false;
    }

    memset(&wless_common_param, 0x00, sizeof(wless_common_param));

	//dt_current_time_get((char *)wless_common_param.UpTimeBuf);
	// 去掉年的高两位
    sscanf(pSysPara->ParaTime + 2, "%02d-%02d-%02dT%02d:%02d:%02d",
		&wless_common_param.UpTimeBuf[1],
		&wless_common_param.UpTimeBuf[2],
		&wless_common_param.UpTimeBuf[3],
		&wless_common_param.UpTimeBuf[4],
		&wless_common_param.UpTimeBuf[5],
		&wless_common_param.UpTimeBuf[6]);
	wless_common_param.UpTimeBuf[0] = 20;

	wless_common_param.VibIndexPeriod = pSysPara->WlessIndexPeriod; //无线传感器的指标间隔
	wless_common_param.MeasDefPeriod = pSysPara->WlessWavePeriod;   //无线传感器的波形间隔
	wless_common_param.TemperaturePeriod = pSysPara->WlessIndexPeriod;

	// 当前默认所有通道的定义一样, 所以只需检查第一个通道的
	struct VibChannalPara *vcp = &pCollectPara->VibChannelList[0];
	for (int i = 0; i < vcp->VibParaCnt; i++) {
		struct VibPara *vp = &vcp->VibParaList[i];
		if (vp->Code == VIB_CODE_LONGWAVE) {
			// long wave
			wless_common_param.LongWavePeriod = pSysPara->LongWavePeriod / 3600;
			wless_common_param.LongWaveLength = vp->Length;
			wless_common_param.LongWaveFreq = vp->HighFreq;
			continue;
		}

		if (vp->Code == VIB_CODE_HIGH_ACC) {
			// high freq
			wless_common_param.HighFreqDaqLength = vp->Length;
            wless_common_param.HighFreqSaveLength = 0; // no save length?
            wless_common_param.HighDaqFreq = vp->HighFreq;
			continue;
		}

		if (vp->Code == VIB_CODE_LOW_ACC) {
			// low freq
			wless_common_param.LowFreqDaqLength = vp->Length;
            wless_common_param.LowFreqSaveLength = 0; // no save length?
            wless_common_param.LowDaqFreq = vp->HighFreq;
            continue;
		}
	}

	wless_common_param.IndexResetPeriod = 0; // no reset period?
	wless_common_param.VibChannelNum = pCollectPara->VibChannelCnt;

	DT_WLS_LOG("para time = %02d%02d-%02d-%02d %02d:%02d:%02d\r\n",
		wless_common_param.UpTimeBuf[0], wless_common_param.UpTimeBuf[1], wless_common_param.UpTimeBuf[2], wless_common_param.UpTimeBuf[3],
		wless_common_param.UpTimeBuf[4], wless_common_param.UpTimeBuf[5], wless_common_param.UpTimeBuf[6]);
	DT_WLS_LOG("VibIndexPeriod = %d, MeasDefPeriod = %d, TemperaturePeriod = %d, LongWavePeriod = %d, LongWaveLength = %d, LongWaveFreq = %d, "
		"LowFreqDaqLength = %d, HighFreqDaqLength = %d, LowFreqSaveLength = %d, HighFreqSaveLength = %d, LowDaqFreq = %d, HighDaqFreq = %d, "
		"IndexResetPeriod = %d VibChannelNum = %d\r\n",
		wless_common_param.VibIndexPeriod, wless_common_param.MeasDefPeriod, wless_common_param.TemperaturePeriod, wless_common_param.LongWavePeriod,
		wless_common_param.LongWaveLength, wless_common_param.LongWaveFreq, wless_common_param.LowFreqDaqLength, wless_common_param.HighFreqDaqLength,
		wless_common_param.LowFreqSaveLength, wless_common_param.HighFreqSaveLength, wless_common_param.LowDaqFreq, wless_common_param.HighDaqFreq,
		wless_common_param.IndexResetPeriod, wless_common_param.VibChannelNum);

	dt_write(fd, &wless_common_param, sizeof(wless_common_param));

    //这里按文件字节存储写入顺序
    for(int i = 0; i < MAX_WLS_VIB_CH_NUM; i++) {
        vcp = &pCollectPara->VibChannelList[i];
        if (vcp->ChNo == 0) {
            continue;
        }

        DT_WLS_LOG("channelno = %d\r\n", vcp->ChNo);
        temp_char = vcp->ChNo - 100;
        dt_write(fd, &temp_char, 1); 		//传感器通道

        temp_float = retrive_stoplimit(temp_char, INDEX_TYPE_VEL_RMS, pCollectPara);
        dt_write(fd, &temp_float, 4);

		/* 上位机只会下发3,4级, 不会下发1,2级 */
		// 温度报警一级门限
        temp_char = (unsigned char)retrive_temperature_limit(3, vcp);
        dt_write(fd, &temp_char, 1);
		// 温度报警二级门限
        temp_char = (unsigned char)retrive_temperature_limit(4, vcp);
        dt_write(fd, &temp_char, 1);
		// 温度报警一级变化率, 没有用到
        temp_char = 0; // no tmprateone?
        dt_write(fd, &temp_char, 1);
		// 温度报警二级变化率, 没有用到
        temp_char = 0; // no tmpratetwo?
        dt_write(fd, &temp_char, 1);

        insert_measdef(fd, VIB_DIR_Z, vcp);
        insert_measdef(fd, VIB_DIR_X, vcp);
        insert_measdef(fd, VIB_DIR_Y, vcp);
    }

    for (int i = 0; i < MAX_WLS_VIB_CH_NUM; i++) {
        vcp = &pCollectPara->VibChannelList[i];
        if (vcp->ChNo == 0) {
            continue;
        }
        // DT_WLS_LOG("channelno = %d\r\n", vcp->ChNo);
        temp_char = vcp->ChNo - 100;
        dt_write(fd, &temp_char, 1);

        insert_index_freq_band(fd, VIB_DIR_Z, LOW_ENERGY_1, vcp);
        insert_index_freq_band(fd, VIB_DIR_Z, LOW_ENERGY_2, vcp);
        insert_index_freq_band(fd, VIB_DIR_Z, LOW_ENERGY_3, vcp);
        insert_index_freq_band(fd, VIB_DIR_Z, LOW_ENERGY_4, vcp);
        insert_index_freq_band(fd, VIB_DIR_Z, VEL_RMS, vcp);

        insert_index_freq_band(fd, VIB_DIR_Z, HIGH_ENERGY_1, vcp);
        insert_index_freq_band(fd, VIB_DIR_Z, HIGH_ENERGY_2, vcp);
        insert_index_freq_band(fd, VIB_DIR_Z, HIGH_ENERGY_3, vcp);
        insert_index_freq_band(fd, VIB_DIR_Z, HIGH_ENERGY_4, vcp);

        insert_index_freq_band(fd, VIB_DIR_X, LOW_ENERGY_1, vcp);
        insert_index_freq_band(fd, VIB_DIR_X, LOW_ENERGY_2, vcp);
        insert_index_freq_band(fd, VIB_DIR_X, LOW_ENERGY_3, vcp);
        insert_index_freq_band(fd, VIB_DIR_X, LOW_ENERGY_4, vcp);
        insert_index_freq_band(fd, VIB_DIR_X, VEL_RMS, vcp);

        insert_index_freq_band(fd, VIB_DIR_X, HIGH_ENERGY_1, vcp);
        insert_index_freq_band(fd, VIB_DIR_X, HIGH_ENERGY_2, vcp);
        insert_index_freq_band(fd, VIB_DIR_X, HIGH_ENERGY_3, vcp);
        insert_index_freq_band(fd, VIB_DIR_X, HIGH_ENERGY_4, vcp);

        insert_index_freq_band(fd, VIB_DIR_Y, LOW_ENERGY_1, vcp);
        insert_index_freq_band(fd, VIB_DIR_Y, LOW_ENERGY_2, vcp);
        insert_index_freq_band(fd, VIB_DIR_Y, LOW_ENERGY_3, vcp);
        insert_index_freq_band(fd, VIB_DIR_Y, LOW_ENERGY_4, vcp);
        insert_index_freq_band(fd, VIB_DIR_Y, VEL_RMS, vcp);

        insert_index_freq_band(fd, VIB_DIR_Y, HIGH_ENERGY_1, vcp);
        insert_index_freq_band(fd, VIB_DIR_Y, HIGH_ENERGY_2, vcp);
        insert_index_freq_band(fd, VIB_DIR_Y, HIGH_ENERGY_3, vcp);
        insert_index_freq_band(fd, VIB_DIR_Y, HIGH_ENERGY_4, vcp);
    }

    for (int i = 0; i < MAX_WLS_VIB_CH_NUM; i++) {
        vcp = &pCollectPara->VibChannelList[i];
        if (vcp->ChNo == 0) {
            continue;
        }

        // DT_WLS_LOG("channelno = %d\r\n", vcp->ChNo);
        temp_char = vcp->ChNo - 100;
        dt_write(fd, &temp_char, 1);

        insert_index_alarm(fd, VIB_DIR_Z, LOW_FREQ_RMS, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, LOW_FREQ_KURT, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, LOW_ENERGY_1, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, LOW_ENERGY_2, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, LOW_ENERGY_3, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, LOW_ENERGY_4, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, VEL_RMS, vcp);

        insert_index_alarm(fd, VIB_DIR_Z, HIGH_FREQ_RMS, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, HIGH_FREQ_KURT, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, HIGH_ENERGY_1, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, HIGH_ENERGY_2, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, HIGH_ENERGY_3, vcp);
        insert_index_alarm(fd, VIB_DIR_Z, HIGH_ENERGY_4, vcp);

        insert_index_alarm(fd, VIB_DIR_X, LOW_FREQ_RMS, vcp);
        insert_index_alarm(fd, VIB_DIR_X, LOW_FREQ_KURT, vcp);
        insert_index_alarm(fd, VIB_DIR_X, LOW_ENERGY_1, vcp);
        insert_index_alarm(fd, VIB_DIR_X, LOW_ENERGY_2, vcp);
        insert_index_alarm(fd, VIB_DIR_X, LOW_ENERGY_3, vcp);
        insert_index_alarm(fd, VIB_DIR_X, LOW_ENERGY_4, vcp);
        insert_index_alarm(fd, VIB_DIR_X, VEL_RMS, vcp);

        insert_index_alarm(fd, VIB_DIR_X, HIGH_FREQ_RMS, vcp);
        insert_index_alarm(fd, VIB_DIR_X, HIGH_FREQ_KURT, vcp);
        insert_index_alarm(fd, VIB_DIR_X, HIGH_ENERGY_1, vcp);
        insert_index_alarm(fd, VIB_DIR_X, HIGH_ENERGY_2, vcp);
        insert_index_alarm(fd, VIB_DIR_X, HIGH_ENERGY_3, vcp);
        insert_index_alarm(fd, VIB_DIR_X, HIGH_ENERGY_4, vcp);

        insert_index_alarm(fd, VIB_DIR_Y, LOW_FREQ_RMS, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, LOW_FREQ_KURT, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, LOW_ENERGY_1, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, LOW_ENERGY_2, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, LOW_ENERGY_3, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, LOW_ENERGY_4, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, VEL_RMS, vcp);

        insert_index_alarm(fd, VIB_DIR_Y, HIGH_FREQ_RMS, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, HIGH_FREQ_KURT, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, HIGH_ENERGY_1, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, HIGH_ENERGY_2, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, HIGH_ENERGY_3, vcp);
        insert_index_alarm(fd, VIB_DIR_Y, HIGH_ENERGY_4, vcp);
    }

    close(fd);

    dt_system("sync");

    return ret;
}
