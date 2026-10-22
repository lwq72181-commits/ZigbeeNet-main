#ifndef UARTCONFIG4G5G_H
#define UARTCONFIG4G5G_H
#include <QObject>
#include <termios.h>

class uartConfig4g5g
{
public:
    uartConfig4g5g();
    int init(QString com);
    int open_serial_port(char *com);
    void close_serial_port();
    int set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop);
    int uart_send_data(char *data, int len);
    int uart_receive_data(char *buf, int buf_len, int timeout_s);
private:
    int m_fd;
    struct termios uartsave;
};

#endif // UARTCONFIG4G5G_H
