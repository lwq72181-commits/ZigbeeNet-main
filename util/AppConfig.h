#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "BasicType.h"
#include <stddef.h>
#include <string>
#include <sstream>
#include <cstring>
#include <QString>
#include "HexString.h"
#include <QTime>


class AppConfig
{
public:
    AppConfig();

	static QString getFilePath();

    static void setFilePath(const QString& filepath);

    static bool getString(const QString& key, QString& value);
    static bool getString(const QString& key, std::string& value);

    static bool setString(const QString& key, const QString& value);
    static bool setString(const QString& key, const std::string& value);

    template < class T >
    static bool get(const QString& key, T& t)
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

    static bool get(const QString &key, QString & str)
    {
        return getString(key, str);
    }

    static bool get(const QString &key, QByteArray & ba)
    {
        QString str;
        if (!getString(key, str))
        {
            return false;
        }

        ba = HexString::fromHex(str);
        return true;
    }

    static bool get(const QString &key, std::string & str)
    {
        QString value;
        if (!getString(key, value))
        {
            return false;
        }

        str = value.toStdString();
        return true;
    }

    static bool get(const QString& key, bool& t)
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

    static bool get(const QString &key, QTime& t)
    {
        QString str;
        if (!get(key, str))
        {
            return false;
        }

        bool found = false;
        QTime time = QTime::fromString(str, "hh:mm:ss");
        if (time.isValid())
        {
            t = time;
            found = true;
        }
        return found;
    }




    template < class T >
    static bool set(const QString& key, const T& t)
    {
        std::ostringstream oss;
        oss << t;
        QString qstr = QString::fromStdString(oss.str());
        return setString(key, qstr);
    }

    static bool set(const QString &key, const QString &t)
    {
        return setString(key, t);
    }

    static bool set(const QString &key, const QByteArray &t)
    {
        return setString(key, t.toHex());
    }

    static bool set(const QString &key, const QTime& t)
    {
        return setString(key, t.toString("hh:mm:ss"));
    }


    template < class T>
    static T optGet(const QString& key, const T& defValue)
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

	static QString optGet(const QString& key, const QString& defValue)
	{
		QString value;
		getString(key, value);
		if (value.isEmpty())
		{
			return defValue;
		}
		return value;
	}

    static QString optGet(const QString& key, const char* defValue)
    {
		QString value;
        getString(key, value);
		if (value.isEmpty())
		{
			return defValue;
		}
        return value;
    }

    static bool optGet(const QString& key, bool defValue)
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

    static QTime optGet(const QString& key, const QTime& defValue)
    {
        QTime value(defValue);
        get(key, value);
        return value;
    }

protected:
	static QString	s_filepath;

};

#endif // APPCONFIG_H
