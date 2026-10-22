#ifndef MQTTHANDLE_H
#define MQTTHANDLE_H
#include <QObject>
#include<QDebug>
#include <QUrl>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimerEvent>
#include <QJsonObject>
#include <QJsonArray>
#include "Acquisitiontype.h"
#include "QtMqtt/qmqttclient.h"
//#include"qmqttclient.h"
#include "AdcHandle.h"
#include "JsonFileHandle.h"
class mqtthandle :public QObject
{
    Q_OBJECT
public:
    mqtthandle();
    int sendMqttIndexData(QMqttClient* mqttClient,FreqCollectPara* localColect, FRESysParam* localPara, qint64 *startsec);
    int sendMqttWaveData(QMqttClient* mqttClient,FreqCollectPara* localColect, FRESysParam* localPara, int flag, qint64 *startsec);
    bool sendMqMessage(QMqttClient* mqttClient,QString& topic, QJsonObject* data);
    int sendMqttERRORNO(FreqCollectPara* localColect,int errcode);
    int waiting_function(int timeout_seconds, FreqCollectPara* localColect);
    int waiting_functionsend(int timeout_seconds, FreqCollectPara* localColect);
    void signal_function(FreqCollectPara* localColect);
    int sendStoredWave(FreqCollectPara* localColect, FRESysParam* localPara, qint64 *startsec, int flag);
    int sendStoredIndex(FreqCollectPara* localColect, FRESysParam* localPara, qint64 *startsec);
    int testPackMqttData(QMqttClient* mqttClient,FreqCollectPara* localColect, FRESysParam* localPara);
    void checkNetwork();
    FREWaveValue m_highwave;
    FREWaveValue m_lowwave;
private:
    QString intArrayToBase64(int* intArray, int len);
    double calculateRMS(double *data, int n);
    uint GetCRC32(QString bytes);
    int divideCeilToInt(int numerator, int denominator) ;
    QList<FREIndexValue> m_IndexList;
    QString m_uploadTopic;
    JSONFileHandle m_JSONHandle;
    TCPParam *m_param;
    QString m_waveName;
    QString m_LwaveName;
    bool m_sendwflag;
    bool m_sendlflag;
public slots:
     // doWork定义了线程要执行的操作
    void dowork(QMqttClient* mqttClient, FRESysParam* localPara, FreqCollectPara* localColect,AdcHandle* adhanle_l,TCPParam * param);
// 线程完成工作时发送的信号
signals:
    void resultReady(const int result);
    bool sendsig(const QString& topic, const QJsonValue& data, const int& flag);
};

#endif // MQTTHANDLE_H
