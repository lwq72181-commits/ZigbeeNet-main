#include "JsonFileHandle.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QDebug>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <QThread>
#include "Acquisitiontype.h"
JSONFileHandle::JSONFileHandle()
{
    initRecFloder();
}

int JSONFileHandle::initRecFloder()
{
    char floder[200] = {0};
    int ret = 0;
    if(access(FREQ_DIR,0) == -1)
    {
        if (mkdir(FREQ_DIR,0777))
        {
            qDebug("create floder_cam path err %s errno:%d",FREQ_DIR,errno);
            if(errno != 17)
            ret = -1;
        }
    }
    sprintf(floder,"%s%s",FREQ_DIR,SAVE_FOLDER);
    if(access(floder,0) == -1)
    {
        if (mkdir(floder,0777))
        {
            qDebug("create floder_alarm path err %s errno:%d",floder,errno);
            if(errno != 17)
            ret = -1;
        }
    }
    sprintf(floder,"%s%s",FREQ_DIR,SAVE_485_FOLDER);
    if(access(floder,0) == -1)
    {
        if (mkdir(floder,0777))
        {
            qDebug("create floder_alarm path err %s errno:%d",floder,errno);
            if(errno != 17)
            ret = -1;
        }
    }
    sprintf(floder,"%s%s",FREQ_DIR,SAVE_TEST);
    if(access(floder,0) == -1)
    {
        if (mkdir(floder,0777))
        {
            qDebug("create floder_alarm path err %s errno:%d",floder,errno);
            if(errno != 17)
            ret = -1;
        }
    }

    return ret;
}

QString JSONFileHandle::GetSaveFileName(int flag,int channel)
{
    QString fileName;
    //QDateTime currentTime = QDateTime::currentDateTimeUtc();
    QDateTime currentDateTime = QDateTime::currentDateTime();
    if(flag == 0)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'A'");
    if(flag == 1)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'B'");
    if(flag == 2)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'C'");
    QString fullname = QString("%1%2/%3%4").arg(FREQ_DIR).arg(SAVE_FOLDER).arg(fileName).arg(channel);
    qDebug("get store file name:%s",qPrintable(fullname));
    return fullname;
}

QString JSONFileHandle::GetSaveFileNameP(int flag,int channel,int sec)
{
    QString fileName;
    QDateTime currentDateTime = QDateTime::fromSecsSinceEpoch(sec);
    //QDateTime currentTime = QDateTime::currentDateTimeUtc();
    if(flag == 0)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'A'");
    if(flag == 1)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'B'");
    if(flag == 2)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'C'");
    QString fullname = QString("%1%2/%3%4").arg(FREQ_DIR).arg(SAVE_FOLDER).arg(fileName).arg(channel);
    qDebug("get store file name:%s",qPrintable(fullname));
    return fullname;
}

QString JSONFileHandle::GetSaveFileNameSTime(int flag,int channel,QString &time)
{
    QString fileName;
    //QDateTime currentDateTime = QDateTime::fromSecsSinceEpoch(sec);
    //QDateTime currentTime = QDateTime::currentDateTimeUtc();
    if(flag == 0)
    fileName = QString("%1B").arg(time);
    if(flag == 1)
    fileName = QString("%1C").arg(time);
    QString fullname = QString("%1%2/%3%4").arg(FREQ_DIR).arg(SAVE_FOLDER).arg(fileName).arg(channel);
    qDebug("get store file name:%s",qPrintable(fullname));
    return fullname;
}

QString JSONFileHandle::GetSaveFileNameUartP(int flag,int channel,int sec)
{
    QString fileName;
    QDateTime currentDateTime = QDateTime::fromSecsSinceEpoch(sec);
    //QDateTime currentTime = QDateTime::currentDateTimeUtc();
    if(flag == 0)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'A'");
    if(flag == 1)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'B'");
    if(flag == 2)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'C'");
    QString fullname = QString("%1%2/%3%4").arg(FREQ_DIR).arg(SAVE_485_FOLDER).arg(fileName).arg(channel);
    qDebug("get store file name:%s",qPrintable(fullname));
    return fullname;
}

