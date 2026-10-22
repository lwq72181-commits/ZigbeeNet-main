#ifndef ACQUISITIONTYPE_H
#define ACQUISITIONTYPE_H
#include <QString>
#include <QStringList>
#include <vector>
#include <QList>

struct FreqTriggerLimitPara//触发⻔限
{
    QList<int> Channels;//通道集合（这些通道同步触发,多设备不同参数）
    float AccLimit;//加速度⻔限值
    float VerLimit;//速度⻔限值
    float RpmLimit;//转速⻔限值
    float ShockLimit;//冲击⻔限值
};

struct FreqTriggerPara//触发采集参数
{
    int Freq;//分析频率(HZ)
    int Length;//采集⻓度
    int TriggerGroup;//触发采集组数
    int TriggerType;//触发类型 1-加速度；2-速度；3-转速 4-加速度\转速；5-速度\转速；
    QList<FreqTriggerLimitPara> TriggerLimitList;//触发⻔限列表
};

struct FreqRtuChannelPara//Rtu通道参数
{
    int ChNo;//Rtu通道号
    int RTUDeviceAdd;//设备地址
    int FunCode;//功能码
    int RegAddr;//寄存器起始地址
    int DataLength;//数据⻓度
    int DevId;//设备id

};

struct FreqSpeedChannelPara//转速通道参数
{
    int ChNo;//转速通道号
    int TriggerCount;//每圈触发数
    int DevId;//设备id
};

struct FreqTechChannelPara//⼯艺通道参数
{
    int ChNo;//⼯艺通道号
    float SensCoef;//灵敏度系数
    float Amendment;//修正项
    int DevId;//设备id
};

struct FreqLimitPara //报警⻔限参数
{
    int Code;//指标类型（100-温度101-低频总值107-速度总值112-⾼频总值）
    int Level;//报警等级
    int Limit;//报警⻔限
    int Dir;//⽅向（0-单轴；1-Z轴；2-X轴；3-Y轴）
};

struct FreqVibPara //振动采集参数
{
    int Code;//采集定义编码0-⾳频波形；1-⻓波形；11-⾼频波形；12-低频波形；15-速度波形
    int LowFreq;//下限分析频率（HZ）
    int HighFreq;//上限分析频率（HZ）-该频率⽤于采集
    int Length;//采集⻓度
    int PlusLength;//附加⻓度
    QString Unit;//⼯程单位（m/s^2）
    int Dir;//⽅向（0-单轴；1-Z轴；2-X轴；3-Y轴
};

struct FreqVibChannalPara//振动通道
{
    int ChNo;//振动通道号
    int SpeedNo;//转速通道号（振动关联转速）有线
    int SensType;//传感器类型（0-加速度； 1-位移）
    float SensCoef;//灵敏度系数（有线）
    int CnstCurSrcState;//控制恒流源状态（0-关闭；1-打开）（有线）
    QList<FreqVibPara> VibParaList;
    //VibPara *VibParaList;//振动采集参数列表
    QList<FreqLimitPara> LimitParaList;
    //LimitPara *LimitParaList;//报警⻔限参数（如果不涉及报警加采可传空数组）
    int DevId;//设备id（有线）
    int IsTem;//是否测温（0 --不测温 1--测温）
    float SpeedDiffRate;//转差率 （⽤于磁⼒计计算转频使⽤）
    int PolePairsNum;//极对数（⽤于磁⼒计计算转频使⽤）
};

struct FreqCollectPara//采集参数
{
    QList<FreqVibChannalPara> VibChannelList;
   // VibChannalPara *VibChannelList;//振动/⾳频通道列表
    QList<FreqTechChannelPara> TechChannelList;
    //TechChannelPara *TechChannelList;//⼯艺通道列表（有线）
    QList<FreqSpeedChannelPara> SpeedChannelList;
    //SpeedChannelPara *SpeedChannelList;//转速通道列表（有线）
    QList<FreqRtuChannelPara> RtuChannelList;
    //RtuChannelPara *RtuChannelList;//Modbus Rtu通道列表
    QList<FreqTriggerPara> TriggerList;
    //TriggerPara *TriggerList;//触发采集参数列表（有线）
    QString Topic;
    pthread_mutex_t mutex;//
    pthread_cond_t cond;//
    pthread_mutex_t mutexwork;//
    pthread_cond_t condwork;//
    pthread_mutex_t mutexsend;//
    pthread_cond_t condsend;//
    QList<int> reupload;
    int topicStatus;
    int waverecved;
    int condition;//
    int conditionwork;
    int conditionsend;
    int initFalg;
    int messageid;
    int sendlen;
    int loop;
    int sendflag;
    int updateflag;
    bool IsResume;
    std::atomic_bool mqttopen;
};


