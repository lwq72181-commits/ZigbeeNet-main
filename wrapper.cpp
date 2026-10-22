#include <QString>
#include <QDebug>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "wrapper.h"
#include "AppConfig.h"
#include "zigbee/spi_manager.h"
#include "JsonFileHandle.h"
#include "Acquisitiontype.h"
#include "ThreadSafeMultiVector.h"
#include "SensorLogManager.h"
#include "AppService.h"
JSONFileHandle m_JSONHandle;
pthread_mutex_t     g_store_mutex=PTHREAD_MUTEX_INITIALIZER;
int set(char *name,int value)
{
    QString data = QString::fromUtf8(name);
    AppConfig::set(data, value);
    return  0;
}

int get(char *name)
{
    int ret = 0;
    QString value = QString::fromUtf8(name);
    AppConfig::get(value, ret);
    qDebug("value:%s,ret:%d",name,ret);
    return ret;
}

int get_string(char *name,char *param)
{
    int ret = 0;
    QString data;
    QString value = QString::fromUtf8(name);
    AppConfig::get(value, data);
    strcpy(param, data.toUtf8().constData());
    qDebug("value:%s,param:%s",name,param);
    return ret;
}

int storeWaveData(char *pwave_buf,unsigned char DataType,char *sn,int wavfilesize)
{
    uint16_t index = 0;
    int sec;
    float freq;
    float convCoe;
    int count= (wavfilesize+2048-1)/2048;
    char date[30] = {0};
    int type;
    int Code = 12;
    int mode;
    QString time;
    QString stime;
    QJsonArray baseArray;
    spi_storage_measdef_index_t * wavhead;
    spi_storage_longwave_index_t * lwavhead;
    printf("count:%d,sn:%s\n",count,sn);
//    for(int i=0;i<255;i++)
//    {
//        printf("%x ",pwave_buf[i]);
//    }
//    printf("\n");
    SensorLogManager * l_logManager  = AppService::logManager();
    if(NORMAL_SYNC_MEA == DataType || NORMAL_ASYNC_MEA == DataType)
    {
        type = 0;
        wavhead = (spi_storage_measdef_index_t*)pwave_buf;
        freq = wavhead->sampleFreq;
        convCoe = wavhead->convCoe;
        Code = 12;
        sprintf(date,"%02d%02d-%02d-%02dT%02d:%02d:%02d",
                wavhead->sampleTime[0],
                wavhead->sampleTime[1],
                wavhead->sampleTime[2],
                wavhead->sampleTime[3],
                wavhead->sampleTime[4],
                wavhead->sampleTime[5],
                wavhead->sampleTime[6]);
        printf("time :%s\n",date);
        time = QString::fromUtf8(date);
        memset(date,0,sizeof(date));
        sprintf(date,"%02d%02d%02d%02d_%02d%02d%02d000",
                wavhead->sampleTime[0],
                wavhead->sampleTime[1],
                wavhead->sampleTime[2],
                wavhead->sampleTime[3],
                wavhead->sampleTime[4],
                wavhead->sampleTime[5],
                wavhead->sampleTime[6]);
        printf("time save file :%s\n",date);
        stime = QString::fromUtf8(date);
    }
    else if(NORMAL_LW == DataType)
    {
        type =1;
        Code = 1;
        lwavhead = (spi_storage_longwave_index_t*)pwave_buf;
        freq = lwavhead->freq;
        convCoe = lwavhead->convCoe;
        sprintf(date,"%02d%02d-%02d-%02dT%02d:%02d:%02d",
                lwavhead->sampleTime[0],
                lwavhead->sampleTime[1],
                lwavhead->sampleTime[2],
                lwavhead->sampleTime[3],
                lwavhead->sampleTime[4],
                lwavhead->sampleTime[5],
                lwavhead->sampleTime[6]);
        time = QString::fromUtf8(date);
        memset(date,0,sizeof(date));
        sprintf(date,"%02d%02d%02d%02d_%02d%02d%02d000",
                lwavhead->sampleTime[0],
                lwavhead->sampleTime[1],
                lwavhead->sampleTime[2],
                lwavhead->sampleTime[3],
                lwavhead->sampleTime[4],
                lwavhead->sampleTime[5],
                lwavhead->sampleTime[6]);
        stime = QString::fromUtf8(date);
    }
    else
        return 1;
    int lsn = atoi(sn);
    for(index = 0;index<count;index++)
    {
        QJsonObject obj;
        obj.insert("Id", lsn);
        obj.insert("PackCount", count);
        obj.insert("PackIndex", index);
        obj.insert("ChNo", lsn);
        obj.insert("Dir", 1);
        obj.insert("Code", Code);
        obj.insert("Time", time);
        obj.insert("PlusLength", 0);
        obj.insert("Freq", freq);
        obj.insert("RPM", 0);//TODO :need to judge
        obj.insert("TimeMark", 0);//TODO :need to judge
        obj.insert("Coef", convCoe);
        obj.insert("Src", 0);
        obj.insert("IsSensor", 1);//TODO :need to judge
        obj.insert("zipType", mode);//0就是32位，1就是16位
        obj.insert("Type", 1);//0就是32位，1就是16位
        QByteArray byteArray;
        if(index!=(count-1))
            byteArray.append(&pwave_buf[2048*index],2048);
        else
        {
            int size = wavfilesize-2048*index;
            printf("========last size:%d\n", size);
            byteArray.append(&pwave_buf[2048*index],size);
        }
        QString content = byteArray.toBase64();
        obj.insert("Content",content);
        baseArray.append(obj);
        byteArray.clear();
    }
    AppConfig::get("Device/update", mode);
    pthread_mutex_lock(&g_store_mutex);
    if((mode>>1)&0x01 == 0x01)
    {
        while(m_JSONHandle.get_partation_percent(FREQ_DIR)<0.1)
            m_JSONHandle.rm_oldest_day_file();
        QString fileName = m_JSONHandle.GetSaveFileNameSTime(type,lsn, stime);
        //QString fileName = m_JSONHandle.GetSaveFileNameP(type, m_sn,m_sec);
        qDebug("save wave file :%s",qPrintable(fileName));
        m_JSONHandle.saveToJsonFile(baseArray, fileName);
        QString result = fileName.section('/', -1);  // -1 表示从右边开始取第一段

        if (!l_logManager->containsWaveInfo(lsn)) {
            qDebug() << "SN" << lsn << "not exists in IndexInfo, adding...";
            if (l_logManager->addWaveInfo(lsn)) {
                qDebug() << "IndexInfo for SN" << lsn << "added successfully";
            } else {
                qWarning() << "Failed to add IndexInfo for SN" << lsn;
            }
            l_logManager->setMaxWaveRecvCount(lsn, 2);
        }
        l_logManager->addWaveRecv(lsn,result);
//        QString fileName1 = m_JSONHandle.GetSaveFileNameTestP(type, m_sn,m_sec);
//        qDebug("save wave file :%s",qPrintable(fileName1));
//        m_JSONHandle.saveToJsonFile(m_baseArray, fileName1);
    }
    pthread_mutex_unlock(&g_store_mutex);
    return 0;
}

