#include "BuleTooth.h"
#include <QDebug>
#include <QTcpSocket>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <termios.h>
#include <time.h>
#include <sys/select.h>
#include <libgen.h>
#include <QPath.h>
#include <sys/socket.h>
#include <unistd.h>
#include <QProcess>
#include <QFile>
#include "ResolvConf.h"
#include "NetworkConf.h"
#include "AppConst.h"
#include "AppService.h"
#include "AppConfigUtil.h"
#include "AppConfig.h"
#include "BleHandle.h"
#include "SensorLogManager.h"

Buletooth::Buletooth()
{
    m_networkConf.load();
    snflag = 0;
}

int Buletooth::open_serial_port(char *com)
{
    int fd;
    fd = open(com, O_RDWR | O_NOCTTY| O_NDELAY);
    if (-1 == fd) {
        return -1;
    }

    if(fcntl(fd, F_SETFL, 0) < 0) {
        qDebug("fcntl failed!\n");
        return -1;
    }
    return fd;
}

void Buletooth::close_serial_port(int fd)
{
    /* Restore original terminal settings on exit */
    tcsetattr(fd, TCSANOW, &uartsave);
    close(fd);
}


int Buletooth::set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop)
{
  struct termios newtio, oldtio;

    if (tcgetattr(fd, &oldtio) != 0) {
        qDebug("Get serial port attr failed\n");
        return -1;
    }

    uartsave = oldtio;

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    /* Input setting */
    newtio.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
    /* Output setting */
    newtio.c_oflag  &= ~OPOST;

    switch (nbits) {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch (nevent) {
        case 'O':
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E':
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N':
            newtio.c_cflag &= ~PARENB;
            break;
    }

    switch (baudrate) {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
    }

    if(nstop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nstop == 2)
        newtio.c_cflag |= CSTOPB;

    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd,TCIFLUSH);

    if((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
        qDebug("Set serial port failed\n");
        return -1;
    }

    return 0;
}

int Buletooth::init_uart_bt(int fd,const char *name)
{
    int ret;
    char at_cmd_buf[64];
//+++
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "+++");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        qDebug("BL:%s",at_cmd_buf);
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("exit outside mode failed\n");
        }
    } else {
        qDebug("exit outside mode failed\n");
    }

    /* Reset bt module */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QRST\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Reset bt module failed\n");
            return -1;
        }
    } else {
        qDebug("Reset bt module failed\n");
        return -1;
    }

    /* Wait for reset to complete */
    sleep(1);

    /* Performs BLE initialization for peripherals */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QBLEINIT=2\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Performs BLE initialization for peripherals failed\n");
            return -1;
        }
    } else {
        qDebug("Performs BLE initialization for peripherals failed\n");
        return -1;
    }


    /* Set BLE broadcast parameters */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QBLEADVPARAM=150,150\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Set BLE broadcast parameters failed\n");
            return -1;
        }
    } else {
        qDebug("Set BLE broadcast parameters failed\n");
        return -1;
    }

    /* Create the BLE service and set the service UUID to fff1 */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QBLEGATTSSRV=fff1\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Create the BLE service and set the service UUID to fff1 failed\n");
            return -1;
        }
    } else {
        qDebug("Create the BLE service and set the service UUID to fff1 failed\n");
        return -1;
    }

    /* Set the GATT characteristic value UUID to fff2 */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QBLEGATTSCHAR=fff2\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Set the GATT characteristic value UUID to fff2 failed\n");
            return -1;
        }
    } else {
        qDebug("Set the GATT characteristic value UUID to fff2 failed\n");
        return -1;
    }

    /* Complete service addition */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QBLEGATTSSRVDONE\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Complete service addition failed\n");
            return -1;
        }
    } else {
        qDebug("Complete service addition failed\n");
        return -1;
    }

    /* Set the BLE name to QuecHCM111Z */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    sprintf(at_cmd_buf,"AT+QBLENAME=%s\r\n",name);
    //snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QBLENAME=QuecHCM111Z\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    qDebug("set ble:%s",at_cmd_buf);
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Set the BLE name to failed\n");
            return -1;
        }
    } else {
        qDebug("Set the BLE name to failed\n");
        return -1;
    }
    //qDebug("set ble over");
    /* Enable BLE broadcast */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QBLEADVSTART\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Enable BLE broadcast failed\n");
            return -1;
        }
    } else {
        qDebug("Enable BLE broadcast failed\n");
        return -1;
    }
    //AT+QBLETRANMODE=<UUID>
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    snprintf(at_cmd_buf, sizeof(at_cmd_buf), "AT+QBLETRANMODE=fff2\r\n");
    write(fd, at_cmd_buf, strlen(at_cmd_buf));
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        qDebug("BL:%s",at_cmd_buf);
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("enter outside mode failed\n");
            return -1;
        }
    } else {
        qDebug("enter outside mode failed\n");
        return -1;
    }
    //qDebug("set ble broadcast");
    return 0;
}

#if 1
int Buletooth::uart_bt_send_data(int fd, char *data, int len)
{
    int ret;
    char at_cmd_buf[250]={0};
//    char readbuf[120]={0};
    //char mid_buf[100] = {0};
//    char *send = "AT+QBLEGATTSNTFY=0,FFF2,3,0D0A00";
    if (data == NULL)
        return -1;
//    for(int i=0;i<len;i++)
//    {
//        sprintf(&mid_buf[i * 2], "%02x", data[i]); // 格式化为 2 位大写十六进制
//    }
    qDebug("BL source len:%d",len);
    /* Send data through GATT */
//    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
//    sprintf(at_cmd_buf,"AT+QBLEGATTSNTFY=0,fff2,%d,%s",len,mid_buf);
//    char mid[100] = {0x0d,0x0a,0x00,0x01,0x03,0x0d,0x0a,0x00,0x01};
//    write(fd, mid, 9);

    memcpy(at_cmd_buf,data,len);

//{
//    qDebug("BLsend data:%s",send);
//    int size = strlen(send);
//    write(fd, send, size);
//}
//    qDebug("BLsend data:%s",at_cmd_buf);
//    int size = strlen(at_cmd_buf);
//    write(fd, at_cmd_buf, size);
    write(fd, at_cmd_buf, len);
    return 0;
}
#endif
#if 0
int Buletooth::uart_bt_send_data(int fd, char *data, int len)
{
    int ret;
    char at_cmd_buf[128];

    if (data == NULL)
        return -1;
    qDebug("BL source len:%d",len);
    /* Send data through GATT */
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
//    snprintf(at_cmd_buf, sizeof(at_cmd_buf),
//        "AT+QBLEGATTSNTFY=0,fff2,%s\r\n", data);
    strcpy(at_cmd_buf,"AT+QBLEGATTSNTFY=0,fff2,");
    int size = strlen(at_cmd_buf);
    for(int i=0;i<len;i++)
        at_cmd_buf[size+i]=data[i];
    at_cmd_buf[size+len] = '\r';
    at_cmd_buf[size+len+1] = '\n';
    write(fd, at_cmd_buf, size+len+2);
//    for(int k=0;k<size+len+2;k++)
//        qDebug("%x ",at_cmd_buf[k]);
//    qDebug("BL replay len:%d",size+len+2);
    //usleep(300000);
//    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
//    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
//    if (ret > 0) {
//        if (strstr(at_cmd_buf, "OK") == NULL) {
//            qDebug("Send data failed\n");
//            return -1;
//        }
//    } else {
//        qDebug("Send data failed\n");
//        return -1;
//    }
//    qDebug("BL send over");
    return 0;
}
#endif

int Buletooth::freq_uart_read(void *buf, int len)
{
    int read_len = 0;
    int recv_len = len;
    int ret;
    fd_set input;
    struct timeval timeout;

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    FD_ZERO(&input);
    FD_SET(m_fd, &input);
    ret = select(m_fd + 1, &input, NULL, NULL, NULL);//zuse
    if (ret == -1) {
        qDebug("Wait for data failed.\n");
        return -1;
    }
    while (recv_len > 0) {
        int len = read(m_fd, (char *)buf+read_len, recv_len);
        if (len <= 0)
            return read_len;
        read_len += len;
        recv_len -= len;
    }

    return read_len;
}

