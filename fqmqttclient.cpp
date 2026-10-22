#include <QUrl>
#include <QJsonDocument>
#include <QDateTime>
#include <QProcess>
#include <QTimerEvent>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QCryptographicHash>
#include <QThread>
#include <cmath>
#include <stdlib.h>
#include "fqmqttclient.h"
#include "CLog.h"
#include "NetworkUtil.h"
#include "NetworkConf.h"
#include "AppConfig.h"
#include "AppConfigUtil.h"
#include "AppConst.h"
#include "zigbee/proto_common.h"
QmqttClient::QmqttClient():
m_mqttClient(new QMqttClient())
{
    m_pos = 0;
    m_mqttTimer = 0;
    m_timeOutTimer = 0;
    m_discount = 0;
    uart_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_localPara.mutex = PTHREAD_MUTEX_INITIALIZER;
    m_localPara.cond = PTHREAD_COND_INITIALIZER;
    m_localPara.condition = 0;
    m_localPara.mutexwork = PTHREAD_MUTEX_INITIALIZER;
    m_localPara.condwork = PTHREAD_COND_INITIALIZER;
    m_localPara.conditionwork = 0;
    m_localPara.loop = 0;
    m_localPara.updateflag = 0;
    m_localPara.messageid = 0;
    m_localPara.sendlen = 512;
    m_sysparam.UploadPeriod = 15;
    m_httpUpdateinfo.updateflag = 0;
    m_sysparam.IndexPeriod = 60;
    m_networkConf.load();
    connect(m_mqttClient, &QMqttClient::connected, this, &QmqttClient::on_connected);
    connect(m_mqttClient, &QMqttClient::disconnected, this, &QmqttClient::on_disconnected);
    connect(m_mqttClient, &QMqttClient::messageReceived, this, &QmqttClient::on_messageReceived);
    m_handle = new mqtthandle();
    connect(m_handle, &mqtthandle::sendsig, this, &QmqttClient::sendMqMessage, Qt::QueuedConnection);
    m_handle->moveToThread(&m_UploadThread);
    m_adhanle = new AdcHandle();
    connect(this, &QmqttClient::operate, m_handle, &mqtthandle::dowork);
    //该线程结束时销毁
    connect(&m_UploadThread, &QThread::finished, m_handle, &QObject::deleteLater);
//    connect(m_handle, SIGNAL(resultReady(int)), this, SLOT(handleResults(int)));
    //启动线程
    m_UploadThread.start();

}

void QmqttClient::close()
{
    if (!m_opened)
    {
        return;
    }

    m_opened = false;

    //
    stopMqttTimer();

    m_connected = false;
    m_mqttClient->disconnectFromHost();
    startMqttTimer(120);
}

bool QmqttClient::isOpen()
{
    return m_opened;
}

bool QmqttClient::isConnected()
{
    return m_connected;
}

void QmqttClient::setMqUrl(const QString& url, const QString& username,const QString& password, const QString& orgname, \
                           const QString& devid, const int& port,TCPParam *param,ZIGBEEParam * ZParam)
{
    m_mqUrl = url;
    m_port = port;
    m_mqUser = username;
    m_mqPassword = password;
    m_orgName = orgname;
    m_devID = devid;
    m_param = param;
    m_ZIGBEEParam = ZParam;
    setupMqtt();
}

uartConfig4g5g *QmqttClient::getUart()
{
    return &m_uart;
}

pthread_mutex_t *QmqttClient::getmutex()
{
    return &uart_mutex;
}

void QmqttClient::setdevID(const QString& devid)
{
    m_devID = devid;
}

void QmqttClient::setupMqtt()
{
    QUrl url(m_mqUrl);
//    QString host = url.host();
//    int port = url.port();

//    CLog::debug("setupMqtt. url: %s, user: %s, passwd: %s,orgname:%s,devid:%s\n",
//        qPrintable(m_mqUrl), qPrintable(m_mqUser), qPrintable(m_mqPassword), qPrintable(m_orgName), qPrintable(m_devID));
    qDebug("setupMqtt. url: %s, user: %s, passwd: %s,orgname:%s,devid:%s\n",
        qPrintable(m_mqUrl), qPrintable(m_mqUser), qPrintable(m_mqPassword), qPrintable(m_orgName), qPrintable(m_devID));
    QThread::sleep(5);
    m_deviceTopic = QString("%1/%2").arg(m_orgName).arg(m_devID);
    m_uploadTopic = QString("%1/Online/%2").arg(m_orgName).arg(m_devID);
    qDebug("m_deviceTopic:%s,m_uploadTopic:%s\n",qPrintable(m_deviceTopic),qPrintable(m_uploadTopic));
    m_localPara.Topic = m_uploadTopic;
    QString clientId = m_orgName + m_devID;
    m_mqttClient->setClientId(clientId);
    m_mqttClient->setUsername(m_mqUser);
    m_mqttClient->setPassword(m_mqPassword);

    m_mqttClient->setHostname(m_mqUrl);
    m_mqttClient->setPort(m_port);

    m_mqttClient->setAutoKeepAlive(true);

    {
//        QJsonObject userJson;
//        toJson(m_user, userJson);

        QJsonObject json;
        json["id"] = m_devID;
        json["ts"] = QDateTime::currentMSecsSinceEpoch();
//        json["method"] = MeetMessage::USER_OFF;
//        json["from"] = m_userTopic;
//        json["data"] = userJson;

        QJsonDocument doc(json);
        QString topic = QString("%1/#").arg(m_deviceTopic);
        m_mqttClient->setWillTopic(m_deviceTopic);
        m_mqttClient->setWillQoS(0);
        m_mqttClient->setWillMessage(doc.toJson());
    }

    m_mqttClient->connectToHost();
}


