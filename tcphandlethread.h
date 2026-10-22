#ifndef TCPHANDLETHREAD_H
#define TCPHANDLETHREAD_H
#include <QCoreApplication>
#include <QThread>
#include <QObject>
#include <QDebug>
#include <QJsonObject>
#include "NetworkConf.h"
#include "JsonFileHandle.h"
#include "Acquisitiontype.h"
#include "ThreadSafeMultiVector.h"
class TCPHandleThread : public QObject//: public QThread
{
    Q_OBJECT
public:
    //explicit TCPHandleThread(int id, QObject *parent = nullptr);
    TCPHandleThread(int id, TCPParam *param,ThreadSafeMultiVector *vector);
    int handle_recv_data(char *buf, int len, int fd);
    int check_cmd_data(char *cmd);
    int judge_ask_data(char *buf, int len);
    int setSysNetwork(unsigned char *net);
    int packReplayBuff(char *replay_buf,char flag);
    int replay_ask_data(char *replay_buf, int len, int fd);
    int setMqttInfo(char *net, int len);
    int setMqttServer(unsigned char *net);
    int setAcquistInfo(char *net);
    int replyLocalNetInfo(unsigned char *replay_buf);
    int replySNInfo(char *replay_buf);
    unsigned short CRC16_MODBUS(unsigned char *buf, int len);
    int handle_gate_data(unsigned char* buf, int fd);
    int replay_package_info(int fd);
    int storeIndexData(unsigned char* buf, int len);
    void replay_recved_index(int count, int fd);
    void replay_recved_over(int fd);
    int storeWaveData(unsigned char* buf, int len);
    void applySaveWave(int type);
    int storeLWaveData(unsigned char* buf, int len);
    void dorun();
    int initMVector(int count);
protected:
    //void run() override;

private:
    int m_fd;
    JSONFileHandle m_JSONHandle;
    NetworkConf m_networkConf;
    TCPParam * m_param;
    ThreadSafeMultiVector *m_vector;
    int m_sn;
    uint16_t m_len;
    uint16_t m_freeq;
    int m_indexTime;
    int m_waveTime;
    int m_lwaveTime;
    int m_set;
    int m_length;
    int m_count;
    int m_zigbeechannel;
    int m_nu[2048];
    int m_sec;
    char m_saved;
    char m_type;
    unsigned char m_cmd[1022];
    QString m_time;
    QJsonArray m_baseArray;
public slots:
    void SetTimeperiod(int index,int wave,int longwave);
signals:
    void MqttSignal();
};

#endif // TCPHANDLETHREAD_H
