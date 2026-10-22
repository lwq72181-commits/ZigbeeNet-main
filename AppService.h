#ifndef APPSERVICE_H
#define APPSERVICE_H
#include"Dmamemcpy.h"
#include "fqmqttclient.h"
#include "ThreadSafeMultiVector.h"
#include "Acquisitiontype.h"
#include "SensorLogManager.h"
class AppService
{
public:
    AppService();
    static DmaMemcpy*  dmaMemcpy();
    static QmqttClient& mqttService();
    static ThreadSafeMultiVector * threadVector();
    static SensorLogManager * logManager();
    static TCPParam* tcpParam();
};

#endif // APPSERVICE_H
