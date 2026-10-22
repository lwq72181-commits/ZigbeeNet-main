#include <QTimer>
#include "Application.h"
#include "QPath.h"
#include "CLog.h"
#include "AppLog.h"
#include "AppConst.h"
#include "AppConfig.h"
#include "AppDir.h"
#include "TRandom.h"
#include "QFileUtil.h"
#include "SettingArchive.h"
#include "AppConfigUtil.h"
#include "Socket.h"
#include <QWizard>
#include "Acquisitiontype.h"
#include "NetworkConf.h"
#include "ResolvConf.h"


Application::Application(int &argc, char **argv):
    QApplication(argc, argv),
    m_mainWindow()
{
//    QString md = "DA01D434FB3AAD80DB38E3D4D5B4CDBF";
//    if(verifyFileMd5("/userdata/update.img",md))
//    {
//        qDebug("OK=============");
//    }
    m_netmanager = new QNetworkAccessManager();
    m_LogManager = new SensorLogManager();
//    comn::Random::makeSeed();
//    comn::Socket::startup();
    m_downloadCount = 0;
    m_led = 0;
    m_ledFlag = 1;
    m_ledFlagold = -1;
    m_errorled = -1;
    setupPath(argv[0]);
    if(checkConfigFile()==-1)
    {
        setupPath(argv[0]);
    }
    else
        bakeLocalParam();
    setupNetwork();
    //setupWpa();

    //setupNtpService();

    //setupADC();
    //setupDMA();

    loadQss();
    setupLang();
    setupBlueTooth();
    emit operateBLE();
    setupTCP();
    startPeriodicCheck(1000);
    setupUart485();
    emit operate485(m_TCPparam,m_threadVector);
    setupZigbee();
    //emit operateDMA();
//    setupUdp();
    //QThread::sleep(10);
    setupMqtt();

    //emit operateUDP();
    initSignal();
}

Application::~Application()
{
    quit();

    //teardownWpa();

    //comn::Socket::cleanup();
}

int Application::initSignal()
{
    return 0;
}

int Application::recoverLocalParam()
{
    QProcess process;
    //QString cmd = "rm -f /userdata/etc/Acquisition.ini.lock && cp /oem/dma/qt/Acquisition.ini /userdata/etc/ && sync";
    QString cmd = "rm -f /userdata/etc/*.lock && cp /oem/dma/qt/etc/* /userdata/etc/ && sync";
    qDebug()<<cmd;
    process.start("/bin/sh", QStringList() << "-c" << cmd);
    //process.start(cmd);
    if (!process.waitForStarted()) {
        qDebug() << "Failed to saveLocalParam.";
        return -1;
    }
    if (!process.waitForFinished()) {
        qDebug() << "Process saveLocalParam did not finish date in time.";
        return -1;
    }
    return 0;
}

int Application::bakeLocalParam()
{
    QProcess process;
    //QString cmd = "cp /userdata/etc/Acquisition.ini /oem/dma/qt/  && sync";
    QString cmd = "cp /userdata/etc/* /oem/dma/qt/etc/  && sync";
    qDebug()<<cmd;
    process.start("/bin/sh", QStringList() << "-c" << cmd);
    //process.start(cmd);
    if (!process.waitForStarted()) {
        qDebug() << "Failed to saveLocalParam.";
        return -1;
    }
    if (!process.waitForFinished()) {
        qDebug() << "Process saveLocalParam did not finish date in time.";
        return -1;
    }
    return 0;
}

int Application::checkConfigFile()
{
    int mid = 0;
    AppConfig::get("App/NtpTimer", mid);
    if(mid != 2)
    {
        qDebug() << "check error need to recoverLocalParam";
        recoverLocalParam();
        return -1;
    }
    bool ret = false;
    NetworkConf networkConf;
    ret = networkConf.load();
    if(!ret)
    {
        qDebug() << "check error need to recoverLocalParam net";
        recoverLocalParam();
        return -1;
    }
    if (!m_LogManager->load(LOG_PATH)) {
        qWarning() << "Failed to load sensor data file";
        recoverLocalParam();
        return -1;
    }
    return 0;
}

