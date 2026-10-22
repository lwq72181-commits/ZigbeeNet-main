#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <QThread>
#include <QObject>
#include <QDebug>
#include "tcphandlethread.h"
#include "Acquisitiontype.h"
#include "ThreadSafeMultiVector.h"
class TCPServer : public QObject
{
    Q_OBJECT
public:
    //TCPServer(QObject* parent = nullptr);
    TCPServer(TCPParam *param,ThreadSafeMultiVector*vector);
    void dorun();
    void TickTimePeriod(int index,int wave,int longwave);
signals:
    void restartMqttSignal();
    void setUpdateTimeSignal(int index,int wave,int longwave);
public slots:
    //void TickTimePeriod(int index,int wave,int longwave);
protected:
    TCPHandleThread *m_workerThread;
    QThread *m_thread;
    TCPParam *m_param;
    ThreadSafeMultiVector *m_vector;
    //void run() override;
};

#endif // TCPSERVER_H
