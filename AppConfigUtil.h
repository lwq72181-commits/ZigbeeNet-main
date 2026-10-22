#ifndef APPCONFIGUTIL_H
#define APPCONFIGUTIL_H

#include <QLineEdit>
#include <QString>
#include <QSpinBox>
#include <QComboBox>
#include "Acquisitiontype.h"

class AppConfigUtil
{
public:
    AppConfigUtil();

    static bool load(QLineEdit* lineEdit, const QString& key, int defValue);
    static bool load(QLineEdit* lineEdit, const QString& key, const QString& defValue);

    static bool load(QSpinBox* box, const QString& key, int defValue);

    static bool load(QComboBox* cmb, const QString& key, int defValue);
    static bool load(QComboBox* cmb, const QString& key, const QString& defValue);

    static bool save(QLineEdit* lineEdit, const QString& key);
    static bool save(QSpinBox* box, const QString& key);
    static bool save(QComboBox* cmb, const QString& key);

    static bool loadUser(FREUser& user);
    static bool loadDeviceID(QString& id);

    static bool loadChannel(QStringList& names);
    static bool saveChannel(const QStringList& names);
    static bool saveMqttServer(QString &url,int &port);
    static bool saveMqttInfo(QString &username,QString &password,QString &orgname);
    static bool saveWifiInfo(QString &wifiname,QString &password);
    static bool saveBLInfo(QString &name);
    static bool loadBLInfo(QString &name);
    static bool loadMqttServer(QString &url,int &port);
    static bool loadMqttInfo(QString &username,QString &password,QString &orgname);
    static bool loadWifiInfo(QString &wifiname,QString &password);

};

#endif // APPCONFIGUTIL_H
