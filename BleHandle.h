#ifndef BLEHANDLE_H
#define BLEHANDLE_H

#include <QCoreApplication>
#include <QDebug>
#include <termios.h>
#include "NetworkConf.h"
class BleHandle: public QObject
{
    Q_OBJECT
public:
    BleHandle(int id);
    int set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop);
    void close_serial_port(int fd);
    int open_serial_port(char *com);
    int init_uart_bt(int fd,const char *name);
    int uart_bt_send_data(int fd, char *data, int len);
    int uart_bt_receive_data(int fd, char *buf, int buf_len, int timeout_s);
    int init(QString com);
protected:
    //int handle_recv_data(char *buf, int len, int fd);
    int replySNInfo(char *replay_buf);
    int replyLocalNetInfo(unsigned char *replay_buf);
    int replyServerInfo(char *replay_buf);
    int replyMqttInfo(char *replay_buf);
    int replyAcquistInfo(char *replay_buf);
    int replyWifiInfo(char *replay_buf);
    int replyBLInfo(char *replay_buf);
    int replyCardInfo(char *replay_buf);
    int packReplayBuff(char *replay_buf,char flag);
    int replay_ask_data(char *replay_buf, int len, int fd);
    int setAcquistInfo(char *net);
    int setWifiInfo(char *net, int len);
    int setBLNameInfo(char *net, int len);
    int setNetSelect(char net);
    int setMqttInfo(char *net, int len);
    int setMqttServer(unsigned char *net);
    int setSysNetwork(unsigned char *net);
    int judge_ask_data(char *buf, int len);
    int check_cmd_data(char *cmd);
    int super_system(const char * cmd, char *retmsg, int msg_len);
private:
    int m_fd;
    int m_len;
    char m_data[128];
    struct termios uartsave;
    NetworkConf m_networkConf;
public slots:
    int handle_recv_data(char *buf, int len, int fd);
};

#endif // BLEHANDLE_H
