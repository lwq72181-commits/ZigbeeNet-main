#ifndef QMQTTCLIENT_H
#define QMQTTCLIENT_H
//#include <QtCore>
#include <QObject>
#include "QtMqtt/qmqttclient.h"
#include <atomic>
#include<QThread>
#include "mqtthandle.h"
#include "Acquisitiontype.h"
#include "AdcHandle.h"
#include "uartConfig4g5g.h"
#include "NetworkConf.h"
#include "zigbee/proto_parse.h"
class QmqttClient :public QObject
{
    Q_OBJECT
public:
    QmqttClient();
    void close();
    bool isOpen();
    bool isConnected();
    void setMqUrl(const QString& url, const QString& username, const QString& password, const QString& orgname, const QString& devid,\
                  const int& port,TCPParam *param,ZIGBEEParam * ZParam);
    uartConfig4g5g *getUart();
    pthread_mutex_t *getmutex();
protected:
    void startMqttTimer();
    void startMqttTimer(int second);
    void stopMqttTimer();
    void onMqttTimer();
    void setupMqtt();
    void reconnect();
    void reconnect(int time);
    void publishStatus();
    void startTimeOutTimer();
    void stopTimeOutTimer();
    void onTimeOutTimer();
    void signal_function(FreqCollectPara* localColect);
    //void signal_functionsemd(FreqCollectPara* localColect);
    int waiting_function(int timeout_seconds, FreqCollectPara* localColect);
    void saveUpdateFile(const QString& data);
    void checkUpdateRK();
    bool verifyFileMd5(const QString& filePath, const QString& expectedMd5);
    int SetSysTimeLocal(const QString& datetime);
    void reconnect4gNet();
    int SuperSystem(const QString& cmd);
    void SystemReset();
    int setupzigbeeparam();
protected:
    std::atomic_bool    m_opened;
    FreqCollectPara m_localPara;
    QString m_orgName;
    QString m_devID;
    QString m_mqUrl;
    int m_port;
    QString m_mqUser;
    QString m_mqPassword;
    QMqttClient*    m_mqttClient;
    std::atomic_bool    m_connected;
    FRESysParam m_sysparam;
    UpdateInfo m_updateinfo;
    HttpUpgradeInfo m_httpUpdateinfo;
    QString m_deviceTopic;
    QString m_uploadTopic;
    QString m_ParaTime;
    QString m_System;
    pthread_mutex_t uart_mutex;
    uartConfig4g5g m_uart;
    NetworkConf m_networkConf;
    int m_discount;
    int m_mqttTimer;
    int m_mqttTimernew;
    int m_timeOutTimer;
    int m_pos;
    QThread m_UploadThread;
    TCPParam * m_param;
    ZIGBEEParam * m_ZIGBEEParam;
    mqtthandle *m_handle;
    AdcHandle *m_adhanle;
    virtual void timerEvent(QTimerEvent* event);
public slots:
    void on_connected();
    void on_disconnected();
    void on_messageReceived(const QByteArray &message, const QMqttTopicName &topic = QMqttTopicName());
    void on_publishADCData(const unsigned char * data, const int len, const int flag);
    bool sendMqMessage(const QString& topic, const QJsonValue& data, const int& flag);
    void setdevID(const QString& devid);
    void setUpdateInfo();
signals:
    // 发送信号触发线程
    void operate(QMqttClient*, FRESysParam*, FreqCollectPara*, AdcHandle*,TCPParam *);
    void updateHttp(const HttpUpgradeInfo &info);
    void setLedTime(int set);
    void setmqttUpdateTimeSignal(int index,int wave,int longwave);
    void restartMqtt();
};

#endif // QMQTTCLIENT_H
