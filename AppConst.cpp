#include "AppConst.h"
#include "Path.h"
#include "TextCodecUtil.h"
#include <QApplication>
#include "QPath.h"

int AppConst::HTTP_PORT = 8000;


//float    AppConst::DISK_SWITCH_SCALE = 0.90f;
//float    AppConst::DISK_FULL_SCALE = 0.95f;

int  AppConst::PAGE_SIZE = 25;

const char*  AppConst::DNS_SERVER1 = "223.5.5.5";
const char*  AppConst::DNS_SERVER2 = "114.114.114.114";


static const char* TMP_DIR = "/var";
static const char* STORAGE_DIR = "/var/disk";
static const char* USBDISK_DIR = "/var/disk/udisk";


QString  AppConst::s_appDirPath;
QString  AppConst::s_uploadPath = TMP_DIR;
QString  AppConst::s_rkVersion = "2.0.2";
//QString  AppConst::s_rkVersion = "1.0.22";

AppConst::AppConst()
{
}

void AppConst::setAppDirPath(const QString& dirpath)
{
    s_appDirPath = dirpath;
    s_uploadPath = getDefUploadDir();
}

std::string AppConst::getAppDir()
{
    QString dirpath = s_appDirPath;
    return TextCodecUtil::convert(dirpath);
}

QString AppConst::appDirPath()
{
    return s_appDirPath;
}

QString AppConst::getStorageDir()
{
    return STORAGE_DIR;
}

QString AppConst::getUsbDiskDir()
{
    return USBDISK_DIR;
}

QString AppConst::getEtcDir()
{
    return QPath::join("/userdata", "etc");
    //return QPath::join(appDirPath(), "etc");
}

QString AppConst::getVersion()
{
    return s_rkVersion;
    //return QPath::join(appDirPath(), "etc");
}

QString AppConst::getUploadDir()
{
    return s_uploadPath;
}

void AppConst::setUploadDir(const QString& dirpath)
{
    s_uploadPath = dirpath;
}

QString AppConst::getDefUploadDir()
{
    return QPath::join(TMP_DIR, "upload");
}