void QmqttClient::on_connected()
{
    emit setLedTime(0);
    qDebug("mqtt connected.\n");
    m_pos = 0;
    m_connected = true;
    m_opened = true;
    m_localPara.mqttopen = true;
    m_discount = 0;
    QString topic = QString("%1/#").arg(m_deviceTopic);
    m_mqttClient->subscribe(QMqttTopicFilter(topic),0);
    QString servertopic = QString("%1/ServerOnline/#").arg(m_orgName);
    m_mqttClient->subscribe(QMqttTopicFilter(servertopic),1);
    stopMqttTimer();
    stopTimeOutTimer();
    //QThread::sleep(20);
    publishStatus();
    startTimeOutTimer();
    //system("echo 1 > /sys/class/gpio/gpio58/value");
//    castDeviceInfo(m_devInfo);
}

void QmqttClient::reconnect4gNet()
{
    char at_cmd_buf[128];
    qDebug("reconnect4gNet");
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+CFUN=0\r\n");
    pthread_mutex_lock(&uart_mutex);
    m_uart.init("/dev/ttyUSB2");
    m_uart.uart_send_data(at_cmd_buf, strlen(at_cmd_buf));
    QThread::sleep(1);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+CFUN=1\r\n");
    m_uart.uart_send_data(at_cmd_buf, strlen(at_cmd_buf));
    m_uart.close_serial_port();
    pthread_mutex_unlock(&uart_mutex);
}

void QmqttClient::on_disconnected()
{
    int flag = 0;
    qDebug("mqtt disconnected1.\n");
    //bool changed = m_connected;
    m_connected = false;
    m_localPara.mqttopen = false;
    m_discount++;
    if(m_discount>1)
    {
        int mid = 0;
        m_discount=0;
        AppConfig::get("Network/NET", mid);
        char* ccmd="";
        if(mid == 0)
        {
            qDebug() << "====================on_disconnected";
            qDebug("====================on_disconnected");
            //ccmd = "killall udhcpc;udhcpc -i eth1 &";
            m_networkConf.load();
            int pos = m_networkConf.findConfig("eth1");
            qDebug("find eth1 pos:%d,dhcp:%d",pos,m_networkConf.m_configs[pos].dhcp);
            if(m_networkConf.m_configs[pos].dhcp)
            {
               // qDebug() << "====================setNetSelect1";
             //   qDebug("====================setNetSelect1");
                ccmd = "ip route del default;killall udhcpc;sleep 1;udhcpc -i eth1 &";
            }
            else
            {
                flag = 1;
                m_networkConf.applycard(pos);
            }
        }
        else if(mid == 1)
        {
            int wifidhcp;
            AppConfig::get("Network/dhcp", wifidhcp);
            QString ip;
            QString netmask;
            QString gateway;
            QString dns;
            QString cmd;
            QString name,passwd;
            AppConfigUtil::loadWifiInfo(name,passwd);
            AppConfig::get("Network/ip", ip);
            AppConfig::get("Network/netmask", netmask);
            AppConfig::get("Network/gateway", gateway);
            AppConfig::get("Network/dns", dns);
            if(wifidhcp==1)
            {
                qDebug() << "====================on_disconnected11";
                qDebug("====================================start");
                //ccmd = "killall udhcpc;udhcpc -i wlan0 &";
                cmd = QString("ip route del default;killall wpa_supplicant;killall udhcpc;sleep 1;/userdata/etc/wifi_setup.sh -i %1 -p %2").arg(name).arg(passwd);
                ccmd = cmd.toUtf8().data();
                qDebug("====================================over");
            }
            else
            {
                qDebug() << "====================on_disconnected111";
                qDebug("====================================start");
                cmd = QString("ip route del default;killall wpa_supplicant;killall udhcpc;ifconfig wlan0 down;sleep 2;ifconfig wlan0 up;sleep 2;/userdata/etc/wifi_setup.sh -i %1 -p %2 -a %3 -m %4 -g %5 -n %6").arg(name).arg(passwd).arg(ip).arg(netmask).arg(gateway).arg(dns);
                qDebug("%s",qPrintable(cmd));
                ccmd = cmd.toUtf8().data();
//                AppConfig::get("Network/ip", ip);
//                AppConfig::get("Network/netmask", netmask);
//                AppConfig::get("Network/gateway", gateway);
//                AppConfig::get("Network/dns", dns);
//                cmd = QString("/userdata/etc/wifi_setup.sh -i %1 -p %2 -a %3 -m %4 -g %5 -n %6").arg(name).arg(passwd).arg(ip).arg(netmask).arg(gateway).arg(dns);
            }
        }
        else if(mid == 2)
        {
            reconnect4gNet();
            qDebug() << "====================on_disconnected1";
            qDebug("====================on_disconnected1");
            ccmd = "ip route del default;killall udhcpc;udhcpc -i usb0 &";
        }
        else if(mid == 3)
        {
            qDebug() << "====================on_disconnected3";
            qDebug("====================on_disconnected3");
            ccmd = "ip route del default;killall udhcpc;ifconfig pcie0 up;ifconfig sipa_dummy0 up;udhcpc -i pcie0 &";
        }
        if(flag==0)
        system(ccmd);
        emit setLedTime(1);
    }
    stopMqttTimer();
    startMqttTimer(120);
}

void QmqttClient::setUpdateInfo()
{
    m_httpUpdateinfo.updateflag = 0;
}

void QmqttClient::reconnect(int time)
{
    if (!m_opened)
    {
        qDebug("we need to connect mqtt server");
        startMqttTimer();
        return;
    }
    stopMqttTimer();
    stopTimeOutTimer();
    //qDebug("mqtt reconnect.\n");
    startMqttTimer(time);
}

void QmqttClient::reconnect()
{
    if (!m_opened)
    {
        startMqttTimer();
        return;
    }
    m_opened = false;
    stopMqttTimer();
    stopTimeOutTimer();
    m_connected = false;
    //m_mqttClient->disconnectFromHost();
    qDebug("mqtt reconnect.\n");
    startMqttTimer();
}

