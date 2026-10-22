#ifndef UART485SERVER_H
#define UART485SERVER_H
#include <QObject>
#include <termios.h>
#include "JsonFileHandle.h"
#include "Acquisitiontype.h"
#include "ThreadSafeMultiVector.h"

class Uart485Server:public QObject
{
public:
    Uart485Server();
    int init(QString com);
    int open_serial_port(char *com);
    void close_serial_port();
    int set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop);
    int uart_send_data(int fd, char *data, int len);
    int uart_receive_data(int fd, char *buf, int buf_len, int timeout_s);
    int handle_recv_data(char *buf, int len, int fd);
    int send_local_index();
    int sendAddrData(int addr,int size);
    int config_serial_port();
    void updateinfo();
    int waiting_function(TCPParam* param);
    int storeIndexData(char* buf, int len);
    unsigned short CRC16_MODBUS(unsigned char *buf, int len);
private:
    int m_fd;
    int m_sn;
    int m_sec;
    struct termios uartsave;
    char m_revbuff[512];
    QByteArray m_date;
    JSONFileHandle m_JSONHandle;
    QList<int> loadSN;
    TCPParam *m_param;
    ThreadSafeMultiVector *m_vector;
    QJsonArray m_baseArray;
public slots:
    int UartServerHandle();
};

#endif // UART485SERVER_H
