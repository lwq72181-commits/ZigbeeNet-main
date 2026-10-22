#include "BleHandle.h"
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <termios.h>
#include <sys/select.h>
#include <libgen.h>
#include <QPath.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ResolvConf.h"
#include "NetworkConf.h"
#include "AppConst.h"
#include "AppService.h"
#include "AppConfigUtil.h"
#include "AppConfig.h"

BleHandle::BleHandle(int id)
    :  m_fd(id)
{
}

int BleHandle::open_serial_port(char *com)
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

void BleHandle::close_serial_port(int fd)
{
    /* Restore original terminal settings on exit */
    tcsetattr(fd, TCSANOW, &uartsave);
    close(fd);
}


int BleHandle::set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop)
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

int BleHandle::init_uart_bt(int fd,const char *name)
{
    int ret;
    char at_cmd_buf[64];

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
    //qDebug("set ble broadcast");
    return 0;
}

int BleHandle::uart_bt_send_data(int fd, char *data, int len)
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
    usleep(300000);
    memset(at_cmd_buf, 0, sizeof(at_cmd_buf));
    ret = read(fd, at_cmd_buf, sizeof(at_cmd_buf));
    if (ret > 0) {
        if (strstr(at_cmd_buf, "OK") == NULL) {
            qDebug("Send data failed\n");
            return -1;
        }
    } else {
        qDebug("Send data failed\n");
        return -1;
    }
    qDebug("BL send over");
    return 0;
}

int BleHandle::uart_bt_receive_data(int fd, char *buf, int buf_len, int timeout_s)
{
    int ret;
    fd_set input;
    struct timeval timeout;

    timeout.tv_sec = timeout_s;
    timeout.tv_usec = 0;

    qDebug("Ready to receive data, timeout is %ds\n", timeout_s);

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

int BleHandle::init(QString com)
{
    int ret = 0;
    QString name;
    m_fd = open_serial_port(com.toLocal8Bit().data());
    if (m_fd < 0) {
        qDebug("Open serial port failed\n");
        return -1;
    }
    /* Set serial port, 115200, 8N1 */
    ret = set_serial_port(m_fd, 9600, 8, 'N', 1);
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
    return 0;
}

int BleHandle::check_cmd_data(char *cmd)
{
    int ret = 0;
    switch(cmd[1])
    {
        case 3:
        if(cmd[3]==0)
            ret = 10;
        else if(cmd[3]==0xf1)
            ret = 26;
        break;
        case 5:
        if(cmd[3]==6)
            ret = 16;
        break;
        case 7:
        ret = (cmd[4]<<8)|cmd[3]+10;
        break;
        case 9:
        if(cmd[3]==6)
            ret = 16;
        break;
        case 0x0b:
        ret = (cmd[4]<<8)|cmd[3]+10;
        break;
        case 0x0d:
        ret = (cmd[4]<<8)|cmd[3]+10;
        break;
        case 0x0f:
        if(cmd[3]==0x01)
            ret = 11;
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
        case 0xe0:
        if(cmd[3]==0)
            ret = 10;
        break;
        default:
        break;
    }
    return ret;
}

int BleHandle::judge_ask_data(char *buf, int len)
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

int BleHandle::super_system(const char * cmd, char *retmsg, int msg_len)
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
                qDebug("Err: Fuc: popen error:");
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

int BleHandle::setSysNetwork(unsigned char *net)
{
    QString dns = QString("%1.%2.%3.%4").arg(net[12]).arg(net[13]).arg(net[14]).arg(net[15]);
    NetworkConf& networkConf = m_networkConf;
    {
        NetworkConfig& config = networkConf.m_configs[0];
        config.ip = QString("%1.%2.%3.%4").arg(net[0]).arg(net[1]).arg(net[2]).arg(net[3]);
        config.mask = QString("%1.%2.%3.%4").arg(net[4]).arg(net[5]).arg(net[6]).arg(net[7]);
        config.gate = QString("%1.%2.%3.%4").arg(net[8]).arg(net[9]).arg(net[10]).arg(net[11]);
        config.dhcp = false;
        qDebug("ip:%s",config.ip.toUtf8().constData());
        qDebug("mask:%s",config.mask.toUtf8().constData());
        qDebug("gate:%s",config.gate.toUtf8().constData());
    }
    networkConf.save();
    networkConf.apply();

    ResolvConf conf;
    conf.load(ResolvConf::getFilePath());
    conf.addDns(dns.toStdString());
    conf.save(ResolvConf::getFilePath());

    QString filepath = QPath::join(AppConst::getEtcDir(), ResolvConf::FILE_NAME);
    conf.save(filepath.toStdString());
    return 0;
}

int BleHandle::setMqttServer(unsigned char *net)
{
    QString url = QString("%1.%2.%3.%4").arg(net[0]).arg(net[1]).arg(net[2]).arg(net[3]);
    int port = net[4]|(net[5]<<8);
    AppConfigUtil::saveMqttServer(url, port);
    qDebug("Device. ip:%s,port:%d\n", url.toUtf8().constData(), port);
    return 0;
}

int BleHandle::setMqttInfo(char *net, int len)
{
    qDebug("len:%d",len);
    QString str = QString::fromUtf8(net,len);
    qDebug() << str;
    QStringList splitList = str.split(":");
    for(const auto &str : splitList) {
            qDebug() << str;
        }
    AppConfigUtil::saveMqttInfo(splitList[1],splitList[2],splitList[0]);

    //emit BLRestartMqtt();
//    AppService::mqttService().close();
//    FREUser user;
//    AppConfigUtil::loadUser(user);
//    qDebug("Device. username:%s, password: %s,orgname: %s, devid: %s\n",
//        qPrintable(user.username), qPrintable(user.password), qPrintable(user.orgname), qPrintable(user.devid));
//    AppService::mqttService().setMqUrl(user.url, user.username, user.password, user.orgname, user.devid, user.port);
    return 0;
}

int BleHandle::setAcquistInfo(char *net)
{
    return 0;
}

int BleHandle::setWifiInfo(char *net, int len)
{
    char retmsg[50] = {0};
    qDebug("len:%d",len);
    QString str = QString::fromUtf8(net,len);
    qDebug() << str;
    QStringList splitList = str.split(":");
    for(const auto &str : splitList) {
            qDebug() << str;
        }
    AppConfigUtil::saveWifiInfo(splitList[1],splitList[2]);
    QString cmd = QString("/userdata/etc/wifi_setup.sh -i %1 -p %2").arg(splitList[1]).arg(splitList[2]);
    super_system(qPrintable(cmd), retmsg, sizeof(retmsg));
    return 0;
}

int BleHandle::setBLNameInfo(char *net, int len)
{
    int ret;
    qDebug("len:%d",len);
    QString str = QString::fromUtf8(net,len);
    qDebug() << str;
    AppConfigUtil::saveBLInfo(str);
    ret = init_uart_bt(m_fd,net);
    if (ret != 0) {
        qDebug("setBLNameInfo Init uart bt failed\n");
        return -1;
    }
    return 0;
}

int BleHandle::setNetSelect(char net)
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
    return 0;
}

