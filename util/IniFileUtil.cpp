#include "IniFileUtil.h"
#include "HexString.h"
#include "TStringUtil.h"


IniFileUtil::IniFileUtil()
{
}

bool IniFileUtil::queryValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, QString& value)
{
    std::string str;
    if (!iniFile.queryValue(sectionName, keyName, str))
    {
        return false;
    }

    value = QString::fromStdString(str);
    return true;
}

bool IniFileUtil::queryValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, QByteArray &value)
{
    std::string str;
    if (!iniFile.queryValue(sectionName, keyName, str))
    {
        return false;
    }

    value = HexString::fromHex(QString::fromStdString(str));
    return true;
}

void IniFileUtil::setValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, const QString& value)
{
    iniFile.setValue(sectionName, keyName, value.toStdString());
}

void IniFileUtil::setValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, const QByteArray& value)
{
    std::string str = HexString::toHex(value).toStdString();
    iniFile.setValue(sectionName, keyName, str);
}

void IniFileUtil::setValue(comn::IniFile &iniFile, const char *sectionName, const char *keyName, const QRect &value)
{
    std::string str = comn::StringUtil::format("[%d,%d,%d,%d]", value.left(), value.top(), value.width(), value.height());
    iniFile.setValue(sectionName, keyName, str);
}

bool IniFileUtil::queryValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, QRect& value)
{
    std::string str;
    if (!iniFile.queryValue(sectionName, keyName, str))
    {
        return false;
    }

    if (str.size() <= 2)
    {
        value = QRect();
    }
    else
    {
        str = str.substr(1, str.size()-2);
        std::vector< std::string > sections;
        comn::StringUtil::split(str, ',', sections);
        if (sections.size() >= 4)
        {
            int left = 0;
            int top = 0;
            int width = 0;
            int height = 0;
            comn::StringCast::toValue(sections[0], left);
            comn::StringCast::toValue(sections[1], top);
            comn::StringCast::toValue(sections[2], width);
            comn::StringCast::toValue(sections[3], height);
            value = QRect(left, top, width, height);
        }
        else
        {
            value = QRect();
        }
    }

    return true;
}

bool IniFileUtil::queryValue(comn::IniFile::IniSection& section, const char *keyName, QString &value)
{
    std::string str;
    if (!section.get(keyName, str))
    {
        return false;
    }

    value = QString::fromStdString(str);
    return true;
}
