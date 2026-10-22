#ifndef ZIGBEEBASE_H
#define ZIGBEEBASE_H
#include "zigbee/proto_parse.h"
#include "zigbee/zigbeetype.h"
#include "SensorLogManager.h"
class zigbeebase
{
public:
    zigbeebase();
    int init(ZIGBEEParam * Param,SensorLogManager *manager);
    int setupLogInfo();
public:
    ZIGBEEParam * m_ZIGBEEParam;
    SensorLogManager *m_LogManager;
};

#endif // ZIGBEEBASE_H