int Buletooth::uart_bt_receive_data(int fd, char *buf, int buf_len, int timeout_s)
{
    int ret;
    fd_set input;
    struct timeval timeout;

    timeout.tv_sec = timeout_s;
    timeout.tv_usec = 0;

    qDebug("Ready to receive data, timeout is %ds, buflen:%d\n", timeout_s,buf_len);

    FD_ZERO(&input);
    FD_SET(fd, &input);

    //ret = select(fd + 1, &input, NULL, NULL, &timeout);
    ret = select(fd + 1, &input, NULL, NULL, NULL);//zuse
    if (ret == -1) {
        qDebug("Wait for data failed.\n");
        return -1;
    }
    else if (ret) {
        ret = read(fd, buf, buf_len);
        if (ret <= 0) {
            qDebug("Receive data failed.\n");
            return -1;
        }
    }
    else {
        qDebug("Wait for data timeout.\n");
        return -1;
    }

    return ret;
}

int Buletooth::init(QString com,ZIGBEEParam *param)
{
    int ret = 0;
    QString name;
    m_ZIGBEEParam = param;
    m_fd = open_serial_port(com.toLocal8Bit().data());
    if (m_fd < 0) {
        qDebug("Open serial port failed\n");
        return -1;
    }
    /* Set serial port, 115200, 8N1 */
    ret = set_serial_port(m_fd, 115200, 8, 'N', 1);
    if (ret) {
        qDebug("Set serial port error!\n");
        return -1;
    }
    AppConfigUtil::loadBLInfo(name);
    ret = init_uart_bt(m_fd,name.toLocal8Bit().data());
    if (ret != 0) {
        qDebug("Init uart bt failed\n");
        return -1;
    }
    qDebug("init ble over");
//    workerThread = new BleHandle(m_fd);//starthandle(char *data,int len,int fd);
//    workerThread->moveToThread(&m_BLEThread);
//    connect(this, &Buletooth::starthandle, workerThread, &BleHandle::handle_recv_data);

    //connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
    //workerThread->start();//emit processCharArray(data, length);
    return 0;
}

int Buletooth::check_cmd_data(char *cmd)
{
    int ret = 0;
    switch(cmd[1])
    {
        case 3:
        if(cmd[3]==0)
            ret = 10;
        else if(cmd[3]==0x10)
            ret = 26;
        break;
        case 5:
        if(cmd[3]==6)
            ret = 16;
        break;
        case 7:
        ret = (cmd[2]<<8)|cmd[3]+10;
        break;
        case 9:
        if(cmd[3]==6)
            ret = 16;
        break;
        case 0x0b:
        ret = (cmd[2]<<8)|cmd[3]+10;
        break;
        case 0x0d:
        ret = (cmd[2]<<8)|cmd[3]+10;
        break;
        case 0x0f:
        if(cmd[3]==0x01)
            ret = 11;
        break;
        case 0x11:
        ret = (cmd[2]<<8)|cmd[3]+10;
        break;
        case 0x13:
        ret = 11;
        break;
        case 0x15:
        ret = 11;
        break;
        case 0x17:
        ret = 11;
        break;
        case 0x19:
        ret = (cmd[2]<<8)|cmd[3]+10;;
        break;
        case 0x1b:
        ret = 12;
        break;
        case 0x23:
        ret = (cmd[2]<<8)|cmd[3]+10;;
        break;
        case 0xf1:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xf3:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xf5:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xf7:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xf9:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xfb:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xfd:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xe0:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xe2:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xe4:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xe6:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xe8:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xea:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xd4:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xd6:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xd8:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xda:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0xdc:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0x67:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0x6a:
            ret = (cmd[2]<<8)|cmd[3]+10;
        break;
        case 0x6b:
        if(cmd[3]==0)
            ret = 10;
        break;
        case 0x6d:
            ret = (cmd[2]<<8)|cmd[3]+10;
        break;
        default:
        break;
    }
    qDebug("get cmd len:%d",ret);
    return ret;
}

int Buletooth::judge_ask_data(char *buf, int len)
{
    int ret = 0;
    int i = 0;
    int data = 0;
    qDebug("len:%d",len);
    for(i =10; i<(len); i++)
    {
        data = data^buf[i];
    }
    qDebug("recvdate:%d, check date:%d\n", buf[9], data);
    if(buf[9] == data)
    {
        ret = 1;
    }
    return ret;
}

char Buletooth::Get_Crc(char *buf, int len)
{
    int i = 0;
    char data = 0;
    qDebug("len:%d",len);
    for(i =0; i<(len); i++)
    {
        data = data^buf[i];
    }
    return data;
}

int Buletooth::super_system(const char * cmd, char *retmsg, int msg_len)
{
        FILE * fp;
        if (cmd == NULL || retmsg == NULL || msg_len < 0)
        {
                qDebug("Err: Fuc: system paramer invalid!");
                return -1;
        }
        if ((fp = popen(cmd, "r") ) == NULL)
        {
                perror("popen");
                qDebug("Err: Fuc: popen error: ");
                return -2;
        }
        else
        {
                memset(retmsg, 0, msg_len);
                fgets(retmsg, msg_len, fp);
                qDebug("read:%s,len:%d",retmsg,msg_len);
//                while(fgets(retmsg, msg_len, fp));
//                {
//                        _DEBUG("read:%s,len:%d",retmsg,msg_len);
//                }
                //drop #012 from system result retmsg.
                if(strlen(retmsg) <1)
                {
                    qDebug("find file name is null!!!\n");
                    pclose(fp);
                    return -3;
                }
                retmsg[strlen(retmsg)-1] = '\0';
                int result = pclose(fp); // 关闭子进程并获取其退出状态
                if (result == -1) {
                    qDebug("pclose failed");
                    return -4;
                }

                return 0;
        }
}

// 同步 Ping 检测（跨平台）
bool Buletooth::pingHost(const QString &ip, int timeoutMs) {
    QProcess ping;
    QStringList args;
#ifdef Q_OS_WIN
    args << "-n" << "1" << "-w" << QString::number(timeoutMs) << ip;
#else
    args << "-c" << "1" << "-W" << QString::number(timeoutMs / 1000) << ip;
#endif
    ping.start("ping", args);
    return ping.waitForFinished(timeoutMs) && ping.exitCode() == 0;
}

CheckResult Buletooth::checkPortWithPing(const QString &ip, quint16 port, int timeoutMs) {
    QTcpSocket socket;
    socket.connectToHost(ip, port);

    if (socket.waitForConnected(timeoutMs)) {
        socket.close();
        return Success;
    }

    QAbstractSocket::SocketError err = socket.error();
    switch (err) {
        case QAbstractSocket::ConnectionRefusedError:
            // 立即拒绝 → 端口明确未开放（无服务），IP 可达
            // 但也可归为 PortFiltered？实际上 RST 表示端口未监听，没有防火墙干预。
            // 这里保持为“端口未开放”但为了统一，仍返回 PortFiltered 并注明。
            // 为了更细致，可以单独定义一个 PortClosed 枚举值。
            // 简单起见，我们返回 PortFiltered 并额外输出日志。
            qDebug() << "连接被拒绝，端口未开放（无服务）";
            return PortFiltered;
        case QAbstractSocket::SocketTimeoutError:
            // 超时：可能是 IP 不通，也可能是防火墙丢弃包
            if (pingHost(ip, timeoutMs)) {
                qDebug() << "Ping 通 IP，但 TCP 连接超时 → 很可能是防火墙过滤该端口";
                return PortFiltered;
            } else {
                qDebug() << "Ping 不通 IP → 网络不可达或主机离线";
                return IpUnreachable;
            }
        case QAbstractSocket::HostNotFoundError:
        case QAbstractSocket::NetworkError:
        default:
            // 其他错误一律视为 IP 不通
            return IpUnreachable;
    }
}

int Buletooth::getMqttStatus()
{
    int ret = 0;
    if(AppService::mqttService().isOpen())
    {
        ret = 1;
    }
    else
    {
        emit BLRestartMqtt();
        for(int i=0;i<3;i++)
        {
            sleep(1);
            if(AppService::mqttService().isOpen())
            {
                ret = 1;
            }
        }
    }
    return ret;
}

