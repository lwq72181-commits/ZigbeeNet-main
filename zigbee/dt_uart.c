#include "dt_uart.h"
#include <termios.h>
#include <unistd.h>
#include "common.h"


int dt_uart_write(int fd, void *buf, int len)
{
    int ret = 0;
    int send_len = 0;
    int rest_len = len;

    while (rest_len > 0) {
        ret = write(fd, (char *)buf + send_len, rest_len);
        if (ret < 0) {
            DT_ERR("Uart Write error, ret = %d\n", ret);
            return -1;
        }
        rest_len -= ret;
        send_len += ret;
    }
    return ret;
}

int dt_uart_read(int fd, void *buf, int len)
{
    int read_len = 0;
    int recv_len = len;

    while (recv_len > 0) {
        int len = read(fd, (char *)buf+read_len, recv_len);
        if (len <= 0)
            return read_len;
        read_len += len;
        recv_len -= len;
    }

    return read_len;
}

int dt_uart_set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio;
    bzero(&newtio, sizeof(newtio));
#if 0
    if (tcgetattr(fd, &newtio) != 0) {
        LOGMSG("tcgetattr failed...\n");
        return -1;
    }
#endif
    newtio.c_cflag |= CLOCAL;

    newtio.c_cflag |= CREAD;

    newtio.c_cflag &= ~CSIZE;
    switch (nBits) {
    case 7:
      newtio.c_cflag |= CS7;
      break;
    case 8:
      newtio.c_cflag |= CS8;
      break;
    }

    //newtio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    switch (nEvent) {
    case 'O':
      newtio.c_cflag |= PARENB;
      newtio.c_cflag |= PARODD;
      newtio.c_iflag |= INPCK;
      break;
    case 'E':
      newtio.c_iflag |= INPCK;
      newtio.c_cflag |= PARENB;
      newtio.c_cflag &= ~PARODD;
      break;
    case 'N':
      newtio.c_cflag &= ~PARENB;
      break;
    }


    switch (nSpeed) {
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
    case 230400:
      cfsetispeed(&newtio, B230400);
      cfsetospeed(&newtio, B230400);
      break;
    case 921600:
      cfsetispeed(&newtio, B921600);
      cfsetospeed(&newtio, B921600);
      break;
    default:
      cfsetispeed(&newtio, B115200);
      cfsetospeed(&newtio, B115200);
      break;
    }

    if(1 == nStop)
        newtio.c_cflag &= ~CSTOPB;
    else if (2 == nStop)
        newtio.c_cflag |= CSTOPB;

    newtio.c_cc[VTIME] = 1;     // 字节之间间隔100ms
    newtio.c_cc[VMIN] = 0;      // 至少接收字节数
    tcflush(fd, TCIOFLUSH);

    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
        DT_ERR("tcsetattr failed...\n");
        return DT_ERROR;
    }

    return DT_EOK;
}

int dt_uart_open(const char *port)
{
    int fd = -1;

    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd > 0)
        DT_LOG("open %s success, fd = %d...\n", port, fd);

    return fd;
}