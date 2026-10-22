#ifndef QSETTINGSSHIM_H
#define QSETTINGSSHIM_H

#include "BasicType.h"
#include <QSettings>
#include <string>
#include <stddef.h>
#include <string>
#include <sstream>
#include <cstring>
#include "HexString.h"

class QSettingsShim
{
public:
    QSettingsShim(QSettings& settings);

    bool getString(const QString& key, QString& value);
    bool getString(const QString& key, std::string& value);

    bool setString(const QString& key, const QString& value);
    bool setString(const QString& key, const std::string& value);

    template < class T >
    bool get(const QString& key, T& t)
    {
        QString value;
        if (!getString(key, value))
        {
            return false;
        }

        std::string str = value.toStdString();
        std::istringstream iss(str);
        iss >> t;
        return true;
    }

    bool get(const QString &key, QString & str)
    {
        return getString(key, str);
    }

    bool get(const QString &key, QByteArray & ba)
    {
        QString str;
        if (!getString(key, str))
        {
            return false;
        }

        ba = HexString::fromHex(str);
        return true;
    }

    bool get(const QString &key, std::string & str)
    {
        QString value;
        if (!getString(key, value))
        {
            return false;
        }

        str = value.toStdString();
        return true;
    }

    bool get(const QString& key, bool& t)
    {
        QString str;
        if (!get(key, str))
        {
            return false;
        }

        if (str.isEmpty())
        {
            t = false;
        }
        else if (str == "0" || str == "false" || str == "False" || str == "FALSE")
        {
            t = false;
        }
        else
        {
            t = true;
        }
        return true;
    }





    template < class T >
    bool set(const QString& key, const T& t)
    {
        std::ostringstream oss;
        oss << t;
        QString qstr = QString::fromStdString(oss.str());
        return setString(key, qstr);
    }

    bool set(const QString &key, const QString &t)
    {
        return setString(key, t);
    }

    bool set(const QString &key, const QByteArray &t)
    {
        return setString(key, t.toHex());
    }


    template < class T>
    T optGet(const QString& key, const T& defValue)
    {
        T value(defValue);
        QString str;
        if (get(key, str))
        {
            std::istringstream iss(str.toStdString());
            iss >> value;
        }
        return value;
    }

    std::string optGet(const QString& key, const char* defValue)
    {
        std::string value(defValue);
        QString str;
        if (get(key, str))
        {
            value = str.toStdString();
        }
        return value;
    }

    bool optGet(const QString& key, bool defValue)
    {
        bool value(defValue);
        QString str;
        if (get(key, str))
        {
            if (str.isEmpty())
            {
                value = false;
            }
            else if (str == "0" || str == "false" || str == "False" || str == "FALSE")
            {
                value = false;
            }
            else
            {
                value = true;
            }
        }
        return value;
    }

protected:
    QSettings&  m_settings;
};

#endif // QSETTINGSSHIM_H
