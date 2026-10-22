#ifndef INIFILEUTIL_H
#define INIFILEUTIL_H

#include "IniFile.h"
#include <QString>
#include <QRect>


class IniFileUtil
{
public:
    IniFileUtil();

    static bool queryValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, QString& value);
    static bool queryValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, QByteArray& value);

    static void setValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, const QString& value);
    static void setValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, const QByteArray& value);

    static void setValue(comn::IniFile &iniFile, const char *sectionName, const char *keyName, const QRect &value);
    static bool queryValue(comn::IniFile& iniFile, const char* sectionName, const char* keyName, QRect& value);

    static bool queryValue(comn::IniFile::IniSection& section, const char *keyName, QString &value);

};

#endif // INIFILEUTIL_H
