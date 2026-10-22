#include "AppDir.h"
#include "AppConst.h"
#include "DateTime.h"
#include "QPath.h"
#include "AppConfig.h"
#include <QDir>


AppDir::AppDir()
{
}


QString AppDir::getAppDir()
{
    return "/userdata";
    QString dirpath = AppConst::appDirPath();
    if (dirpath.isEmpty())
    {
        dirpath = QPath::getWorkDir();
    }
    return dirpath;
}

QString AppDir::getEtcDir()
{
    return AppConst::getEtcDir();
}

QString AppDir::getUploadDir()
{
    return AppConst::getUploadDir();
}


QString AppDir::getUsbDiskDir()
{
    return AppConst::getUsbDiskDir();
}

QString AppDir::getLogDir()
{
    QString dir = getAppDir();
    return QPath::join(dir, "log");
}