void QmqttClient::publishStatus()
{

//    std::string ip = NetworkUtil::getHostIp();
//    qDebug("getHostIp :%s\n",ip);
//    QString ipAddr = QString::fromStdString(ip);
//    std::string gate;
//    NetworkUtil::getGateway(gate);
//    qDebug("getGateway :%s\n",gate);
//    config.gate = QString::fromStdString(gate);

    QString macAddr;
    char * ethname;
    int mid;
    AppConfig::get("Network/NET", mid);
    if(mid == 0)
    {
        ethname = "eth1";
    }
    else if(mid == 1)
    {
        ethname = "wlan0";
    }
    else if(mid == 2)
    {
        ethname = "usb0";
    }
    else if(mid == 3)
    {
        ethname = "rmnet_mhi0.1";
    }


    NetworkConf::getMacAddr(ethname, macAddr);
    net::NetConfig config;
    if (!net::getNetConfig(ethname, config))
    {
        qDebug("getNetConfig eth1 error!!!");
    }
    else
        qDebug("get ip:%s,gateway:%s,mask:%s\n",config.ip.c_str(),config.gateway.c_str(),config.mask.c_str());
//    jsonText = JsonCast::toString(config);

    QJsonObject obj;
    obj.insert("Ver", AppConst::getVersion());
    obj.insert("Code", m_devID);
    obj.insert("Mac", macAddr);
    obj.insert("SN", m_devID);
    obj.insert("LocalIp", QString::fromStdString(config.ip));
    obj.insert("ServerIp", m_mqUrl);
    obj.insert("GateWay", QString::fromStdString(config.gateway));
    obj.insert("SubnetMask", QString::fromStdString(config.mask));
    obj.insert("ServerPort", m_port);
    obj.insert("ConnType", mid);
    obj.insert("DasType", 20);
    obj.insert("RSSI", 0);
//    int mid;
    AppConfig::get("Device/zgbFreqBand", mid);
    obj.insert("Channel", mid/*m_param->channel*/);

    QJsonObject versionh;
    QJsonObject versions;
    QJsonArray array;
    versionh.insert("Code", 1);
    versionh.insert("AppVer", AppConst::getVersion());
    versionh.insert("HardVer", "V1.0.0");
    versionh.insert("ZGBBoardVer", QString::fromUtf8(m_ZIGBEEParam->McuVersion));
//    versions.insert("AppVer", "V1.0.0");
//    versions.insert("HardVer", "");
//    versions.insert("Code", 5);
    //向Json数组中追加数据
    array.append(versionh);
    array.append(versions);
    obj.insert("BoardInfoList", array);

    QJsonArray sensorListArray;
    for(int i=0;i<m_ZIGBEEParam->count;i++)
    {
        QJsonObject sensor;
        sensor.insert("IndexNo",m_ZIGBEEParam->sensor_info[i].Channel);
        sensor.insert("SN",m_ZIGBEEParam->sensor_info[i].Sn);
        sensorListArray.append(sensor);
    }
    obj.insert("SensorList",sensorListArray);

    QString topic;
    topic = QString("%1/Register/0").arg(m_uploadTopic);
    qDebug("send topic:%s\n",topic.toUtf8().constData());
    sendMqMessage(topic, obj, 0);

}

bool QmqttClient::sendMqMessage(const QString& topic, const QJsonValue& data, const int& flag)
{
    int timeout = 0;
    QJsonDocument doc;
    //qDebug("sendMqMessage");
//    doc.setObject(data); // 设置为QJsonObject
//    doc.setArray(data);   // 设置为QJsonArray
//    if(!m_opened)
//    {
//        qDebug("mqtt is disconnected need to reconnect!");
//        stopMqttTimer();
//        onMqttTimer();
//    }
//    else
//    {
//        stopMqttTimer();
//        startMqttTimer(m_sysparam.UploadPeriod);
//    }
//    while(1)
//    {
//        if(m_opened)
//            break;
//        else
//        {
//            QThread::sleep(1);
//            timeout++;
//            if(timeout > 20)
//            {
//                qDebug("sendMqMessage timeout:%d\n",timeout);
//                break;
//            }
//        }
//    }
//    stopMqttTimer();
//    startMqttTimer(m_sysparam.UploadPeriod);
    if(flag == 1)
    {
        doc.setArray(data.toArray());
    }
    else
    {
        doc.setObject(data.toObject());
    }
    QByteArray ba = doc.toJson();
    //qDebug()<<ba;
    //qDebug("\nsend data!! mqtt1\n");
    qint32 ret = m_mqttClient->publish(QMqttTopicName(topic), ba);
    //qDebug("\nsend data!! over\n");
    return ret >= 0;
}

void QmqttClient::on_publishADCData(const unsigned char * data, const int len, const int flag)
{
//    if(flag == 0x00)
//    {

//    }
//    QJsonObject obj;
//    obj.insert("Ver", "V1.0.0");
//    obj.insert("Code", m_devID);
//    obj.insert("Mac", macAddr);
//    obj.insert("SN", m_devID);
//    obj.insert("LocalIp", QString::fromStdString(config.ip));
//    obj.insert("ServerIp", m_mqUrl);
//    obj.insert("GateWay", QString::fromStdString(config.gateway));
//    obj.insert("SubnetMask", QString::fromStdString(config.mask));
//    obj.insert("ServerPort", m_port);
//    obj.insert("ConnType", 3);
//    obj.insert("DasType", 20);
//    obj.insert("RSSI", 0);

//    QJsonObject versionh;
//    QJsonObject versions;
//    QJsonArray array;
//    versionh.insert("Code", 1);
//    versionh.insert("AppVer", "V1.0.0");
//    versionh.insert("HardVer", "");
//    versions.insert("AppVer", "V1.0.0");
//    versions.insert("HardVer", "");
//    versions.insert("Code", 5);
//    //向Json数组中追加数据
//    array.append(versionh);
//    array.append(versions);
//    obj.insert("BoardInfoList", array);

//    QString topic;
//    topic = QString("%1/Register/0").arg(m_uploadTopic);
//    qDebug("send topic:%s\n",topic.toUtf8().constData());
//    sendMqMessage(topic, obj, 0);
}

