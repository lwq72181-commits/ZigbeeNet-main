#ifndef WPACLIENT_H
#define WPACLIENT_H

#include <QObject>
#include "BasicType.h"
#include <string>
#include <QSocketNotifier>
#include <vector>

#include "ShellProcess.h"


struct WpaScanResult
{
    int id;
    QString bssid;
    int freq;
    int level;
    QString flags;
    QString ssid;

    WpaScanResult():
        id(),
        freq(),
        level()
    {
    }
};

struct WpaNetwork
{
    int id;
    QString ssid;
    QString  bssid;
    QString  flags;

    WpaNetwork():
        id()
    {
    }
};


struct WpaStatus
{
    QString bssid;
    QString ssid;
    int id;
    QString passphrase;
    QString psk;
    QString mode;
    QString pairwise_cipher;
    QString group_cipher;
    QString key_mgmt;
    QString wpa_state;
    QString address;
    int signal_level;

    WpaStatus():
        id(),
        signal_level()
    {
    }
};


typedef std::vector< WpaNetwork >       WpaNetworks;
typedef std::vector< WpaScanResult >    WpaScanResults;


class WpaClient : public QObject
{
    Q_OBJECT
public:
    WpaClient(QObject *parent = nullptr);
    ~WpaClient();

    bool open(const char* ctrl_path, const char *ifname);

    void close();

    bool isOpen();

    int ctrlRequest(const char *cmd, char *reply, size_t *reply_len);

    int ctrlRequest(const char *cmd);

    int ctrlRequest(const std::string& cmd);

    int ctrlRequest(const char *cmd, QString& value);

public:
    int status(WpaStatus& wpaStatus);

    int list_network(WpaNetworks& networks);

    int scan();

    //int scan_result(WpaScanResults& results);

    int get_network(int index, const char* variable, QString& value);
    int set_network(int index, const char* variable, const QString& value);

    int enable_network(int index);
    int disable_network(int index);
    int select_network(int index);

    int add_network();
    int remove_network(int index);

    int bss(int index, WpaScanResult& scanResult);


public:
    void setConfPath(const QString& filepath);

    bool getNetwork(QString& ssid, QString& psk);
    bool setNetwork(const QString& ssid, const QString& psk);

    bool isWpaConnected();

    bool getIp(QString& ip);

    bool setDhcp();

public:
    static bool getNetwork(const QString& filepath, QString& ssid, QString& psk);
    static bool setNetwork(const QString& filepath, const QString& ssid, const QString& psk);

    static QString getDefConf();

    static bool isConnected(const WpaStatus& wpaStatus);

    static bool setIfAddr(const char *ifname, QString& ip);
    static bool getIfAddr(const char *ifname, QString& ip);
    static bool parseDhcp(const QStringList& lines, QString& ip, int& leaseTime);

public:

    static bool parseReply(const char* reply, size_t len, WpaStatus& wpaStatus);
    static bool parseReply(const QString& line, WpaNetwork& network);
    static bool parseReply(const char* reply, size_t len, WpaNetworks& networks);
    static bool parseReply(const char* reply, size_t len, WpaScanResult& result);

    static bool isFail(const char* reply, size_t len);

    static int split(const QString& line, char seperator, QString& key, QString& value);

    typedef std::map<QString, QString>  QStringMap;
    static bool getConf(const QStringList& lines, QStringMap& table);
    static bool setConf(QStringList& lines, const QStringMap& table);

    static bool load(const QString& filepath, QStringList& lines);
    static bool save(const QString& filepath, const QStringList& lines);

    static int findKey(const QStringList& lines, const QString& key);
    static bool getValue(const QStringList& lines, const QString& key, QString& value);
    static bool setValue(QStringList& lines, const QString& key, const QString& value);
    static bool getUnquoteValue(const QStringList& lines, const QString& key, QString& value);

    static QString quote(const QString& text);
    static QString unquote(const QString& text);

public:



protected:
    virtual void timerEvent(QTimerEvent *event) override;

protected:
    int openCtrlConnection(const char* ctrl_path, const char *ifname);
    void closeCtrlConnection();
    int reopenCtrlConnection();

    void startConnTimer();
    void stopConnTimer();
    void onConnTimer();

    void ping();

    void processMsg(char *msg);

    void startRetryTimer();
    void stopRetryTimer();
    void onRetryTimer();

protected slots:
    void receiveMsgs();
    void onSocketNotifierActivated(int socket);

    void onCtrlConnected();

    void onWpaConnected();

    void onDhcpFinished(int error, const QStringList& lines);

signals:
    void wpaConnected();
    void wpaDisconnected();
    void wpaScanResult();

    void ctrlConnectionStatus(int status);

protected:
    std::string m_ctrlPath;
    std::string m_ifname;

    struct wpa_ctrl* m_ctrlConnection;
    struct wpa_ctrl* m_monitorConntion;

    QSocketNotifier* m_socketNotifier;
    int m_connTimer;
    int m_retryTimer;

    QString m_confpath;

    ShellProcess    m_dhcp;

};

#endif // WPACLIENT_H
