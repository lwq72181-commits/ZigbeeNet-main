#include "Uart485Server.h"
#include <QDebug>
#include <QFile>
#include <QThread>
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
#include "AppConfig.h"

Uart485Server::Uart485Server()
{

}


int Uart485Server::open_serial_port(char *com)
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

void Uart485Server::close_serial_port()
{
    /* Restore original terminal settings on exit */
    tcsetattr(m_fd, TCSANOW, &uartsave);
    close(m_fd);
}


int Uart485Server::set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop)
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
        case 19200:
            cfsetispeed(&newtio, B19200);
            cfsetospeed(&newtio, B19200);
        break;
        case 38400:
            cfsetispeed(&newtio, B38400);
            cfsetospeed(&newtio, B38400);
        break;
        case 57600:
            cfsetispeed(&newtio, B57600);
            cfsetospeed(&newtio, B57600);
        break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 230400:
            cfsetispeed(&newtio, B230400);
            cfsetospeed(&newtio, B230400);
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

int Uart485Server::uart_send_data(int fd, char *data, int len)
{
    int ret;
    if (data == NULL)
        return -1;
    qDebug("uart_send_data 485\n");
    write(fd, data, len);
    return 0;
}

int Uart485Server::uart_receive_data(int fd, char *buf, int buf_len, int timeout_s)
{
    int ret;
    fd_set input;
    struct timeval timeout;

    timeout.tv_sec = timeout_s;
    timeout.tv_usec = 0;

    qDebug("Ready to receive data, timeout is %ds, buflen:%d\n", timeout_s,buf_len);

    FD_ZERO(&input);
    FD_SET(fd, &input);

    ret = select(fd + 1, &input, NULL, NULL, &timeout);
    //ret = select(fd + 1, &input, NULL, NULL, NULL);//zuse
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

int Uart485Server::init(QString com)
{
    int ret = 0;
    int bitrate = 9600;
    QString name;
    AppConfig::get("Device/485sn", m_sn);
    m_fd = open_serial_port(com.toLocal8Bit().data());
    if (m_fd < 0) {
        qDebug("Open serial port failed\n");
        return -1;
    }
    AppConfig::get("Device/485bitrate", bitrate);
    /* Set serial port, 115200, 8N1 */
    ret = set_serial_port(m_fd, bitrate, 8, 'N', 1);
    if (ret) {
        qDebug("Set serial port error!\n");
        return -1;
    }
    qDebug("init 485 port over m_sn:%d",m_sn);
    return 0;
}

int Uart485Server::config_serial_port()
{
    int bitrate = 9600;
    int ret = 0;
    AppConfig::get("Device/485bitrate", bitrate);
    qDebug("config_serial_port bitrate:%d",bitrate);
    /* Set serial port, 115200, 8N1 */
    ret = set_serial_port(m_fd, bitrate, 8, 'N', 1);
    if (ret) {
        qDebug("Set serial port error!\n");
        return -1;
    }
    qDebug("config_serial_port bitrate:%d",bitrate);
    return ret;
}

int Uart485Server::send_local_index()
{
    QString fileName;
    char index[58] = {0};
    while(m_JSONHandle.GetOldestFileNameUart(fileName, 0)==0)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open file:" << file.errorString();
        }
        int len = file.read(index, 57);
//        for(int i = 0; i < len; i++)
//        {
//            qDebug("0x%02X ", (unsigned char)index[i]);
//        }
        uart_send_data(m_fd, index, 57);
        m_JSONHandle.deleteFile(fileName);
    }
    return 0;
}

unsigned short Uart485Server::CRC16_MODBUS(unsigned char *buf, int len)
{
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++)
  {
    crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc
    for (int i = 8; i != 0; i--)   // Loop over each bit
    {
      if ((crc & 0x0001) != 0)   // If the LSB is set
      {
        crc >>= 1; // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else // Else LSB is not set
      {
        crc >>= 1;    // Just shift right
      }
    }
  }
  //conversion between high and low bytes
  crc = ((crc & 0x00ff) << 8) | ((crc & 0xff00) >> 8);
  return crc;
}