int QmqttClient::SetSysTimeLocal(const QString& datetime)
{
    QProcess process;
    QString cmd = QString("date -s \"%1\"").arg(datetime);
    qDebug()<<cmd;
    //process.start("/bin/sh", QStringList() << "-c" << cmd);
    process.start(cmd);
    if (!process.waitForStarted()) {
        qDebug() << "Failed to start  date process.";
        return -1;
    }
    if (!process.waitForFinished()) {
        qDebug() << "Process did not finish date in time.";
        return -1;
    }
    else
    {
        process.start("hwclock --systohc -u");
        if (!process.waitForStarted()) {
            qDebug() << "Failed to start hwclock process.";
            return -2;
        }
        if (!process.waitForFinished()) {
            qDebug() << "Process did not finish hwclock in time.";
            return -2;
        }
    }
    qDebug() << "Standard Output:" << process.readAllStandardOutput();
    qDebug() << "Standard Error:" << process.readAllStandardError();
    return 0;
}

int QmqttClient::setupzigbeeparam()
{
    qDebug("set zgbtype:%d",m_ZIGBEEParam->zgbtype);
    int mid = m_ZIGBEEParam->zgbtype;
    AppConfig::set("Device/zgbtype", mid);
    config_zgb_syspara();
    AppConfig::set("Device/zgbUpdate", 1);
    //system("echo 0 > /sys/class/gpio/gpio101/value;sleep 2;echo 1 > /sys/class/gpio/gpio101/value");
    return 0;
}

void QmqttClient::on_messageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
//    QJsonObject json = doc.object();