QString JSONFileHandle::GetSaveFileNameTestP(int flag,int channel,int sec)
{
    QString fileName;
    QDateTime currentDateTime = QDateTime::fromSecsSinceEpoch(sec);
    //QDateTime currentTime = QDateTime::currentDateTimeUtc();
    if(flag == 0)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'A'");
    if(flag == 1)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'B'");
    if(flag == 2)
    fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'C'");
    QString fullname = QString("%1%2/%3%4").arg(FREQ_DIR).arg(SAVE_TEST).arg(fileName).arg(channel);
    qDebug("get store file name:%s",qPrintable(fullname));
    return fullname;
}

int JSONFileHandle::super_system(const char * cmd, char *retmsg, int msg_len)
{
        FILE * fp;
        if (cmd == NULL || retmsg == NULL || msg_len < 0)
        {
                qDebug("Err: Fuc: system paramer invalid!");
                return -1;
        }
        if ((fp = popen(cmd, "r") ) == NULL)
        {
                perror("popen");
                qDebug("Err: Fuc: popen error: ");
                return -2;
        }
        else
        {
                memset(retmsg, 0, msg_len);
                fgets(retmsg, msg_len, fp);
                qDebug("read:%s,len:%d",retmsg,msg_len);
//                while(fgets(retmsg, msg_len, fp));
//                {
//                        _DEBUG("read:%s,len:%d",retmsg,msg_len);
//                }
                //drop #012 from system result retmsg.
                if(strlen(retmsg) <1)
                {
                    qDebug("find file name is null!!!\n");
                    pclose(fp);
                    return -3;
                }
                retmsg[strlen(retmsg)-1] = '\0';
                int result = pclose(fp); // 关闭子进程并获取其退出状态
                if (result == -1) {
                    qDebug("pclose failed");
                    return -4;
                }

                return 0;
        }
}

float JSONFileHandle::get_partation_percent(char * partation)
{
    unsigned long long blocksize;
    unsigned long long totalsize;
    unsigned long long availableDisk;
    float freeper = 0;
    struct statfs diskInfo;
    statfs(partation, &diskInfo);
    blocksize = diskInfo.f_bsize;
    totalsize = blocksize * diskInfo.f_blocks;
    availableDisk = diskInfo.f_bavail * blocksize;
    freeper = (float)availableDisk/(float)totalsize;
    return freeper;
}

int JSONFileHandle::rm_oldest_day_file()
{
    char dir[100] = {0};
    char cmd[200] = {0};
    char retmsg[50] = {0};
    int ret = 0;
    sprintf(dir,"%s%s",FREQ_DIR,SAVE_FOLDER);
    sprintf(cmd,"ls %s -rt |sed -n '1p'",dir);
    super_system(cmd, retmsg, sizeof(retmsg));
    if(strlen(retmsg)>0)
    {
        sprintf(dir,"%s%s/%s",FREQ_DIR,SAVE_FOLDER,retmsg);
        if(access(dir,0) == -1)
        {
            //notexist
            qDebug("can not access the floder!!!:%s  errno:%d",dir,errno);
            ret = -1;
        }
        else
        {
            char *ptr = strtok(dir, "_");
            if(ptr != NULL)
            {
                sprintf(cmd,"rm -Rf %s*",ptr);
                qDebug("%s",cmd);
                super_system(cmd, retmsg, sizeof(retmsg));
            }
        }
    }
    else
    {
        qDebug("can not find the oldest floder!!!");
        ret = -2;
    }
    return ret;
}

int JSONFileHandle::rm_oldest_day_485file()
{
    char dir[100] = {0};
    char cmd[200] = {0};
    char retmsg[50] = {0};
    int ret = 0;
    sprintf(dir,"%s%s",FREQ_DIR,SAVE_485_FOLDER);
    sprintf(cmd,"ls %s -rt |sed -n '1p'",dir);
    super_system(cmd, retmsg, sizeof(retmsg));
    if(strlen(retmsg)>0)
    {
        sprintf(dir,"%s%s/%s",FREQ_DIR,SAVE_485_FOLDER,retmsg);
        if(access(dir,0) == -1)
        {
            //notexist
            qDebug("can not access the floder!!!:%s  errno:%d",dir,errno);
            ret = -1;
        }
        else
        {
            char *ptr = strtok(dir, "_");
            if(ptr != NULL)
            {
                sprintf(cmd,"rm -Rf %s*",ptr);
                qDebug("%s",cmd);
                super_system(cmd, retmsg, sizeof(retmsg));
            }
        }
    }
    else
    {
        qDebug("can not find the oldest floder!!!");
        ret = -2;
    }
    return ret;
}