int Buletooth::setSysNetwork(unsigned char *net, int len, int select)
{
    bool dhcp = false;
    QString dns;
    if(len == 10)
    {
        dhcp = true;
        if(select == 1)
            AppConfig::set("Network/dhcp", 1);
        //return 0;
    }
    else
    {
        dns = QString("%1.%2.%3.%4").arg(net[12]).arg(net[13]).arg(net[14]).arg(net[15]);
        if(select == 1)
            AppConfig::set("Network/dhcp", 0);
    }
    qDebug("len:%d,select:%d,dhcp:%d",len,select,dhcp);
    if(select==0)
    {
        NetworkConf& networkConf = m_networkConf;
        {
            int pos = networkConf.findConfig("eth1");
            qDebug("find eth1 pos:%d",pos);
            NetworkConfig& config = networkConf.m_configs[pos];
            //if((config.dhcp != dhcp)||(dhcp == false))
            {
                if(dhcp == false)
                {
                    config.ip = QString("%1.%2.%3.%4").arg(net[0]).arg(net[1]).arg(net[2]).arg(net[3]);
                    config.mask = QString("%1.%2.%3.%4").arg(net[4]).arg(net[5]).arg(net[6]).arg(net[7]);
                    config.gate = QString("%1.%2.%3.%4").arg(net[8]).arg(net[9]).arg(net[10]).arg(net[11]);
                }
                config.dhcp = dhcp;
                qDebug("ip:%s",config.ip.toUtf8().constData());
                qDebug("mask:%s",config.mask.toUtf8().constData());
                qDebug("gate:%s",config.gate.toUtf8().constData());
                qDebug("dhcp:%d",dhcp);
                networkConf.save();
                int mid =0;
                AppConfig::get("Network/NET", mid);
                if(mid == 0)
                {
                    networkConf.applycard(pos);
                    //networkConf.apply();
                }
            }

        }
        if(dhcp == false)
        {
            ResolvConf conf;
            conf.load(ResolvConf::getFilePath());
            conf.addDns(dns.toStdString());
            conf.save(ResolvConf::getFilePath());
            QString filepath = QPath::join(AppConst::getEtcDir(), ResolvConf::FILE_NAME);
            conf.save(filepath.toStdString());
        }
    }
    else if(select==1)
    {
        QString ip;
        QString mask;
        QString gate;
        if(dhcp == false)
        {
            AppConfig::set("Network/dns", dns);
            ip = QString("%1.%2.%3.%4").arg(net[0]).arg(net[1]).arg(net[2]).arg(net[3]);
            mask = QString("%1.%2.%3.%4").arg(net[4]).arg(net[5]).arg(net[6]).arg(net[7]);
            gate = QString("%1.%2.%3.%4").arg(net[8]).arg(net[9]).arg(net[10]).arg(net[11]);
            AppConfig::set("Network/ip", ip);
            AppConfig::set("Network/netmask", mask);
            AppConfig::set("Network/gateway", gate);
        }
    }
    return 0;
}

int Buletooth::setMqttServer(unsigned char *net)
{
    QString url = QString("%1.%2.%3.%4").arg(net[0]).arg(net[1]).arg(net[2]).arg(net[3]);
    int port = net[4]|(net[5]<<8);
    AppConfigUtil::saveMqttServer(url, port);
    qDebug("Device. ip:%s,port:%d\n", url.toUtf8().constData(), port);
    return 0;
}

int Buletooth::setMqttInfo(char *net, int len)
{
    qDebug("len:%d",len);
    QString str = QString::fromUtf8(net,len);
    qDebug() << str;
    QStringList splitList = str.split(";");
    for(const auto &str : splitList) {
            qDebug() << str;
        }
    qDebug("size:%d",splitList.size());
    if(splitList.size()==3)
    AppConfigUtil::saveMqttInfo(splitList[1],splitList[2],splitList[0]);

    emit BLRestartMqtt();
//TODO
//    AppService::mqttService().close();
//    FREUser user;
//    AppConfigUtil::loadUser(user);
//    qDebug("Device. username:%s, password: %s,orgname: %s, devid: %s\n",
//        qPrintable(user.username), qPrintable(user.password), qPrintable(user.orgname), qPrintable(user.devid));
//    AppService::mqttService().setMqUrl(user.url, user.username, user.password, user.orgname, user.devid, user.port);
    return 0;
}

int Buletooth::setAcquistInfo(char *net)
{
    return 0;
}

int Buletooth::setWifiInfo(char *net, int len)
{
    char retmsg[50] = {0};
    qDebug("len:%d",len);
    QString str = QString::fromUtf8(net,len);
    qDebug() << str;
    QStringList splitList = str.split(";");
    for(const auto &str : splitList) {
            qDebug() << str;
        }
    if(splitList.size()==2)
    {
        AppConfigUtil::saveWifiInfo(splitList[0],splitList[1]);
        int mid;
        QString cmd;
        QString ip;
        QString netmask;
        QString gateway;
        QString dns;
        AppConfig::get("Network/NET", mid);
        int wifiDhcp;
        AppConfig::get("Network/dhcp", wifiDhcp);
        if(mid == 1)
        {
            if(wifiDhcp)
            {
                cmd = QString("ip route del default;killall wpa_supplicant;killall udhcpc;sleep 1;/userdata/etc/wifi_setup.sh -i %1 -p %2").arg(splitList[0]).arg(splitList[1]);
                qDebug("%s",qPrintable(cmd));
            }
            else
            {
                AppConfig::get("Network/ip", ip);
                AppConfig::get("Network/netmask", netmask);
                AppConfig::get("Network/gateway", gateway);
                AppConfig::get("Network/dns", dns);
                cmd = QString("ip route del default;killall wpa_supplicant;killall udhcpc;ifconfig wlan0 down;sleep 2;ifconfig wlan0 up;sleep 2;/userdata/etc/wifi_setup.sh -i %1 -p %2 -a %3 -m %4 -g %5 -n %6").arg(splitList[0]).arg(splitList[1]).arg(ip).arg(netmask).arg(gateway).arg(dns);
                qDebug("%s",qPrintable(cmd));
            }
            //if(mid == 1)
            system(cmd.toUtf8().data());
            //cmd = QString("killall wpa_supplicant;sleep 2;/userdata/etc/wifi_setup.sh -i %1 -p %2").arg(splitList[0]).arg(splitList[1]);
            //super_system(cmd.toUtf8().data(), retmsg, sizeof(retmsg));

//            QProcess process;
//            qDebug()<<cmd;
//            process.start("/bin/sh", QStringList() << "-c" << cmd);
//            if (!process.waitForStarted()) {
//                qDebug() << "Failed to saveLocalParam.";
//                return -1;
//            }
//            if (!process.waitForFinished()) {
//                qDebug() << "Process saveLocalParam did not finish date in time.";
//                return -1;
//            }
        }

    }
    return 0;
}

int Buletooth::saveLocalParam()
{
    QProcess process;
    QString cmd = "cp /userdata/etc/Acquisition.ini /oem/dma/qt/;sync";
    qDebug()<<cmd;
    process.start("/bin/sh", QStringList() << "-c" << cmd);
    //process.start(cmd,QStringList());
    if (!process.waitForStarted()) {
        qDebug() << "Failed to saveLocalParam.";
        return -1;
    }
    if (!process.waitForFinished()) {
        qDebug() << "Process saveLocalParam did not finish date in time.";
        return -1;
    }
    return 0;
}

int Buletooth::setBLNameInfo(char *net, int len)
{
    int ret;
    QString name;
    qDebug("len:%d",len);
    QString str = QString::fromUtf8(net,len);
    qDebug() << str;
    AppConfigUtil::saveBLInfo(str);
    AppConfig::get("Device/BLName", name);
    if(name == str)
    {
        saveLocalParam();
    }
    ret = init_uart_bt(m_fd,str.toUtf8().data());
    if (ret != 0) {
        qDebug("setBLNameInfo Init uart bt failed\n");
        return -1;
    }
    return 0;
}

int Buletooth::setSN(char *sn, int len)
{
    qDebug("len:%d",len);
    QString str = QString::fromUtf8(sn,len);
    AppConfig::set("Device/devid", str);
    BLSetDEVID();
    qDebug() << str;
    return 0;
}

int Buletooth::refresh_sensor_file_info()
{
    int ret = 0;
    int file_fd       = -1;
    char filename[FILE_NAME_LEN] = {0};
        /* 寻找文件，并判断是否有文件 */
    snprintf(filename, sizeof(filename), "%s/%s", FW_SENSOR_CFG_DIR, WLESS_SENSOR_CONFIG);
//    if (0 != access(filename, F_OK)) {
//        ret = DT_EEMPTY;
//        return DT_ERROR;
//    }
    /* 将文件读取到内存 */
    file_fd = open(filename, O_RDWR | O_CREAT);
    if (file_fd < 0) {
        close(file_fd);
        DT_WLS_ERR("open cfg table file failed(%s).\r\n", filename);
        return DT_ERROR;
    }

    // 清空文件内容：将文件截断为0字节
    if (ftruncate(file_fd, 0) == -1) {
        DT_WLS_ERR("清空文件失败");
        close(file_fd);
        return DT_ERROR;
    }
    lseek(file_fd, 0, SEEK_SET);
    if (dt_write(file_fd, m_ZIGBEEParam->sensor_info, m_ZIGBEEParam->count*sizeof(uart_cmd_ret_sensor_info_t))<=0) {
        DT_WLS_ERR("file_fd write fail.\r\n");
        close(file_fd);
        return DT_ERROR;
    }
    close(file_fd);
    return DT_EOK;
}

