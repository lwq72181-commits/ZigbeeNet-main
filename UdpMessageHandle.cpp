#include "UdpMessageHanlde.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
//#include <netinet/in.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sem.h>
#include <list>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <QThread>
#include "Acquisitiontype.h"
#include "NetConfig.h"

UdpMessageHandle::UdpMessageHandle()
{

}

int UdpMessageHandle::getWifiAddr(char *addr)
{
    int ret = 0;
    net::NetConfig config = net::NetConfig();
    if (!net::getNetConfig("wlan0", config))
    {
        ret = -1;
        qDebug("getNetConfig wlan02 error!!!");
    }
    else
        qDebug("get ip:%s,gateway:%s,mask:%s\n",config.ip.c_str(),config.gateway.c_str(),config.mask.c_str());
    strcpy(addr,config.ip.c_str());
    return ret;
}

int UdpMessageHandle::initSocket()
{
    int nerr, nloop;
    struct ip_mreq mreq;
    int ret = 0;
    char addr[20];
    //===================创建cmd socket===================
    m_UDPSocket = socket (AF_INET, SOCK_DGRAM, 0);

    if (-1 == m_UDPSocket)
    {
        ret = -1;
        qDebug ("%d:socket() m_UDPSocket create error\n",  __LINE__);
    }

    /* 绑定本地 IP地址和端口号 */
    memset(&local_addr, 0, sizeof(sockaddr_in));
    local_addr.sin_family = AF_INET;
    /* 设置本地IP地址为任意网口的IP地址 */
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr ("192.168.21.146");//
    local_addr.sin_port = htons(CMD_PORT);  /* 本地端口号 */

    if (-1 == bind (m_UDPSocket, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_in)))
    {
        ret = -2;
        qDebug ("%d:bind() m_UDPSocket Bind error\n", __LINE__);
    }
    while(1)
    {
        if(getWifiAddr(addr)==0)
            break;
        else
            QThread::sleep(2);
    }
    /* 加入组播地址  */
    mreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);  /* 组播地址 */
    mreq.imr_interface.s_addr = inet_addr(addr);//htonl(INADDR_ANY); //inet_addr("192.168.21.146");//     /* 网络接口为默认 */
    nerr = setsockopt (m_UDPSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if(nerr < 0)
    {
        ret = -3;
        qDebug("+++++++++++++=========ret:%d,errno:%d",nerr,errno);
        //qDebug ("%d:setsockopt():IP_ADD_MEMBERSHIP for m_UDPSocket\n", __LINE__);
        perror ("setsockopt():IP_ADD_MEMBERSHIP for m_UDPSocket\n\n");
    }

    /* 本地不接收环回许可 */
    nloop = 0; //0 不接收   1接收
    nerr = setsockopt (m_UDPSocket, IPPROTO_IP, IP_MULTICAST_LOOP, &nloop, sizeof(nloop));
    if(nerr < 0)
    {
        ret = -4;
        qDebug ("%d:setsockopt():IP_MULTICAST_LOOP failed!\n", __LINE__);
    }

    /* 地址复用 */
    nloop = 1;
    if(setsockopt (m_UDPSocket, SOL_SOCKET, SO_REUSEADDR, &nloop, sizeof(nloop)) < 0)
    {
        ret = -5;
        qDebug("%d:setsockopt():SO_REUSEADDR failed!\n", __LINE__);
    }

    /* 初始化组播目标地址 */
    memset(&mcast_addr, 0, sizeof(mcast_addr));         /* 初始化IP多播地址为0 */
    mcast_addr.sin_family = AF_INET;                    /* 设置协议族类行为AF */
    mcast_addr.sin_addr.s_addr = inet_addr(MCAST_ADDR); /* 设置多播IP地址 */
    mcast_addr.sin_port = htons(CMD_PORT);              /* 设置多播端口 */
                                                        /* 向多播地址发送数据 */
    qDebug("init udp socket over!!!!");
    return ret;
}

int UdpMessageHandle::getIPFromString(char *IP, unsigned char *fields)
{
    char *ptr = strtok(IP, ".");
    int i = 0;
    while (ptr != NULL) {
        fields[i++] = atoi(ptr);
        ptr = strtok(NULL, ".");
    }
    return 0;
}

int UdpMessageHandle::SendNetInfo()
{
    unsigned char ip[4] = {0};
    unsigned char send[14] = {0};
    unsigned char crc = 0;
    send[0] = 0xff;
    send[1] = 0x02;
    send[3] = 0x04;
    net::NetConfig config = net::NetConfig();
    if (!net::getNetConfig("wlan0", config))
    {
        qDebug("getNetConfig wlan01 error!!!");
    }
    else
        qDebug("get ip:%s,gateway:%s,mask:%s\n",config.ip.c_str(),config.gateway.c_str(),config.mask.c_str());
    char qip[20] = {0};
    strcpy(qip,config.ip.c_str());
    getIPFromString(qip, ip);
    memcpy(&send[10],ip,sizeof(ip));
    for(int i=0;i<4;i++)
    {
        qDebug("crc value:%d",send[10+i]);
        crc^=send[10+i];
    }
    send[9] = crc;
    int ret = sendto(m_UDPSocket,                    /* 套接字描述符 */
            send,                    /* 数据 */
            14,                 /* 长度 */
            0,
            (struct sockaddr*)&mcast_addr,
           sizeof(mcast_addr));
    qDebug("send over!!!ret:%d,errno:%d",ret,errno);
    return 0;
}

int UdpMessageHandle::MulticastHandle()
{
    fd_set readfds, readfds_;
    int max_fd = 0, /*lan_msg_id = 0,*/ ret;
    initSocket();
    FD_ZERO (&readfds);
    FD_SET (m_UDPSocket, &readfds);
    unsigned char buff[20] = {0};
    max_fd = m_UDPSocket;
    max_fd += 1;
    qDebug ("!!!!!!!!!-------------%d %d\n",m_UDPSocket, max_fd);
    while (1)
    {
        readfds_ = readfds;
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        ret = select(max_fd, &readfds_, NULL, NULL, &timeout);

        if (ret < 0)
        {
            qDebug("socket fd descriptor is not ready!\n");
        }
        else if (0 == ret)
        {
            //qDebug ("select timeout\n");
            continue;
        }
        else
        {
            if (FD_ISSET(m_UDPSocket, &readfds_))
            {
                int rev_cmd_len;
                rev_cmd_len = recv (m_UDPSocket, buff, 20, MSG_DONTWAIT);
                qDebug("============================recv udp data size:%d\n",rev_cmd_len);
                if (rev_cmd_len == 10)
                {
                    if(buff[0]==0xff && buff[1]==0x01)
                    {
                        SendNetInfo();
                    }
                }
            }
        }
    }
    close(m_UDPSocket);
}
