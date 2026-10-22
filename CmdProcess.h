#ifndef CMDPROCESS_H
#define CMDPROCESS_H

#include <QObject>
#include <QProcess>


class CmdProcess : public QObject
{
    Q_OBJECT

public:
    template < class String, class Receiver, class Slot >
    static bool execute(const QString &name, const String &cmd, Receiver receiver, Slot s)
    {
        CmdProcess* cmdProcess = new CmdProcess();
        cmdProcess->setName(name);
        connect(cmdProcess, &CmdProcess::cmdFinished, receiver, s);
        connect(cmdProcess, &CmdProcess::finished, cmdProcess, &QObject::deleteLater);
        return cmdProcess->execute(cmd);
    }

    template < class Receiver, class Slot >
    static bool execute(const QString &name, const QString& program, const QStringList& arguments, Receiver receiver, Slot s)
    {
        CmdProcess* cmdProcess = new CmdProcess();
        cmdProcess->setName(name);
        connect(cmdProcess, &CmdProcess::cmdFinished, receiver, s);
        connect(cmdProcess, &CmdProcess::finished, cmdProcess, &QObject::deleteLater);
        return cmdProcess->execute(program, arguments);
    }

public:
    explicit CmdProcess(QObject* parent = NULL);
    virtual ~CmdProcess();

    void setName(const QString& name);
    const QString& getName() const;

    bool execute(const QString& cmd);
    bool execute(const std::string& cmd);
    bool execute(const QString& program, const QStringList& arguments);

    const QString& getCmd() const;

    void close();

    bool isRunning();

signals:
    /// exitCode < 0 表示失败.
    void cmdFinished(const QString& name, int exitCode);

    void finished();

protected:
    void run();

private slots:
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);

protected:
    QString m_name;
    QString m_cmd;
    QProcess*   m_process;

};

#endif // CmdProcess_H
