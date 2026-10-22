#ifndef APPCONST_H
#define APPCONST_H

#include <string>
#include <QString>

class AppConst
{
public:
    AppConst();

	static int AUDIO_CHANNELS;
	static int AUDIO_SAMPLERATE;

	static const int THREAD_STACK_SIZE = 1024 * 1024;

    static const int WEBSOCKET_PORT = 9000;
    static const int DOWNLOAD_RETRY_INTERVAL = 10;
    static int MSG_PORT;
    static int RTSP_PORT;
    static int HTTP_PORT;

    static float    DISK_SWITCH_SCALE;      /// 切换磁盘阈值.
    static float    DISK_FULL_SCALE;        /// 磁盘满阈值.

    static int		PAGE_SIZE;


    static const char*  DNS_SERVER1;
    static const char*  DNS_SERVER2;


    static void setAppDirPath(const QString& dirpath);

    static std::string getAppDir();
    static QString appDirPath();

    static QString getStorageDir();

    static QString getUsbDiskDir();

    static QString getEtcDir();
    static QString getVersion();

    static QString getUploadDir();
    static void setUploadDir(const QString& dirpath);

    static QString getDefUploadDir();

protected:
    static QString  s_appDirPath;
    static QString  s_uploadPath;
    static QString  s_rkVersion;

};

#endif // APPCONST_H
