#include "Uart458client.h"
#include <QDebug>
#include <QFile>
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

Uart458Client::Uart458Client()
{

}


int Uart458Client::open_serial_port(char *com)
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

void Uart458Client::close_serial_port()
{
    /* Restore original terminal settings on exit */
    tcsetattr(m_fd, TCSANOW, &uartsave);
    close(m_fd);
}


int Uart458Client::set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop)
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

bool Uart458Client::loadDataPairs(const QString& filePath, QVector<DataPair>& dataPairs) {
    // 打开并读取JSON文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open the config file.");
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // 解析JSON文档
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    // 获取根对象
    QJsonObject rootObj = jsonDoc.object();
    if (!rootObj.contains("dataPairs")) {
        qWarning("Missing 'dataPairs' in JSON.");
        return false;
    }

    QJsonValue pairsValue = rootObj.value("dataPairs");
    if (!pairsValue.isArray()) {
        qWarning("The 'dataPairs' is not a valid JSON array.");
        return false;
    }

    // 解析数据对
    QJsonArray pairsArray = pairsValue.toArray();
    for (const QJsonValueRef pairValue : pairsArray) {
        if (pairValue.isObject()) {
            QJsonObject pairObj = pairValue.toObject();
            if (!pairObj.contains("name") || !pairObj.contains("values")) {
                qWarning("Each pair must have 'name' and 'values'.");
                continue;
            }

            DataPair dataPair;
            dataPair.name = pairObj.value("name").toInt(); // 将name转换为整数
            QJsonArray valuesArray = pairObj.value("values").toArray();
            for (const QJsonValue &value : valuesArray) {
                int intValue = value.toInt();
                if (intValue >= 1000000 && intValue <= 9999999) { // 确保值为7位数
                    dataPair.values.append(intValue);
                } else {
                    qWarning() << "Invalid value:" << intValue << "(not a 7-digit number)";
                }
            }

            dataPairs.append(dataPair);
        } else {
            qWarning("Invalid pair found in JSON array.");
        }
    }

    return true;
}

int Uart458Client::uart_send_data(int fd, char *data, int len)
{
    int ret;
    if (data == NULL)
        return -1;
    qDebug("uart_send_data 485\n");
    write(fd, data, len);
    return 0;
}

int Uart458Client::uart_receive_data(int fd, char *buf, int buf_len, int timeout_s)
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

int Uart458Client::init(QString com)
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

int Uart458Client::config_serial_port()
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

int Uart458Client::send_local_index()
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

unsigned short Uart458Client::CRC16_MODBUS(unsigned char *buf, int len)
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

int Uart458Client::sendAddrData(int addr,int size)
{
    int flag=0;
    int start=0;
    int count = 0;
    QString name;
    char index[255] = {0};
    int station;
    AppConfig::get("Device/485sn", station);
    index[0] = station;
    index[1] = 0x04;
    index[2] = 2*size;
    count = size/2;
    start = addr%100/2;
    flag = (addr/100)%10;
    name = m_param->name.at(flag);
    qDebug("addr:%d,size:%d,count:%d,start:%d,name:%s",addr,size,count,start,qPrintable(name));
    for(int i=0;i<count;i++)
    {
        float mid;
        mid = m_vector->readElement(name,start+i);
        memcpy(&index[3+4*i],&mid,4);
        qDebug("value:%f",mid);
    }
    int16_t crc16 = CRC16_MODBUS( reinterpret_cast<unsigned char*>(index), 3+2*size);
#ifdef MODBUS_CRC_HIGH
    index[3+2*size] = (crc16>>8) & 0xff;//(crc16>>8) & 0xff;
    index[3+2*size+1] = crc16 & 0xff; //crc16 & 0xff;
#else
    index[3+2*size] = crc16 & 0xff;//(crc16>>8) & 0xff;
    index[3+2*size+1] = (crc16>>8) & 0xff;//crc16 & 0xff;
#endif
    uart_send_data(m_fd, index, 3+2*size+2);
    return 0;
}


int Uart458Client::handle_recv_data(char *buf, int len, int fd)
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
            if(addr<0||addr>999||size>80)
            {
                //sendFormateerror();
            }
            else
            {
                int16_t crc16 = CRC16_MODBUS( reinterpret_cast<unsigned char*>(m_revbuff), 6);
                qDebug("modbus crc:%x,%x",crc16 & 0xff,(crc16>>8) & 0xff);
#ifdef MODBUS_CRC_HIGH
                if((m_revbuff[7]==(crc16 & 0xff))&&(m_revbuff[6]==((crc16>>8) & 0xff)))
#else
                if((m_revbuff[6]==(crc16 & 0xff))&&(m_revbuff[7]==((crc16>>8) & 0xff)))
#endif
                {
                    //if(addr%100!=0)
                    sendAddrData(addr,size);
                }

            }

        }
    }
    else
        qDebug("485 date format error :%d   %d",m_sn,m_revbuff[0]);
    return 0;
}

