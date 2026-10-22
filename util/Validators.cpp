#include "Validators.h"
#include <QIntValidator>
#include <QRegExpValidator>
#include <limits>


Validators::Validators()
{

}

QValidator* Validators::port()
{
    return new QIntValidator(0, std::numeric_limits<uint16_t>::max());
}

QValidator* Validators::ip()
{
    QString pattern = "^(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|[1-9])\\."
            "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\."
            "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\."
            "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)$";

    return new QRegExpValidator(QRegExp(pattern));
}

QValidator* Validators::name(int minLength, int maxLength)
{
    QString pattern = QString("\\S{%1,%2}").arg(minLength).arg(maxLength);
    return new QRegExpValidator(QRegExp(pattern));
}

QValidator* Validators::digit(int minLength, int maxLength)
{
    QString pattern = QString("\\d{%1,%2}").arg(minLength).arg(maxLength);
    return new QRegExpValidator(QRegExp(pattern));
}

QValidator* Validators::number(int minValue, int maxValue)
{
    return new QIntValidator(minValue, maxValue);
}

QValidator* Validators::user(int minLength, int maxLength)
{
    QString pattern = QString("\\S{%1,%2}").arg(minLength).arg(maxLength);
    return new QRegExpValidator(QRegExp(pattern));
}