bool JSONFileHandle::deleteFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.exists()) {
        if (file.remove()) {
            qDebug() << "File removed successfully.";
            return true;
        } else {
            qWarning() << "Failed to remove file:" << file.errorString();
            return false;
        }
    } else {
        qWarning() << "File does not exist.";
        return false;
    }
}

int JSONFileHandle::GetOldestFileName(QString &name, int flag)
{
    char dir[100] = {0};
    char cmd[200] = {0};
    char retmsg[50] = {0};
    int ret = 0;
    sprintf(dir,"%s%s",FREQ_DIR,SAVE_FOLDER);
    if(flag ==0)
        sprintf(cmd,"ls %s -rt | grep 'A' |sed -n '1p'",dir);
    else if(flag == 1)
        sprintf(cmd,"ls %s -rt | grep 'B' |sed -n '1p'",dir);
    else if(flag == 2)
        sprintf(cmd,"ls %s -rt | grep 'C' |sed -n '1p'",dir);
    else
        sprintf(cmd,"ls %s -rt | grep 'A' |sed -n '1p'",dir);
    super_system(cmd, retmsg, sizeof(retmsg));
    if(strlen(retmsg)>0)
    {
        sprintf(dir,"%s%s/%s",FREQ_DIR,SAVE_FOLDER,retmsg);
        if(access(dir,0) == -1)
        {
            //notexist
            qDebug("can not access the file!!!:%s  errno:%d",dir,errno);
            ret = -1;
        }
        else
        {
            name = QString::fromUtf8(dir);
            ret = 0;
        }
    }
    else
    {
        qDebug("can not find the oldest file!!!");
        ret = -2;
    }
    return ret;
}

int JSONFileHandle::GetOldestFileNameUart(QString &name, int flag)
{
    char dir[100] = {0};
    char cmd[200] = {0};
    char retmsg[50] = {0};
    int ret = 0;
    sprintf(dir,"%s%s",FREQ_DIR,SAVE_485_FOLDER);
    if(flag ==0)
        sprintf(cmd,"ls %s -rt | grep 'A' |sed -n '1p'",dir);
    else if(flag == 1)
        sprintf(cmd,"ls %s -rt | grep 'B' |sed -n '1p'",dir);
    else if(flag == 2)
        sprintf(cmd,"ls %s -rt | grep 'C' |sed -n '1p'",dir);
    else
        sprintf(cmd,"ls %s -rt | grep 'A' |sed -n '1p'",dir);
    super_system(cmd, retmsg, sizeof(retmsg));
    if(strlen(retmsg)>0)
    {
        sprintf(dir,"%s%s/%s",FREQ_DIR,SAVE_485_FOLDER,retmsg);
        if(access(dir,0) == -1)
        {
            //notexist
            qDebug("can not access the file!!!:%s  errno:%d",dir,errno);
            ret = -1;
        }
        else
        {
            name = QString::fromUtf8(dir);
            ret = 0;
        }
    }
    else
    {
        qDebug("can not find the oldest file!!!");
        ret = -2;
    }
    return ret;
}

bool JSONFileHandle::saveToJsonFile(const QJsonArray &array, const QString &filePath)
{
    QString midFile;
    QString midtime;
    QJsonDocument doc(array);
    QDateTime currentDateTime = QDateTime::currentDateTime();
    midtime = currentDateTime.toString("yyyyMMdd_hhmmsszzz");
    midFile = QString("%1%2/%3%4").arg(FREQ_DIR).arg(SAVE_FOLDER).arg(midtime);
    QFile file(midFile);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << file.errorString();
        return false;
    }
    QTextStream out(&file);
    out << doc.toJson(QJsonDocument::Indented);
    // 尝试刷新缓冲区
    if (!file.flush()) {
        qDebug() << "无法刷新文件缓冲区";
        QThread::sleep(1);
        if (!file.flush())
        {
            qDebug() << "无法刷新文件缓冲区1";
            file.remove();
            return false;
        }
    }
    int fd = file.handle();
    if (fd != -1 && fsync(fd) == -1) {
        qDebug() << "无法同步文件到磁盘2";
        if (QFile::exists(filePath))
        {
            file.remove();
            return false;
        }
        else
        {
            if(!file.rename(filePath))
            {
                file.remove();
            }
        }
    }
    else
    {
        if (QFile::exists(filePath))
        {
            file.remove();
            return false;
        }
        else
        {
            if(!file.rename(filePath))
            {
                file.remove();
                return false;
            }
        }
    }
