#ifndef NETWORKUTIL_H
#define NETWORKUTIL_H

#include "BasicType.h"
#include <string>


class NetworkUtil
{
public:
    NetworkUtil();

    static std::string getHostIp();

    static bool getMacAddr(std::string& mac);

    static std::string formatMacAddr(const std::string& mac);

    static bool getGateway(std::string& ip);

};

#endif // NETWORKUTIL_H
