#include "SettingArchive.h"
#include "AppConst.h"
#include "Path.h"
#include "TStringUtil.h"
#include "QFileUtil.h"
#include <QPath.h>


SettingArchive::SettingArchive()
{

}

bool SettingArchive::save(const std::string& filepath)
{
    std::string etcDir = AppConst::getEtcDir().toStdString();
    std::string appDir = comn::Path::dirname(etcDir);

    std::string cmd = comn::StringUtil::format(
        "cd %s && tar -czf %s etc &",
        appDir.c_str(), filepath.c_str(), etcDir.c_str());
    system(cmd.c_str());
    return true;
}

bool SettingArchive::load(const std::string& filepath)
{
    std::string etcDir = AppConst::getEtcDir().toStdString();

    std::string cmd = comn::StringUtil::format(
        "cd /var/ && tar -zxf %s && cp -rf /var/etc/* %s && rm -rf /var/etc &",
         filepath.c_str(), etcDir.c_str());
    system(cmd.c_str());
    return true;
}

void SettingArchive::reset()
{
    QString etcDir = AppConst::getEtcDir();
    QFileUtil::remove(etcDir);

    QPath::createDirectories(etcDir);

    QString src = QPath::join(AppConst::appDirPath(), SETUP_DIR);
    QFileUtil::copySub(src, etcDir, true);
}

QString SettingArchive::SETUP_DIR = "setup";