//    QJsonObject data = json["data"].toObject();
//    QString method = json["method"].toString();

    qDebug("on_messageReceived: topic:%s, message:%s\n", qPrintable(topic.name()), message.data());

    QString topicName = topic.name();
    if (topicName.contains(m_deviceTopic))
    {
//        QString fromTopic = json["from"].toString();

//        QString id = json["id"].toString();

        if (topicName.contains("Register"))
        {
            QStringList stringList = topicName.split("/");
            QString ret = stringList[stringList.count()-1];
            if(stringList.count()!=5 || (ret.toInt()!=1) || m_pos!=0)
            {
                reconnect();
            }
            else
            {
                QString topic;
                QJsonObject obj;
                m_pos=1;
                topic = QString("%1/UpdateFileList/0").arg(m_uploadTopic);
                qDebug("send topic:%s\n",topic.toUtf8().constData());
                sendMqMessage(topic, obj,0);
                stopTimeOutTimer();
                startTimeOutTimer();
            }
        }
        else if (topicName.contains("UpdateFileList"))
        {
            QStringList stringList = topicName.split("/");
            QString ret = stringList[stringList.count()-1];
            if(stringList.count()!=5 || (ret.toInt()!=1)|| m_pos!=1)
            {
                reconnect();
            }
            else
            {
                QJsonArray array = doc.array();
                if(array.isEmpty())
                {
                    m_pos = 2;
                    QString topic;
                    QJsonObject obj;
                    topic = QString("%1/SysPara/0").arg(m_uploadTopic);
                    qDebug("send topic:%s\n",topic.toUtf8().constData());
                    sendMqMessage(topic, obj,0);
                    stopTimeOutTimer();
                    startTimeOutTimer();
                }
                else
                {
                    //update
                    //for (int i = 0; i < array.count(); i++)
                    {
                        QJsonValue value = array.at(0);
                        if(value.isObject())
                        {
                            QJsonObject json = value.toObject();
                            m_httpUpdateinfo.url = json["RemotePath"].toString();
                            m_httpUpdateinfo.version = json["Ver"].toString();
                            m_httpUpdateinfo.name = json["Name"].toString();
                            QString md5 = json["Md5Code"].toString();
                            m_httpUpdateinfo.type = json["type"].toInt();
                            m_httpUpdateinfo.md5 = md5.remove(' ');
                            if(m_httpUpdateinfo.updateflag == 0 && (((m_httpUpdateinfo.type==0)&&AppConst::getVersion()!=m_httpUpdateinfo.version)||m_httpUpdateinfo.type>0))
                            {
                                //stopTimeOutTimer();//TODO CHECK
                                m_httpUpdateinfo.updateflag = 1;
                                m_ZIGBEEParam->update = 0;
                                emit updateHttp(m_httpUpdateinfo);
                            }
                            if((m_httpUpdateinfo.type==0&&(AppConst::getVersion()==m_httpUpdateinfo.version))||\
                                    (m_httpUpdateinfo.type==1&&m_ZIGBEEParam->update==1))
                            {
                                QJsonObject obj;
                                m_httpUpdateinfo.updateflag = 0;
                                obj.insert("Md5Code", m_httpUpdateinfo.md5);
                                obj.insert("IsConfirm", 1);
                                QString topic;
                                topic = QString("%1/UpdateFile/0").arg(m_uploadTopic);
                                qDebug("send topic:%s\n",topic.toUtf8().constData());
                                sendMqMessage(topic, obj, 0);
                                m_ZIGBEEParam->update = 0;
                            }

                            m_updateinfo.rkname = json["Name"].toString();
                            m_updateinfo.rksize = json["Size"].toInt();
                            m_updateinfo.rkpkgcount = static_cast<int>(std::ceil(static_cast<double>(m_updateinfo.rksize) / 1024));
                            m_updateinfo.rkpkgindex = 0;
                            qDebug("update name:%s,size :%d,count:%d",qPrintable(m_updateinfo.rkname),m_updateinfo.rksize,m_updateinfo.rkpkgcount);
                        }
                    }
                    //TODO CHECK AVOID update not get param
                    m_pos = 2;
                    QString topic;
                    QJsonObject obj;
                    topic = QString("%1/SysPara/0").arg(m_uploadTopic);
                    qDebug("send topic:%s\n",topic.toUtf8().constData());
                    sendMqMessage(topic, obj,0);
                    stopTimeOutTimer();
                    startTimeOutTimer();
                }
            }
        }
        else if (topicName.contains("SysPara"))
        {
            QStringList stringList = topicName.split("/");
            QString ret = stringList[stringList.count()-1];
            if(stringList.count()!=5 || (ret.toInt()!=1)|| m_pos!=2)
            {
                reconnect();
            }
            else
            {
                m_pos=3;
                QJsonObject json = doc.object();
                if(m_sysparam.IndexPeriod!=json["IndexPeriod"].toInt()||(m_sysparam.WavePeriod != json["WavePeriod"].toInt())
                        ||(m_sysparam.LongWavePeriod != json["LongWavePeriod"].toInt()))
                {
                    m_sysparam.IndexPeriod = json["IndexPeriod"].toInt();
                    m_sysparam.WavePeriod = json["WavePeriod"].toInt();
                    m_sysparam.LongWavePeriod = json["LongWavePeriod"].toInt();
                    emit setmqttUpdateTimeSignal(m_sysparam.IndexPeriod,m_sysparam.WavePeriod,m_sysparam.LongWavePeriod);
                    qDebug()<<"start setmqttUpdateTimeSignal";
                }
                m_sysparam.CollectType = json["CollectType"].toInt();
                m_sysparam.OutType485 = json["OutType485"].toInt();
                m_sysparam.TechPeriod = json["TechPeriod"].toInt();
                m_sysparam.RpmPeriod = json["RpmPeriod"].toInt();
                m_sysparam.RtuPeriod = json["RtuPeriod"].toInt();
                m_sysparam.UploadPeriod = json["UploadPeriod"].toInt();
                m_System = json["SysTime"].toString();
                QStringList parts = m_System.split("+", Qt::SkipEmptyParts);
                //QDateTime currentTime = QDateTime::fromString(parts[0], "yyyy-MM-dd'T'hh:mm:ss");
                SetSysTimeLocal(parts[0]);
                if(m_ParaTime!=json["ParaTime"].toString())
                {
                    QString topic;
                    QJsonObject obj;
                    m_ParaTime = json["ParaTime"].toString();
                    topic = QString("%1/CollectPara/0").arg(m_uploadTopic);
                    qDebug("send topic:%s\n",topic.toUtf8().constData());
                    sendMqMessage(topic, obj, 0);
                    stopTimeOutTimer();
                    startTimeOutTimer();
                }
                else
                {
                    stopTimeOutTimer();
                    //reconnect(m_sysparam.UploadPeriod);//capture;
                }

            }
        }
        else if (topicName.contains("CollectPara"))
        {
            QStringList stringList = topicName.split("/");
            QString ret = stringList[stringList.count()-1];
            if(stringList.count()!=5 || (ret.toInt()!=1) || m_pos!=3)
            {
                reconnect();
            }
            else
            {
                m_pos = 4;
                QJsonObject json = doc.object();
                QJsonArray Vibarr = json["VibChannelList"].toArray();//VibChannelList
                QJsonArray Techarr = json["TechChannelPara"].toArray();
                QJsonArray Speedarr = json["SpeedChannelPara"].toArray();
                QJsonArray Rtuarr = json["RtuChannelPara"].toArray();
                QJsonArray vTriggerarr = json["TriggerPara"].toArray();
                m_localPara.VibChannelList.clear();
                qDebug("VibChannelList count:%d\n",Vibarr.count());
                for (int i = 0; i < Vibarr.count(); i++)
                {
                    QJsonValue value = Vibarr.at(i);
                    if(value.isObject())
                    {
                        QJsonObject json = value.toObject();
                        FreqVibChannalPara Vibmid;
                        Vibmid.ChNo = json["ChNo"].toInt();
                        Vibmid.SpeedNo = json["SpeedNo"].toInt();
                        Vibmid.SensType = json["SensType"].toInt();
                        Vibmid.SensCoef = static_cast<float>(json["SensCoef"].toDouble());
                        Vibmid.CnstCurSrcState = json["CnstCurSrcState"].toInt();
                        if(json["VibParaList"].isArray())
                        {
                            QJsonArray VibParaArr = json["VibParaList"].toArray();
                            int count = VibParaArr.count();
                            qDebug("VibParaList count:%d\n",count);
                            for(int j=0;j<count;j++)
                            {
                                FreqVibPara mid;
                                QJsonObject VibValue = VibParaArr.at(j).toObject();
                                mid.Code = VibValue["Code"].toInt();
                                mid.LowFreq = VibValue["LowFreq"].toInt();
                                mid.HighFreq = VibValue["HighFreq"].toInt();
                                mid.Length = VibValue["Length"].toInt();
                                mid.PlusLength = VibValue["PlusLength"].toInt();
                                mid.Unit = VibValue["Unit"].toString();
                                mid.Dir = VibValue["Dir"].toInt();
                                Vibmid.VibParaList.append(mid);
                            }
                        }
                        if(json["LimitParaList"].isArray())
                        {
                            QJsonArray LimitParaArr = json["LimitParaList"].toArray();
                            int count = LimitParaArr.count();
                            for(int j=0;j<count;j++)
                            {
                                FreqLimitPara mid;
                                QJsonObject LimitValue = LimitParaArr.at(j).toObject();
                                mid.Code = LimitValue["Code"].toInt();
                                mid.Level = LimitValue["Level"].toInt();
                                mid.Limit = LimitValue["Limit"].toInt();
                                mid.Dir = LimitValue["Dir"].toInt();
                                Vibmid.LimitParaList.append(mid);
                            }
                        }
                        Vibmid.DevId = json["DevId"].toInt();
                        Vibmid.IsTem = json["IsTem"].toInt();
                        Vibmid.SpeedDiffRate = static_cast<float>(json["SpeedDiffRate"].toDouble());
                        Vibmid.PolePairsNum = json["PolePairsNum"].toInt();
                        int mid = json["ZigbeeTemplate"].toInt();//ZigbeeTemplate
                        if(mid != m_ZIGBEEParam->zgbtype && mid != 0)
                        {
                            m_ZIGBEEParam->zgbtype = mid;
                            setupzigbeeparam();
                        }
                        m_localPara.VibChannelList.append(Vibmid);
                        //m_VibList.append(Vibmid);
                    }
                    else
                        qDebug("VibChannelList have no value");

                }

                m_localPara.TechChannelList.clear();
                for (int i = 0; i < Techarr.count(); i++)
                {
                    QJsonValue value = Techarr.at(i);
                    if(value.isObject())
                    {
                        QJsonObject json = value.toObject();
                        FreqTechChannelPara Techmid;
                        Techmid.ChNo = json["ChNo"].toInt();
                        Techmid.SensCoef = static_cast<float>(json["SensCoef"].toDouble());
                        Techmid.Amendment = static_cast<float>(json["Amendment"].toDouble());
                        Techmid.DevId = json["DevId"].toInt();
                        m_localPara.TechChannelList.append(Techmid);
                        //m_TechList.append(Techmid);
                    }

                }

                m_localPara.SpeedChannelList.clear();
                for (int i = 0; i < Speedarr.count(); i++)
                {
                    QJsonValue value = Speedarr.at(i);
                    if(value.isObject())
                    {
                        QJsonObject json = value.toObject();
                        FreqSpeedChannelPara Speedmid;
                        Speedmid.ChNo = json["ChNo"].toInt();
                        Speedmid.TriggerCount = json["TriggerCount"].toInt();
                        Speedmid.DevId = json["DevId"].toInt();
                        //m_SpeedList.append(Speedmid);
                        m_localPara.SpeedChannelList.append(Speedmid);
                    }

                }

                m_localPara.RtuChannelList.clear();
                for (int i = 0; i < Rtuarr.count(); i++)
                {
                    QJsonValue value = Rtuarr.at(i);
                    if(value.isObject())
                    {
                        QJsonObject json = value.toObject();
                        FreqRtuChannelPara Rtumid;
                        Rtumid.ChNo = json["ChNo"].toInt();
                        Rtumid.RTUDeviceAdd = json["RTUDeviceAdd"].toInt();
                        Rtumid.FunCode = json["FunCode"].toInt();
                        Rtumid.RegAddr = json["RegAddr"].toInt();
                        Rtumid.DataLength = json["DataLength"].toInt();
                        Rtumid.DevId = json["DevId"].toInt();
                        //m_RtuList.append(Rtumid);
                        m_localPara.RtuChannelList.append(Rtumid);
                    }

                }

                m_localPara.TriggerList.clear();
                for (int i = 0; i < vTriggerarr.count(); i++)
                {
                    QJsonValue value = vTriggerarr.at(i);
                    if(value.isObject())
                    {
                        QJsonObject json = value.toObject();
                        FreqTriggerPara Triggermid;

                        QList<FreqTriggerLimitPara> TriggerLimitList;//触发⻔限列表
                        Triggermid.Freq = json["Freq"].toInt();
                        Triggermid.Length = json["Length"].toInt();
                        Triggermid.TriggerGroup = json["TriggerGroup"].toInt();
                        Triggermid.TriggerType = json["TriggerType"].toInt();
                        QJsonArray limitarr = json["TriggerLimitList"].toArray();
                        FreqTriggerLimitPara  TriggerLimitmid;
                        for(int j = 0; j < limitarr.count(); j++)
                        {
                            QJsonObject limit = limitarr.at(i).toObject();
                            QList<int> Channels;//通道集合（这些通道同步触发,多设备不同参数）
                            int count = limit["Channels"].toArray().count();
                            QJsonArray chanarr = limit["Channels"].toArray();
                            for(int k = 0; k < count; k++)
                            {
                                TriggerLimitmid.Channels.append(chanarr.at(k).toInt());
                            }
                            TriggerLimitmid.AccLimit = static_cast<float>(limit["AccLimit"].toDouble());
                            TriggerLimitmid.VerLimit = static_cast<float>(limit["VerLimit"].toDouble());
                            TriggerLimitmid.RpmLimit = static_cast<float>(limit["RpmLimit"].toDouble());
                            TriggerLimitmid.ShockLimit = static_cast<float>(limit["ShockLimit"].toDouble());
                            Triggermid.TriggerLimitList.append(TriggerLimitmid);
                        }
                        //m_TriggerList.append(Triggermid);
                        m_localPara.TriggerList.append(Triggermid);
                    }

                }
//                if(m_localPara.loop == 1)
//                {
//                    m_localPara.conditionwork = 0;
//                    m_localPara.loop = 0;
//                    if(waiting_function(150,&m_localPara)==0)
//                    {
//                        m_localPara.loop = 1;
//                        emit operate(m_mqttClient,&m_sysparam, &m_localPara, m_adhanle,m_param);
//                        qDebug("UploadPeriod:%d\n",m_sysparam.UploadPeriod);
//                        //reconnect(m_sysparam.UploadPeriod);
//                    }
//                    else
//                    {
//                        m_localPara.loop = 1;
//                        qDebug("stop mqtt work thread error");
//                    }
//                }
//                else
                if(m_localPara.loop == 0)
                {
                    m_localPara.loop = 1;
                    emit operate(m_mqttClient,&m_sysparam, &m_localPara, m_adhanle,m_param);
                    qDebug("UploadPeriod:%d\n",m_sysparam.UploadPeriod);
                    //reconnect(m_sysparam.UploadPeriod);
                }

                stopTimeOutTimer();
                //close();
                //reconnect();
            }
        }
        else if (topicName.contains("IndexData"))
        {
            QStringList stringList = topicName.split("/");
            QString ret = stringList[stringList.count()-1];
            QString id = stringList[stringList.count()-2];
            if(id.toInt()!=m_localPara.messageid)
            {
                qDebug("================recv old IndexData index=========");
                return;
            }
            if(stringList.count()!=5)
            {
                //reconnect();
            }
            else
            {
                m_localPara.topicStatus = ret.toInt();
                signal_function(&m_localPara);
            }
        }
        else if (topicName.contains("IsWaveDataResume"))//
        {
            QStringList stringList = topicName.split("/");
            QString ret = stringList[stringList.count()-1];
            QString id = stringList[stringList.count()-2];
            if(stringList.count()!=5)
            {
                //reconnect();
            }
            else
            {
                if(id.toInt()!=m_localPara.messageid)
                {
                    qDebug("================recv old IsWaveDataResume index=========");
                    return;
                }
                QJsonObject json = doc.object();
                if(json.contains("IsResume"))
                {
                    if(json["IsResume"].toBool()==true)
                    {
                    }
                    m_localPara.IsResume = json["IsResume"].toBool();
                    QJsonArray wavearr = json["UnRceivePack"].toArray();
                    qDebug("UnRceivePack count:%d\n",wavearr.count());
                    for (int i = 0; i < wavearr.count(); i++)
                    {
                        QJsonValue value = wavearr.at(i);
                        m_localPara.reupload.append(value.toInt());
                    }
                    m_localPara.waverecved = 1;
                }
                m_localPara.topicStatus = ret.toInt();
                signal_function(&m_localPara);
            }
        }
        else if (topicName.contains("WaveData"))
        {
            QStringList stringList = topicName.split("/");
            QString ret = stringList[stringList.count()-1];
            QString id = stringList[stringList.count()-2];
            if(stringList.count()!=5)
            {
                //reconnect();
            }
            else
            {
                if(id.toInt()!=m_localPara.messageid)
                {
                    qDebug("================recv old wave unreceived index=========");
                    return;
                }
                QJsonObject json = doc.object();
                if(json.contains("UnRceivePack"))
                {
                    QJsonArray wavearr = json["UnRceivePack"].toArray();
                    qDebug("UnRceivePack count:%d\n",wavearr.count());
                    for (int i = 0; i < wavearr.count(); i++)
                    {
                        QJsonValue value = wavearr.at(i);
                        m_localPara.reupload.append(value.toInt());
                    }
                    m_localPara.waverecved = 1;
                }
                m_localPara.topicStatus = ret.toInt();
                signal_function(&m_localPara);


//                if(ret.toInt() == 0 || ret.toInt() == 1 || ret.toInt() == -5)
//                {
//                    QJsonObject json = doc.object();
//                    QJsonArray wavearr = json["UnRceivePack"].toArray();
//                    qDebug("UnRceivePack count:%d\n",wavearr.count());
//                    for (int i = 0; i < wavearr.count(); i++)
//                    {
//                        QJsonValue value = wavearr.at(i);
//                        m_localPara.reupload.append(value.toInt());
//                    }
//                }
//                m_localPara.topicStatus = ret.toInt();
//                signal_function(&m_localPara);
            }
        }
        else if(topicName.contains("ServerSwitch"))
        {
            QJsonObject json = doc.object();
            if(json.contains("ServerIP"))
            {
                QString ip = json["ServerIP"].toString();
                int port = json["ServerPort"].toInt();
                AppConfigUtil::saveMqttServer(ip, port);
                qDebug("Device. ip:%s,port:%d\n", ip.toUtf8().constData(), port);
            }
            if(json.contains("UserName"))
            {
                QString username = json["UserName"].toString();
                QString password = json["PassWord"].toString();
                QString orgname = json["OrgName"].toString();
                AppConfigUtil::saveMqttInfo(username,password,orgname);
                restartMqtt();
                qDebug("UserName:%s,PassWord:%s,OrgName:%s\n", username.toUtf8().constData(), \
                       password.toUtf8().constData(),orgname.toUtf8().constData());
            }


        }
        else if(topicName.contains("SensorInfo"))
        {
            QStringList stringList = topicName.split("/");
            QString ret = stringList[stringList.count()-1];
            QString id = stringList[stringList.count()-2];
            if(id.toInt()!=m_localPara.messageid)
            {
                qDebug("================recv old SensorInfo index=========");
                return;
            }
            if(stringList.count()==5)
            {
                m_localPara.topicStatus = ret.toInt();
                signal_function(&m_localPara);
            }
        }
        else if(topicName.contains("UpdateFile")&&(!topicName.contains("IssueUpdateFile")))
        {
            QJsonObject json = doc.object();
            QString data = json["Content"].toString();
            saveUpdateFile(data);
            int index = json["PackIndex"].toInt();
            if(index == m_updateinfo.rkpkgcount-1)
            {
                checkUpdateRK();
                return;
            }
            QJsonObject obj;
            obj.insert("Md5Code", m_updateinfo.rkmd5code);
            obj.insert("PackCount", m_updateinfo.rkpkgcount);
            obj.insert("PackIndex", m_updateinfo.rkpkgindex++);
            if(m_updateinfo.rkpkgindex != m_updateinfo.rkpkgcount)
            obj.insert("PackMaxLength", 1024);
            else
            obj.insert("PackMaxLength", m_updateinfo.rksize%1024);
            obj.insert("IsPlan", 1);
            if(m_updateinfo.rkpkgindex != m_updateinfo.rkpkgcount)
            obj.insert("IsConfirm", 0);
            else
            obj.insert("IsConfirm", 1);
            QString topic;
            topic = QString("%1/UpdateFile/0").arg(m_uploadTopic);
            qDebug("send topic:%s\n",topic.toUtf8().constData());
            sendMqMessage(topic, obj, 0);
        }
        else if(topicName.contains("IssuePara"))
        {
            QString topic;
            QJsonObject obj;
            topic = QString("%1/IssuePara/0/1").arg(m_uploadTopic);
            qDebug("send topic:%s\n",topic.toUtf8().constData());
            sendMqMessage(topic, obj, 0);

    //        QString topic;
    //        QJsonObject obj;
            m_pos = 2;
            topic = QString("%1/SysPara/0").arg(m_uploadTopic);
            qDebug("send topic:%s\n",topic.toUtf8().constData());
            sendMqMessage(topic, obj, 0);
//            topic = QString("%1/CollectPara/0").arg(m_uploadTopic);
//            qDebug("send topic:%s\n",topic.toUtf8().constData());
//            sendMqMessage(topic, obj, 0);
        }
        else if(topicName.contains("IssueUpdateFile"))
        {
            QString topic;
            QJsonObject obj;

            qDebug("local updateflag:%d,update:%d",m_httpUpdateinfo.updateflag,m_ZIGBEEParam->update);
            if(m_ZIGBEEParam->update==1)
            {
//                topic = QString("%1/IssueUpdateFile/0/1").arg(m_uploadTopic);
//                qDebug("send topic:%s\n",topic.toUtf8().constData());
//                sendMqMessage(topic, obj, 0);
//                topic = QString("%1/UpdateFileList/0").arg(m_uploadTopic);
//                qDebug("send topic:%s\n",topic.toUtf8().constData());
//                sendMqMessage(topic, obj, 0);
                QJsonObject obj;
                obj.insert("Md5Code", m_httpUpdateinfo.md5);
                obj.insert("IsConfirm", 1);
                QString topic;
                topic = QString("%1/UpdateFile/0").arg(m_uploadTopic);
                qDebug("send topic:%s\n",topic.toUtf8().constData());
                sendMqMessage(topic, obj, 0);
                m_ZIGBEEParam->update = 0;
                m_httpUpdateinfo.updateflag = 0;
            }
            else if(m_httpUpdateinfo.updateflag == 0)
            {
                topic = QString("%1/IssueUpdateFile/0/1").arg(m_uploadTopic);
                qDebug("send topic:%s\n",topic.toUtf8().constData());
                sendMqMessage(topic, obj, 0);
                topic = QString("%1/UpdateFileList/0").arg(m_uploadTopic);
                qDebug("send topic:%s\n",topic.toUtf8().constData());
                sendMqMessage(topic, obj, 0);
            }
        }//
        else if(topicName.contains("IssueRestart"))
        {
            SystemReset();
        }
    }
    else if (topicName.contains("ServerOnline"))
    {
        //m_localPara.mqttopen = false;
        close();
//        QString head;
//        QString name;
//        QStringUtil::rsplit(topicName, '/', head, name);

    }
}

