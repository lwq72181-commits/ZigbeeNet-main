#include "zigbeebase.h"
#include "zigbee/common.h"
#include "zigbee/dt_uart_manager.h"
#include "zigbee/proto_common.h"
zigbeebase::zigbeebase()
{
    //init();
}

int zigbeebase::setupLogInfo()
{
    for(int j=0;j<m_ZIGBEEParam->count;j++)
    {
        int sn = atoi(m_ZIGBEEParam->sensor_info[j].Sn);
        m_LogManager->addIndexInfo(sn);
        m_LogManager->setMaxIndexSendCount(sn, 5); // 限制最多3条
        m_LogManager->setMaxIndexRecvCount(sn, 2); // 限制最多3条
        m_LogManager->addWaveInfo(sn);
        m_LogManager->setMaxWaveSendCount(sn, 5); // 限制最多3条
        m_LogManager->setMaxWaveRecvCount(sn, 2); // 限制最多3条
    }
    return 0;
}

int zigbeebase::init(ZIGBEEParam * Param,SensorLogManager *manager)
{
    m_ZIGBEEParam = Param;
    m_LogManager = manager;
    //DT_PRINTF("m_ZIGBEEParam:%d,sensor:%d",m_ZIGBEEParam,m_ZIGBEEParam->sensor_info);
    /* 文件初始化 */
    dt_file_dir_init();
    /* 内存模块检查初始化 */
    dt_heap_module_init();
    /* 串口模块初始化 */
    dt_uart_module_init();
    /* 无线数据接入模块*/
    DT_PRINTF("dt_wireless_access_init\n");
    dt_wireless_access_init(m_ZIGBEEParam);
    DT_PRINTF("zigbeebase init over\n");
    setupLogInfo();
    return 0;
}