void Application::startPeriodicCheck(int interval)
{
    m_checktime = new QTimer(this);
    connect(m_checktime, &QTimer::timeout, this, &Application::IOCtrlTime);
    m_checktime->start(interval); // 设置定时器间隔
//
    m_reboottime = new QTimer(this);
    int mode;
    AppConfig::get("Device/update", mode);
    if((mode>>1)&0x01 == 0x01)
    {
        connect(m_reboottime, &QTimer::timeout, this, &Application::SysRebootTime);
        m_reboottime->start(3600*1000); // 设置定时器间隔
    }
    IOCtrlTime();
}

void Application::startTickLed(int set)
{
    m_ledFlag = set;
    if(m_reboottime->isActive()&&set == 0)
    {
        qDebug("====stopTick system reboot");
        m_reboottime->stop();
    }
    return;
    if(set == 0)
    {
        if(m_checktime->isActive())
        {
            qDebug("====stopTickLed");
            m_checktime->stop();
        }
        system("echo 1 > /sys/class/gpio/gpio58/value");
    }
    else
    {
        if(!m_checktime->isActive())
        {
            qDebug("====startTickLed");
            m_checktime->start(800); // 设置定时器间隔
        }
    }
}

int Application::setupZigbee()
{
    //qDebug("m_ZIGBEEParam:%d",m_ZIGBEEParam);
    //qDebug("m_ZIGBEEParam:%d,sensor:%d",m_ZIGBEEParam,m_ZIGBEEParam->sensor_info);
    m_zigbeebase = new zigbeebase();
    m_zigbeebase->init(m_ZIGBEEParam,m_LogManager);
    return 0;
}

int Application::setupTCP()
{
    m_TCPparam = new TCPParam();
    m_TCPparam->index = 600;
    m_TCPparam->wave = 7200;
    m_TCPparam->longwave = 86400;
    m_TCPparam->changed = false;
    m_TCPparam->flag = false;
    m_TCPparam->channel = 0;
    m_TCPparam->status = 0;
    m_TCPparam->mutex = PTHREAD_MUTEX_INITIALIZER;
    m_TCPparam->cond = PTHREAD_COND_INITIALIZER;
    m_TCPparam->condition = 0;
    m_TCPparam->name.append("ZHighSquare");
    m_TCPparam->name.append("ZHighSpeed");
    m_TCPparam->name.append("ZLowSquare");
    m_TCPparam->name.append("ZLowSpeed");
    m_TCPparam->name.append("XLowSquare");
    m_TCPparam->name.append("XLowSpeed");
    m_TCPparam->name.append("YLowSquare");
    m_TCPparam->name.append("YLowSpeed");
    m_TCPparam->name.append("Temperature");
    m_TCPparam->name.append("Vlotage");
    m_threadVector = new ThreadSafeMultiVector();
    m_TCPS = new TCPServer(m_TCPparam,m_threadVector);
    m_tcpThread = new QThread;
    m_TCPS->moveToThread(m_tcpThread);

    //connect(this, &Application::operateTCP, m_TCPS, &TCPServer::dorun);
    connect(m_tcpThread, &QThread::started, m_TCPS, &TCPServer::dorun);
    connect(m_tcpThread, &QThread::finished, m_TCPS, &QObject::deleteLater);
    connect(m_tcpThread, &QThread::finished, m_tcpThread, &QThread::deleteLater);
    m_tcpThread->start();
//    connect(m_TCPS, &QThread::finished, m_TCPS, &QObject::deleteLater);
//    connect(m_TCPS, &QThread::finished, [](){
//            qDebug("thread finished!!!");
//        });
    //connect(m_TCPS, &TCPServer::restartMqttSignal, this, &Application::restartMqtt);
    return 0;
}

void Application::setTCPParm(int index,int wave,int longwave)
{
    //m_TCPS->TickTimePeriod();
    m_TCPparam->changed = true;
    m_TCPparam->index = index;
    m_TCPparam->wave = wave;
    m_TCPparam->longwave = longwave;
}

