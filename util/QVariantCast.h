#ifndef QVARIANTCAST_H
#define QVARIANTCAST_H

#include <QVariant>
#include "QStringUtil.h"

class QVariantCast
{
public:
    template < class T>
    static bool toValue(const QVariant& variant, T& value)
    {
        return false;
    }

    static bool toValue(const QVariant &variant, bool &value)
    {
        if (variant.type() == QVariant::Bool)
        {
            value = variant.toBool();
        }
        else if(variant.type() == QVariant::String)
        {
            QString str = variant.toString();
            QStringUtil::toValue(str, value);
        }
        else if (variant.isNull())
        {
            value = false;
        }
        else
        {
            value = variant.toBool();
        }

        return true;
    }

    static bool toValue(const QVariant &variant, int &value)
    {
        bool ok = false;
        int v = variant.toInt(&ok);
        if (ok)
        {
            value = v;
        }
        else if (variant.type() == QVariant::String)
        {
            QString str = variant.toString();
            QStringUtil::toValue(str, value);
            ok = true;
        }
        else if (variant.type() == QVariant::Int || variant.type() == QVariant::UInt)
        {
            value = variant.toInt();
        }
        else if (variant.type() == QVariant::Double)
        {
            value = variant.toDouble();
        }
        else if (variant.type() == QVariant::Bool)
        {
            value = variant.toBool();
        }
        else
        {
            QString str;
            toValue(variant, str);
            QStringUtil::toValue(str, value);
        }
        return ok;
    }


    static bool toValue(const QVariant &variant, double &value)
    {
        bool ok = false;
        double v = variant.toDouble(&ok);
        if (ok)
        {
            value = v;
        }
        else if (variant.type() == QVariant::String)
        {
            QString str = variant.toString();
            QStringUtil::toValue(str, value);
            ok = true;
        }
        else if (variant.type() == QVariant::Int || variant.type() == QVariant::UInt)
        {
            value = variant.toInt();
        }
        else if (variant.type() == QVariant::Int || variant.type() == QVariant::UInt)
        {
            value = variant.toInt();
        }
        else if (variant.type() == QVariant::Bool)
        {
            value = variant.toBool();
        }
        else
        {
            QString str;
            toValue(variant, str);
            QStringUtil::toValue(str, value);
        }
        return ok;
    }

    static bool toValue(const QVariant &variant, int64_t &value)
    {
        bool ok = false;
        int64_t v = variant.toLongLong(&ok);
        if (ok)
        {
            value = v;
        }
        else if (variant.type() == QVariant::String)
        {
            QString str = variant.toString();
            QStringUtil::toValue(str, value);
            ok = true;
        }
        else if (variant.type() == QVariant::Int || variant.type() == QVariant::UInt)
        {
            value = variant.toInt();
        }
        else if (variant.type() == QVariant::Double)
        {
            value = variant.toDouble();
        }
        else if (variant.type() == QVariant::Bool)
        {
            value = variant.toBool();
        }
        else
        {
            QString str;
            toValue(variant, str);
            QStringUtil::toValue(str, value);
        }
        return ok;
    }

    static bool toValue(const QVariant &variant, QString &value)
    {
        //QVariant::Type t = variant.type();

        if (variant.type() == QVariant::String)
        {
            value = variant.toString();
        }
        else if (variant.type() == QVariant::Int || variant.type() == QVariant::UInt)
        {
            value = QString::number(variant.toInt());
        }
        else if (variant.type() == QVariant::Double)
        {
            value = QString::number(variant.toDouble());
        }
        else if (variant.type() == QVariant::LongLong || variant.type() == QVariant::ULongLong)
        {
            value = QString::number(variant.toLongLong());
        }
        else if (variant.type() == QVariant::Bool)
        {
            value = QString::number(variant.toBool());
        }
        else if (variant.isNull())
        {
            value.clear();
        }

        return true;
    }

    static bool toValue(const QVariant &variant, std::string &value)
    {
        //QVariant::Type t = variant.type();

        if (variant.type() == QVariant::String)
        {
            value = variant.toString().toStdString();
        }
        else if (variant.type() == QVariant::Int || variant.type() == QVariant::UInt)
        {
            value = QString::number(variant.toInt()).toStdString();
        }
        else if (variant.type() == QVariant::Double)
        {
            value = QString::number(variant.toDouble()).toStdString();
        }
        else if (variant.type() == QVariant::LongLong || variant.type() == QVariant::ULongLong)
        {
            value = QString::number(variant.toLongLong()).toStdString();
        }
        else if (variant.type() == QVariant::Bool)
        {
            value = QString::number(variant.toBool()).toStdString();
        }
        else if (variant.isNull())
        {
            value.clear();
        }
        return true;
    }

};


#endif // QVARIANTCAST_H