int Buletooth::setSNList(char *date, int len, char flag)
{
    int pos =0;
    bool check = true;
    if(snflag == 0)
    {
        memset(m_ZIGBEEParam->sensor_info,0,MAX_SENSOR_CNT*sizeof(uart_cmd_ret_sensor_info_t));
        snflag = 1;
    }
    if(flag == 1)
        snflag = 0;
    qDebug("len:%d",len);
    while(len>0)
    {
        m_ZIGBEEParam->sensor_info[date[pos]-1].Bind = 2;
        m_ZIGBEEParam->sensor_info[date[pos]-1].Channel = date[pos];
        int num = 0;
        memcpy(&num,&date[pos+1],4);
        sprintf(m_ZIGBEEParam->sensor_info[date[pos]-1].Sn, "%d", num);
        strcpy(m_ZIGBEEParam->sensor_info[date[pos]-1].DevNumber,"new");
        qDebug("pos:%d,sn:%s",date[pos],m_ZIGBEEParam->sensor_info[date[pos]-1].Sn);
        m_ZIGBEEParam->count = date[pos];
        pos+=5;
        len-=5;
    }
    if(flag == 1)
    {
        for(int i=0;i<m_ZIGBEEParam->count;i++)
        {
            if(m_ZIGBEEParam->sensor_info[i].Channel == 0)
            {
                qDebug("=====sn list data lost not saved=====");
                check = false;
            }
        }
        if(check)
            refresh_sensor_file_info();
    }

    return check;
}

int Buletooth::set5gAPN(char *date, int len)
{
    bool check = true;
    char apn[50]={0};
    int ret = 0;
    char at_cmd_buf[64]={0};
    if(len<100)
    {
        memcpy(apn,date,len);
        m_mutex = AppService::mqttService().getmutex();
        pthread_mutex_lock(m_mutex);
        m_uart = AppService::mqttService().getUart();
        ret = m_uart->init("/dev/ttyUSB2");
        if(ret < 0)
        {
            return 0;
        }
        for(int i=0;i<4;i++)
        {
            memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
            sprintf(at_cmd_buf, "AT+QICSGP=1,1,\"%s\",\"\",\"\",0\r\n", apn);
            qDebug("==send BL0:%s",at_cmd_buf);
            m_uart->uart_send_data(at_cmd_buf, strlen(at_cmd_buf));
            memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
            ret = m_uart->uart_receive_data(at_cmd_buf, 64, 5);
            qDebug("==read BL:%s",at_cmd_buf);
            if (ret > 0) {
                if (strstr(at_cmd_buf, "OK") == NULL) {
                    qDebug("set apn failed\n");
                    check = false;
                }
                else
                {
                    qDebug("set apn ok\n");
                    QString str = QString::fromUtf8(apn,len);
                    AppConfig::set("Network/APN", str);
                    break;
                }
            } else {
                qDebug("set apn failed\n");
                check = false;
            }
        }
        m_uart->close_serial_port();
        pthread_mutex_unlock(m_mutex);
    }
    return check;
}

int restartZGBboard()
{
    system("echo 0 > /sys/class/gpio/gpio101/value;sleep 2;echo 1 > /sys/class/gpio/gpio101/value");
    return 0;
}

int Buletooth::setZigbeeInfo(unsigned char chn,unsigned char power)
{
    qDebug("setZigbeeInfo chn:%d,power:%d",chn,power);
    AppConfig::set("Device/zgbFreqBand", static_cast<int>(chn));
    AppConfig::set("Device/zgbTxpower", static_cast<int>(power));
    AppConfig::set("Device/zgbUpdate", 1);
    //restartZGBboard();
    return 0;
}

int Buletooth::setModbusRTUStation(unsigned char nu)
{
    qDebug("setModbusRTUStation:%d",nu);
    AppConfig::set("Device/485sn", static_cast<int>(nu));
    emit BLset485sn();
    return 0;
}

int Buletooth::setUpdatemode(unsigned char nu)
{
    qDebug("setUpdatemode:%d",nu);
    AppConfig::set("Device/update", static_cast<int>(nu));
    bool rs485en = nu&0x01;
    bool mqtten = (nu>>1)&0x01;
    qDebug("485en:%d,mqtten:%d",rs485en,mqtten);
    emit BLUpdateMode(nu);
    return 0;
}

int Buletooth::setModbusRTUBitrate(unsigned char nu)
{
    int bitrate=9600;
    qDebug("setModbusRTUBitrate:%d",nu);
    switch(nu)
    {
        case 0:
        bitrate=2400;
        break;
        case 1:
        bitrate=4800;
        break;
        case 2:
        bitrate=9600;
        break;
        case 3:
        bitrate=19200;
        break;
        case 4:
        bitrate=38400;
        break;
        case 5:
        bitrate=57600;
        break;
        case 6:
        bitrate=115200;
        break;
        case 7:
        bitrate=230400;
        break;
        default:
        bitrate=9600;
        break;
    }

    AppConfig::set("Device/485bitrate", bitrate);
    emit BLRestart485();
    return 0;
}

int Buletooth::setNetSelect(char net)
{
    switch(net)
    {
        case 0x00://net
        break;
        case 0x01://wifi
        break;
        case 0x02://4g
        break;
        case 0x03://5g
        break;
        default:
        break;
    }
    AppConfig::set("Network/NET", (int)net);

    char* ccmd;
    if(net == 0)
    {
        int pos = m_networkConf.findConfig("eth1");
        qDebug("find eth1 pos:%d",pos);
        if(m_networkConf.m_configs[pos].dhcp)
        {
            qDebug() << "====================setNetSelect1";
            qDebug("====================setNetSelect1");
            ccmd = "ip route del default;killall udhcpc;sleep 1;udhcpc -i eth1 &";
        }
        else
        m_networkConf.applycard(pos);
    }
    else if(net == 1)
    {
        int wifidhcp;
        AppConfig::get("Network/dhcp", wifidhcp);
        if(wifidhcp==1)
        {
            qDebug() << "====================setNetSelect";
            qDebug("====================setNetSelect");
            ccmd = "ip route del default;killall udhcpc;sleep 1;udhcpc -i wlan0 &";
        }
        else
        {
            ccmd = "";//TODO   IF NOT SET THEN CCMD = "killall udhcpc;sleep 1;udhcpc -i wlan0 &"  SO CRAZY
        }
        qDebug("====================setNetSelect11111");
        qDebug()<<ccmd;

    }
    else if(net == 2)
    {
        qDebug() << "====================setNetSelect2";
        qDebug("====================setNetSelect2");
        ccmd = "ip route del default;killall udhcpc;sleep 1;udhcpc -i usb0 &";
    }
    else if(net == 3)
    {
        qDebug() << "====================setNetSelect3";
        qDebug("====================setNetSelect3");
        ccmd = "ip route del default;killall udhcpc;sleep 1;ifconfig pcie0 up;ifconfig sipa_dummy0 up;udhcpc -i pcie0 &";
    }
    //system("killall udhcpc");
    system(ccmd);
    return 0;
}

int Buletooth::replay_ask_data(char *replay_buf, int len, int fd)
{
    int ret = 0;
    usleep(100000);//TODO
    ret = uart_bt_send_data(fd, replay_buf, len);
    return ret;
//    int senddata = len;
//    int real_send = 0;
//    int ret = 0;
//    qDebug("replay data len:%d   ",len);
//    for(int i=0; i<len; i++)
//    {
//        qDebug("%02x ", replay_buf[i]);
//    }
//    while(senddata != 0)
//    {
//        ret = send(fd, replay_buf + real_send, senddata, 0);
//        if(ret == -1 || ret == 0) return -1;
//        senddata -= ret;
//        real_send += ret;
//    }
//    return 0;
}

int Buletooth::packReplayBuff(char *replay_buf,char flag)
{
   replay_buf[0] = 0xff;
   replay_buf[1] = flag;
   replay_buf[2] = 0x00;
   replay_buf[3] = 0x00;
   replay_buf[4] = 0x00;
   replay_buf[5] = 0x00;
   replay_buf[6] = 0x00;
   replay_buf[7] = 0x00;
   replay_buf[8] = 0x00;
   replay_buf[9] = 0x00;
   return 10;
}