struct FRESysParam//策略参数
{
    int IndexPeriod;//指标间隔(s)
    int WavePeriod;//波形间隔(s)
    int LongWavePeriod;//⻓波形间隔(s)
    int ZigbeeDB;
    int CollectType;//采集类型(1-定时，2-连续) 仅⽀持有线
    int OutType485;//485输出类型（0-⾼频；1-低频；2速度）
    int TechPeriod;//⼯艺数据采集间隔（单位：s）
    int RpmPeriod; //转速数据采集间隔（单位：s）
    int RtuPeriod; //Rtu数据轮询间隔（单位：s）
    int UploadPeriod; //数据上传周期（单位：s）
};

struct FREIndexValue
{
    QList<float> HFAcceleration;
    QList<float> LFSquare;
    QList<float> speed;
    QList<float> temp;
    QList<QString> Time;
    int type;
    int chn;
    int count;
};

struct FREWaveValue
{
    int freq;
    int len;
    int maxlen;
    int code;
    QList<int > chn;
    int **data;
    QString time;
};

struct FREUser
{
    QString url;
    QString username;
    QString password;
    QString orgname;
    QString devid;
    int port;
    int ndevid;

    FREUser():
        port(),
        ndevid()
    {
    }

    FREUser(const QString& url, const QString& uname,const QString& password, const QString& orgname,const QString& devid):
        url(url),
        username(uname),
        password(password),
        orgname(orgname),
        devid(devid),
        port(),
        ndevid()
    {
    }
};

#define DMA_MEMCPY_PATH     "/dev/dma_memcpy"

/* commands codes */
#define DMA_MEMCPY_SETUP    0x01000000
#define DMA_MEMCPY_START    0x02000000
#define DMA_MEMCPY_GET_TIME 0x03000000
#define DMA_MEMCPY_SHUTDOWN 0x04000000

struct DMASetupParams {
    unsigned int dev_phy_addr;
    unsigned int dst_phy_addr;
    unsigned int transfer_direction;
    unsigned int transfer_single_size;
    unsigned int transfer_total_size;
};

struct DMATransferInfo {
    float       dma_r_rate;
    float       dma_w_rate;
    float       app_r_rate;
    float       app_w_rate;
    uint32_t    dma_r_cost_time;
    uint32_t    dma_w_cost_time;
    uint32_t    app_r_cost_time;
    uint32_t    app_w_cost_time;
    uint32_t    error_count;
};

struct DmaParams {
    uint32_t    address;
    uint32_t    size;
    uint32_t    cycles;
    char        dev[64];
};

struct DMAInfo {
    int         dma_fd;
    int         input_fd;

    uint16_t    *map_base_addr;
    void    *vir_addr;

    struct DmaParams        params;
    struct DMATransferInfo  dma_trans_info;
};

struct UpdateInfo
{
    QString rkname;
    QString rkmd5code;
    int rksize;
    int rkpkgcount;
    int rkpkgindex;
};

struct HttpUpgradeInfo
{
    QString version;
    QString url;
    QString name;
    QString md5;
    char type;//0:rk 1:mcu 2:sensor
    char updateflag;
};

struct _Params {
    char        dev[64];
    uint32_t    speed;
    uint32_t    size;
    uint32_t    cycle;
    uint32_t    mode;
    uint8_t     bits;
    uint16_t    delay;
    uint32_t    protocol;
};

struct CmdLineParams {
    uint32_t    address;
    uint32_t    size;
    char        dev[64];
};

struct TCPParam {
    int index;
    int wave;
    int longwave;
    int condition;
    pthread_mutex_t mutex;//
    pthread_cond_t cond;//
    QList<int > sn;
    QList<QString> name;
    int count;
    int channel;
    int status;
    bool flag;
    bool changed;
};

struct DataPair {
    int name;
    QVector<int> values;
};

#define MODBUS_MODE_SEND
#define MODBUS_CRC_HIGH
//#define GATE_485_SERVER 1
#define SAVE_FOLDER "data"
#define SAVE_485_FOLDER "uartdata"
#define SAVE_TEST "testdata"
#define FREQ_DIR "/userdata/"
#define MCAST_ADDR "239.1.1.122"
#define CMD_PORT 8888
#define CTL_PORT 10101
#define DMA_BUFFER_SIZE 524288
#define DMA_BLOCK_SIZE 4096//512*8
#define POLYNOMIAL_CRC8 0x8B  // CRC-8 polynomial: x^8 + x^2 + x + 1
#define LOG_PATH "/userdata/etc/sensor_data.json"
#define MAX_BLUETOOTH_LENGTH 150//230

enum CheckResult {
    Success,            // IP 可达，端口开放
    PortFiltered,       // IP 可达，但端口无响应（可能被防火墙过滤）
    IpUnreachable       // IP 不通（包括主机离线、网络故障）
};


#endif // ACQUISITIONTYPE_H
