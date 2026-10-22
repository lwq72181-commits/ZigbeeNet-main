#include<sys/types.h>
#include<ctype.h>
#include<strings.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<errno.h>
#include<sys/time.h>
#include<stdio.h>
#include<string.h>
#include<sys/select.h>
#include<stdlib.h>
#include<signal.h>
#include <string.h>
#include<sys/wait.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <net/if.h>
#include <pthread.h>
#include <poll.h>
#include <dirent.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <linux/sockios.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include "TCPServer.h"
#include "Acquisitiontype.h"

TCPServer::TCPServer(TCPParam *param,ThreadSafeMultiVector*vector)
{
    m_param = param;
    m_vector = vector;
}

void TCPServer::TickTimePeriod(int index,int wave,int longwave)
{
    qDebug()<<"TCPServer::TickTimePeriod";
    emit setUpdateTimeSignal(index,wave,longwave);
}

void TCPServer::dorun()
{
    int sockfd,new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    int sin_size;
    int on = 1;
    qDebug("start tcp server for handle clients!!!\n");
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
            qDebug("tcp new error\n");
            goto CLIENTS_ERROR;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&my_addr,sizeof(my_addr));
    my_addr.sin_family=AF_INET;
    my_addr.sin_port = htons(CTL_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
            qDebug("tcp bind error\n");
            goto CLIENTS_ERROR;
    }

    if(listen(sockfd, 20) == -1)
    {
            qDebug("tcp listen error\n");
            goto CLIENTS_ERROR;
    }

    qDebug("tcp server for handle clients create success!!!\n");

    fd_set fd_read;
    struct timeval tmval;
    tmval.tv_sec = 1;
    tmval.tv_usec = 0;
    while(1)
    {
        FD_ZERO(&fd_read);
        FD_SET(sockfd, &fd_read);
        if (select(sockfd+1, &fd_read, NULL, NULL, NULL/*&tmval*/))
        {
            if (FD_ISSET(sockfd, &fd_read))
            {

                sin_size = sizeof(struct sockaddr_in);
                if((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, (socklen_t *)&sin_size)) == -1)
                {
                        qDebug("tcp accept error\n");
                        goto CLIENTS_ERROR;
                }
                unsigned fromip = their_addr.sin_addr.s_addr;
                struct in_addr aa;
                aa.s_addr = fromip;
                char *ip = inet_ntoa(aa);
                qDebug("we need to create thread!!!\n");
                qDebug("ip: %s, fd = %d!!!!!!!!!!!!!!!!!!!!!!!!!\n", ip, new_fd);
                m_param->status = 1;
                m_workerThread = new TCPHandleThread(new_fd,m_param,m_vector);
                m_thread = new QThread;
                m_workerThread->moveToThread(m_thread);
                connect(m_thread, &QThread::started, m_workerThread, &TCPHandleThread::dorun);
                connect(m_thread, &QThread::finished, m_workerThread, &QObject::deleteLater);
                connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
                //connect(this, &TCPServer::setUpdateTimeSignal, m_workerThread, &TCPHandleThread::SetTimeperiod, Qt::QueuedConnection);
                m_thread->start();

//                connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
//                bool connected = connect(this, &TCPServer::setUpdateTimeSignal, m_workerThread, &TCPHandleThread::SetTimeperiod, Qt::QueuedConnection);
//                if (!connected) {
//                    qDebug() << "Failed to connect setUpdateTimeSignal to SetTimeperiod";
//                }
//                connect(m_workerThread, &TCPHandleThread::MqttSignal, this, &TCPServer::restartMqttSignal, Qt::QueuedConnection);
//                m_workerThread->start(); // 启动线程


            }
        }
        else
        {
            //qDebug("time out select tcp server!!!\n");
            //sleep(2);
        }
    }
CLIENTS_ERROR:
    if (sockfd)
            close(sockfd);
}

//void TCPServer::createAndStartWorkerThreads()
//{
//    for(int i = 0; i < 5; ++i) { // 假设创建5个工作线程
//        WorkerThread *workerThread = new WorkerThread(i);
//        workerThread->start(); // 启动线程
//        // 可能需要管理这些线程对象的生命周期，比如连接finished信号到deleteLater槽
//        connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
//    }
//}
