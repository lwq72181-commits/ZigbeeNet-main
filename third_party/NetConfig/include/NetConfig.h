/*
 * NetConfig.h
 *
 *  Created on: 2019年3月20日
 *      Author: terry
 */

#ifndef NETCONFIG_H_
#define NETCONFIG_H_

#include "BasicType.h"
#include <string>
#include <vector>

namespace net
{

typedef std::vector< std::string >	StringArray;

struct NetConfig
{
    std::string ifname;
	bool dhcp;
	std::string ip;
	std::string mask;
	std::string gateway;
};

int enumNetInterface(StringArray& ifs);

bool getNetConfig(const char* ifname, NetConfig& cfg);

bool setNetConfig(const char* ifname, const NetConfig& cfg);

bool getMacAddr(const char* ifname, std::string& mac);

int getDns(StringArray& ips);

void setDns(StringArray& ips);

}



#endif /* NETCONFIG_H_ */
