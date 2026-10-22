#ifndef JSONFILEHANDLE_H
#define JSONFILEHANDLE_H
#include <QJsonObject>
#include <QString>
#include <QJsonDocument>
#include <QJsonArray>
class JSONFileHandle
{
public:
    JSONFileHandle();
public:
    int saveToJsonFile(const QJsonObject &obj, const QString &filePath);
    bool saveToJsonFile(const QJsonArray &array, const QString &filePath);
    bool saveToFile(const uint8_t *array,const int len, const QString &filePath);
    QJsonArray loadArrayFromJsonFile(const QString &filePath);
    QJsonObject loadObjectFromJsonFile(const QString &filePath);
    int super_system(const char * cmd, char *retmsg, int msg_len);
    int rm_oldest_day_file();
    int rm_oldest_day_485file();
    bool deleteFile(const QString &filePath);
    float get_partation_percent(char * partation);
    QString GetSaveFileName(int flag,int channel);
    QString GetSaveFileNameP(int flag,int channel,int sec);
    QString GetSaveFileNameSTime(int flag,int channel,QString &time);
    QString GetSaveFileNameUartP(int flag,int channel,int sec);
    QString GetSaveFileNameTestP(int flag,int channel,int sec);
    int GetOldestFileName(QString &name, int flag);
    int GetOldestFileNameUart(QString &name, int flag);
    int initRecFloder();
};

#endif // JSONFILEHANDLE_H