void QmqttClient::SystemReset()
{
    qDebug("SystemReset");
    qDebug("================33=================!!!\n");
    system("reboot");
}

int QmqttClient::SuperSystem(const QString& cmd) {
    QProcess process;
    process.start("sh", {"-c", cmd});  // 强制通过 shell 执行
    if (!process.waitForStarted()) return -1;
    if (!process.waitForFinished()) return -1;
    qDebug() << "Output:" << process.readAllStandardOutput();
    qDebug() << "Error:" << process.readAllStandardError();
    return 0;
}

bool QmqttClient::verifyFileMd5(const QString& filePath, const QString& expectedMd5) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot open file: %s", qPrintable(file.errorString()));
        return false;
    }
    QCryptographicHash hash(QCryptographicHash::Md5);
    // 分块读取文件并更新哈希对象
    while (!file.atEnd()) {
        QByteArray block = file.read(4096); // 每次读取4KB的数据
        if (block.isEmpty()) {
            break; // 如果读取失败或到达文件末尾，则停止
        }
        hash.addData(block);
    }

    file.close();

    // 获取最终的哈希结果，并转换为16进制表示的字符串
    QByteArray result = hash.result();
    QString calculatedMd5 = result.toHex().toUpper(); // 确保都转成以便比较

    // 比较计算出的MD5和预期的MD5
    return calculatedMd5 == expectedMd5.toLower();
}

