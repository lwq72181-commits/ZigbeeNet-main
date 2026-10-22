#ifndef APPLOG_H
#define APPLOG_H

#include <QMessageLogger>

class AppLog
{
public:
    AppLog();

    static void fileMessageHandler(QtMsgType, const QMessageLogContext &, const QString &);

    static void setFile(const char* fileName);

    static void setup();
    static void cleanup();

protected:
    static QtMessageHandler s_prevHandler;

};

#endif // APPLOG_H