int Uart485Server::sendAddrData(int addr,int size)
{
    int flag=0;
    int start=0;
    int count = 0;
    QString name;
    char index[255] = {0};
    index[0] = 0x01;
    index[1] = 0x04;
    index[2] = 2*size;
    count = size/2;
    start = addr%100;
    flag = (addr/100)%10;
    name = m_param->name.at(flag);
    qDebug("addr:%d,size:%d,count:%d",addr,size,count);
    for(int i=0;i<count;i++)
    {
        float mid;
        mid = m_vector->readElement(name,start+i-1);
        memcpy(&index[3+4*i],&mid,4);
    }
    int16_t crc16 = CRC16_MODBUS( reinterpret_cast<unsigned char*>(index), 3+2*size);
    index[3+2*size] = crc16 & 0xff;//(crc16>>8) & 0xff;
    index[3+2*size+1] = (crc16>>8) & 0xff;//crc16 & 0xff;
    uart_send_data(m_fd, index, 3+2*size+2);
    return 0;
}


int Uart485Server::handle_recv_data(char *buf, int len, int fd)
{
    int addr;
    int size;
    if(m_sn == m_revbuff[0])
    {
        if(m_revbuff[1] == 0x04)
        {
            addr = (m_revbuff[2]<<8)|m_revbuff[3];
            size = (m_revbuff[4]<<8)|m_revbuff[5];
            qDebug("modbus 485 addr:%d, size:%d",addr,size);
            if(addr<30001||addr>30999||size>80)
            {
                //sendFormateerror();
            }
            else
            {
                if(addr%100!=0)
                sendAddrData(addr,size);
            }

        }
    }
    return 0;
}

void Uart485Server::updateinfo()
{
    AppConfig::get("Device/485sn", m_sn);
    qDebug("get 485sn:%d\n",m_sn);
}

int Uart485Server::waiting_function(TCPParam* param)
{
    int ret = 0;
    pthread_mutex_lock(&param->mutex);
    qDebug("waiting_function\n");
    pthread_cond_wait(&param->cond, &param->mutex);
    pthread_mutex_unlock(&param->mutex);
    qDebug("waiting_function end\n");
    return ret;
}

//int Uart485Server::UartServerHandle()
//{
//    int rev_cmd_len;
//    char buf[20] = {0};
//    qint64 checktime = QDateTime::currentDateTime().toSecsSinceEpoch();
//    qDebug("init startsec value:%lld\n",checktime);

//}

#if 1
int Uart485Server::UartServerHandle()
{
    int rev_cmd_len;
    char buf[20] = {0};
    qint64 checktime = QDateTime::currentDateTime().toSecsSinceEpoch();
    qDebug("init startsec value:%lld\n",checktime);
    QString SNList;
    AppConfig::get("Device/gatesn", SNList);
    qDebug("==========gatesn:%s",qPrintable(SNList));
    QStringList parts = SNList.split(".", Qt::SkipEmptyParts);
    qDebug("==========size:%d",parts.size());
    for(int i=0;i<parts.size();i++)
    {
        bool ok;
        int mid = parts[i].toInt(&ok);
        if (!ok) {
            qDebug() << "Invalid gate sn part";
            break;
        }
        else
        {
            qDebug("==========sn:%d",mid);
            loadSN.append(mid);
        }

    }
    buf[1] = 0x04;
    buf[2] = 0x00;
    buf[3] = 0x01;
    buf[4] = 0x00;
    buf[5] = 0x34;
    buf[6] = 0x11;
    buf[7] = 0x11;
    while (1)
    {
        //memset(m_revbuff,0,sizeof(m_revbuff));57
        qint64 nowsec = QDateTime::currentDateTime().toSecsSinceEpoch();
        if((nowsec-checktime) >= 300)
        {
            qDebug("==========need to get 485 index");
            checktime = nowsec;
            m_date.clear();
            for(int i=0;i<loadSN.size();i++)
            {
                m_sn = loadSN[i];
                buf[0] = loadSN[i]%100;
                qDebug("sn:%d,station:%x",m_sn,buf[0]);
                int16_t crc16 = CRC16_MODBUS( reinterpret_cast<unsigned char*>(buf), 6);
                buf[6] = crc16 & 0xff;//(crc16>>8) & 0xff;
                buf[7] = (crc16>>8) & 0xff;//crc16 & 0xff;

                uart_send_data(m_fd, buf, 8);
                while((rev_cmd_len = uart_receive_data(m_fd, m_revbuff, 57, 5))>0)
                {
                    m_date.append(m_revbuff, rev_cmd_len);
                }
                qDebug("recv date len:%d,index count :%d",m_date.size(),m_date.size()/57);
                for(int i=0;i<m_date.size()-56;i++)
                {
                    if((m_date[i]==buf[0])&&(m_date[i+1]==0x04)&&(m_date[i+2]==0x34))
                    {
                        int16_t crc16 = CRC16_MODBUS( reinterpret_cast<unsigned char*>(m_date.data()), 55);
                        if((m_date[55]==(crc16 & 0xff))&&(m_date[56]==((crc16>>8) & 0xff)))
                        {
                            QByteArray index = m_date.mid(i,57);
                            if(index.size()==57)
                            storeIndexData(index.data(), 57);
                        }
                    }
                }
            }

        }
        else
        QThread::msleep(500);
    }
    return 0;
}

