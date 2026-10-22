#ifndef NTPSERVICE_H
#define NTPSERVICE_H

#include <QObject>
#include <QString>
#include "NtpSynchro.h"


class NtpService : public QObject
{
    Q_OBJECT
public:
    explicit NtpService(QObject *parent = nullptr);

    void setServer(const QString& server);
    void setEnabled(bool enabeld);

    void setCycle(int seconds);

    void trySync();

    void setupNtpClient();

signals:


protected:
    virtual void timerEvent(QTimerEvent *event) override;

protected slots:
    void on_ntpFinsihed(int exitCode);


protected:
    void startNtpTimer();
    void stopNtpTimer();



protected:
    NtpSynchro  m_ntpSynchro;
    QString m_server;
    bool    m_enabled;
    int     m_cycle;

    int     m_timerId;

};

#endif // NTPSERVICE_H
