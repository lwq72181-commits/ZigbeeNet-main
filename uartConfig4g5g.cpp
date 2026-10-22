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
#include "uartConfig4g5g.h"

uartConfig4g5g::uartConfig4g5g()
{

}

int uartConfig4g5g::open_serial_port(char *com)
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

void uartConfig4g5g::close_serial_port()
{
    /* Restore original terminal settings on exit */
    tcsetattr(m_fd, TCSANOW, &uartsave);
    close(m_fd);
}


int uartConfig4g5g::set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop)
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

int uartConfig4g5g::uart_send_data(char *data, int len)
{
    int ret;
    if (data == NULL)
        return -1;
    qDebug("uart_send_data 4g5g\n");
    write(m_fd, data, len);
    return 0;
}

int uartConfig4g5g::uart_receive_data(char *buf, int buf_len, int timeout_s)
{
    int ret;
    fd_set input;
    struct timeval timeout;

    timeout.tv_sec = timeout_s;
    timeout.tv_usec = 0;

    qDebug("Ready to receive data, timeout is %ds, buflen:%d\n", timeout_s,buf_len);

    FD_ZERO(&input);
    FD_SET(m_fd, &input);

    ret = select(m_fd + 1, &input, NULL, NULL, &timeout);
    //ret = select(m_fd + 1, &input, NULL, NULL, NULL);//zuse
    if (ret == -1) {
        qDebug("Wait for data failed.\n");
        return -1;
    }
    else if (ret) {
        ret = read(m_fd, buf, buf_len);
        if (ret <= 0) {
            qDebug("Receive data failed.\n");
            return -1;
        }
        else
            qDebug("uart read len:%d.\n",ret);
    }
    else {
        qDebug("Wait for data timeout.\n");
        return -1;
    }

    return ret;
}

int uartConfig4g5g::init(QString com)
{
    int ret = 0;
    int bitrate = 115200;
    m_fd = open_serial_port(com.toLocal8Bit().data());
    if (m_fd < 0) {
        qDebug("Open serial port failed\n");
        return -1;
    }
    /* Set serial port, 115200, 8N1 */
    ret = set_serial_port(m_fd, bitrate, 8, 'N', 1);
    if (ret) {
        qDebug("Set serial port error!\n");
        return -1;
    }
    qDebug("init 4g5g port over");
    return 0;
}