int initMVector(int count,ThreadSafeMultiVector * threadVector)
{
    int current_count = threadVector->size("ZHighSquare");
    if(current_count == 0)
    {
        TCPParam * l_Param = AppService::tcpParam();
        pthread_mutex_lock(&l_Param->mutex);
        pthread_cond_signal(&l_Param->cond); // 通知等待线程
        pthread_mutex_unlock(&l_Param->mutex);

        threadVector->addVector("ZHighSquare");
        threadVector->addVector("ZHighSpeed");
        threadVector->addVector("ZLowSquare");
        threadVector->addVector("ZLowSpeed");
        threadVector->addVector("XLowSquare");
        threadVector->addVector("XLowSpeed");
        threadVector->addVector("YLowSquare");
        threadVector->addVector("YLowSpeed");
        threadVector->addVector("Temperature");
        threadVector->addVector("Vlotage");
    }
    for(int i=current_count;i<count;i++)
    {
        threadVector->append("ZHighSquare",0.0);
        threadVector->append("ZHighSpeed",0.0);
        threadVector->append("ZLowSquare",0.0);
        threadVector->append("ZLowSpeed",0.0);
        threadVector->append("XLowSquare",0.0);
        threadVector->append("XLowSpeed",0.0);
        threadVector->append("YLowSquare",0.0);
        threadVector->append("YLowSpeed",0.0);
        threadVector->append("Temperature",0.0);
        threadVector->append("Vlotage",0.0);
    }
    qDebug("count:%d,len:%d",count,threadVector->size("ZHighSquare"));
    qDebug("count:%d,len:%d",count,threadVector->size("ZHighSpeed"));
    qDebug("count:%d,len:%d",count,threadVector->size("ZLowSquare"));
    qDebug("count:%d,len:%d",count,threadVector->size("ZLowSpeed"));
    qDebug("count:%d,len:%d",count,threadVector->size("XLowSquare"));
    qDebug("count:%d,len:%d",count,threadVector->size("XLowSpeed"));
    qDebug("count:%d,len:%d",count,threadVector->size("YLowSquare"));
    qDebug("count:%d,len:%d",count,threadVector->size("YLowSpeed"));
    qDebug("count:%d,len:%d",count,threadVector->size("Temperature"));
    qDebug("count:%d,len:%d",count,threadVector->size("Vlotage"));
    return 0;
}

