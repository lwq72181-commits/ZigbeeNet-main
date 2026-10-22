#include "AppConfigUtil.h"
#include "AppConfig.h"
#include "QWidgetText.h"
#include "QVariantCast.h"
#include "Acquisitiontype.h"

bool setValue(QComboBox* cmb, int value)
{
    bool found = false;
    for (int i =0; i < cmb->count(); i ++)
    {
        if (cmb->itemData(i).toInt() == value)
        {
            cmb->setCurrentIndex(i);
            found = true;
            break;
        }
    }

    if (!found)
    {
        if (cmb->isEditable())
        {
            QString text = QString::number(value);
            cmb->setCurrentText(text);
            found = true;
        }
    }

    return found;
}

bool setValue(QComboBox* cmb, const QString& value)
{
    bool found = false;
    for (int i =0; i < cmb->count(); i ++)
    {
        QVariant data = cmb->itemData(i);
        if (data.isValid())
        {
            if (data.toString() == value)
            {
                cmb->setCurrentIndex(i);
                found = true;
                break;
            }
        }
        else
        {
            if (cmb->itemText(i) == value)
            {
                cmb->setCurrentIndex(i);
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        if (cmb->isEditable())
        {
            cmb->setCurrentText(value);
            found = true;
        }
    }

    return found;
}



AppConfigUtil::AppConfigUtil()
{

}

bool AppConfigUtil::load(QLineEdit* lineEdit, const QString& key, int defValue)
{
    AppConfig::get(key, defValue);
    QWidgetText::setText(lineEdit, defValue);
    return true;
}

bool AppConfigUtil::load(QLineEdit* lineEdit, const QString& key, const QString& defValue)
{
    QString value(defValue);
    AppConfig::get(key, value);
    QWidgetText::setText(lineEdit, value);
    return true;
}

bool AppConfigUtil::load(QSpinBox* widget, const QString& key, int defValue)
{
    AppConfig::get(key, defValue);
    widget->setValue(defValue);
    return true;
}

bool AppConfigUtil::load(QComboBox* cmb, const QString& key, int defValue)
{
    int value = defValue;
    AppConfig::get(key, value);
    return setValue(cmb, value);
}

bool AppConfigUtil::load(QComboBox* cmb, const QString& key, const QString& defValue)
{
    QString value(defValue);
    AppConfig::get(key, value);
    return setValue(cmb, value);
}



bool AppConfigUtil::save(QLineEdit* lineEdit, const QString& key)
{
    QString value;
    QWidgetText::getText(lineEdit, value);
    return AppConfig::set(key, value);
}

bool AppConfigUtil::save(QSpinBox* box, const QString& key)
{
    int value = box->value();
    return AppConfig::set(key, value);
}

bool AppConfigUtil::save(QComboBox* cmb, const QString& key)
{
    QVariant variant = cmb->currentData();
    if (variant.type() == QVariant::String)
    {
        return AppConfig::set(key, variant.toString());
    }
    else if (variant.type() == QVariant::Int)
    {
        return AppConfig::set(key, variant.toInt());
    }
    else if (variant.type() == QVariant::Double)
    {
        return AppConfig::set(key, variant.toDouble());
    }
    else
    {
        return AppConfig::set(key, variant.toString());
    }
}

//bool AppConfigUtil::loadRecordEncProfile(Hid_VideoEncProfile& profile)
//{
//    int resolution = AppConfig::optGet("Record/Resolution", (int)RESOLUTION_720P);
//    int framerate = AppConfig::optGet("Record/Framerate", 25);
//    int bitrate = AppConfig::optGet("Record/Bitrate", 2000);

//    profile.bitrate = bitrate;
//    profile.fps = framerate;
//    if (!Resolutions::getResolution(resolution, profile.width, profile.height))
//    {
//        profile.width = 1280;
//        profile.height = 720;
//    }

//    return true;
//}

//bool AppConfigUtil::saveRecordEncProfile(const Hid_VideoEncProfile& profile)
//{
//    Resolution resolution = RESOLUTION_720P;
//    Resolutions::match(profile.width, profile.height, resolution);

//    AppConfig::set("Record/Resolution", (int)resolution);
//    AppConfig::set("Record/Framerate", profile.fps);
//    AppConfig::set("Record/Bitrate", profile.bitrate);

//    return true;
//}

bool AppConfigUtil::saveMqttServer(QString &url,int &port)
{
    bool set = AppConfig::set("Device/url", url);
    AppConfig::set("Device/port", port);
    return set;
}

bool AppConfigUtil::loadMqttServer(QString &url,int &port)
{
    bool get = AppConfig::get("Device/url", url);
    AppConfig::get("Device/port", port);
    return get;
}

bool AppConfigUtil::loadMqttInfo(QString &username,QString &password,QString &orgname)
{
    bool get = AppConfig::get("Device/username", username);
    AppConfig::get("Device/password", password);
    AppConfig::get("Device/orgname", orgname);
    return get;
}

bool AppConfigUtil::saveMqttInfo(QString &username,QString &password,QString &orgname)
{
    bool set = AppConfig::set("Device/username", username);
    AppConfig::set("Device/password", password);
    AppConfig::set("Device/orgname", orgname);
    return set;
}

bool AppConfigUtil::saveWifiInfo(QString &wifiname,QString &password)
{
    bool set = AppConfig::set("Network/wifiname", wifiname);
    AppConfig::set("Network/password", password);
    return set;
}

bool AppConfigUtil::loadWifiInfo(QString &wifiname,QString &password)
{
    bool get = AppConfig::get("Network/wifiname", wifiname);
    AppConfig::get("Network/password", password);
    return get;
}

bool AppConfigUtil::saveBLInfo(QString &name)
{
    bool set = AppConfig::set("Device/BLName", name);
    return set;
}

bool AppConfigUtil::loadBLInfo(QString &name)
{
    bool got = AppConfig::get("Device/BLName", name);
    return got;
}

//bool AppConfigUtil::saveNETInfo(QString &name)
//{
//    bool set = AppConfig::set("Device/NET", name);
//    return set;
//}

//bool AppConfigUtil::loadNETInfo(QString &name)
//{
//    bool got = AppConfig::get("Device/BLName", name);
//    return got;
//}

bool AppConfigUtil::loadUser(FREUser& user)
{
    bool got = AppConfig::get("Device/url", user.url);
    AppConfig::get("Device/port", user.port);
    AppConfig::get("Device/username", user.username);
    AppConfig::get("Device/password", user.password);
    AppConfig::get("Device/orgname", user.orgname);
    AppConfig::get("Device/devid", user.devid);
//    QStringList stringList = user.url.split(":");
//    if(stringList.count()==2)
//    {
//        user.port = stringList[1].toInt();
//        qDebug("get mqtt server port:%d\n",user.port);
//    }
//    else
//    {
//        qDebug("get mqtt server port error!!!!!\n");
//    }


    return got;
}

bool AppConfigUtil::loadDeviceID(QString& id)
{
    return AppConfig::get("Device/ID", id);
}

bool AppConfigUtil::loadChannel(QStringList& names)
{
//    QStringList defNames;
//    defNames << "HDMI-1";
//    defNames << "HDMI-2";
//    defNames << "HDMI-3";
//    defNames << "SDI";
//    defNames << AppSymbol::getMixer();

//    defNames << "IPC-1";
//    defNames << "IPC-2";

//    for (int i = 0; i < HIMEET_MAX_CHANNEL; i ++)
//    {
//        QString path = QString("Channel/%1").arg(i+1);
//        QString name = AppConfig::optGet(path, defNames[i]);
//        names.push_back(name);
//    }
    return true;
}

bool AppConfigUtil::saveChannel(const QStringList& names)
{
//    for (int i = 0; i < names.size(); ++ i)
//    {
//        QString path = QString("Channel/%1").arg(i+1);
//        AppConfig::set(path, names[i]);
//    }
    return true;
}
