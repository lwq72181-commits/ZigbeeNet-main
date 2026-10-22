#include "QtTimeUtil.h"
#include <QDateTime>
#include "TStringUtil.h"


QtTimeUtil::QtTimeUtil()
{

}

QString QtTimeUtil::format(time_t t)
{
    QDateTime dt = QDateTime::fromTime_t(t);
    QString fmt = "yyyy-MM-dd hh:mm:ss";
    return dt.toString(fmt);
}

bool QtTimeUtil::parse(const QString& str, time_t& t)
{
    QDateTime dt = QDateTime::fromString(str, "yyyy-MM-dd hh:mm:ss");
    t = dt.toSecsSinceEpoch();
    return true;
}

QString QtTimeUtil::formatDuration(int duration)
{
    int seconds = duration % 60;
    int minutes = (duration / 60) % 60;
    int hours = duration / 3600;
    std::string text = comn::StringUtil::format("%02d:%02d:%02d", hours, minutes, seconds);
    return QString::fromStdString(text);
}