//    file.close();


//    QFile file(filePath);
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        qWarning() << "Failed to open file for writing:" << file.errorString();
//        return false;
//    }

//    QTextStream out(&file);
//    out << doc.toJson(QJsonDocument::Indented);
//    file.close();
    return true;
}

int JSONFileHandle::saveToJsonFile(const QJsonObject &obj, const QString &filePath)
{
//    // 将 QJsonObject 封装到 QJsonDocument 中
//    QJsonDocument doc(obj);
//    // 打开文件用于写入
//    QFile file(filePath);
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        qWarning() << "Failed to open file for writing:" << file.errorString();
//        return -1;
//    }
//    // 将 QJsonDocument 序列化为字符串，并写入文件
//    QTextStream out(&file);
//    out << doc.toJson(QJsonDocument::Indented);
//    // 关闭文件
//    file.close();

    QString midFile;
    QString midtime;
    QJsonDocument doc(obj);
    QDateTime currentDateTime = QDateTime::currentDateTime();
    midtime = currentDateTime.toString("yyyyMMdd_hhmmsszzz");
    midFile = QString("%1%2/%3%4").arg(FREQ_DIR).arg(SAVE_FOLDER).arg(midtime);
    QFile file(midFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out << doc.toJson(QJsonDocument::Indented);
    // 尝试刷新缓冲区
    if (!file.flush()) {
        qDebug() << "无法刷新文件缓冲区";
        QThread::sleep(1);
        if (!file.flush())
        {
            qDebug() << "无法刷新文件缓冲区1";
            file.remove();
            return false;
        }
    }
    int fd = file.handle();
    if (fd != -1 && fsync(fd) == -1) {
        qDebug() << "无法同步文件到磁盘2";
        if (QFile::exists(filePath))
        {
            file.remove();
            return false;
        }
        else
        {
            if(!file.rename(filePath))
            {
                file.remove();
            }
        }
    }
    else
    {
        if (QFile::exists(filePath))
        {
            file.remove();
            return false;
        }
        else
        {
            if(!file.rename(filePath))
            {
                file.remove();
                return false;
            }
        }
    }

    return 0;
}

bool JSONFileHandle::saveToFile(const uint8_t *array,const int len, const QString &filePath)
{
//    QFile file(filePath);
//    if (!file.open(QIODevice::WriteOnly)) {
//        qWarning() << "Failed to open file for writing:" << file.errorString();
//        return false;
//    }
//    file.write(reinterpret_cast<const char*>(array), len);
//    file.close();

    QString midFile;
    QDateTime currentDateTime = QDateTime::currentDateTime();
    midFile = currentDateTime.toString("yyyyMMdd_hhmmsszzz");
    QFile file(midFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << file.errorString();
        return false;
    }
    file.write(reinterpret_cast<const char*>(array), len);
    // 尝试刷新缓冲区
    if (!file.flush()) {
        qDebug() << "无法刷新文件缓冲区";
        file.close();
        return false;
    }
    int fd = file.handle();
    if (fd != -1 && fsync(fd) == -1) {
        qDebug() << "无法同步文件到磁盘";
        file.rename(filePath);
        file.close();
        return false;
    }
    else
    {
        file.rename(filePath);
    }
    file.close();
    return true;
}

// 用于从文件读取 QJsonArray
QJsonArray JSONFileHandle::loadArrayFromJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for reading:" << file.errorString();
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON:" << parseError.errorString();
        return QJsonArray();
    }

    if (!doc.isArray()) {
        qWarning() << "JSON document is not an array.";
        return QJsonArray();
    }

    return doc.array();
}

QJsonObject JSONFileHandle::loadObjectFromJsonFile(const QString &filePath)
{
    // 打开文件用于读取
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for reading:" << file.errorString();
        return QJsonObject(); // 返回空对象
    }

    // 读取文件内容
    QByteArray data = file.readAll();
    file.close();

    // 反序列化 QJsonDocument
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON:" << parseError.errorString();
        return QJsonObject(); // 返回空对象
    }

    // 从 QJsonDocument 中提取 QJsonObject
    if (!doc.isObject()) {
        qWarning() << "JSON document is not an object.";
        return QJsonObject(); // 返回空对象
    }

    return doc.object();
}
