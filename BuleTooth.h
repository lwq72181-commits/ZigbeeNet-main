#ifndef BULETOOTH_H
#define BULETOOTH_H
#include <termios.h>
#include <QObject>
#include <QThread>
#include "NetworkConf.h"
#include "BleHandle.h"
#include "zigbee/proto_parse.h"
#include "Acquisitiontype.h"
#include "JsonFileHandle.h"
#include "uartConfig4g5g.h"
class Buletooth:public QObject
{
    Q_OBJECT
public:
    Buletooth();
    int set_serial_port(int fd, int baudrate, int nbits, char nevent, int nstop);
    void close_serial_port(int fd);
    int open_serial_port(char *com);
    int init_uart_bt(int fd,const char *name);
    int uart_bt_send_data(int fd, char *data, int len);
    int freq_uart_read(void *buf, int len);
    int uart_bt_receive_data(int fd, char *buf, int buf_len, int timeout_s);
    int init(QString com, ZIGBEEParam *param);
protected:
    int handle_recv_data(char *buf, int len, int fd);
    int replySNInfo(char *replay_buf);
    int replyLocalNetInfo(unsigned char *replay_buf, int select);
    int replyServerInfo(char *replay_buf);
    int replyMqttInfo(char *replay_buf);
    int replyAcquistInfo(char *replay_buf);
    int replyWifiInfo(char *replay_buf);
    int replyBLInfo(char *replay_buf);
    int replyCardInfo(char *replay_buf);
    int replyRTUStationInfo(char *replay_buf);
    int replyRTUBitrateInfo(char *replay_buf);
    int replyUploadInfo(char *replay_buf);
    int packReplayBuff(char *replay_buf,char flag);
    int replay_ask_data(char *replay_buf, int len, int fd);
    int replySNListInfo(char *replay_buf,int fd);
    int replyNETLinkInfo(char *replay_buf);
    int replyMQTTLinkInfo(char *replay_buf);
    int replyLOGInfo(char *replay_buf);
    int replyIndexInfo(char *replay_buf);
    int replyZGBIndex(char *replay_buf,int fd);
    int replyLOGData(char *replay_buf,int fd);
    int replyLOGDatareupload(char *replay_buf,int fd);
    int reuploadIndexIndexSet(char *date, int len);
    int replyIndexDatareupload(char *replay_buf,int fd);
    int replyAPNInfo(char *replay_buf);
    int setAcquistInfo(char *net);
    int setWifiInfo(char *net, int len);
    int setBLNameInfo(char *net, int len);
    int setNetSelect(char net);
    int setSNList(char *date, int len, char flag);
    int refresh_sensor_file_info();
    int setZigbeeInfo(unsigned char chn,unsigned char power);
    int replyZGBInfo(char *replay_buf);
    int setMqttInfo(char *net, int len);
    int setMqttServer(unsigned char *net);
    int setSysNetwork(unsigned char *net,int len, int select);
    int setSN(char *sn, int len);
    int setModbusRTUStation(unsigned char nu);
    int setModbusRTUBitrate(unsigned char nu);
    int setUpdatemode(unsigned char nu);
    int set5gAPN(char *date, int len);
    int judge_ask_data(char *buf, int len);
    int check_cmd_data(char *cmd);
    int super_system(const char * cmd, char *retmsg, int msg_len);
    int saveLocalParam();
    bool pingHost(const QString &ip, int timeoutMs = 3000);
    CheckResult checkPortWithPing(const QString &ip, quint16 port, int timeoutMs = 3000);
    int getMqttStatus();
    QByteArray mergeJsonFilesToByteArray();
    char Get_Crc(char *buf, int len);
    int reuploadLogIndexSet(char *date, int len);
private:
    int snflag;
    int m_fd;
    struct termios uartsave;
    char m_revbuff[512];
    NetworkConf m_networkConf;
    ZIGBEEParam * m_ZIGBEEParam;
    QByteArray m_IndexData;
    QByteArray m_LogData;
    QList<int> m_reuploadlog;
    QList<int> m_reuploadindex;
    JSONFileHandle m_JSONHandle;
    uartConfig4g5g *m_uart;
    pthread_mutex_t *m_mutex;
//    BleHandle *workerThread;
//    QThread m_BLEThread;
public slots:
    int BLEHandle();
signals:
    void BLRestartMqtt();
    void BLRestart485();
    void BLset485sn();
    void BLSetDEVID();
    void BLUpdateMode(unsigned char nu);
//    void starthandle(char *data,int len,int fd);
};

#endif // BULETOOTH_H
