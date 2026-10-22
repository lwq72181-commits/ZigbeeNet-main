#ifndef NTPSYNCHRO_H
#define NTPSYNCHRO_H

#include "BasicType.h"
#include <QString>
#include <QObject>
#include <string>
#include "TCriticalSection.h"


class NtpSynchro : public QObject
{
    Q_OBJECT

public:
    enum NtpCommand
    {
        kNtpd = 0,
        kNtpClient
    };

    static void setNtpCommand(NtpCommand cmd);

    static NtpCommand s_ntpCommand;

public:
    explicit NtpSynchro(QObject* parent=NULL);
    virtual ~NtpSynchro();

    bool isWorking();

    void close();

public slots:
    void sync(const QString& server);

signals:
    void ntpFinished(int exitCode);

protected slots:
    void cmdFinished(const QString& name, int exitCode);

protected:

    void setWorking(bool working);

    QString makeCommandLine(NtpCommand cmd, const QString& server);

protected:
    comn::CriticalSection   m_cs;
    bool    m_working;


};

#endif // NTPSYNCHRO_H