void Application::setupMqtt()
{
    FREUser user;
    AppConfigUtil::loadUser(user);
    connect(&m_mqtt, &QmqttClient::updateHttp, this, &Application::onUpdateHttp);
    connect(this, &Application::resetUpdateInfo,&m_mqtt, &QmqttClient::setUpdateInfo);
    connect(&m_mqtt, &QmqttClient::setLedTime,this, &Application::startTickLed);
    //connect(&m_mqtt, &QmqttClient::setmqttUpdateTimeSignal,m_TCPS, &TCPServer::TickTimePeriod);
    connect(&m_mqtt, &QmqttClient::setmqttUpdateTimeSignal,this, &Application::setTCPParm);
    connect(&m_mqtt, &QmqttClient::restartMqtt, this, &Application::restartMqtt);
    //emit operateTCP();
    //m_TCPS->start(); // 启动线程
    qDebug() << "====================m_TCPS thread:" << m_TCPS->thread();
    qDebug() << "====================m_mqtt thread:" << m_mqtt.thread();
    qDebug("Device. username:%s, password: %s,orgname: %s, devid: %s\n",
        qPrintable(user.username), qPrintable(user.password), qPrintable(user.orgname), qPrintable(user.devid));
    m_mqtt.setMqUrl(user.url, user.username, user.password, user.orgname, user.devid, user.port,m_TCPparam,m_ZIGBEEParam);
}

int Application::setupUdp()
{
    m_UdpMessage = new UdpMessageHandle();
    m_UdpMessage->moveToThread(&m_UDPThread);
    connect(this, &Application::operateUDP, m_UdpMessage, &UdpMessageHandle::MulticastHandle, Qt::QueuedConnection);
    m_UDPThread.start();
    return 0;
}

int Application::setupADC()
{
    m_ADC = new AD4134();
    m_ADC->init();
    return 0;
}

int Application::setupBlueTooth()
{
    m_ZIGBEEParam = new ZIGBEEParam();
    m_ZIGBEEParam->update = 0;
    AppConfig::get("Device/BLTTYName", m_name);
    m_bluetooth = new Buletooth();
    m_bluetooth->init("/dev/ttyS3"/*m_name*/,m_ZIGBEEParam);
    m_bluetooth->moveToThread(&m_BLUETOOTHThread);
    connect(this, &Application::operateBLE, m_bluetooth, &Buletooth::BLEHandle, Qt::QueuedConnection);
    connect(m_bluetooth, &Buletooth::BLRestartMqtt, this, &Application::restartMqtt);
    connect(m_bluetooth, &Buletooth::BLSetDEVID, this, &Application::restartMqtt);
    //connect(m_bluetooth, SIGNAL(BLSetDEVID(const QString&)), &m_mqtt, SLOT(setdevID(const QString&)));
    m_BLUETOOTHThread.start();
    return 0;
}

int Application::setupUart485()
{
#ifndef GATE_485_SERVER
    m_uart485 = new Uart458Client();
    m_uart485->init("/dev/ttyS7"/*m_name*/);
    m_uart485->moveToThread(&m_Uart485Thread);
    connect(this, &Application::operate485, m_uart485, &Uart458Client::UartHandle);
    connect(m_bluetooth, &Buletooth::BLRestart485, this, &Application::restart485);
    connect(m_bluetooth, &Buletooth::BLset485sn, this, &Application::set485sn);
    m_Uart485Thread.start();
#else
    m_uart485server = new Uart485Server();
    m_uart485server->init("/dev/ttyS7"/*m_name*/);
    m_uart485server->moveToThread(&m_Uart485Thread);
    connect(this, &Application::operate485, m_uart485server, &Uart485Server::UartServerHandle);
    connect(m_bluetooth, &Buletooth::BLRestart485, this, &Application::restart485);
    connect(m_bluetooth, &Buletooth::BLset485sn, this, &Application::set485sn);
    m_Uart485Thread.start();
#endif
    return 0;
}


int Application::setupDMA()
{
    m_dma = new DmaMemcpy();
    m_dma->moveToThread(&m_DMAThread);
    connect(this, &Application::operateDMA, m_dma, &DmaMemcpy::dowork, Qt::QueuedConnection);
    m_DMAThread.start();
    //m_dma->dowork();
    return 0;
}

void Application::bootstrap(const QString& filepath)
{

#ifndef WIN32
    qputenv("QT_IM_MODULE", QByteArray("Qt5Input"));
#endif //

}

bool Application::init()
{
    //return true;
    //setupSystemLog();
//    FREUser user;
//    AppConfigUtil::loadUser(user);
//    connect(&m_mqtt, &QmqttClient::updateHttp, this, &Application::onUpdateHttp);
//    qDebug("Device. username:%s, password: %s,orgname: %s, devid: %s\n",
//        qPrintable(user.username), qPrintable(user.password), qPrintable(user.orgname), qPrintable(user.devid));
//    m_mqtt.setMqUrl(user.url, user.username, user.password, user.orgname, user.devid, user.port);

    return true;
}

