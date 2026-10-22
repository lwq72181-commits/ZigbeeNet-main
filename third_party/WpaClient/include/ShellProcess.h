#ifndef SHELLPROCESS_H
#define SHELLPROCESS_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class ShellProcess : public QObject
{
    Q_OBJECT

public:
    enum Error
    {
        ERROR_Ok = 0,
        ERROR_CrashExit,
        ERROR_FAILED
    };

    template < class String, class Receiver, class Slot >
    static bool execute(const String &cmd, Receiver receiver, Slot s)
    {
        ShellProcess* process = new ShellProcess();
        connect(process, &ShellProcess::finished, receiver, s);
        connect(process, &ShellProcess::closed, process, &QObject::deleteLater);
        return process->execute(cmd);
    }

    template < class Receiver, class Slot >
    static bool execute(const QString& program, const QStringList& arguments, Receiver receiver, Slot s)
    {
        ShellProcess* process = new ShellProcess();
        connect(process, &ShellProcess::finished, receiver, s);
        connect(process, &ShellProcess::closed, process, &QObject::deleteLater);
        return process->execute(program, arguments);
    }


    static bool read(const QString& cmd, QStringList& lines);

public:
    explicit ShellProcess(QObject *parent = nullptr);

    ~ShellProcess();

signals:
    void finished(int error, const QStringList& lines);

    void closed();

public slots:
    bool execute(const QString& cmd);
    bool execute(const QString& program, const QStringList& arguments);

    void close();

    bool isRunning();


protected:
    void run();

private slots:
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);
    void on_readyReadStandardOutput();

protected:
    QString m_cmd;
    QProcess*   m_process;
    QStringList m_lines;

};

#endif // SHELLPROCESS_H
