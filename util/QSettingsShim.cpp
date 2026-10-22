#include "QSettingsShim.h"

QSettingsShim::QSettingsShim(QSettings& settings):
    m_settings(settings)
{

}

bool QSettingsShim::getString(const QString& key, QString& value)
{
    if (!m_settings.contains(key))
    {
        return false;
    }

    value = m_settings.value(key).toString();
    return true;
}

bool QSettingsShim::getString(const QString& key, std::string& value)
{
    if (!m_settings.contains(key))
    {
        return false;
    }

    value = m_settings.value(key).toString().toStdString();
    return true;
}

bool QSettingsShim::setString(const QString& key, const QString& value)
{
    m_settings.setValue(key, value);
    return true;
}

bool QSettingsShim::setString(const QString& key, const std::string& value)
{
    m_settings.setValue(key, QString::fromStdString(value));
    return true;
}