void Application::onUpdateHttp(const HttpUpgradeInfo &info)
{
    QString filepath;
    m_downloadCount = 0;
    filepath = QString("/userdata/%1").arg(info.name);
    if (QFile(filepath).exists())
    {
        qDebug("update file exits need to rm!!!!");
        QFile(filepath).remove();
    }
    downLoadUpdate(info, filepath);
}

void Application::restart485()
{
#ifndef GATE_485_SERVER
    m_uart485->config_serial_port();
#else
    m_uart485server->config_serial_port();
#endif
}

void Application::set485sn()
{
#ifndef GATE_485_SERVER
    m_uart485->updateinfo();
#else
    m_uart485server->updateinfo();
#endif
}


void Application::restartMqtt()
{
    FREUser user;
    AppConfigUtil::loadUser(user);
    qDebug("restartMqtt Device. username:%s, password: %s,orgname: %s, devid: %s\n",
        qPrintable(user.username), qPrintable(user.password), qPrintable(user.orgname), qPrintable(user.devid));
    m_mqtt.close();
    m_mqtt.setMqUrl(user.url, user.username, user.password, user.orgname, user.devid, user.port, m_TCPparam, m_ZIGBEEParam);
}

void Application::quit()
{
    //m_mutex.close();

}

void Application::SysRebootTime()
{
    qDebug("net error now need to restart rk board!!!!\n");
    qDebug("================22=================!!!\n");
    system("reboot");
}

void Application::IOCtrlTime()
{
    if(m_ledFlag == 0)
    {
        if(m_ledFlagold != m_ledFlag)
        {
            system("echo 1 > /sys/class/gpio/gpio58/value");
            m_ledFlagold = m_ledFlag;
        }
        if(m_led == 0)
        {
            m_led = 1;
            system("echo 1 > /sys/class/gpio/gpio18/value");
        }
        else
        {
            m_led = 0;
            system("echo 0 > /sys/class/gpio/gpio18/value");
        }
    }
    else if(m_ledFlag == 1)
    {
        if(m_ledFlagold != m_ledFlag)
        {
            m_ledFlagold = m_ledFlag;
        }
        if(m_led == 0)
        {
            m_led = 1;
            system("echo 1 > /sys/class/gpio/gpio58/value;echo 1 > /sys/class/gpio/gpio18/value");
        }
        else
        {
            m_led = 0;
            system("echo 0 > /sys/class/gpio/gpio58/value;echo 0 > /sys/class/gpio/gpio18/value");
        }
    }
    else
    {
        if(m_led == 0)
        {
            m_led = 1;
            system("echo 1 > /sys/class/gpio/gpio18/value");//feed watchdog
        }
        else
        {
            m_led = 0;
            system("echo 0 > /sys/class/gpio/gpio18/value");
        }
    }
    if(m_errorled!=m_TCPparam->status)
    {
        qDebug("=============led:%d,status:%d",m_errorled,m_TCPparam->status);
        m_errorled = m_TCPparam->status;
        if(m_TCPparam->status == 0)
        {
            system("echo 1 > /sys/class/gpio/gpio57/value");
        }
        else
        {
            system("echo 0 > /sys/class/gpio/gpio57/value");
        }
    }

}