int Buletooth::replyLocalNetInfo(unsigned char *replay_buf, int select)
{
//    AppConfig::set("Network/ip", ip);
//    AppConfig::set("Network/netmask", mask);
//    AppConfig::set("Network/gateway", gate);
    ResolvConf conf;
    conf.load(ResolvConf::getFilePath());
    ResolvConf::StringArray servers = conf.getDns();
    if (servers.size() > 0)
    {
        QString dns;
        if(select==0)
            dns = QString::fromStdString(servers[0]);
        else
            AppConfig::get("Network/dns", dns);
        qDebug("dns:%s",qPrintable(dns));
        QStringList partsdns = dns.split(' ');
        // 确保分割后至少有两个部分（IP地址和注释）
        if (partsdns.size() < 1) {
            qDebug() << "Invalid DNS input format.";
        }
        else
        {
            QString ipAddress = partsdns.first();
            QStringList parts = ipAddress.split(".", Qt::SkipEmptyParts);
            if (parts.size() == 4) {
                for(int i = 0; i < 4; ++i) {
                    bool ok;
                    replay_buf[22+i] = parts[i].toInt(&ok);
                    if (!ok) {
                        qDebug() << "Invalid DNS part";
                        break;
                    }
                }
            }
        }
    }
    else
        qDebug() << "Invalid DNS part";
    if(select==0)
    {
        net::NetConfig config;
        if (!net::getNetConfig("eth1", config))
        {
            qDebug("getNetConfig eth1 error!!!");
            int pos = m_networkConf.findConfig("eth1");
            qDebug("find eth1 pos:%d",pos);
            NetworkConfig& configlocal = m_networkConf.m_configs[pos];
            replay_buf[26] = configlocal.dhcp;
        }
        else
        {

            int pos = m_networkConf.findConfig("eth1");
            qDebug("find eth1 pos:%d",pos);
            NetworkConfig& configlocal = m_networkConf.m_configs[pos];
            replay_buf[26] = configlocal.dhcp;
            qDebug("get ip:%s,gateway:%s,mask:%s,dhcp:%d\n",config.ip.c_str(),config.gateway.c_str(),config.mask.c_str(),configlocal.dhcp);
            QString qIp = QString::fromStdString(config.ip);
            QStringList partsip = qIp.split(".", Qt::KeepEmptyParts);
            if (partsip.size() == 4) {
                for(int i = 0; i < 4; ++i) {
                    bool ok;
                    replay_buf[10+i] = partsip[i].toInt(&ok);
                    if (!ok) {
                        qDebug() << "Invalid IP address part";
                        break;
                    }
                }
            }
            else
                qDebug() << "Invalid IP address part";
            QString qgateway = QString::fromStdString(config.gateway);
            QStringList partsGate = qgateway.split(".", Qt::KeepEmptyParts);
            if (partsGate.size() == 4) {
                for(int i = 0; i < 4; ++i) {
                    bool ok;
                    replay_buf[18+i] = partsGate[i].toInt(&ok);
                    if (!ok) {
                        qDebug() << "Invalid netmask part";
                        break;
                    }
                }
            }
            else
                qDebug() << "Invalid netmask part";

            QString qNetMask = QString::fromStdString(config.mask);
            QStringList partsmask = qNetMask.split(".", Qt::KeepEmptyParts);
            if (partsmask.size() == 4) {
                for(int i = 0; i < 4; ++i) {
                    bool ok;
                    replay_buf[14+i] = partsmask[i].toInt(&ok);
                    if (!ok) {
                        qDebug() << "Invalid IP address part";
                        break;
                    }
                }
            }
            else
                qDebug() << "Invalid IP address part";
        }
    }
    else
    {
        int dhcp;
        AppConfig::get("Network/dhcp", dhcp);
        replay_buf[26] = dhcp;
        QString qIp;
        AppConfig::get("Network/ip", qIp);
        QStringList partsip = qIp.split(".", Qt::KeepEmptyParts);
        if (partsip.size() == 4) {
            for(int i = 0; i < 4; ++i) {
                bool ok;
                replay_buf[10+i] = partsip[i].toInt(&ok);
                if (!ok) {
                    qDebug() << "Invalid IP address part";
                    break;
                }
            }
        }
        else
            qDebug() << "Invalid IP address part";
        QString qgateway;
        AppConfig::get("Network/gateway", qgateway);
        QStringList partsGate = qgateway.split(".", Qt::KeepEmptyParts);
        if (partsGate.size() == 4) {
            for(int i = 0; i < 4; ++i) {
                bool ok;
                replay_buf[18+i] = partsGate[i].toInt(&ok);
                if (!ok) {
                    qDebug() << "Invalid netmask part";
                    break;
                }
            }
        }
        else
            qDebug() << "Invalid netmask part";

        QString qNetMask;
        AppConfig::get("Network/netmask", qNetMask);
        QStringList partsmask = qNetMask.split(".", Qt::KeepEmptyParts);
        if (partsmask.size() == 4) {
            for(int i = 0; i < 4; ++i) {
                bool ok;
                replay_buf[14+i] = partsmask[i].toInt(&ok);
                if (!ok) {
                    qDebug() << "Invalid IP address part";
                    break;
                }
            }
        }
        else
            qDebug() << "Invalid IP address part";
    }

    replay_buf[0] = 0xff;
    replay_buf[1] = 0xf2;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x11;
    replay_buf[4] = 0x00;
    if(select==0)
    replay_buf[5] = 0x00;
    else
    replay_buf[5] = 0x01;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<16; i++)
        crc ^= replay_buf[10+i];

    //replay_buf[9] = crc;//crc
    replay_buf[27] = '\0';
    return 27;
}

int Buletooth::replySNInfo(char *replay_buf)
{
    FREUser user;
    AppConfigUtil::loadUser(user);
    char *sn = user.devid.toUtf8().data();
    int len = user.devid.size();
    //qDebug("sn:%s, len:%d",sn,len);
    strcpy(&replay_buf[10],sn);
    qDebug("sn:%s, len:%d",sn,len);
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xf4;
    replay_buf[2] = 0x00;
    replay_buf[3] = len&0xff;
    replay_buf[4] = (len>>8)&0xff;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<len; i++)
        crc ^= replay_buf[10+i];
    //replay_buf[9] = crc;//crc
    replay_buf[10+len] = '\0';
//    for(int j=0;j<10+len;j++)
//        qDebug("%x ",replay_buf[j]);
    return 10+len;
}

int Buletooth::replyServerInfo(char *replay_buf)
{
    QString url;
    int port;
    AppConfigUtil::loadMqttServer(url, port);
    QStringList partsurl = url.split(".", Qt::KeepEmptyParts);
    if (partsurl.size() == 4) {
        for(int i = 0; i < 4; ++i) {
            bool ok;
            replay_buf[10+i] = partsurl[i].toInt(&ok);
            if (!ok) {
                qDebug() << "Invalid IP address part";
                break;
            }
        }
    }
    replay_buf[14] = port & 0xff;
    replay_buf[15] = (port>>8)&0xff;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xf6;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x06;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<6; i++)
        crc ^= replay_buf[10+i];
    //replay_buf[9] = crc;//crc
    replay_buf[16] = '\0';
    return 16;
}

int Buletooth::replyMqttInfo(char *replay_buf)
{
    QString username;
    QString password;
    QString orgname;
    AppConfigUtil::loadMqttInfo(username,password,orgname);
    QString url = QString("%1;%2;%3").arg(orgname).arg(username).arg(password);
    memcpy(&replay_buf[10], url.toUtf8().data(),url.size());
    int len = url.size();
    strcpy(&replay_buf[10],url.toUtf8().data());
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xf8;
    replay_buf[2] = 0x00;
    replay_buf[3] = len&0xff;
    replay_buf[4] = (len>>8)&0xff;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<len; i++)
        crc ^= replay_buf[10+i];
    //replay_buf[9] = crc;//crc
    replay_buf[10+len] = '\0';
    return 10+len;
}

int Buletooth::replyAcquistInfo(char *replay_buf)
{
    //TODO: REAL LENGTH and time to respond
    QString url;
    int port;
    AppConfigUtil::loadMqttServer(url, port);
    QStringList partsurl = url.split(".", Qt::KeepEmptyParts);
    if (partsurl.size() == 4) {
        for(int i = 0; i < 4; ++i) {
            bool ok;
            replay_buf[10+i] = partsurl[i].toInt(&ok);
            if (!ok) {
                qDebug() << "Invalid IP address part";
                break;
            }
        }
    }
    replay_buf[14] = port & 0xff;
    replay_buf[15] = (port>>8)&0xff;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xfa;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x06;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<6; i++)
        crc ^= replay_buf[10+i];
    //replay_buf[9] = crc;//crc
    replay_buf[16] = '\0';
    return 16;
}