void Uart458Client::updateinfo()
{
    AppConfig::get("Device/485sn", m_sn);
    qDebug("get 485sn:%d\n",m_sn);
}

int Uart458Client::waiting_function(TCPParam* param)
{
    int ret = 0;
    pthread_mutex_lock(&param->mutex);
    qDebug("waiting_function\n");
    pthread_cond_wait(&param->cond, &param->mutex);
    pthread_mutex_unlock(&param->mutex);
    qDebug("waiting_function end\n");
    return ret;
}

int Uart458Client::UartHandle(TCPParam *param,ThreadSafeMultiVector *vector)
{
    int rev_cmd_len;
    int datalen = 0;
    int station;
    m_param = param;
    m_vector = vector;
    waiting_function(param);

    while (1)
    {
        //memset(m_revbuff,0,sizeof(m_revbuff));
        rev_cmd_len = uart_receive_data(m_fd, m_revbuff, 64, 5);
        //qDebug("%s",m_revbuff);
        qDebug("------485 Handle : recv data -------:%drev_cmd_len\n",rev_cmd_len);
        for(datalen = 0; datalen < rev_cmd_len; datalen++)
        {
            qDebug("0x%02X ", (unsigned char)m_revbuff[datalen]);
        }
        qDebug("handle data lem:%d\n", rev_cmd_len);
        AppConfig::get("Device/485sn", station);
        qDebug("start modbus 485 UartHandle station:%d!!!!!!!!!!!!",station);
        //if(m_revbuff[0]==0xff)
#ifdef MODBUS_MODE_SEND
        handle_recv_data(m_revbuff, rev_cmd_len, m_fd);
#else
        for(int i=0;i<rev_cmd_len-7;i++)
        {
//            buf[1] = 0x04;
//            buf[2] = 0x00;
//            buf[3] = 0x01;
//            buf[4] = 0x00;
//            buf[5] = 0x34;
//            buf[6] = 0x11;
//            buf[7] = 0x11;
//            int16_t crc16 = CRC16_MODBUS( reinterpret_cast<unsigned char*>(buf), 6);
//            buf[6] = crc16 & 0xff;//(crc16>>8) & 0xff;
//            buf[7] = (crc16>>8) & 0xff;//crc16 & 0xff;
            if(m_revbuff[i]==station&&m_revbuff[i+1]==0x04&&m_revbuff[i+2]==0x00&&m_revbuff[i+3]==0x01)
            {
                int16_t crc16 = CRC16_MODBUS( reinterpret_cast<unsigned char*>(&m_revbuff[i]), 6);
                if((m_revbuff[i+6]==(crc16 & 0xff))&&(m_revbuff[i+7]==((crc16>>8) & 0xff)))
                {
                    send_local_index();
                }
            }
        }
#endif
        qDebug("handle over\n");
    }
    return 0;
}

//int Uart458Client::UartServerHandle()
//{
//    int rev_cmd_len;
//    int datalen = 0;
//    char buf[20] = {0};
//    char sn;
//    QString SNList;
//    AppConfig::get("Device/gatesn", SNList);
//    QStringList parts = SNList.split(",", Qt::SkipEmptyParts);
//    for(int i=0;i<parts.size();i++)
//    {
//        bool ok;
//        int mid = parts[i].toInt(&ok);
//        if (!ok) {
//            qDebug() << "Invalid gate sn part";
//            break;
//        }
//        else
//            loadSN.append(mid);
//    }
//    buf[1] = 0x04;
//    buf[2] = 0x00;
//    buf[3] = 0x01;
//    buf[4] = 0x00;
//    buf[5] = 0x34;
//    buf[6] = 0x11;
//    buf[7] = 0x11;
//    while (1)
//    {
//        //memset(m_revbuff,0,sizeof(m_revbuff));57
//        for(int i=0;i<loadSN.size();i++)
//        {
//            buf[0] = loadSN[i]%100;
//            uart_send_data(m_fd, buf, 8);
//        }
//        rev_cmd_len = uart_receive_data(m_fd, m_revbuff, 57, 5);

//        qDebug("handle data lem:%d\n", rev_cmd_len);
//    }
//    return 0;
//}

