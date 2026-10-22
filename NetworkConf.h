#ifndef NETWORKCONF_H
#define NETWORKCONF_H

#include "BasicType.h"
#include <QString>
#include <vector>
#include <string>

#include "NetConfig.h"


struct NetworkConfig
{
    QString ifname;
    bool dhcp;
    QString ip;
    QString mask;
    QString gate;

    NetworkConfig():
        dhcp()
    {
    }
};

typedef std::vector< NetworkConfig >    NetworkConfigs;

struct AuxNetworkAddr
{
    QString ip;
    QString mask;

    AuxNetworkAddr()
    {
    }
};

typedef std::vector< AuxNetworkAddr >   AuxNetworkAddrs;



class NetworkConf
{
public:
    static const char* FILE_NAME;
    static const int   MAX_AUX_ADDR = 4;

    static std::string getConfPath();
    static std::string getScriptPath(const std::string& ifname);
    static std::string getAuxScriptPath();
    static std::string getAuxInterface();

    static bool getMacAddr(const QString& ifname, QString& mac);

    static net::NetConfig convert(const NetworkConfig& config);
    static NetworkConfig convert(const net::NetConfig& config);

public:
    NetworkConf();

    NetworkConfigs m_configs;
    AuxNetworkAddrs m_auxAddrs;

    int findConfig(const QString& ifname);

    bool findConfig(const QString& ifname, NetworkConfig& config);

    bool getConfig(int index, NetworkConfig& config);
    NetworkConfig getConfig(int index);

    void ensure(size_t count);


    bool load();
    bool save() const;

    bool check(const NetworkConfig& networkConfig) const;

    bool apply();
    bool applycard(int i);
    bool checkApply(const NetworkConfig& networkConfig);

    void execute();

	void dump();
	
    void applyAuxAddrs();

    std::string getAuxIfName() const;

private:
    bool saveScript(const NetworkConfig& networkConfig) const;

    bool isConfigChanged(const std::string& ifname, const NetworkConfig& networkConfig);

    bool saveAuxScript() const;

};

#endif // NETWORKCONF_H