int storeIndexData(uart_0x34_indicator_data_t* buf, char *lsn, int count, int pos)
{
#if 1
    SensorLogManager * l_logManager  = AppService::logManager();
    ThreadSafeMultiVector * l_threadVector = AppService::threadVector();
    if(l_threadVector->size("ZHighSquare")<count)
    {
        initMVector(count,l_threadVector);
    }
    QJsonArray arry;
    QJsonObject obj;
    QJsonObject objSensorInfo;
    int16_t index = 0;
    uint16_t crc16 = 0;
    uint16_t midi;
    uint8_t date[60] = {0};
    int sn = atoi(lsn);
    obj.insert("ChNo", sn);
    objSensorInfo.insert("SN", sn);
    obj.insert("ChType", 0);
    QJsonArray array;
    QJsonObject HFAContent;
    HFAContent.insert("Dir", 1);
    HFAContent.insert("Code", 112);
    QJsonArray HFAarray;
    float highe;
    memcpy(&highe,&buf->Z_HighFreqAcc.Value,4);
    HFAarray.append(highe);
    HFAContent.insert("Values", HFAarray);
    array.append(HFAContent);

    QJsonObject LFSContent;
    LFSContent.insert("Dir", 1);
    LFSContent.insert("Code", 101);
    QJsonArray LFAarray;
    float lowe;
    memcpy(&lowe,&buf->Z_LowFreqAcc.Value,4);
    LFAarray.append(lowe);
    LFSContent.insert("Values", LFAarray);
    array.append(LFSContent);

    QJsonObject speedContent;
    speedContent.insert("Dir", 1);
    speedContent.insert("Code", 107);
    QJsonArray speedAarray;
    float zspeed;
    memcpy(&zspeed,&buf->Z_LowFreqVel.Value,4);
    speedAarray.append(zspeed);
    speedContent.insert("Values", speedAarray);
    array.append(speedContent);
#if 1
    QJsonObject Rms0To1000;
    Rms0To1000.insert("Dir", 1);
    Rms0To1000.insert("Code", 504);
    QJsonArray Rms0To1000Aarray;
    float rms;
    memcpy(&rms,&buf->Z_HighFreqKurt.Value,4);
    Rms0To1000Aarray.append(rms);
    Rms0To1000.insert("Values", Rms0To1000Aarray);
    array.append(Rms0To1000);

    QJsonObject Peaks10To1000;
    Peaks10To1000.insert("Dir", 1);
    Peaks10To1000.insert("Code", 503);
    QJsonArray Peaks10To1000Aarray;
    float Peaks;
    memcpy(&Peaks,&buf->Z_HighFreq_1.Value,4);
    Peaks10To1000Aarray.append(Peaks);
    Peaks10To1000.insert("Values", Peaks10To1000Aarray);
    array.append(Peaks10To1000);

    QJsonObject  Peaks1000To10000;
    Peaks1000To10000.insert("Dir", 1);
    Peaks1000To10000.insert("Code", 505);
    QJsonArray  Peaks1000To10000Aarray;
    float PeaksH;
    memcpy(&PeaksH,&buf->Z_HighFreq_2.Value,4);
    Peaks1000To10000Aarray.append(PeaksH);
    Peaks1000To10000.insert("Values", Peaks1000To10000Aarray);
    array.append(Peaks1000To10000);

    QJsonObject Kurtosis;
    Kurtosis.insert("Dir", 1);
    Kurtosis.insert("Code", 501);
    QJsonArray KurtosisAarray;
    float Kurt;
    memcpy(&Kurt,&buf->Z_HighFreq_3.Value,4);
    KurtosisAarray.append(Kurt);
    Kurtosis.insert("Values", KurtosisAarray);
    array.append(Kurtosis);

    QJsonObject EnhancePeaks;
    EnhancePeaks.insert("Dir", 1);
    EnhancePeaks.insert("Code", 502);
    QJsonArray EnhancePeaksAarray;
    float ePeaks;
    memcpy(&ePeaks,&buf->Z_HighFreq_4.Value,4);
    EnhancePeaksAarray.append(ePeaks);
    EnhancePeaks.insert("Values", EnhancePeaksAarray);
    array.append(EnhancePeaks);
#endif

    QJsonObject tempContent;
    tempContent.insert("Dir", 1);
    tempContent.insert("Code", 100);
    QJsonArray tempAarray;
    float temp;
    memcpy(&temp,&buf->temperature.Value,4);
    tempAarray.append(temp);
    tempContent.insert("Values", tempAarray);
    array.append(tempContent);
    objSensorInfo.insert("Temp", temp);
    signed char RSSI = buf->Y_LowFreq_2.HaveWave;
    objSensorInfo.insert("RSSI", static_cast<int>(RSSI));
    float sensorinfo;
    memcpy(&sensorinfo,&buf->Y_LowFreq_2.Value,4);
    objSensorInfo.insert("Vol", sensorinfo);
    float sensorinfoCur = 0.01;
    objSensorInfo.insert("Cur", sensorinfoCur);
    QString version = QString::fromUtf8(buf->Version);
    //qDebug("sensor version:%s",version);
    objSensorInfo.insert("AppVer", version);

    QJsonObject xLowContent;
    xLowContent.insert("Dir", 2);
    xLowContent.insert("Code", 101);
    QJsonArray xLowAarray;
    float xLow;
    memcpy(&xLow,&buf->X_LowFreqAcc.Value,4);
    xLowAarray.append(xLow);
    xLowContent.insert("Values", xLowAarray);
    array.append(xLowContent);

    QJsonObject xSpeedContent;
    xSpeedContent.insert("Dir", 2);
    xSpeedContent.insert("Code", 107);
    QJsonArray xSpeedAarray;
    float xSpeed;
    memcpy(&xSpeed,&buf->X_LowFreqVel.Value,4);
    xSpeedAarray.append(xSpeed);
    xSpeedContent.insert("Values", xSpeedAarray);
    array.append(xSpeedContent);

    QJsonObject yLowContent;
    yLowContent.insert("Dir", 3);
    yLowContent.insert("Code", 101);
    QJsonArray yLowAarray;
    float yLow;
    memcpy(&yLow,&buf->Y_LowFreqAcc.Value,4);
    yLowAarray.append(yLow);
    yLowContent.insert("Values", yLowAarray);
    array.append(yLowContent);

    QJsonObject ySpeedContent;
    ySpeedContent.insert("Dir", 3);
    ySpeedContent.insert("Code", 107);
    QJsonArray ySpeedAarray;
    float ySpeed;
    memcpy(&ySpeed,&buf->Y_LowFreqVel.Value,4);
    ySpeedAarray.append(ySpeed);
    ySpeedContent.insert("Values", ySpeedAarray);
    array.append(ySpeedContent);

    obj.insert("Content", array);

//    QDateTime currentTime = QDateTime::currentDateTimeUtc();
//    QString isoFormat = currentTime.toString("yyyy-MM-dd'T'hh:mm:ss");
    QJsonArray timearray;
    int sec;
    memcpy(&sec,&buf->SampleTime,4);
    sec+=1325347200;
    QDateTime dateTimeFromSec = QDateTime::fromSecsSinceEpoch(sec);
    QString isoFormat = dateTimeFromSec.toString("yyyy-MM-dd'T'hh:mm:ss");

    timearray.append(isoFormat);
    obj.insert("Time", timearray);
    objSensorInfo.insert("Clock", isoFormat);
    arry.append(obj);
    arry.append(objSensorInfo);
    int mode =0;
    AppConfig::get("Device/update", mode);
    pthread_mutex_lock(&g_store_mutex);
    if((mode>>1)&0x01 == 0x01)
    {
        while(m_JSONHandle.get_partation_percent(FREQ_DIR)<0.1)
            m_JSONHandle.rm_oldest_day_file();
        QString fileName = m_JSONHandle.GetSaveFileNameP(0,sn,sec);
        qDebug("save index file :%s",qPrintable(fileName));
        m_JSONHandle.saveToJsonFile(arry, fileName);
        QString result = fileName.section('/', -1);  // -1 表示从右边开始取第一段
        qDebug()<<"insert index info;"<<result<<"sn"<<sn;
        if (!l_logManager->containsIndexInfo(sn)) {
            qDebug() << "SN" << sn << "not exists in IndexInfo, adding...";
            if (l_logManager->addIndexInfo(sn)) {
                qDebug() << "IndexInfo for SN" << sn << "added successfully";
            } else {
                qWarning() << "Failed to add IndexInfo for SN" << sn;
            }
            l_logManager->setMaxIndexRecvCount(sn, 2);
        }
        l_logManager->addIndexRecv(sn,result);
//        QString fileName1 = m_JSONHandle.GetSaveFileNameTestP(0,m_sn,m_sec);
//        qDebug("save index file :%s",qPrintable(fileName1));
//        m_JSONHandle.saveToJsonFile(arry, fileName1);
    }
    pthread_mutex_unlock(&g_store_mutex);
#endif
    l_threadVector->writeElement("ZHighSquare", pos, highe);
    l_threadVector->writeElement("ZHighSpeed", pos, zspeed);
    l_threadVector->writeElement("ZLowSquare", pos, lowe);
    l_threadVector->writeElement("ZLowSpeed", pos, zspeed);
    l_threadVector->writeElement("XLowSquare", pos, xLow);
    l_threadVector->writeElement("XLowSpeed", pos, xSpeed);
    l_threadVector->writeElement("YLowSquare", pos, yLow);
    l_threadVector->writeElement("YLowSpeed", pos, ySpeed);
    l_threadVector->writeElement("Temperature", pos, temp);
    l_threadVector->writeElement("Vlotage", pos, sensorinfo);
    return 0;
}