int Uart485Server::storeIndexData(char* buf, int len)
{
    QJsonArray arry;
    QJsonObject obj;
    QJsonObject objSensorInfo;
    char sn[6] = {0};
    int nu =0;
    memcpy(sn,&buf[3],6);
    nu = atoi(sn);
    qDebug("sn:%s,nu:%d",sn,nu);
    obj.insert("ChNo", nu);
    objSensorInfo.insert("SN", nu);
    obj.insert("ChType", 0);
    QJsonArray array;
    QJsonObject HFAContent;
    HFAContent.insert("Dir", 1);
    HFAContent.insert("Code", 112);
    QJsonArray HFAarray;
    float highe;
    //memcpy(&highe,&buf[18],4);
    memcpy(&highe,&buf[13],4);
    HFAarray.append(highe);
    HFAContent.insert("Values", HFAarray);
    array.append(HFAContent);

    QJsonObject LFSContent;
    LFSContent.insert("Dir", 1);
    LFSContent.insert("Code", 101);
    QJsonArray LFAarray;
    float lowe;
    //memcpy(&lowe,&buf[26],4);
    memcpy(&lowe,&buf[21],4);
    LFAarray.append(lowe);
    LFSContent.insert("Values", LFAarray);
    array.append(LFSContent);

    QJsonObject speedContent;
    speedContent.insert("Dir", 1);
    speedContent.insert("Code", 107);
    QJsonArray speedAarray;
    float zspeed;
    //memcpy(&zspeed,&buf[30],4);
    memcpy(&zspeed,&buf[25],4);
    speedAarray.append(zspeed);
    speedContent.insert("Values", speedAarray);
    array.append(speedContent);

    QJsonObject tempContent;
    tempContent.insert("Dir", 1);
    tempContent.insert("Code", 100);
    QJsonArray tempAarray;
    float temp;
    //memcpy(&temp,&buf[50],4);
    memcpy(&temp,&buf[45],4);
    tempAarray.append(temp);
    tempContent.insert("Values", tempAarray);
    array.append(tempContent);
    objSensorInfo.insert("Temp", temp);
    signed char RSSI = static_cast<signed char>(buf[49]);
//    memcpy(&RSSI,&buf[54],2);
    objSensorInfo.insert("RSSI", static_cast<int>(RSSI));
    float sensorinfo;
    //memcpy(&sensorinfo,&buf[56+41],4);
    memcpy(&sensorinfo,&buf[51],4);
    objSensorInfo.insert("Vol", sensorinfo/1000);
//    memcpy(&sensorinfo,&buf[56+45],4);
    objSensorInfo.insert("Cur", 0.01);


    QJsonObject xLowContent;
    xLowContent.insert("Dir", 2);
    xLowContent.insert("Code", 101);
    QJsonArray xLowAarray;
    float xLow;
    //memcpy(&xLow,&buf[34],4);
    memcpy(&xLow,&buf[29],4);
    xLowAarray.append(xLow);
    xLowContent.insert("Values", xLowAarray);
    array.append(xLowContent);

    QJsonObject xSpeedContent;
    xSpeedContent.insert("Dir", 2);
    xSpeedContent.insert("Code", 107);
    QJsonArray xSpeedAarray;
    float xSpeed;
    //memcpy(&xSpeed,&buf[38],4);
    memcpy(&xSpeed,&buf[33],4);
    xSpeedAarray.append(xSpeed);
    xSpeedContent.insert("Values", xSpeedAarray);
    array.append(xSpeedContent);

    QJsonObject yLowContent;
    yLowContent.insert("Dir", 3);
    yLowContent.insert("Code", 101);
    QJsonArray yLowAarray;
    float yLow;
    //memcpy(&yLow,&buf[42],4);
    memcpy(&yLow,&buf[37],4);
    yLowAarray.append(yLow);
    yLowContent.insert("Values", yLowAarray);
    array.append(yLowContent);

    QJsonObject ySpeedContent;
    ySpeedContent.insert("Dir", 3);
    ySpeedContent.insert("Code", 107);
    QJsonArray ySpeedAarray;
    float ySpeed;
    //memcpy(&ySpeed,&buf[46],4);
    memcpy(&ySpeed,&buf[41],4);
    ySpeedAarray.append(ySpeed);
    ySpeedContent.insert("Values", ySpeedAarray);
    array.append(ySpeedContent);

    obj.insert("Content", array);

//    QDateTime currentTime = QDateTime::currentDateTimeUtc();
//    QString isoFormat = currentTime.toString("yyyy-MM-dd'T'hh:mm:ss");
    QJsonArray timearray;
    //memcpy(&m_sec,&buf[10],4);
    //memcpy(&m_sec,&buf[9],4);
    QDateTime dateTimeFromSec = QDateTime::currentDateTime();
    //QDateTime dateTimeFromSec = QDateTime::fromSecsSinceEpoch(m_sec);
    QString isoFormat = dateTimeFromSec.toString("yyyy-MM-dd'T'hh:mm:ss");

    timearray.append(isoFormat);
    obj.insert("Time", timearray);
    objSensorInfo.insert("Clock", isoFormat);
    arry.append(obj);
    arry.append(objSensorInfo);
    int mode =0;
    AppConfig::get("Device/update", mode);
    if(((mode>>1)&0x01) == 0x01)
    {
        while(m_JSONHandle.get_partation_percent(FREQ_DIR)<0.1)
            m_JSONHandle.rm_oldest_day_file();
        qint64 checktime = QDateTime::currentDateTime().toSecsSinceEpoch();
        QString fileName = m_JSONHandle.GetSaveFileNameP(0,nu,checktime);
        qDebug("save index file :%s",qPrintable(fileName));
        m_JSONHandle.saveToJsonFile(arry, fileName);
//        QString fileName1 = m_JSONHandle.GetSaveFileNameTestP(0,nu,checktime);
//        qDebug("save index file :%s",qPrintable(fileName1));
//        m_JSONHandle.saveToFile(reinterpret_cast<const uint8_t*>(buf), len, fileName1);
    }
//    int station;
//    AppConfig::get("Device/485sn", station);
//    date[0] = station;
//    date[1] = 0x04;
//    date[2] = 0x34;
//    memcpy(&date[3],sn,5);//sn
//    memcpy(&date[9],&buf[2+8],4);//timestamp
//    memcpy(&date[13],&buf[10+8],38);//index
//    memcpy(&date[51],&buf[89+8],4);//index
//    crc16 = CRC16_MODBUS( date, 55);
//    date[55] = crc16 & 0xff;//(crc16>>8) & 0xff;
//    date[56] = (crc16>>8) & 0xff;//crc16 & 0xff;
    return 0;
}
#endif
