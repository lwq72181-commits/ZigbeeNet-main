#include "CmdProcess.h"
#include "CLog.h"

CmdProcess::CmdProcess(QObject* parent):
    m_process()
{
}

CmdProcess::~CmdProcess()
{
    close();
}

void CmdProcess::setName(const QString& name)
{
    m_name = name;
}

const QString& CmdProcess::getName() const
{
    return m_name;
}

bool CmdProcess::execute(const QString& cmd)
{
    m_cmd = cmd;

    run();

    return true;
}

bool CmdProcess::execute(const std::string& cmd)
{
    return execute(QString::fromStdString(cmd));
}

bool CmdProcess::execute(const QString& program, const QStringList& arguments)
{
    QString cmd(program);
    if (arguments.size() > 0)
    {
        cmd += ' ';
        cmd += arguments.join(' ');
    }
    return execute(cmd);
}

const QString& CmdProcess::getCmd() const
{
    return m_cmd;
}

void CmdProcess::run()
{
    qDebug("CmdProcess::run. %s\n", qPrintable(m_cmd));

    if (m_process)
    {
        disconnect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished), this, &CmdProcess::onFinished);
        disconnect(m_process, &QProcess::errorOccurred, this, &CmdProcess::onErrorOccurred);

        delete m_process;
        m_process = NULL;
    }

    m_process = new QProcess(this);
    connect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished), this, &CmdProcess::onFinished);
    connect(m_process, &QProcess::errorOccurred, this, &CmdProcess::onErrorOccurred);

    m_process->start(m_cmd);

}

void CmdProcess::close()
{
    if (m_process)
    {
        m_process->terminate();

        m_process->deleteLater();
        m_process = NULL;
    }
}

bool CmdProcess::isRunning()
{
    return (m_process != nullptr);
}


void  CmdProcess::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("CmdProcess::finished. exitCode:%d\n", exitCode);

    emit cmdFinished(m_name, exitCode);

    close();

    emit finished();
}

void CmdProcess::onErrorOccurred(QProcess::ProcessError error)
{
    qDebug("CmdProcess::errorOccurred. error:%d\n", error);

    emit cmdFinished(m_name, -1);

    close();

    emit finished();
}
