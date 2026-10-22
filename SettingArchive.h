#ifndef SETTINGARCHIVE_H
#define SETTINGARCHIVE_H

#include "BasicType.h"
#include <string>
#include <QString>


class SettingArchive
{
public:

public:
    SettingArchive();

    static bool save(const std::string& filepath);

    static bool load(const std::string& filepath);

    static void reset();

    static QString SETUP_DIR;

};

#endif // SETTINGARCHIVE_H
