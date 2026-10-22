#include "AppLog.h"

#include <QMutex>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include "CLog.h"



QtMessageHandler AppLog::s_prevHandler = NULL;


void AppLog::fileMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    int level = CLog::kDebug;
    if (type == QtWarningMsg)
    {
        level = CLog::kWarning;
    }
    else if (type == QtCriticalMsg)
    {
        level = CLog::kError;
    }
    else if (type == QtFatalMsg)
    {
        level = CLog::kFatal;
    }

    std::string text = msg.toStdString();
    text += "\n";
    CLog::writeLog(level, text.c_str(), text.length());

    if (s_prevHandler)
    {
        (s_prevHandler)(type, context, msg);
    }
}


AppLog::AppLog()
{

}

void AppLog::setup()
{
    if (s_prevHandler == NULL)
    {
        s_prevHandler = qInstallMessageHandler(fileMessageHandler);
    }
}

void AppLog::cleanup()
{
    qInstallMessageHandler(0);
}