int Buletooth::replyWifiInfo(char *replay_buf)
{
    QString wifiname;
    QString password;
    AppConfigUtil::loadWifiInfo(wifiname,password);
    QString url = QString("%1;%2").arg(wifiname).arg(password);
    memcpy(&replay_buf[10], url.toUtf8().data(),url.size());
    int len = url.size();
    strcpy(&replay_buf[10],url.toUtf8().data());
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xfc;
    replay_buf[2] = 0x00;
    replay_buf[3] = len&0xff;
    replay_buf[4] = (len>>8)&0xff;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<len; i++)
        crc ^= replay_buf[10+i];
    //replay_buf[9] = crc;//crc
    replay_buf[10+len] = '\0';
    return 10+len;
}

int Buletooth::replyBLInfo(char *replay_buf)
{
    QString url;
    AppConfigUtil::loadBLInfo(url);
    memcpy(&replay_buf[10], url.toUtf8().data(),url.size());
    int len = url.size();
    strcpy(&replay_buf[10],url.toUtf8().data());
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xfe;
    replay_buf[2] = 0x00;
    replay_buf[3] = len&0xff;
    replay_buf[4] = (len>>8)&0xff;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<len; i++)
        crc ^= replay_buf[10+i];
    //replay_buf[9] = crc;//crc
    replay_buf[10+len] = '\0';
    return 10+len;
}

int Buletooth::replyCardInfo(char *replay_buf)
{
    char card;
    int mid;
    AppConfig::get("Network/NET", mid);
    card = (char)mid;
    replay_buf[10] = card;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xe1;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x01;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[11] = '\0';
    return 11;
}

int Buletooth::replyRTUStationInfo(char *replay_buf)
{
    char card;
    int mid;
    AppConfig::get("Device/485sn", mid);
    replay_buf[10] = mid;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xe3;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x01;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[11] = '\0';
    return 11;
}

int Buletooth::replyRTUBitrateInfo(char *replay_buf)
{
    char card;
    int mid;
    unsigned char bitrate = 2;
    AppConfig::get("Device/485bitrate", mid);
    switch(mid)
    {
        case 2400:
        bitrate=0;
        break;
        case 4800:
        bitrate=1;
        break;
        case 9600:
        bitrate=2;
        break;
        case 19200:
        bitrate=3;
        break;
        case 38400:
        bitrate=4;
        break;
        case 57600:
        bitrate=5;
        break;
        case 115200:
        bitrate=6;
        break;
        case 230400:
        bitrate=7;
        break;
        default:
        bitrate=2;
        break;
    }
    replay_buf[10] = bitrate;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xe5;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x01;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[11] = '\0';
    return 11;
}

int Buletooth::replyUploadInfo(char *replay_buf)
{
    int mid;
    AppConfig::get("Device/update", mid);
    replay_buf[10] = mid;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xe7;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x01;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[11] = '\0';
    return 11;
}

int Buletooth::replyZGBInfo(char *replay_buf)
{
    int mid;
    AppConfig::get("Device/zgbFreqBand", mid);
    replay_buf[10] = mid;
    AppConfig::get("Device/zgbTxpower", mid);
    replay_buf[11] = mid;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xeb;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x02;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[12] = '\0';
    return 12;
}

int Buletooth::replyNETLinkInfo(char *replay_buf)
{
    int mid;
    QString url;
    int port;
    AppConfigUtil::loadMqttServer(url, port);
    CheckResult l_result = checkPortWithPing(url, port, 3000);
    switch (l_result) {
    case Success:
        replay_buf[10] = 1;
        replay_buf[11] = 1;
        break;
    case PortFiltered:
        replay_buf[10] = 1;
        replay_buf[11] = 0;
        break;
    case IpUnreachable:
        replay_buf[10] = 0;
        replay_buf[11] = 0;
        break;
    default:
        replay_buf[10] = 0;
        replay_buf[11] = 0;
        break;
    }
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xd5;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x02;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[12] = '\0';
    return 12;
}

int Buletooth::replyMQTTLinkInfo(char *replay_buf)
{
    char status = getMqttStatus();
    replay_buf[10] = status;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xd7;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x01;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[11] = '\0';
    return 11;
}

int Buletooth::replyLOGInfo(char *replay_buf)
{
    SensorLogManager * l_logManager  = AppService::logManager();
    m_LogData = l_logManager->readFileContent(LOG_PATH);
    int length = m_LogData.size();
    int count = (length+MAX_BLUETOOTH_LENGTH-1)/MAX_BLUETOOTH_LENGTH;
    memcpy(&replay_buf[10],&count,4);
    memcpy(&replay_buf[14],&length,4);
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xd9;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x08;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[18] = '\0';
    return 18;
}

int Buletooth::reuploadLogIndexSet(char *date, int len)
{
    unsigned short nu;
    int i=0;
    for(int i=0;2*i<len;i++)
    {
        memcpy(&nu, &date[2*i],2);
        m_reuploadlog.append(nu);
    }
    return 0;
}

int Buletooth::reuploadIndexIndexSet(char *date, int len)
{
    unsigned short nu;
    int i=0;
    for(int i=0;2*i<len;i++)
    {
        memcpy(&nu, &date[2*i],2);
        m_reuploadindex.append(nu);
    }
    return 0;
}

int Buletooth::replyLOGDatareupload(char *replay_buf,int fd)
{
    char *chararray = m_LogData.data();
    int length = m_LogData.size();
    int count = (length+MAX_BLUETOOTH_LENGTH-1)/MAX_BLUETOOTH_LENGTH;
    unsigned short len = 0;
    unsigned short lenmid = 0;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0x68;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x2c;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    for(int k=0; k<m_reuploadlog.size();k++)
    {
        memset(&replay_buf[10],0,MAX_BLUETOOTH_LENGTH+10);
        //memcpy(&replay_buf[10],&chararray[m_reuploadlog[k]*MAX_BLUETOOTH_LENGTH],MAX_BLUETOOTH_LENGTH);

        if(m_reuploadlog[k]<count-1)
        {
            memcpy(&replay_buf[10],&chararray[m_reuploadlog[k]*MAX_BLUETOOTH_LENGTH],MAX_BLUETOOTH_LENGTH);
            len = MAX_BLUETOOTH_LENGTH+10;
        }
        else if(m_reuploadlog[k]==count-1)
        {
            memcpy(&replay_buf[10],&chararray[m_reuploadlog[k]*MAX_BLUETOOTH_LENGTH],(length-MAX_BLUETOOTH_LENGTH*(count-1)));
            len = length-MAX_BLUETOOTH_LENGTH*(count-1)+10;
        }
        lenmid = len -10;
        int pos = m_reuploadlog[k];
        memcpy(&replay_buf[5],&pos,4);
        memcpy(&replay_buf[3],&lenmid,2);
        replay_buf[9] = Get_Crc(&replay_buf[10], len-10);
//        for(int k=0;k<len;k++)
//        printf("%x ",replay_buf[k]);
//        printf("\n");
        replay_ask_data(replay_buf,len,fd);
        //usleep(100000);
    }
    m_reuploadlog.clear();
    replay_buf[0] = 0xff;
    replay_buf[1] = 0x69;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x00;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    replay_buf[9] = 0x00;
    replay_ask_data(replay_buf,10,fd);
    return 0;
}

int Buletooth::replyLOGData(char *replay_buf,int fd)
{
    replay_buf[0] = 0xff;
    replay_buf[1] = 0x68;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x2c;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    // 获取 char 数组（QByteArray 保证以 '\0' 结尾）
//    QByteArray logDataCopy = m_LogData;   // 深拷贝
//    const char* chararray = logDataCopy.data();
//    int length = logDataCopy.size();
    char *chararray = m_LogData.data();
    int length = m_LogData.size();
    int len = 0;
    unsigned short lenmid = 0;
    int count = (length+MAX_BLUETOOTH_LENGTH-1)/MAX_BLUETOOTH_LENGTH;
    for(int i=0;i<count;i++)
    {
        memset(&replay_buf[10],0,MAX_BLUETOOTH_LENGTH);
        if(i<count-1)
        {
            memcpy(&replay_buf[10],&chararray[i*MAX_BLUETOOTH_LENGTH],MAX_BLUETOOTH_LENGTH);
            len = MAX_BLUETOOTH_LENGTH+10;
        }
        else
        {
            memcpy(&replay_buf[10],&chararray[i*MAX_BLUETOOTH_LENGTH],(length-MAX_BLUETOOTH_LENGTH*(count-1)));
            len = length-MAX_BLUETOOTH_LENGTH*(count-1)+10;
        }
        lenmid = len -10;
        memcpy(&replay_buf[5],&i,4);
        memcpy(&replay_buf[3],&lenmid,2);
        qDebug("length:%d,count:%d,len:%d,lenmid:%hu",length,count,len,lenmid);
        //qDebug() << "length:" << length << "count:" << count << "len:" << len << "lenmid:" << lenmid;
        replay_buf[9] = Get_Crc(&replay_buf[10], lenmid);
        replay_ask_data(replay_buf,len,fd);
        //usleep(100000);
    }
    m_reuploadlog.clear();
    replay_buf[0] = 0xff;
    replay_buf[1] = 0x69;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x00;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    replay_buf[9] = 0x00;
    replay_ask_data(replay_buf,10,fd);
    return 0;
}

