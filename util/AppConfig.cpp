#include "AppConfig.h"
#include <QSettings>
#include <QCoreApplication>
#include <memory>


QString AppConfig::s_filepath;

typedef std::shared_ptr< QSettings >    QSettingsPtr;
static QSettingsPtr s_settings;


AppConfig::AppConfig()
{

}

QString AppConfig::getFilePath()
{
	return s_filepath;
}

void AppConfig::setFilePath(const QString& filepath)
{
    s_filepath = filepath;

    s_settings.reset(new QSettings(s_filepath, QSettings::IniFormat));
    s_settings->setIniCodec("utf-8");
    s_settings->setAtomicSyncRequired(true);
}

bool AppConfig::getString(const QString& key, QString& value)
{
    if (!s_settings)
	{
		return false;
	}

    if (!s_settings->contains(key))
    {
        return false;
    }

	QVariant variant = s_settings->value(key);
	if (variant.isValid() && !variant.isNull())
	{ 
		value = variant.toString();
	}
    
    return true;
}

bool AppConfig::setString(const QString& key, const QString& value)
{
    if (!s_settings)
	{
		return false;
	}

    s_settings->setValue(key, value);
    s_settings->sync();
    return true;
}

bool AppConfig::getString(const QString& key, std::string& value)
{
    bool got = false;
    QString str;
    if (getString(key, str))
    {
        value = str.toStdString();
        got = true;
    }
    return got;
}

bool AppConfig::setString(const QString& key, const std::string& value)
{
    return setString(key, QString::fromStdString(value));
}
