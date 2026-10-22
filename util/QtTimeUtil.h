#ifndef QTTIMEUTIL_H
#define QTTIMEUTIL_H

#include <QString>

class QtTimeUtil
{
public:
    QtTimeUtil();

    static QString format(time_t t);

    static bool parse(const QString& str, time_t& t);

    /// 单位为秒.
    static QString formatDuration(int duration);

};

#endif // QTTIMEUTIL_H
