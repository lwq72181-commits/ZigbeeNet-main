#include "AppService.h"
#include "Application.h"
static Application* getApp()
{
    return (Application*)Application::instance();
}

AppService::AppService()
{

}

DmaMemcpy* AppService::dmaMemcpy()
{
    return getApp()->dmaMemcpy();
}

QmqttClient& AppService::mqttService()
{
    Application* app = (Application*)QApplication::instance();
    return app->mqttService();
}

ThreadSafeMultiVector* AppService::threadVector()
{
    return getApp()->threadVector();
}

SensorLogManager * AppService::logManager()
{
    return getApp()->logManager();
}

TCPParam* AppService::tcpParam()
{
    return getApp()->tcpParam();
}


