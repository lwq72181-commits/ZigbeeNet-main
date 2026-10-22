#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QTranslator>
//#include "SystemLog.h"
#include "NtpSynchro.h"
#include "NtpService.h"
#include "WpaClient.h"
#include <QMainWindow>
#include <QWizard>
#include "fqmqttclient.h"
#include "Dmamemcpy.h"
#include "UdpMessageHanlde.h"
#include "TCPServer.h"
#include "BuleTooth.h"
#include "Uart458client.h"
#include "Uart485Server.h"
#include "AD4134.h"
#include "QHttpClient.h"
#include "Acquisitiontype.h"
#include "ThreadSafeMultiVector.h"
#include "zigbeebase.h"
#include "zigbee/proto_parse.h"
#include "SensorLogManager.h"

class Application : public QApplication
{
    Q_OBJECT
signals:
    void operateDMA();
    void operateUDP();
    void operateBLE();
    void operate485(TCPParam *param,ThreadSafeMultiVector *vector);
    void operateTCP();
    void resetUpdateInfo();
public:
    Application(int &argc, char **argv);
    ~Application();

    static void bootstrap(const QString& filepath);

    bool init();

    void quit();

//    SystemLog*  systemLog();

    NtpService& ntpService();
    QmqttClient& mqttService();
    //WpaClient& wpaClient();
    DmaMemcpy* dmaMemcpy();
    ThreadSafeMultiVector* threadVector();
    SensorLogManager * logManager();
    TCPParam* tcpParam();
    void downLoadUpdate(const HttpUpgradeInfo& info, const QString& filepath);
    void setSystemTime(const QDateTime& dt);
private:
    void checkNetwork();
    void startPeriodicCheck(int interval) ;
public:
    static QMainWindow*	getMainWindow();
    static void setMainWindow(QMainWindow* window);
public slots:
    void restartMqtt();
    void restart485();
    void set485sn();
    void onUpdateHttp(const HttpUpgradeInfo &info);
    void on_upgradeDownloadDone(const HttpUpgradeInfo& info, const QString& filepath);
    void on_upgradeDownloadFail(const HttpUpgradeInfo& info, const QString& filepath);
    void setTCPParm(int index,int wave,int longwave);
    void startTickLed(int set);
    void IOCtrlTime();
    void SysRebootTime();
protected:
    static bool setupNetwork();
    static void setupResolvConf();

protected:
    void setupPath(const QString& path);
    int setupADC();
    int setupBlueTooth();
    int setupUart485();
    int setupDMA();
    int setupUdp();
    int setupTCP();
    void setupMqtt();
    int setupZigbee();
//    void setupLog();
    int initSignal();
    void loadQss();

    void setupLang();

    void setupConfig();

//    void setupSystemLog();

    void setupNtpService();

   // void setupWpa();
    //void teardownWpa();

    bool checkSetTime();
    bool verifyFileMd5(const QString& filePath, const QString& expectedMd5);
    int checkConfigFile();
    int recoverLocalParam();
    int bakeLocalParam();
private:

//    QTranslator m_zhTranslator;

//    SystemLog*  m_systemLog;

    NtpService  m_ntpService;
    QMainWindow*    m_mainWindow;
    //WpaClient   m_wpaClient;
    QmqttClient m_mqtt;
    DmaMemcpy* m_dma;
    QThread m_DMAThread;
    QThread m_UDPThread;
    QThread m_BLUETOOTHThread;
    QThread m_Uart485Thread;
    UdpMessageHandle * m_UdpMessage;
    TCPServer * m_TCPS;
    QThread *m_tcpThread;
    QTimer *m_checktime;
    QTimer *m_reboottime;
    AD4134 * m_ADC;
    Buletooth * m_bluetooth;
    Uart458Client * m_uart485;
    Uart485Server * m_uart485server;
    TCPParam * m_TCPparam;
    ZIGBEEParam * m_ZIGBEEParam;
    ThreadSafeMultiVector * m_threadVector;
    QString m_name;
    QNetworkAccessManager *m_netmanager;
    zigbeebase *m_zigbeebase;
    SensorLogManager *m_LogManager;
    int m_downloadCount;
    int m_downloadTimer;
    bool m_led;
    int m_ledFlagold;
    int m_ledFlag;
    int m_errorled;
};

#endif // APPLICATION_H
