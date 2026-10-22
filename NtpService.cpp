#include "NtpService.h"
#include <QTimerEvent>
#include "CLog.h"
#include "QPath.h"
#include "AppConst.h"


NtpService::NtpService(QObject *parent) : QObject(parent),
    m_server(),
    m_enabled(),
    m_cycle(),
    m_timerId()
{
    connect(&m_ntpSynchro, &NtpSynchro::ntpFinished, this, &NtpService::on_ntpFinsihed);
}

void NtpService::setServer(const QString& server)
{
    m_server = server;
}

void NtpService::setEnabled(bool enabeld)
{
    m_enabled = enabeld;

    if (m_cycle > 0)
    {
        startNtpTimer();
    }
}

void NtpService::setCycle(int seconds)
{
    if (seconds > 0)
    {
        m_cycle = seconds;

        if (m_enabled)
        {
            stopNtpTimer();
            startNtpTimer();
        }
    }
}

void NtpService::trySync()
{
    if (m_server.size() > 0)
    {
        m_ntpSynchro.sync(m_server);
    }
}

void NtpService::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId)
    {
        trySync();
    }
}

void NtpService::on_ntpFinsihed(int exitCode)
{
    qDebug("ntpFinished.\n");
}

void NtpService::stopNtpTimer()
{
    if (m_timerId > 0)
    {
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

void NtpService::startNtpTimer()
{
    if (m_timerId == 0)
    {
        m_timerId = startTimer(m_cycle * 1000);
    }
}

void NtpService::setupNtpClient()
{
    QString cmdPath = QPath::join(AppConst::appDirPath(), "ntpclient");
    if (QPath::exist(cmdPath))
    {
        NtpSynchro::setNtpCommand(NtpSynchro::kNtpClient);
    }
    else
    {
        qDebug("ntpclient does NOT exist. path: %s\n", qPrintable(cmdPath));
    }
}