void QmqttClient::checkUpdateRK()
{
    QString file;
    file = QString("/userdata/%1").arg(m_updateinfo.rkname);
    if(verifyFileMd5(file, m_updateinfo.rkmd5code))
    {
        //TODO update
    }
    else
    {
        qDebug("check update md5 error :%s",qPrintable(m_updateinfo.rkname));
    }
}

void QmqttClient::saveUpdateFile(const QString& data)
{
    QString name;
    name = QString("/userdata/%1").arg(m_updateinfo.rkname);
    QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qWarning() << "Failed to open file for writing:" << file.errorString();
    }
    QByteArray base64Bytes = data.toUtf8();
    QByteArray decodedData = QByteArray::fromBase64(base64Bytes);
    file.write(decodedData);
    file.close();
}

void QmqttClient::signal_function(FreqCollectPara* localColect)
{
    qDebug("signal_function\n");
    pthread_mutex_lock(&localColect->mutex);
    localColect->condition = 1; // 改变条件
    pthread_cond_signal(&localColect->cond); // 通知等待线程
    pthread_mutex_unlock(&localColect->mutex);
}

int QmqttClient::waiting_function(int timeout_seconds, FreqCollectPara* localColect)
{
    int ret = 0;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_seconds; // 设置超时时间点
    pthread_mutex_lock(&localColect->mutexwork);
    qDebug("waiting_function\n");
    while(localColect->conditionwork == 0) {
        // 使用条件变量等待，直到条件满足或超时
        if(pthread_cond_timedwait(&localColect->condwork, &localColect->mutexwork, &ts) == ETIMEDOUT) {
            qDebug("Timeout occurred.\n");
            ret = -1;
            break;
        }
    }
    pthread_mutex_unlock(&localColect->mutexwork);
    return ret;
}

