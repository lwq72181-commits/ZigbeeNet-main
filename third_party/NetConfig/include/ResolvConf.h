#ifndef RESOLVCONF_H
#define RESOLVCONF_H

#include "BasicType.h"
#include <string>
#include <vector>

class ResolvConf
{
public:
    static const char* FILE_NAME;
    static const char* FILE_PATH;

    typedef std::vector< std::string >	StringArray;

    static std::string toString(const StringArray& servers);

    static void split(const std::string& str, StringArray& servers);

    static void merge(const std::string& srcpath, const std::string& dstpath);

	static std::string getFilePath();

public:
    ResolvConf();

    bool load(const std::string& filepath);
    bool save(const std::string& filepath);

    StringArray getDns();

    void setDns(const StringArray& servers);

    void setDns(const std::string& dns, const std::string& dns2);

    void addDns(const std::string& dns);

protected:
    size_t indexOf(const std::string& dns);

protected:
    StringArray m_content;
    StringArray m_servers;

};

#endif // RESOLVCONF_H
