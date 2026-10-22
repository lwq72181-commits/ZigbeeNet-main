#ifndef UDPMESSAGE_H
#define UDPMESSAGE_H
#include <QObject>
#include <netinet/in.h>
class UdpMessageHandle:public QObject
{
    Q_OBJECT
public:
    UdpMessageHandle();

protected:
    int initSocket();
    int SendNetInfo();
    int getIPFromString(char *IP, unsigned char *fields);
    int getWifiAddr(char *addr);
protected:
    int m_UDPSocket;
    struct sockaddr_in local_addr;
    struct sockaddr_in mcast_addr;
public slots:
    int MulticastHandle();
};

#endif // UDPMESSAGE_H
