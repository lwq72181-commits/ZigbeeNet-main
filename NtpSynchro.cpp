#include "NtpSynchro.h"
#include "CmdProcess.h"
#include "AppConst.h"
#include "QPath.h"
#include "TStringUtil.h"
#include "CLog.h"


NtpSynchro::NtpCommand NtpSynchro::s_ntpCommand = NtpSynchro::kNtpd;

void NtpSynchro::setNtpCommand(NtpCommand cmd)
{
    s_ntpCommand = cmd;
}



NtpSynchro::NtpSynchro(QObject* parent):
    QObject(parent),
    m_working()
{
}

NtpSynchro::~NtpSynchro()
{
}

void NtpSynchro::sync(const QString& server)
{
    if (server.isEmpty())
    {
        return;
    }

    if (isWorking())
    {
        return;
    }

    setWorking(true);

    QString cmdLine = makeCommandLine(s_ntpCommand, server);

    CmdProcess::execute(QString(), cmdLine, this, &NtpSynchro::cmdFinished);
}

void NtpSynchro::cmdFinished(const QString& name, int exitCode)
{
    qDebug("NtpSynchro::cmdFinished\n");

    setWorking(false);

    emit ntpFinished(exitCode);
}

bool NtpSynchro::isWorking()
{
    return m_working;
}

void NtpSynchro::setWorking(bool working)
{
    m_working = working;
}


QString NtpSynchro::makeCommandLine(NtpCommand cmd, const QString& server)
{
    std::string svr = server.toStdString();

    if (cmd == kNtpd)
    {
        std::string cmdLine = comn::StringUtil::format("ntpd -n -q -p %s", svr.c_str());
        return QString::fromStdString(cmdLine);
    }

    QString app = QPath::join(AppConst::appDirPath(), "ntpclient");
    std::string cmdLine = comn::StringUtil::format("%s -s -t -h %s", qPrintable(app), svr.c_str());
    return QString::fromStdString(cmdLine);
}

void NtpSynchro::close()
{
    //
}