void Application::checkNetwork()
{
    if(m_led == 0)
    {
        //m_led = 1;
        system("echo 1 > /sys/class/gpio/gpio58/value");
    }
    else
    {
        //m_led = 0;
        system("echo 0 > /sys/class/gpio/gpio58/value");
    }
    return;
    QUrl url("http://www.baidu.com"); // A known URL that returns HTTP 204 No Content
    QNetworkRequest request(url);
    QNetworkReply *reply = m_netmanager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            system("echo 1 > /sys/class/gpio/gpio58/value");
            qDebug() << "Internet connection is available.";
        } else {
            int mid;
            AppConfig::get("Network/NET", mid);
            char* ccmd;
            if(mid == 0)
            {
                qDebug("====================checkNetwork1");
                qDebug() << "====================checkNetwork1";
                ccmd = "killall udhcpc;echo 0 > /sys/class/gpio/gpio58/value;udhcpc -i eth1 &";
            }
            else if(mid == 1)
            {
                int wifidhcp;
                AppConfig::get("Network/dhcp", wifidhcp);
                if(wifidhcp==1)
                {
                    qDebug() << "====================checkNetwork2";
                    qDebug("====================checkNetwork2");
                    ccmd = "killall udhcpc;echo 0 > /sys/class/gpio/gpio58/value;udhcpc -i wlan0 &";
                }
                else
                {
                    qDebug() << "====================checkNetwork22";
                    ccmd = "killall udhcpc;echo 0 > /sys/class/gpio/gpio58/value";//TODO CONFIG WIFI
                }
            }
            else if(mid == 2)
            { qDebug("====================checkNetwork3");
                qDebug() << "====================checkNetwork3";
                ccmd = "killall udhcpc;echo 0 > /sys/class/gpio/gpio58/value;udhcpc -i usb0 &";
            }
            else if(mid == 3)
            { qDebug("====================checkNetwork4");
                qDebug() << "====================checkNetwork4";
                ccmd = "killall udhcpc;echo 0 > /sys/class/gpio/gpio58/value;ifconfig pcie0 up;ifconfig sipa_dummy0 up;udhcpc -i pcie0 &";
            }
            system(ccmd);
            qDebug() << "No internet connection:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void Application::downLoadUpdate(const HttpUpgradeInfo& info, const QString& filepath)
{
    auto successHandler = [=] ()
    {
        on_upgradeDownloadDone(info, filepath);
    };

    auto failHandler = [=] (const QString &, int)
    {
        on_upgradeDownloadFail(info, filepath);
    };
    qDebug("start download file");
    HttpClient(info.url).complete(successHandler).fail(failHandler).download(filepath);
}

bool Application::verifyFileMd5(const QString& filePath, const QString& expectedMd5)
{
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
    qDebug("calculatedMd5:%s,expectedMd5:%s",qPrintable(calculatedMd5),qPrintable(expectedMd5.toUpper()));
    // 比较计算出的MD5和预期的MD5
    return calculatedMd5 == expectedMd5.toUpper();
}

void Application::on_upgradeDownloadDone(const HttpUpgradeInfo& info, const QString& filepath)
{
    qDebug(" download file on_upgradeDownloadDone");
    if(verifyFileMd5(filepath,info.md5))
    {
        QString cmd;
        if(info.type == 0)
        {
            cmd = QString("update ota %1").arg(filepath);
            qDebug("filepath:%s",qPrintable(filepath));
        }
        else if(info.type == 1||info.type == 2)
        {
            cmd = QString("mv %1 /userdata/fw/wless/").arg(filepath);
            qDebug("filepath:%s",qPrintable(filepath));
        }
        qDebug("cmd:%s",qPrintable(cmd));
        system(cmd.toUtf8().data());
    }
}

void Application::on_upgradeDownloadFail(const HttpUpgradeInfo& info, const QString& filepath)
{
    qDebug(" download file on_upgradeDownloadFail");
    m_downloadCount++;
    if (m_downloadCount > 3)
    {
        emit resetUpdateInfo();
    }
    else
    {
        downLoadUpdate(info, filepath);
    }
}

void Application::setupPath(const QString& path)
{
//    QString appDir = QPath::dirname(path);
//    QPath::abspath(appDir);
//    AppConst::setAppDirPath(appDir);

//    QString logDir = AppDir::getLogDir();
//    if (!QPath::exist(logDir))
//    {
//        QPath::createDirectories(logDir);
//    }

    QString etcDir = AppDir::getEtcDir();
    if (!QPath::exist(etcDir))
    {
        QPath::createDirectories(etcDir);
    }

    QString confPath = QPath::join(etcDir, "Acquisition.ini");
    if (!QPath::exist(confPath))
    {
        QString setupDir = QPath::join(AppDir::getAppDir(), SettingArchive::SETUP_DIR);
        QString srcPath = QPath::join(setupDir, "Acquisition.ini");
        QFileUtil::copy(srcPath, etcDir, false);
    }

    AppConfig::setFilePath(confPath);

    //setupLog();

//    setupConfig();

}


//void Application::setupLog()
//{
//    AppLog::setup();

//    CLog::setLogger(CLog::COUT);

//    CLog::setFileParam("himeet.log", 8, 3);
//    CLog::setLogger(CLog::FILE);

//	CLog::flushOn(CLog::kInfo);

//    CLog::info("-------- Acquistion. version: %s\n", APP_VERSION);
//}

void Application::loadQss()
{
//    QString qss;
//    if (QFileUtil::load(":/qss/app.qss", qss))
//    {
//        setStyleSheet(qss);
//    }
}

void Application::setupLang()
{
    QLocale::setDefault(QLocale(QLocale::Chinese));

//    if (m_zhTranslator.load("HiMeet.qm", applicationDirPath()))
//    {
//        if (!installTranslator(&m_zhTranslator))
//        {
//            CLog::warning("failed to installTranslator\n");
//        }
//    }
}


void Application::setupConfig()
{
//    AppConfig::get("Network/MsgPort", AppConst::MSG_PORT);

}

void Application::setupResolvConf()
{
    qDebug("ResolvConf::FILE_NAME:%s",qPrintable(ResolvConf::FILE_NAME));
    QString filepath = QPath::join(AppDir::getEtcDir(), ResolvConf::FILE_NAME);
    qDebug("filepath:%s",qPrintable(filepath));
    qDebug() << QString::fromStdString(ResolvConf::getFilePath());
    ResolvConf::merge(filepath.toStdString(), ResolvConf::getFilePath());
}


bool Application::setupNetwork()
{
    NetworkConf networkConf;
    networkConf.load();
    networkConf.execute();

    setupResolvConf();

    return true;
}

//SystemLog*  Application::systemLog()
//{
//    return m_systemLog;
//}

NtpService& Application::ntpService()
{
    return m_ntpService;
}

QmqttClient& Application::mqttService()
{
    return m_mqtt;
}

//WpaClient& Application::wpaClient()
//{
//    return m_wpaClient;
//}

DmaMemcpy* Application::dmaMemcpy()
{
    return m_dma;
}

ThreadSafeMultiVector* Application::threadVector()
{
    return m_threadVector;
}

SensorLogManager * Application::logManager()
{
    return m_LogManager;
}

TCPParam* Application::tcpParam()
{
    return m_TCPparam;
}

//void Application::setupSystemLog()
//{
//    QString logPath = QPath::join(AppDir::getLogDir(), "userlog.db");
//    std::string fullpath = TextCodecUtil::convert(logPath);
//    m_systemLog->open(fullpath.c_str(), "");
//}

void Application::setupNtpService()
{
    checkSetTime();

    QString timeServer = "ntp1.aliyun.com";
    AppConfig::get("Time/Server", timeServer);

    bool autoSync = true;
    AppConfig::get("Time/AutoSync", autoSync);

    m_ntpService.setupNtpClient();

    m_ntpService.setCycle(3600 * 6);
    m_ntpService.setServer(timeServer);
    m_ntpService.setEnabled(autoSync);

    if (autoSync)
    {
        m_ntpService.trySync();
    }
}

QMainWindow* Application::getMainWindow()
{
    Application* app = (Application*)instance();
    return app->m_mainWindow;
}

void Application::setMainWindow(QMainWindow* window)
{
    Application* app = (Application*)instance();
    app->m_mainWindow = window;
}

//void Application::setupWpa()
//{
//    QString confPath = "/mnt/Acquisition/etc/wpa_supplicant.conf";

//    m_wpaClient.setConfPath(confPath);

//    m_wpaClient.open("/var/run/wpa_supplicant", "wlan0");
//}

//void Application::teardownWpa()
//{
//    m_wpaClient.close();
//}

void Application::setSystemTime(const QDateTime& dt)
{
#ifdef WIN32
    //
#else
    QString str = dt.toString("yyyy-MM-dd HH:mm:ss");
    std::string cmd = QString("date -s \"%1\"").arg(str).toStdString();

    qDebug("%s\n", cmd.c_str());

    system(cmd.c_str());
#endif //

    QDate date = dt.date();
    QTime time = dt.time();

    //Hid_RtcTime rtcTime = Hid_RtcTime();
//    rtcTime.year = date.year();
//    rtcTime.month = date.month();
//    rtcTime.date = date.day();
//    rtcTime.hour = time.hour();
//    rtcTime.minute = time.minute();
//    rtcTime.second = time.second();

    //hid_setRtcTime(rtcTime);
}

bool Application::checkSetTime()
{
    /// 当前时间是 epoch 那天, 就修改为 2020 年.
    bool changed = false;
    QDateTime dt = QDateTime::currentDateTime();
    QDate date = dt.date();
    if (date.year() == 1970)
    {
        date.setDate(2020, 9, 1);
        dt = QDateTime(date, dt.time());

        setSystemTime(dt);

        changed = true;
    }
    return changed;
}
