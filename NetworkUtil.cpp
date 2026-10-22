#include "NetworkUtil.h"
#include "NetworkConf.h"

#include "Socket.h"
#include "TStringUtil.h"

#ifdef WIN32

#include <Iphlpapi.h>
#pragma comment(lib,"Iphlpapi.lib")

#else

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#include <arpa/inet.h>

#endif //


NetworkUtil::NetworkUtil()
{

}

std::string NetworkUtil::getHostIp()
{
    std::string ip = comn::SockAddr::getHostIP();

    return ip;
}

#ifdef WIN32
bool NetworkUtil::getMacAddr(std::string& mac)
{
    std::string ip = comn::SockAddr::getHostIP();

    char strMacAddr[100] = { 0 };

    IPAddr ipAddr = inet_addr(ip.c_str());

    unsigned char macAddr[16] = { 0 };
    ULONG length = 16;

    bool done = false;
    HRESULT hr = SendARP(ipAddr, 0, macAddr, &length);
    if (hr == S_OK)
    {
        mac = comn::StringUtil::format("%02X-%02X-%02X-%02X-%02X-%02X",
            macAddr[0], macAddr[1], macAddr[2],
            macAddr[3], macAddr[4], macAddr[5]);
        done = true;
    }
    return done;
}

#else

bool NetworkUtil::getMacAddr(std::string& mac)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
    {
        return false;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
    {
        return false;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else
        {
        }
    }

    unsigned char mac_address[6];

    if (success)
    {
        mac.assign(ifr.ifr_hwaddr.sa_data, 6);
    }
    return success;
}

#endif //


std::string NetworkUtil::formatMacAddr(const std::string& mac)
{
    std::string str;
    if (mac.size() < 6)
    {
        return str;
    }

    str = comn::StringUtil::format("%02X-%02X-%02X-%02X-%02X-%02X",
                             mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    return str;
}

#ifdef WIN32

bool NetworkUtil::getGateway(std::string& ip)
{
    return false;
}

#else

bool NetworkUtil::getGateway(std::string& ip)
{
    char buff[256];
    int  nl = 0 ;
    struct in_addr gw;
    int flgs, ref, use, metric;
    unsigned long int d,g,m;

    FILE *fp = NULL;

    fp = fopen("/proc/net/route", "r");
    if (fp == NULL)
    {
        return false;
    }

    nl = 0 ;
    memset(buff, 0,sizeof(buff));
    while( fgets(buff, sizeof(buff), fp) != NULL )
    {
        if(nl)
        {
            int ifl = 0;
            while(buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
                ifl++;
            buff[ifl]=0;    /* interface */
            if(sscanf(buff+ifl+1, "%lx%lx%X%d%d%d%lx",
                   &d, &g, &flgs, &ref, &use, &metric, &m)!=7)
            {
                fclose(fp);
                return false;
            }

            ifl = 0;        /* parse flags */

            gw.s_addr   = g;

            if(d==0)
            {
                ip = inet_ntoa(gw);
                fclose(fp);
                return true;
            }
        }
        nl++;
    }

    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }

    return false;
}

#endif //WIN32