void QmqttClient::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_mqttTimer)
    {
        onMqttTimer();
    }
    else if(event->timerId() == m_timeOutTimer)
    {
        qDebug("time out need to reconnect\n");
        reconnect();
        //onTimeOutTimer();
    }

}

void QmqttClient::startMqttTimer()
{
    if (m_mqttTimer <= 0)
    {
        qDebug("startMqttTimer <=0\n");
        m_mqttTimer = startTimer(1000 * 5);
    }
}

void QmqttClient::startMqttTimer(int second)
{
    if (m_mqttTimer <= 0)
    {
        //qDebug("startMqttTimer1 <=0\n");
        m_mqttTimer = startTimer(1000 * second);
    }
}

void QmqttClient::stopMqttTimer()
{
    if (m_mqttTimer > 0)
    {
        killTimer(m_mqttTimer);
        m_mqttTimer = 0;
    }
}

void QmqttClient::onMqttTimer()
{
    qDebug("onMqttTimer");
    m_opened = false;
    m_connected = false;
    m_mqttClient->disconnectFromHost();

    m_mqttClient->connectToHost();
}

void QmqttClient::startTimeOutTimer()
{
    if (m_timeOutTimer <= 0)
    {
        m_timeOutTimer = startTimer(1000 * 15);
    }
}

void QmqttClient::stopTimeOutTimer()
{
    if (m_timeOutTimer > 0)
    {
        killTimer(m_timeOutTimer);
        m_timeOutTimer = 0;
    }
}

void QmqttClient::onTimeOutTimer()
{
    if (!m_opened)
    {
        return;
    }
    m_opened = false;
    m_connected = false;
    m_mqttClient->disconnectFromHost();
    qDebug("mqtt TimeOut reconnect.\n");
    m_mqttClient->connectToHost();
}