int BleHandle::replay_ask_data(char *replay_buf, int len, int fd)
{
    int ret = 0;
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

int BleHandle::packReplayBuff(char *replay_buf,char flag)
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

int BleHandle::replyLocalNetInfo(unsigned char *replay_buf)
{
    ResolvConf conf;
    conf.load(ResolvConf::getFilePath());
    ResolvConf::StringArray servers = conf.getDns();
    if (servers.size() > 0)
    {
        QString dns = QString::fromStdString(servers[0]);
        qDebug("dns:%s",qPrintable(dns));
        QStringList parts = dns.split(".", Qt::KeepEmptyParts);
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
    else
        qDebug() << "Invalid DNS part";
    net::NetConfig config;
    if (!net::getNetConfig("eth1", config))
    {
        qDebug("getNetConfig eth1 error!!!");
    }
    else
    {
        qDebug("get ip:%s,gateway:%s,mask:%s\n",config.ip.c_str(),config.gateway.c_str(),config.mask.c_str());
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
    replay_buf[0] = 0xff;
    replay_buf[1] = 0xf2;
    replay_buf[2] = 0x00;
    replay_buf[3] = 0x11;
    replay_buf[4] = 0x00;
    replay_buf[5] = 0x00;
    replay_buf[6] = 0x00;
    replay_buf[7] = 0x00;
    replay_buf[8] = 0x00;
    unsigned char crc = 0;
    for(int i = 0; i<16; i++)
        crc ^= replay_buf[10+i];
    //replay_buf[9] = crc;//crc
    replay_buf[26] = '\0';
    return 26;
}

int BleHandle::replySNInfo(char *replay_buf)
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

int BleHandle::replyServerInfo(char *replay_buf)
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

int BleHandle::replyMqttInfo(char *replay_buf)
{
    QString username;
    QString password;
    QString orgname;
    AppConfigUtil::loadMqttInfo(username,password,orgname);
    QString url = QString("%1;%2;%3").arg(username).arg(password).arg(orgname);
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

int BleHandle::replyAcquistInfo(char *replay_buf)
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

int BleHandle::replyWifiInfo(char *replay_buf)
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

int BleHandle::replyBLInfo(char *replay_buf)
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

int BleHandle::replyCardInfo(char *replay_buf)
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
    unsigned char crc = 0;
    //replay_buf[9] = 0^replay_buf[10];//crc
    replay_buf[11] = '\0';
    return 11;
}

int BleHandle::handle_recv_data(char *buf, int len, int fd)
{
    int i = 0;
    int ret = 0;
    char replay_buf[100] ={0};
    char *p =NULL;
    int handle_len =0;
    int check_data = 0;
    int cmd_len=0;
    //pthread_t heart_beat_id[50];
    //static_handle_beat = 0;
    p = buf;
    while(handle_len<(len-1))
    {
        for (i=0; i<len-handle_len;i++ )
        {
            if(0xff != p[i])
            {
                if(i==(len-handle_len-1))
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
                setSysNetwork(reinterpret_cast<unsigned char*>(&p[10]));
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
            case 0xf1:
            check_data = judge_ask_data(p, cmd_len);
            if(check_data == 1)
            {
                ret =replyLocalNetInfo(reinterpret_cast<unsigned char*>(replay_buf));
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
            default:
            handle_len+=cmd_len;
            break;
        }
        memset(replay_buf, 0, sizeof(replay_buf));
        p = buf + handle_len;
    }
    return ret;

}


//void BleHandle::run()
//{
//    qDebug() << "Worker Thread" << m_fd << "started. Running in thread:" << QThread::currentThreadId();
//    handle_recv_data(m_data, m_len, m_fd);
//}