QByteArray Buletooth::mergeJsonFilesToByteArray()
{
    QMap<int, QJsonObject> mergedData; // key: ChNo
    SensorLogManager * l_logManager  = AppService::logManager();
    QMap<int, QList<QString>> allRecv = l_logManager->getAllIndexRecv();//getAllIndexSend();
    for (auto it = allRecv.begin(); it != allRecv.end(); ++it) {
        int sn = it.key();
        const QList<QString>& recvList = it.value();
        qDebug() << "SN" << sn << "has" << recvList.size() << "IndexRecv entries:";
        for (const QString& path : recvList) {
            QString filePath = QString("/userdata/uartdata/%1").arg(path);
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning() << "Cannot open file:" << filePath;
                continue;
            }
            QByteArray data = l_logManager->readFileContent(filePath);
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(data, &error);
            if (error.error != QJsonParseError::NoError) {
                qWarning() << "JSON parse error in" << filePath << ":" << error.errorString();
                continue;
            }
            if (!doc.isArray()) {
                qWarning() << "Invalid JSON format in" << filePath << ": expected array";
                continue;
            }
            QJsonArray rootArray = doc.array();
            if (rootArray.size() < 2) {
                qWarning() << "Incomplete JSON in" << filePath << ": need at least 2 objects";
                continue;
            }

            QJsonObject channelObj = rootArray[0].toObject();
            QJsonObject deviceObj = rootArray[1].toObject();

            // 提取 ChNo
            int chNo = channelObj["ChNo"].toInt();
            if (chNo == 0 && !channelObj.contains("ChNo"))
                chNo = deviceObj["SN"].toInt();
            if (chNo == 0) {
                qWarning() << "Cannot determine ChNo in file:" << filePath;
                continue;
            }

            QJsonObject chEntry = mergedData.value(chNo, QJsonObject());
            if (chEntry.isEmpty()) {
                chEntry["ChannelData"] = QJsonArray();
                chEntry["DeviceData"] = QJsonArray();
            }

            QJsonArray channelArray = chEntry["ChannelData"].toArray();
            channelArray.append(channelObj);
            chEntry["ChannelData"] = channelArray;

            QJsonArray deviceArray = chEntry["DeviceData"].toArray();
            deviceArray.append(deviceObj);
            chEntry["DeviceData"] = deviceArray;

            mergedData[chNo] = chEntry;
        }
    }

    if (mergedData.isEmpty()) {
        qWarning() << "No valid data to merge";
        return QByteArray();
    }

    // 构建最终 JSON 对象
    QJsonObject outputObject;
    for (auto it = mergedData.begin(); it != mergedData.end(); ++it) {
        outputObject[QString::number(it.key())] = it.value();
    }
    m_JSONHandle.saveToJsonFile(outputObject, "/userdata/xxx.json");
    QJsonDocument doc(outputObject);
    return doc.toJson(QJsonDocument::Indented);
}

int Buletooth::replyIndexInfo(char *replay_buf)
{
    int length=0;
    int count=0;
    m_IndexData = mergeJsonFilesToByteArray();
    if (m_IndexData.isEmpty()) {
        qDebug() << "Merge failed or no data";
    }
    else
    {
        length = m_IndexData.size();
        count = (length+MAX_BLUETOOTH_LENGTH-1)/MAX_BLUETOOTH_LENGTH;
    }
    memcpy(&replay_buf[10],&count,4);
    memcpy(&replay_buf[14],&length,4);
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xdb;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x08;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    //unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[18] = '\0';
    return 18;
}

int Buletooth::replyAPNInfo(char *replay_buf)
{
    QString apn;
    AppConfig::get("Network/APN", apn);
    //memcpy(&replay_buf[10], apn.toUtf8().data(),apn.size());
    int len = apn.size();
    strcpy(&replay_buf[10],apn.toUtf8().data());
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xdd;
    replay_buf[2] = 0x00;
    replay_buf[3] = len&0xff;
    replay_buf[4] = (len>>8)&0xff;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<len; i++)
        crc ^= replay_buf[10+i];
    //replay_buf[9] = crc;//crc
    replay_buf[10+len] = '\0';
    return 10+len;
}

int Buletooth::replyZGBIndex(char *replay_buf,int fd)
{
//    m_IndexData = mergeJsonFilesToByteArray();
//    if (m_IndexData.isEmpty()) {
//        qDebug() << "Merge failed or no data";
//        return -1;
//    }
    replay_buf[0] = 0xff;
    replay_buf[1] = 0x6c;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x2c;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    // 获取 char 数组（QByteArray 保证以 '\0' 结尾）
    char *chararray = m_IndexData.data();
    int length = m_IndexData.size();
    int len = 0;
    unsigned short lenmid = 0;
    int count = (length+MAX_BLUETOOTH_LENGTH-1)/MAX_BLUETOOTH_LENGTH;
    for(int i=0;i<count;i++)
    {
        memset(&replay_buf[10],0,MAX_BLUETOOTH_LENGTH+10);
        if(i<count-1)
        {
            memcpy(&replay_buf[10],&chararray[i*MAX_BLUETOOTH_LENGTH],MAX_BLUETOOTH_LENGTH);
            len = MAX_BLUETOOTH_LENGTH+10;
        }
        else
        {
            memcpy(&replay_buf[10],&chararray[i*MAX_BLUETOOTH_LENGTH],(length-MAX_BLUETOOTH_LENGTH*(count-1)));
            len = length-MAX_BLUETOOTH_LENGTH*(count-1)+10;
        }
        lenmid = len -10;
        memcpy(&replay_buf[5],&i,4);
        memcpy(&replay_buf[3],&lenmid,2);
        qDebug() << "length:" << length << "count:" << count << "len:" << len;
        replay_buf[9] = Get_Crc(&replay_buf[10], len-10);
//        for(int k=0;k<len;k++)
//        printf("%x ",replay_buf[k]);
//        printf("\n");
        replay_ask_data(replay_buf,len,fd);
        //usleep(100000);
    }
    m_reuploadindex.clear();
    replay_buf[0] = 0xff;
    replay_buf[1] = 0x6e;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x00;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    replay_buf[9] = 0x00;
    replay_ask_data(replay_buf,10,fd);
    return 0;
}

int Buletooth::replyIndexDatareupload(char *replay_buf,int fd)
{
    char *chararray = m_IndexData.data();
    int length = m_IndexData.size();
    int count = (length+43)/44;
    unsigned short len = 0;
    unsigned short lenmid = 0;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0x6c;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x2c;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    for(int k=0; k<m_reuploadindex.size();k++)
    {
        memset(&replay_buf[10],0,MAX_BLUETOOTH_LENGTH+10);
        //memcpy(&replay_buf[10],&chararray[m_reuploadindex[k]*MAX_BLUETOOTH_LENGTH],MAX_BLUETOOTH_LENGTH);

        if(m_reuploadindex[k]<count-1)
        {
            memcpy(&replay_buf[10],&chararray[m_reuploadindex[k]*MAX_BLUETOOTH_LENGTH],MAX_BLUETOOTH_LENGTH);
            len = MAX_BLUETOOTH_LENGTH+10;
        }
        else if(m_reuploadindex[k]==count-1)
        {
            memcpy(&replay_buf[10],&chararray[m_reuploadindex[k]*MAX_BLUETOOTH_LENGTH],(length-MAX_BLUETOOTH_LENGTH*(count-1)));
            len = length-MAX_BLUETOOTH_LENGTH*(count-1)+10;
        }
        int pos = m_reuploadindex[k];
        memcpy(&replay_buf[5],&pos,4);
        lenmid = len -10;
        memcpy(&replay_buf[3],&lenmid,2);
        replay_buf[9] = Get_Crc(&replay_buf[10], len-10);
//        for(int k=0;k<len;k++)
//        printf("%x ",replay_buf[k]);
//        printf("\n");
        replay_ask_data(replay_buf,len,fd);
        //usleep(100000);
    }
    m_reuploadindex.clear();
    replay_buf[0] = 0xff;
    replay_buf[1] = 0x6e;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x00;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    replay_buf[9] = 0x00;
    replay_ask_data(replay_buf,10,fd);
    return 0;
}

int Buletooth::replySNListInfo(char *replay_buf,int fd)
{
    int mid;
    int len = 0;
    unsigned short lenmid = 0;
    int count = (m_ZIGBEEParam->count+4-1)/4;
//    int mod = m_ZIGBEEParam->count%4;
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xe9;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x01;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    for(int i=0;i<count;i++)
    {
        memset(&replay_buf[10],0,64);
        for(int j=0;j<4;j++)
        {
            int pos = 4*i+j;
            if(pos<m_ZIGBEEParam->count)
            {
                replay_buf[10+j*5] = m_ZIGBEEParam->sensor_info[4*i+j].Channel;
                int nu = atoi(m_ZIGBEEParam->sensor_info[4*i+j].Sn);
                memcpy(&replay_buf[10+j*5+1],&nu,4);
                len = 10+j*5+5;
            }

        }
        if(i==(count-1))
            replay_buf[5] = 0x01;
        for(int k=0;k<len;k++)
        printf("%x ",replay_buf[k]);
        printf("\n");
        lenmid = len -10;
        memcpy(&replay_buf[3],&lenmid,2);
        replay_ask_data(replay_buf,len,fd);
        usleep(100000);
    }
    if(count == 0)
    {
        replay_buf[5] = 0x01;
        len = 10;
        replay_ask_data(replay_buf,len,fd);
    }
    return 0;
}

int Buletooth::handle_recv_data(char *buf, int len, int fd)
{
    int i = 0;
    int ret = 0;
    char replay_buf[250] ={0};
    char *p =NULL;
    int handle_len =0;
    int check_data = 0;
    int cmd_len=0;
    //pthread_t heart_beat_id[50];
    //static_handle_beat = 0;
    p = buf;
    while(handle_len<(len-1))
    {
        for (i=handle_len; i<(len-1);i++ )
        {
            if(0xff != buf[i])
            {
                if(i==(len-2))
                {
                    qDebug("recv data text is  wrong!!!\n");
                    return -1;
                }
                else
                {
                    handle_len++;
                    continue;
                }
            }
            else
            {
                p = buf + handle_len;
                cmd_len = check_cmd_data(p);
                if(cmd_len==0 || cmd_len>(len-handle_len))
                {
                    qDebug("cmd len wrong %d\n", cmd_len);
                    handle_len++;
                            continue;

                }
                break;
            }
        }
        i=1;
        qDebug("select %x",p[i]);
        switch(p[i])
        {
            case 0x03:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setSysNetwork(reinterpret_cast<unsigned char*>(&p[10]),cmd_len,p[5]);
                ret = packReplayBuff(replay_buf,4);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x05:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setMqttServer(reinterpret_cast<unsigned char*>(&p[10]));
                //TODO :restart mqtt
                ret = packReplayBuff(replay_buf,6);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x07:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setMqttInfo(&p[10],cmd_len-10);
                ret = packReplayBuff(replay_buf,8);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x09:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setAcquistInfo(&p[10]);
                ret = packReplayBuff(replay_buf,0x0a);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x0b:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setWifiInfo(&p[10],cmd_len-10);
                ret = packReplayBuff(replay_buf,0x0c);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x0d:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setBLNameInfo(&p[10],cmd_len-10);
                ret = packReplayBuff(replay_buf,0x0e);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x0f:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setNetSelect(p[10]);
                ret = packReplayBuff(replay_buf,0x10);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x11:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setSN(&p[10],cmd_len-10);
                ret = packReplayBuff(replay_buf,0x12);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            case 0x13:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setModbusRTUStation(p[10]);
                ret = packReplayBuff(replay_buf,0x14);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            case 0x15:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setModbusRTUBitrate(p[10]);
                ret = packReplayBuff(replay_buf,0x16);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            case 0x17:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setUpdatemode(p[10]);
                ret = packReplayBuff(replay_buf,0x18);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            case 0x19:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                bool status = setSNList(&p[10],cmd_len-10,p[5]);
                if(p[5]==1)
                {
                    ret = packReplayBuff(replay_buf,0x1a);
                    if(status)
                    replay_ask_data(replay_buf, ret, fd);
                }
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            case 0x1b:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                setZigbeeInfo(p[10],p[11]);
                ret = packReplayBuff(replay_buf,0x1c);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

        case 0x23:
        check_data = judge_ask_data(p, cmd_len);
        if(check_data == 1)
        {
            bool status = set5gAPN(&p[10],cmd_len-10);
            if(status)
            {
                ret = packReplayBuff(replay_buf,0x24);
                replay_ask_data(replay_buf, ret, fd);
            }
            handle_len+=cmd_len;
        }
        else
        handle_len++;
        break;


            case 0xf1:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                ret =replyLocalNetInfo(reinterpret_cast<unsigned char*>(replay_buf),p[5]);
                ret =replay_ask_data(replay_buf, ret, fd);
                qDebug("ret:%d",ret);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xf3:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replySNInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xf5:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyServerInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xf7:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyMqttInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xf9:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyAcquistInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xfb:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyWifiInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xfd:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyBLInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xe0:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyCardInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            case 0xe2:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyRTUStationInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xe4:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyRTUBitrateInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xe6:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyUploadInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            case 0xe8:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replySNListInfo(replay_buf, fd);
                //replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xea:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyZGBInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            case 0xd4:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                ret =replyNETLinkInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xd6:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                ret =replyMQTTLinkInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xd8:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                //get_extern_state();
                ret =replyLOGInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xda:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                ret =replyIndexInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0xdc:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                ret =replyAPNInfo(replay_buf);
                replay_ask_data(replay_buf, ret, fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;


            case 0x67:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                ret =replyLOGData(replay_buf,fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x6a:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                reuploadLogIndexSet(&p[10],cmd_len-10);
                if(p[5]==1)
                {
                    replyLOGDatareupload(replay_buf,fd);
                }
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x6b:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                ret =replyZGBIndex(replay_buf,fd);
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;
            case 0x6d:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                reuploadIndexIndexSet(&p[10],cmd_len-10);
                if(p[5]==1)
                {
                    replyIndexDatareupload(replay_buf,fd);
                }
                handle_len+=cmd_len;
            }
            else
            handle_len++;
            break;

            default:
            handle_len+=cmd_len;
            break;
        }
        memset(replay_buf, 0, sizeof(replay_buf));
        //p = buf + handle_len;
    }
    return ret;

}

int Buletooth::BLEHandle()
{
    int rev_cmd_len;
    int datalen = 0;
    int midlen = 0;
    int ret = 0;
    while (1)
    {
//        QString url;
//        int port;
//        AppConfigUtil::loadMqttServer(url, port);
//        int ret = checkPortWithPing(url, port, 3000);
//        qDebug("==============check port and ip result:%d===================",ret);
        memset(m_revbuff,0,sizeof(m_revbuff));
        rev_cmd_len = uart_bt_receive_data(m_fd, m_revbuff, 512, 5);
        //rev_cmd_len = freq_uart_read(m_revbuff, 512);
        qDebug("%s",m_revbuff);
        qDebug("------BLEHandle : recv data -------:%drev_cmd_len\n",rev_cmd_len);
        for(datalen = 0; datalen < rev_cmd_len; datalen++)
        {
            qDebug("0x%02X ", (unsigned char)m_revbuff[datalen]);
        }
#if 1
        if(m_revbuff[0]==0xff)
        {
            midlen = (m_revbuff[4]<<8)| m_revbuff[3]+10;
            if((midlen>rev_cmd_len&&midlen<500)||(midlen<rev_cmd_len&&midlen<500))
            {
                ret = uart_bt_receive_data(m_fd, &m_revbuff[rev_cmd_len], 32, 5);
                for(int i = 0; i < ret; i++)
                {
                    qDebug("0x%02X ", (unsigned char)m_revbuff[i+rev_cmd_len]);
                }
                rev_cmd_len+=ret;
            }
        }

#endif
//        BleHandle *workerThread = new BleHandle(m_fd,m_revbuff,rev_cmd_len);
//        connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
//        workerThread->start();

        //emit starthandle(m_revbuff,rev_cmd_len,m_fd);
        qDebug("handle data lem:%d\n", rev_cmd_len);

        //if(m_revbuff[0]==0xff)
        handle_recv_data(m_revbuff, rev_cmd_len, m_fd);
        qDebug("handle over\n");
    }
}
