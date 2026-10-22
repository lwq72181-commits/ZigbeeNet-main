#include <QThread>
#include <math.h>
#include <QJsonObject>
#include <QFile>
#include "mqtthandle.h"
#include "Acquisitiontype.h"
#include "AppService.h"
#include "AppConfig.h"
uint crcTable[] =
 {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
    0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
     0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
     0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
     0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
     0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
     0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
     0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
     0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
     0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
     0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
     0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
     0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
     0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
     0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
     0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
     0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
     0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
     0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
     0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
     0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
     0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
     0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
     0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
     0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
     0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
     0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
     0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
     0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
     0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
     0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
     0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
     0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
     0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
     0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
     0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
     0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
     0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
     0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
     0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
     0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
     0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
     0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
     0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
     0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
     0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
     0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
     0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
     0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
     0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
     0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
     0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
     0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
     0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
     0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
     0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
     0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
     0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
 };
mqtthandle::mqtthandle()
{
    m_highwave.maxlen = 0;
    m_highwave.data = NULL;
    m_lowwave.maxlen = 0;
    m_lowwave.data = NULL;
    m_sendwflag = 0;
    m_sendlflag = 0;
}

int mqtthandle::divideCeilToInt(int numerator, int denominator) {
    return (int)ceil((double)numerator / denominator);
}

bool mqtthandle::sendMqMessage(QMqttClient* mqttClient,QString& topic, QJsonObject* data)
{
    QJsonDocument doc;
    QByteArray ba = doc.toJson();
    qDebug()<<ba;
    qDebug("\nsend data!! mqtt\n");
    qint32 ret = mqttClient->publish(QMqttTopicName(topic), ba);
    return ret >= 0;
}

int mqtthandle::sendMqttIndexData(QMqttClient* mqttClient,FreqCollectPara* localColect, FRESysParam* localPara, qint64 *startsec)
{
    QJsonArray arry;
    QJsonObject obj;
    int ret = 0;
    ret = sendStoredIndex(localColect,localPara,startsec);
    return ret;
    int i=0;
    qDebug("it size:%d",m_IndexList.length());
    localColect->reupload.clear();
    for (auto it = m_IndexList.rbegin(); it != m_IndexList.rend(); ++it)
    {
        obj.insert("ChNo", it->chn);
        obj.insert("ChType", it->type);
        QJsonArray array;
        QJsonObject HFAContent;
        HFAContent.insert("Dir", 1);
        HFAContent.insert("Code", 1);
        QJsonArray HFAarray;
        for(auto ith = it->HFAcceleration.rbegin(); ith != it->HFAcceleration.rend();++ith)
        {
            HFAarray.append(*ith);
        }
        HFAContent.insert("Values", HFAarray);
        array.append(HFAContent);

        QJsonObject LFSContent;
        LFSContent.insert("Dir", 1);
        LFSContent.insert("Code", 1);
        QJsonArray LFAarray;
        for(auto itl = it->LFSquare.rbegin(); itl != it->LFSquare.rend();++itl)
        {
            LFAarray.append(*itl);
        }
        LFSContent.insert("Values", LFAarray);
        array.append(LFSContent);

        QJsonObject speedContent;
        speedContent.insert("Dir", 1);
        speedContent.insert("Code", 1);
        QJsonArray speedAarray;
        for(auto its = it->speed.rbegin(); its != it->speed.rend();++its)
        {
            speedAarray.append(*its);
        }
        speedContent.insert("Values", speedAarray);
        array.append(speedContent);

        QJsonObject tempContent;
        tempContent.insert("Dir", 1);
        tempContent.insert("Code", 1);
        QJsonArray tempAarray;
        for(auto itt = it->temp.rbegin(); itt != it->temp.rend();++itt)
        {
            tempAarray.append(*itt);
        }
        tempContent.insert("Values", tempAarray);
        array.append(tempContent);

        obj.insert("Content", array);

        QJsonArray timearray;
        for(auto itm = it->Time.rbegin(); itm != it->Time.rend();++itm)
        {
            timearray.append(*itm);
        }
        obj.insert("Time", timearray);

    }
    arry.append(obj);
    QString topic;
    topic = QString("%1/IndexData/%2").arg(m_uploadTopic).arg(localColect->messageid++);
    qDebug("send topic:%s\n",topic.toUtf8().constData());
//    sendMqMessage(mqttClient, topic, &obj);

    do
    {
        if(localColect->mqttopen)
        {
            emit sendsig(topic, arry, 1);
            break;
        }
        else
        {
            QThread::sleep(1);
            i++;
            if(i>5)
                break;
        }

    }while(1);
    localColect->condition = 0;
    if(waiting_function(5,localColect)==0)
    {
        if(localColect->condition == 1)
        {
            m_IndexList.clear();
        }
    }
//    if(sendMqMessage(mqttClient, topic, &obj))
//    {
//        if(waiting_function(5,localColect)==0)
//        {
//            if(localColect->condition == 1)
//            {
//                m_IndexList.clear();
//            }
//        }
//        //m_IndexList.clear();
//    }
    return 0;
}

uint mqtthandle::GetCRC32(QString bytes)
{
    uint crc = 0xFFFFFFFF;
    QByteArray byteArray = bytes.toUtf8();

    for (char byte : byteArray) {
        crc = crcTable[(crc ^ static_cast<quint8>(byte)) & 0xFF] ^ (crc >> 8);
    }

//    for (uint i = 0; i < iCount; i++)
//    {
//        //crc = (crc << 8) ^ crcTable[(crc >> 24) ^ bytes[i]];
//        crc = crcTable[(crc ^ bytes[i]) & 0xFF] ^ (crc >> 8);
//    }
    crc = crc ^ 0xFFFFFFFF;
    return crc;
}

// 计算均方根的函数
double mqtthandle::calculateRMS(double *data, int n) {
    double sumOfSquares = 0.0; // 用于累加平方和
    for(int i = 0; i < n; i++) {
        sumOfSquares += pow(data[i], 2); // 计算每个数值的平方并累加
    }
    double meanOfSquares = sumOfSquares / n; // 计算平方的平均值
    double rmsValue = sqrt(meanOfSquares); // 对平均值开平方根得到RMS

    return rmsValue;
}

// 函数将double指针转换为Base64编码的字符串
QString doubleArrayToBase64(double *doubleArray, int length)
{
    QByteArray byteArray; // 用于存储转换后的字节流

    // 将double数组转换为字节流
    for (int i = 0; i < length; ++i) {
        // 注意：这里直接使用reinterpret_cast将double指针转为uchar指针进行拷贝，假设系统是小端字节序
        // 对于大端字节序系统或其他特殊要求，需手动进行字节序调整
        byteArray.append(reinterpret_cast<const char*>(&doubleArray[i]), sizeof(double));
    }
    // 将字节流转换为Base64编码
    return byteArray.toBase64();
}

// 函数将int数组转换为Base64编码的字符串
QString mqtthandle::intArrayToBase64(int* intArray, int len)
{
    // 将int数组转换为QByteArray
    QByteArray byteArray;
    for(int i=0; i<len; i++)
    {
        byteArray.append(static_cast<char>(intArray[i] & 0xFF));
        byteArray.append(static_cast<char>((intArray[i] >> 8) & 0xFF));
        byteArray.append(static_cast<char>((intArray[i] >> 16) & 0xFF));
    }
    // 将QByteArray转换为Base64编码的字符串
    return byteArray.toBase64();
}

int mqtthandle::sendMqttERRORNO(FreqCollectPara* localColect,int errcode)
{
    QString topic;
    topic = QString("%1/ExceptionInfo/%2").arg(m_uploadTopic).arg(localColect->messageid);
    qDebug("send topic:%s\n",topic.toUtf8().constData());

    QJsonObject obj;
    obj.insert("Info", "zigbee communication error");
    obj.insert("Code", errcode);
    emit sendsig(topic, obj, 0);
    return 0;
}

int mqtthandle::sendStoredIndex(FreqCollectPara* localColect, FRESysParam* localPara, qint64 *startsec)
{
    QJsonObject obj;
    QJsonArray arry;
    QJsonArray indexArray;
    QJsonArray infoArray;
    QString fileName;
    bool ok;
    QString sn;
    int count = 0;
    int flag = 0;
    int ret = 0;
#if 0 //test upload
    char *cmd = "cp /userdata/wave/* /userdata/data";///data
    system(cmd);
    QThread::sleep(1);
#endif
    SensorLogManager * l_logManager  = AppService::logManager();
    while(m_JSONHandle.GetOldestFileName(fileName, 0)==0)
    {
        qDebug("get stored wave file name:%s",qPrintable(fileName));
        //TODO: if TIME diff less than 2s not upload
//        QDateTime currentDateTime = QDateTime::currentDateTime();
//        QStringList partsurl = fileName.split("A", Qt::SkipEmptyParts);
//         fileName = currentDateTime.toString("yyyyMMdd_hhmmsszzz'C'");
        arry = QJsonArray();
        indexArray = QJsonArray();
        infoArray = QJsonArray();
        arry = m_JSONHandle.loadArrayFromJsonFile(fileName);
        if(arry.isEmpty())
        {
            qDebug("file formate error need to delete");
            sn = fileName.section('A', -1);  // -1 表示从右边开始取第一段
            int lsn = sn.toInt(&ok);
            if (!l_logManager->containsIndexInfo(lsn)) {
                qDebug() << "SN" << lsn << "not exists in IndexInfo, adding...";
                if (l_logManager->addIndexInfo(lsn)) {
                    qDebug() << "IndexInfo for SN" << lsn << "added successfully";
                } else {
                    qWarning() << "Failed to add IndexInfo for SN" << lsn;
                }
                l_logManager->setMaxIndexSendCount(lsn, 5);
            }
            QString result = fileName.section('/', -1);
            l_logManager->addIndexSend(lsn,result);
            m_JSONHandle.deleteFile(fileName);
            continue;
        }
        ret++;
        for (int i = 0; i < arry.count(); i++)
        {
            QJsonValue value = arry.at(i);
            if(value.isObject())
            {
                //const QJsonObject &obj = value.toObject();
                obj = value.toObject();
                if(i==0)
                {
                    indexArray.append(obj);
                    localColect->reupload.clear();
                    localColect->condition = 0;
                    QString topic;
                    topic = QString("%1/IndexData/%2").arg(m_uploadTopic).arg(localColect->messageid);
                    qDebug("send topic:%s\n",topic.toUtf8().constData());
                    count = 0;
                    do
                    {
                        if(localColect->mqttopen)
                        {
                            //emit sendsig(topic, obj,0);
                            emit sendsig(topic, indexArray, 1);
                            break;
                        }
                        else
                        {
                            QThread::sleep(1);
                            count++;
                            if(count>15)
                            {
                                count = 0;
                                flag = 1;
                                break;
                            }
                        }

                    }while(1);
                    if(flag==1)
                        break;

                    if(waiting_function(15,localColect)==0)
                    {
                        if((localColect->topicStatus==0||localColect->topicStatus==1))
                        {
                            localColect->messageid++;
                            //m_JSONHandle.deleteFile(fileName);
                        }
                        else
                        {
                            qDebug("status:%d",localColect->topicStatus);
                            sn = fileName.section('A', -1);  // -1 表示从右边开始取第一段
                            int lsn = sn.toInt(&ok);
                            if (!l_logManager->containsIndexInfo(lsn)) {
                                qDebug() << "SN" << lsn << "not exists in IndexInfo, adding...";
                                if (l_logManager->addIndexInfo(lsn)) {
                                    qDebug() << "IndexInfo for SN" << lsn << "added successfully";
                                } else {
                                    qWarning() << "Failed to add IndexInfo for SN" << lsn;
                                }
                                l_logManager->setMaxIndexSendCount(lsn, 5);
                            }
                            QString result = fileName.section('/', -1);
                            l_logManager->addIndexSend(lsn,result);
                            m_JSONHandle.deleteFile(fileName);
                            break;
                        }
                    }
                    else
                    {
                        localColect->messageid++;
                        flag = 1;
                        break;
                    }
                }
                else
                {
                    infoArray.append(obj);
                    localColect->reupload.clear();
                    localColect->condition = 0;
                    QString topic;
                    topic = QString("%1/SensorInfo/%2").arg(m_uploadTopic).arg(localColect->messageid);
                    qDebug("send topic:%s\n",topic.toUtf8().constData());
                    count = 0;
                    do
                    {
                        if(localColect->mqttopen)
                        {
                            //emit sendsig(topic, obj,0);
                            emit sendsig(topic, infoArray, 1);
                            break;
                        }
                        else
                        {
                            QThread::sleep(1);
                            count++;
                            if(count>15)
                            {
                                count = 0;
                                flag = 1;
                                break;
                            }
                        }

                    }while(1);


                    if(waiting_function(15,localColect)==0)
                    {
                        if((localColect->topicStatus==0||localColect->topicStatus==1))
                        {
                            localColect->messageid++;
                            sn = fileName.section('A', -1);  // -1 表示从右边开始取第一段
                            int lsn = sn.toInt(&ok);
                            if (!l_logManager->containsIndexInfo(lsn)) {
                                qDebug() << "SN" << lsn << "not exists in IndexInfo, adding...";
                                if (l_logManager->addIndexInfo(lsn)) {
                                    qDebug() << "IndexInfo for SN" << lsn << "added successfully";
                                } else {
                                    qWarning() << "Failed to add IndexInfo for SN" << lsn;
                                }
                                l_logManager->setMaxIndexSendCount(lsn, 5);
                            }
                            QString result = fileName.section('/', -1);
                            l_logManager->addIndexSend(lsn,result);
                            m_JSONHandle.deleteFile(fileName);
                        }
                        else
                        {
                            qDebug("status:%d",localColect->topicStatus);
                            sn = fileName.section('A', -1);  // -1 表示从右边开始取第一段
                            int lsn = sn.toInt(&ok);
                            if (!l_logManager->containsIndexInfo(lsn)) {
                                qDebug() << "SN" << lsn << "not exists in IndexInfo, adding...";
                                if (l_logManager->addIndexInfo(lsn)) {
                                    qDebug() << "IndexInfo for SN" << lsn << "added successfully";
                                } else {
                                    qWarning() << "Failed to add IndexInfo for SN" << lsn;
                                }
                                l_logManager->setMaxIndexSendCount(lsn, 5);
                            }
                            QString result = fileName.section('/', -1);
                            l_logManager->addIndexSend(lsn,result);
                            m_JSONHandle.deleteFile(fileName);
                        }
                    }
                    else
                    {
                        localColect->messageid++;
                        flag = 1;
                        break;
                    }
                }
            }
            else
            {
                qDebug("file formate error need to delete");
                sn = fileName.section('A', -1);  // -1 表示从右边开始取第一段
                int lsn = sn.toInt(&ok);
                if (!l_logManager->containsIndexInfo(lsn)) {
                    qDebug() << "SN" << lsn << "not exists in IndexInfo, adding...";
                    if (l_logManager->addIndexInfo(lsn)) {
                        qDebug() << "IndexInfo for SN" << lsn << "added successfully";
                    } else {
                        qWarning() << "Failed to add IndexInfo for SN" << lsn;
                    }
                    l_logManager->setMaxIndexSendCount(lsn, 5);
                }
                QString result = fileName.section('/', -1);
                l_logManager->addIndexSend(lsn,result);
                m_JSONHandle.deleteFile(fileName);
                break;
            }
        }
        if(flag == 1)
        {
            ret = -1;
            break;
        }
    }
    return ret;
}

#if 0
int mqtthandle::sendStoredWave(FreqCollectPara* localColect, FRESysParam* localPara, qint64 *startsec, int flag)
{
    QJsonObject obj;
    QJsonArray baseArray;
    QString fileName;
    int ret = 0;
    int count = 0;
    int timeout = 0;
    int select = 0;
    if(flag == 1)
        select = 2;
    else
        select = 1;

#if 1
    while(m_JSONHandle.GetOldestFileName(fileName, select)==0)
    {
        qDebug("get stored wave file name:%s",qPrintable(fileName));
        baseArray = m_JSONHandle.loadArrayFromJsonFile(fileName);
        if(baseArray.isEmpty())
        {
            qDebug("file formate error need to delete");
            m_JSONHandle.deleteFile(fileName);
            continue;
        }
        ret++;
        topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);
        qDebug("send topic:%s\n",topic.toUtf8().constData());
        localColect->reupload.clear();
        localColect->waverecved = 0;
        for (int i = 0; i < baseArray.count(); i++)
        {
            pthread_mutex_lock(&localColect->mutex);
            localColect->condition = 0;
            pthread_mutex_unlock(&localColect->mutex);

            QJsonValue value = baseArray.at(i);
            if(value.isObject())
            {
                //const QJsonObject &obj = value.toObject();
                obj = value.toObject();
                if(obj.contains("Id"))
                {
                    obj.insert("Id", localColect->messageid);
                    //obj["Id"] = localColect->messageid;
                }
                count = 0;
                do
                {
//                    qint64 nowsec = QDateTime::currentDateTime().toSecsSinceEpoch();
//                    if(((nowsec-startsec[0]) >= localPara->IndexPeriod)||
//                        ((nowsec-startsec[1]) >= localPara->WavePeriod)||
//                        ((nowsec-startsec[2]) >= localPara->LongWavePeriod))
//                    {
//                        break;
//                    }
                    if(localColect->mqttopen)
                    {
                        if(i==baseArray.count()-1)
                            QThread::usleep(100000);
                        emit sendsig(topic, obj,0);
                        break;
                    }
                    else
                    {
                        QThread::sleep(1);
                        count++;
                        if(count>15)
                        {
                            count = 0;
                            return -1;
                            //timeout = 1;
                            //break;
                        }
                    }

                }while(1);
            }
        }
        do
        {
            if(localColect->reupload.size()>0)
            {
                qDebug("left size:%d upload data!\n",localColect->reupload.size());
                localColect->waverecved=0;
                for(int k=0; k<localColect->reupload.size();k++)
                {
                    QJsonValue value = baseArray.at(localColect->reupload[k]);
                    if(value.isObject())
                    {
                        //const QJsonObject &obj = value.toObject();
                        obj = value.toObject();
                        if(obj.contains("Id"))
                        {
                            obj.insert("Id", localColect->messageid);
                            //obj["Id"] = localColect->messageid;
                        }
                    }
                    qDebug("send topic:%s\n",topic.toUtf8().constData());
                    pthread_mutex_lock(&localColect->mutex);
                    localColect->condition = 0;
                    pthread_mutex_unlock(&localColect->mutex);
                    count = 0;
                    do
                    {
                        if(localColect->mqttopen)
                        {
                            emit sendsig(topic, obj,0);
                            break;
                        }
                        else
                        {
                            QThread::sleep(1);
                            count++;
                            if(count>15)
                            {
                                count = 0;
                                return -1;
//                                timeout = 1;
//                                break;
                            }
                        }
                    }while(1);
                }
                localColect->reupload.clear();
            }
            if(waiting_function(30,localColect)==0)
            {
                if((localColect->topicStatus !=0 && localColect->topicStatus!=1))
                {
                    qDebug("status:%d",localColect->topicStatus);
                    m_JSONHandle.deleteFile(fileName);
                    break;
                }

                if(localColect->reupload.size()==0 && localColect->waverecved==1)
                {
                    localColect->messageid++;
                    m_JSONHandle.deleteFile(fileName);
                    break;
                }
                else if(localColect->reupload.size()>0)
                    continue;
                else
                    break;
            }
            else
            {
                qDebug("time out recv ask!!!");
                localColect->messageid++;
                return -1;
//                timeout = 1;
//                break;
            }
        }while(1);
        if(timeout == 1)
            break;
    }
#endif
    qDebug("send stored file over!!");
    return ret;
}
#endif

int mqtthandle::sendStoredWave(FreqCollectPara* localColect, FRESysParam* localPara, qint64 *startsec, int flag)
{
    QJsonObject obj;
    QJsonArray baseArray;
    QString fileName;
    int ret = 0;
    int count = 0;
    QString sn;
    bool ok;
    int timeout = 0;
    int select = 0;
    if(flag == 1)
        select = 2;
    else
        select = 1;
#if 0 //test upload
    char *cmd = "cp /userdata/wave/* /userdata/data";
    system(cmd);
    QThread::sleep(1);
#endif
#if 1
    SensorLogManager * l_logManager  = AppService::logManager();
    while(m_JSONHandle.GetOldestFileName(fileName, select)==0)
    {
        if(select == 1)
        {
            if(m_sendwflag==1)
            {
                fileName = m_waveName;
            }
            else
            {
                m_waveName = fileName;
            }
        }
        else if(select == 2)
        {
            if(m_sendlflag==1)
            {
                fileName = m_LwaveName;
            }
            else
            {
                m_LwaveName = fileName;
            }
        }
        qDebug("get stored wave file name:%s",qPrintable(fileName));
        baseArray = m_JSONHandle.loadArrayFromJsonFile(fileName);
        if(baseArray.isEmpty())
        {
            qDebug("file formate error need to delete");
            if(select == 1)
            {
                m_sendwflag = 0;
                sn = fileName.section('B', -1);  // -1 表示从右边开始取第一段
            }
            else if(select == 2)
            {
                sn = fileName.section('C', -1);  // -1 表示从右边开始取第一段
                m_sendlflag = 0;
            }
            int lsn = sn.toInt(&ok);
            if (!l_logManager->containsWaveInfo(lsn)) {
                qDebug() << "SN" << lsn << "not exists in WaveInfo, adding...";
                if (l_logManager->addWaveInfo(lsn)) {
                    qDebug() << "WaveInfo for SN" << lsn << "added successfully";
                } else {
                    qWarning() << "Failed to add WaveInfo for SN" << lsn;
                }
                l_logManager->setMaxWaveSendCount(lsn, 5);
            }
            QString result = fileName.section('/', -1);
            l_logManager->addWaveSend(lsn,result);
            m_JSONHandle.deleteFile(fileName);

            continue;
        }
        ret++;
        localColect->reupload.clear();
        localColect->waverecved = 0;

        QString topic;
        topic = QString("%1/IsWaveDataResume/%2").arg(m_uploadTopic).arg(localColect->messageid);
        qDebug("send topic:%s\n",topic.toUtf8().constData());
        QJsonObject fileinfo;
        QString result = fileName.section('/', -1);  // -1 表示从右边开始取第一段
        fileinfo.insert("WaveName", result);
        QJsonValue nameobj = fileinfo.value("WaveName");
        do
        {
            if(localColect->mqttopen)
            {
                pthread_mutex_lock(&localColect->mutex);
                localColect->condition = 0;
                pthread_mutex_unlock(&localColect->mutex);
                emit sendsig(topic, fileinfo,0);
                qDebug("send date:%s\n",nameobj.toString().toUtf8().constData());
                if(waiting_function(30,localColect)==0)
                    break;
                else
                    timeout =1;
                //mqtt->sendMqMessage(topic, obj,0);
                break;
            }
            else
            {
                QThread::sleep(1);
                count++;
                if(count>15)
                {
                    count = 0;
                    timeout =1;
                    break;
                }
            }

        }while(1);
        if(timeout == 1)
        {
            if(select == 1)
                m_sendwflag = 1;
            else if(select == 2)
                m_sendlflag = 1;
            break;
        }
        qDebug("reupload.size:%d,isresume:%d\n",localColect->reupload.size(),localColect->IsResume);
        if(localColect->reupload.size()==0&&localColect->IsResume)
        {
            localColect->messageid++;
            if(select == 1)
            {
                m_sendwflag = 0;
                sn = fileName.section('B', -1);  // -1 表示从右边开始取第一段
            }
            else if(select == 2)
            {
                sn = fileName.section('C', -1);  // -1 表示从右边开始取第一段
                m_sendlflag = 0;
            }
            int lsn = sn.toInt(&ok);
            if (!l_logManager->containsWaveInfo(lsn)) {
                qDebug() << "SN" << lsn << "not exists in WaveInfo, adding...";
                if (l_logManager->addWaveInfo(lsn)) {
                    qDebug() << "WaveInfo for SN" << lsn << "added successfully";
                } else {
                    qWarning() << "Failed to add WaveInfo for SN" << lsn;
                }
                l_logManager->setMaxWaveSendCount(lsn, 5);
            }
            QString result = fileName.section('/', -1);
            l_logManager->addWaveSend(lsn,result);
            m_JSONHandle.deleteFile(fileName);
            continue;
        }

        if(localColect->reupload.size()==0&&(!localColect->IsResume))
        {
            topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);
            qDebug("send topic:%s\n",topic.toUtf8().constData());
            for (int i = 0; i < baseArray.count(); i++)
            {
                pthread_mutex_lock(&localColect->mutex);
                localColect->condition = 0;
                pthread_mutex_unlock(&localColect->mutex);

                QJsonValue value = baseArray.at(i);
                if(value.isObject())
                {
                    //const QJsonObject &obj = value.toObject();
                    obj = value.toObject();
                    if(obj.contains("Id"))
                    {
                        obj.insert("Id", localColect->messageid);
                        //obj["Id"] = localColect->messageid;
                    }
                    count = 0;
                    do
                    {
                        if(localColect->mqttopen)
                        {
                            if(i==baseArray.count()-1)
                                QThread::usleep(100000);
                            localColect->conditionsend = 0;
                            emit sendsig(topic, obj,0);
//                            if(waiting_functionsend(120,localColect)==0)
//                                break;
//                            else
//                                timeout =1;
                            //mqtt->sendMqMessage(topic, obj,0);
                            break;
                        }
                        else
                        {
                            QThread::sleep(1);
                            count++;
                            if(count>15)
                            {
                                count = 0;
                                //return -1;
                                timeout = 1;
                                break;
                            }
                        }

                    }while(1);
                }
                if(timeout ==1 )
                    break;
            }
        }
        if(timeout == 1 )
        {
            if(select == 1)
                m_sendwflag = 1;
            else if(select == 2)
                m_sendlflag = 1;
            break;
        }
        topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);
        do
        {
            if(localColect->reupload.size()>0)
            {
                qDebug("left size:%d upload data!\n",localColect->reupload.size());
                localColect->waverecved=0;
                for(int k=0; k<localColect->reupload.size();k++)
                {
                    QJsonValue value = baseArray.at(localColect->reupload[k]);
                    if(value.isObject())
                    {
                        //const QJsonObject &obj = value.toObject();
                        obj = value.toObject();
                        if(obj.contains("Id"))
                        {
                            obj.insert("Id", localColect->messageid);
                            //obj["Id"] = localColect->messageid;
                        }
                    }
                    //qDebug("send topic:%s\n",topic.toUtf8().constData());
                    pthread_mutex_lock(&localColect->mutex);
                    localColect->condition = 0;
                    pthread_mutex_unlock(&localColect->mutex);
                    count = 0;
                    do
                    {
                        if(localColect->mqttopen)
                        {
                            emit sendsig(topic, obj,0);
                            break;
                        }
                        else
                        {
                            QThread::sleep(1);
                            count++;
                            if(count>15)
                            {
                                count = 0;
                                //return -1;
                                timeout = 1;
                                break;
                            }
                        }
                    }while(1);
                }
                localColect->reupload.clear();
            }
            if(timeout == 1 )
            {
                if(select == 1)
                    m_sendwflag = 1;
                else if(select == 2)
                    m_sendlflag = 1;
                break;
            }
            if(waiting_function(30,localColect)==0)
            {
                if(localColect->reupload.size()==0 && localColect->waverecved==1)
                {
                    localColect->messageid++;
                    if(select == 1)
                    {
                        m_sendwflag = 0;
                        sn = fileName.section('B', -1);  // -1 表示从右边开始取第一段
                    }
                    else if(select == 2)
                    {
                        sn = fileName.section('C', -1);  // -1 表示从右边开始取第一段
                        m_sendlflag = 0;
                    }
                    int lsn = sn.toInt(&ok);
                    if (!l_logManager->containsWaveInfo(lsn)) {
                        qDebug() << "SN" << lsn << "not exists in WaveInfo, adding...";
                        if (l_logManager->addWaveInfo(lsn)) {
                            qDebug() << "WaveInfo for SN" << lsn << "added successfully";
                        } else {
                            qWarning() << "Failed to add WaveInfo for SN" << lsn;
                        }
                        l_logManager->setMaxWaveSendCount(lsn, 5);
                    }
                    QString result = fileName.section('/', -1);
                    l_logManager->addWaveSend(lsn,result);
                    m_JSONHandle.deleteFile(fileName);
                    break;
                }
                else if(localColect->reupload.size()>0)
                    continue;
                else if(localColect->topicStatus == -3)
                {
                    localColect->messageid++;
                    if(select == 1)
                    {
                        m_sendwflag = 0;
                        sn = fileName.section('B', -1);  // -1 表示从右边开始取第一段
                    }
                    else if(select == 2)
                    {
                        sn = fileName.section('C', -1);  // -1 表示从右边开始取第一段
                        m_sendlflag = 0;
                    }
                    int lsn = sn.toInt(&ok);
                    if (!l_logManager->containsWaveInfo(lsn)) {
                        qDebug() << "SN" << lsn << "not exists in WaveInfo, adding...";
                        if (l_logManager->addWaveInfo(lsn)) {
                            qDebug() << "WaveInfo for SN" << lsn << "added successfully";
                        } else {
                            qWarning() << "Failed to add WaveInfo for SN" << lsn;
                        }
                        l_logManager->setMaxWaveSendCount(lsn, 5);
                    }
                    QString result = fileName.section('/', -1);
                    l_logManager->addWaveSend(lsn,result);
                    m_JSONHandle.deleteFile(fileName);
                    break;
                }
                else
                    break;
            }
            else
            {
                qDebug("time out recv ask!!!");
                localColect->messageid++;
//                return -1;
                timeout = 1;
                break;
            }
        }while(1);
        if(timeout == 1)
        {
                 if(select == 1)
                     m_sendwflag = 1;
                 else if(select == 2)
                     m_sendlflag = 1;
                 break;
        }
    }
#endif
    qDebug("send stored file over!!");
    if(timeout == 1)
        ret = -1;
    return ret;
}

int mqtthandle::sendMqttWaveData(QMqttClient* mqttClient,FreqCollectPara* localColect, FRESysParam* localPara, int flag, qint64 *startsec)
{
    QJsonObject obj;
    QJsonArray baseArray;
    int index = 0;
    FREWaveValue * lwaveP;
    int i,count;
    int ret = 0;
    uint crc;
    int j=0,change = 0,saved = 0;
//    qint64 nowsec = QDateTime::currentDateTime().toSecsSinceEpoch();
//    if((nowsec-startsec[0]) >= localPara->IndexPeriod)

    if(flag==0)
    {
        lwaveP = &m_highwave;
    }
    else
    {
        lwaveP = &m_lowwave;
    }
    qDebug("a");
    qDebug("chn count:%d\n",lwaveP->chn.count());
    ret = sendStoredWave(localColect,localPara,startsec, lwaveP->code);
    return ret;
    qDebug("chn count:%d\n",lwaveP->chn.count());

    for(i=0;i<lwaveP->chn.count();i++)
    {
        localColect->reupload.clear();
        count = divideCeilToInt(lwaveP->len*4,localColect->sendlen);
        qDebug("len:%d,sendlen:%d,count:%d\n",lwaveP->len,localColect->sendlen,count);
        crc = 0;
            obj = QJsonObject();
            int len = lwaveP->len*4;
            QString upstr = intArrayToBase64(lwaveP->data[i], lwaveP->len);

           // if(localColect->reupload.size()==0)
            {
                qDebug("no left upload data!\n");
                for (int k = 0; k < len; k += localColect->sendlen) {
                    QString segment = upstr.mid(k, localColect->sendlen);
                    obj.insert("Id", lwaveP->chn[i]);
                    obj.insert("PackCount", count);
                    obj.insert("PackIndex", index++);
                    obj.insert("ChNo", lwaveP->chn[i]);
                    obj.insert("Dir", 1);
                    obj.insert("Code", lwaveP->code);
                    obj.insert("Time", lwaveP->time);
                    obj.insert("PlusLength", 0);
                    obj.insert("Freq", lwaveP->freq);
                    obj.insert("RPM", 0);//TODO :need to judge
                    obj.insert("TimeMark", 0);//TODO :need to judge
                    obj.insert("Coef", 0.000149200001033023);//TODO :need to judge
                    obj.insert("Src", 1);
                    obj.insert("IsSensor", 1);//TODO :need to judge
                    if((k+localColect->sendlen) < len)
                        obj.insert("CRC", QJsonValue::Null);
                    else
                    {
                        crc = GetCRC32(upstr);
//                        for(int l=0; l<lwaveP->len; l++)
//                            crc^=lwaveP->data[i][l];
                        obj.insert("CRC", static_cast<double>(crc));
                    }
                    obj.insert("Content", segment);
                    QString topic;
                    topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);


                    pthread_mutex_lock(&localColect->mutex);
                    localColect->condition = 0;

                    pthread_mutex_unlock(&localColect->mutex);
                    baseArray.append(obj);
                    j = 0;
                    do
                    {
//                        qint64 nowsec = QDateTime::currentDateTime().toSecsSinceEpoch();
//                        if(((nowsec-startsec[0]) >= localPara->IndexPeriod)||
//                            ((nowsec-startsec[1]) >= localPara->WavePeriod)||
//                            ((nowsec-startsec[2]) >= localPara->LongWavePeriod))
//                        {
//                            if((k+localColect->sendlen) >= len)
//                            {
//                                while(m_JSONHandle.get_partation_percent(FREQ_DIR)<0.1)
//                                    m_JSONHandle.rm_oldest_day_file();
//                                QString fileName = m_JSONHandle.GetSaveFileName(1,lwaveP->chn[i]);
//                                m_JSONHandle.saveToJsonFile(baseArray, fileName);
//                                change = 0;
//                                saved = 1;
//                                break;
//                            }
//                            else
//                            break;
//                        }
                        if(localColect->mqttopen)
                        {
                            qDebug("send topic:%s\n",topic.toUtf8().constData());
                            emit sendsig(topic, obj,0);
                            break;
                        }
                        else
                        {
                            QThread::sleep(1);
                            j++;
                            if(j>5)
                            {
                                change = 1;
                                break;
                            }
                        }

                    }while(1);

                }


//                for(int j=0; j< count; j++)
//                {
//                    obj.insert("Id", lwaveP->chn[i]);
//                    obj.insert("PackCount", count);
//                    obj.insert("PackIndex", index++);
//                    obj.insert("ChNo", lwaveP->chn[i]);
//                    obj.insert("Dir", 1);
//                    obj.insert("Code", lwaveP->code);
//                    obj.insert("Time", lwaveP->time);
//                    obj.insert("PlusLength", 0);
//                    obj.insert("Freq", lwaveP->freq);
//                    obj.insert("RPM", 0);//TODO :need to judge
//                    obj.insert("TimeMark", 0);//TODO :need to judge
//                    obj.insert("Coef", 0.000149200001033023);//TODO :need to judge
//                    obj.insert("Src", 1);
//                    obj.insert("IsSensor", 1);//TODO :need to judge
//                    if(j<count)
//                        obj.insert("CRC", "");
//                    else
//                    {
//                        for(int k=0; k<lwaveP->len; k++)
//                            crc^=lwaveP->data[lwaveP->chn[i]][k];
//                        obj.insert("CRC", crc);
//                    }
//                    obj.insert("Content", str);
//                    QString topic;
//                    topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);
//                    qDebug("send topic:%s\n",topic.toUtf8().constData());
//                    localColect->condition = 0;
//                    emit sendsig(topic, obj,0);

//                }
            }
        do
        {
            if(localColect->reupload.size()>0)
            {
                qDebug("left size:%d upload data!\n",localColect->reupload.size());
                for(int k=0; k<localColect->reupload.size();k++)
                {
                    obj.insert("Id", lwaveP->chn[i]);
                    obj.insert("PackCount", count);
                    obj.insert("PackIndex", localColect->reupload[k]);
                    obj.insert("ChNo", lwaveP->chn[i]);
                    obj.insert("Dir", 1);
                    obj.insert("Code", lwaveP->code);
                    obj.insert("Time", lwaveP->time);
                    obj.insert("PlusLength", 0);
                    obj.insert("Freq", lwaveP->freq);
                    obj.insert("RPM", 0);//TODO :need to judge
                    obj.insert("TimeMark", 0);//TODO :need to judge
                    obj.insert("Coef", 0.000149200001033023);//TODO :need to judge
                    obj.insert("Src", 1);
                    obj.insert("IsSensor", 1);//TODO :need to judge
                    obj.insert("CRC", QJsonValue::Null);

                    QString str = intArrayToBase64(lwaveP->data[i], lwaveP->len);
                    //QByteArray byteArray(lwaveP->data[i], lwaveP->len);
                    //QString str = QString::fromUtf8(byteArray);
                    QString segment = str.mid(localColect->reupload[k]*localColect->sendlen, localColect->sendlen);
                    obj.insert("Content", segment);

                    QString topic;
                    topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);

                    pthread_mutex_lock(&localColect->mutex);
                    localColect->condition = 0;
                    pthread_mutex_unlock(&localColect->mutex);
                    j = 0;
                    do
                    {
                        if(localColect->mqttopen)
                        {
                            qDebug("send topic:%s\n",topic.toUtf8().constData());
                            emit sendsig(topic, obj,0);
                            break;
                        }
                        else
                        {
                            QThread::sleep(1);
                            j++;
                            if(j>5)
                            {
                                change = 1;
                                break;
                            }
                        }

                    }while(1);
                    //emit sendsig(topic, obj,0);
//                    if(!sendMqMessage(mqttClient, topic, &obj))
//                    {
//                        //TODO save data;
//                    }
                }
                localColect->reupload.clear();
            }
            if(waiting_function(5,localColect)==0)
            {
                if(localColect->reupload.size()==0 && (localColect->topicStatus==0||localColect->topicStatus==1))
                {
                    localColect->messageid++;
                    change = 0;
                    break;
                }
                else if(localColect->reupload.size()==0 && (localColect->topicStatus!=0&&localColect->topicStatus!=1))
                {
                    change = 1;
                    break;
                }
                else if(localColect->reupload.size()>0)
                    continue;
            }
            else
            {
                change = 1;
                localColect->messageid++;
                break;
            }
        }while(1);
        if(change == 1 /*&& saved == 0*/)
        {
            while(m_JSONHandle.get_partation_percent(FREQ_DIR)<0.1)
                m_JSONHandle.rm_oldest_day_file();
            QString fileName = m_JSONHandle.GetSaveFileName(1, lwaveP->chn[i]);
            qDebug("save wave file :%s",qPrintable(fileName));
            m_JSONHandle.saveToJsonFile(baseArray, fileName);
        }
        baseArray = QJsonArray();
    }
    return 0;
}

int mqtthandle::testPackMqttData(QMqttClient* mqttClient,FreqCollectPara* localColect, FRESysParam* localPara)
{
    QFile file("/userdata/20241025_145033512C1");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << file.errorString();
    }
    QByteArray data = file.readAll();
    QJsonObject obj;
    QJsonArray baseArray;
    int len = data.size();
    int sendlen = 1024;
    int count = 0;
    int index = 0;
    uint crc;
    QString upstr = data.toBase64();;

    obj = QJsonObject();
    int size = upstr.size();
    count = divideCeilToInt(size,sendlen);
    qDebug("len:%d,size:%d",len,upstr.size());
    for (int k = 0; k < size; k += sendlen) {
        QString segment = upstr.mid(k, sendlen);
        obj.insert("Id", 0);
        obj.insert("PackCount", count);
        obj.insert("PackIndex", index++);
        obj.insert("ChNo", 0);
        obj.insert("Dir", 1);
        obj.insert("Code", 1);
        obj.insert("Time", "2024/10/25 14:50:33");
        obj.insert("PlusLength", 0);
        obj.insert("Freq", 50000);
        obj.insert("RPM", 0);//TODO :need to judge
        obj.insert("TimeMark", 0);//TODO :need to judge
        obj.insert("Coef", 0.000149200001033023);//TODO :need to judge
        obj.insert("Src", 1);
        obj.insert("IsSensor", 1);//TODO :need to judge
        if((k+sendlen) < len)
            obj.insert("CRC", QJsonValue::Null);
        else
        {
            crc = GetCRC32(upstr);
//                        for(int l=0; l<lwaveP->len; l++)
//                            crc^=lwaveP->data[i][l];
            obj.insert("CRC", static_cast<double>(crc));
        }
        obj.insert("Content", segment);
        QString topic;
        topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);


        pthread_mutex_lock(&localColect->mutex);
        localColect->condition = 0;

        pthread_mutex_unlock(&localColect->mutex);
        baseArray.append(obj);
    }
    QString fileName = m_JSONHandle.GetSaveFileName(2, 0);
    qDebug("save wave file :%s",qPrintable(fileName));
    m_JSONHandle.saveToJsonFile(baseArray, fileName);

    file.close();
    return 0;
}

//int mqtthandle::sendMqttLongWaveData(QMqttClient* mqttClient,CollectPara* localColect, FRESysParam* localPara, int flag, qint64 *startsec)
//{
//    QJsonObject obj;
//    QJsonArray baseArray;
//    int index = 0;
//    FREWaveValue * lwaveP;
//    int i,count;
//    uint crc;
//    int j=0,change = 0,saved = 0;
//    if(flag==0)
//    {
//        lwaveP = &m_highwave;
//    }
//    else
//    {
//        lwaveP = &m_lowwave;
//    }
//    qDebug("a");
//    qDebug("chn count:%d\n",lwaveP->chn.count());
//    sendStoredWave(localColect,localPara,startsec);
//    return 0;
//    qDebug("chn count:%d\n",lwaveP->chn.count());

//    for(i=0;i<lwaveP->chn.count();i++)
//    {
//        localColect->reupload.clear();
//        count = divideCeilToInt(lwaveP->len*4,localColect->sendlen);
//        qDebug("len:%d,sendlen:%d,count:%d\n",lwaveP->len,localColect->sendlen,count);
//        crc = 0;
//            obj = QJsonObject();
//            int len = lwaveP->len*4;
//            QString upstr = intArrayToBase64(lwaveP->data[i], lwaveP->len);

//           // if(localColect->reupload.size()==0)
//            {
//                qDebug("no left upload data!\n");
//                for (int k = 0; k < len; k += localColect->sendlen) {
//                    QString segment = upstr.mid(k, localColect->sendlen);
//                    obj.insert("Id", lwaveP->chn[i]);
//                    obj.insert("PackCount", count);
//                    obj.insert("PackIndex", index++);
//                    obj.insert("ChNo", lwaveP->chn[i]);
//                    obj.insert("Dir", 1);
//                    obj.insert("Code", lwaveP->code);
//                    obj.insert("Time", lwaveP->time);
//                    obj.insert("PlusLength", 0);
//                    obj.insert("Freq", lwaveP->freq);
//                    obj.insert("RPM", 0);//TODO :need to judge
//                    obj.insert("TimeMark", 0);//TODO :need to judge
//                    obj.insert("Coef", 0.000149200001033023);//TODO :need to judge
//                    obj.insert("Src", 1);
//                    obj.insert("IsSensor", 1);//TODO :need to judge
//                    if((k+localColect->sendlen) < len)
//                        obj.insert("CRC", QJsonValue::Null);
//                    else
//                    {
//                        crc = GetCRC32(upstr);
////                        for(int l=0; l<lwaveP->len; l++)
////                            crc^=lwaveP->data[i][l];
//                        obj.insert("CRC", static_cast<double>(crc));
//                    }
//                    obj.insert("Content", segment);
//                    QString topic;
//                    topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);


//                    pthread_mutex_lock(&localColect->mutex);
//                    localColect->condition = 0;

//                    pthread_mutex_unlock(&localColect->mutex);
//                    baseArray.append(obj);
//                    j = 0;
//                    do
//                    {
//                        if(localColect->mqttopen)
//                        {
//                            qDebug("send topic:%s\n",topic.toUtf8().constData());
//                            emit sendsig(topic, obj,0);
//                            break;
//                        }
//                        else
//                        {
//                            QThread::sleep(1);
//                            j++;
//                            if(j>5)
//                            {
//                                change = 1;
//                                break;
//                            }
//                        }

//                    }while(1);

//                }
//            }
//        do
//        {
//            if(localColect->reupload.size()>0)
//            {
//                qDebug("left size:%d upload data!\n",localColect->reupload.size());
//                for(int k=0; k<localColect->reupload.size();k++)
//                {
//                    obj.insert("Id", lwaveP->chn[i]);
//                    obj.insert("PackCount", count);
//                    obj.insert("PackIndex", localColect->reupload[k]);
//                    obj.insert("ChNo", lwaveP->chn[i]);
//                    obj.insert("Dir", 1);
//                    obj.insert("Code", lwaveP->code);
//                    obj.insert("Time", lwaveP->time);
//                    obj.insert("PlusLength", 0);
//                    obj.insert("Freq", lwaveP->freq);
//                    obj.insert("RPM", 0);//TODO :need to judge
//                    obj.insert("TimeMark", 0);//TODO :need to judge
//                    obj.insert("Coef", 0.000149200001033023);//TODO :need to judge
//                    obj.insert("Src", 1);
//                    obj.insert("IsSensor", 1);//TODO :need to judge
//                    obj.insert("CRC", QJsonValue::Null);

//                    QString str = intArrayToBase64(lwaveP->data[i], lwaveP->len);
//                    //QByteArray byteArray(lwaveP->data[i], lwaveP->len);
//                    //QString str = QString::fromUtf8(byteArray);
//                    QString segment = str.mid(localColect->reupload[k]*localColect->sendlen, localColect->sendlen);
//                    obj.insert("Content", segment);

//                    QString topic;
//                    topic = QString("%1/WaveData/%2").arg(m_uploadTopic).arg(localColect->messageid);

//                    pthread_mutex_lock(&localColect->mutex);
//                    localColect->condition = 0;
//                    pthread_mutex_unlock(&localColect->mutex);
//                    j = 0;
//                    do
//                    {
//                        if(localColect->mqttopen)
//                        {
//                            qDebug("send topic:%s\n",topic.toUtf8().constData());
//                            emit sendsig(topic, obj,0);
//                            break;
//                        }
//                        else
//                        {
//                            QThread::sleep(1);
//                            j++;
//                            if(j>5)
//                            {
//                                change = 1;
//                                break;
//                            }
//                        }

//                    }while(1);
//                }
//                localColect->reupload.clear();
//            }
//            if(waiting_function(5,localColect)==0)
//            {
//                if(localColect->reupload.size()==0 && (localColect->topicStatus==0||localColect->topicStatus==1))
//                {
//                    localColect->messageid++;
//                    change = 0;
//                    break;
//                }
//                else if(localColect->reupload.size()==0 && (localColect->topicStatus!=0&&localColect->topicStatus!=1))
//                {
//                    change = 1;
//                    break;
//                }
//                else if(localColect->reupload.size()>0)
//                    continue;
//            }
//            else
//            {
//                change = 1;
//                localColect->messageid++;
//                break;
//            }
//        }while(1);
//        if(change == 1 /*&& saved == 0*/)
//        {
//            while(m_JSONHandle.get_partation_percent(FREQ_DIR)<0.1)
//                m_JSONHandle.rm_oldest_day_file();
//            QString fileName = m_JSONHandle.GetSaveFileName(1, lwaveP->chn[i]);
//            qDebug("save wave file :%s",qPrintable(fileName));
//            m_JSONHandle.saveToJsonFile(baseArray, fileName);
//        }
//        baseArray = QJsonArray();
//    }
//    return 0;
//}

void mqtthandle::signal_function(FreqCollectPara* localColect)
{
    qDebug("signal_function\n");
    pthread_mutex_lock(&localColect->mutexwork);
    localColect->conditionwork = 1; // 改变条件
    pthread_cond_signal(&localColect->condwork); // 通知等待线程
    pthread_mutex_unlock(&localColect->mutexwork);
}

int mqtthandle::waiting_function(int timeout_seconds, FreqCollectPara* localColect)
{
    int ret = 0;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_seconds; // 设置超时时间点
    pthread_mutex_lock(&localColect->mutex);
    qDebug("waiting_function\n");
    while(localColect->condition == 0) {
        // 使用条件变量等待，直到条件满足或超时
        if(pthread_cond_timedwait(&localColect->cond, &localColect->mutex, &ts) == ETIMEDOUT) {
            qDebug("Timeout occurred.\n");
            ret = -1;
            break;
        }
    }
    pthread_mutex_unlock(&localColect->mutex);
    return ret;
}

int mqtthandle::waiting_functionsend(int timeout_seconds, FreqCollectPara* localColect)
{
    int ret = 0;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_seconds; // 设置超时时间点
    pthread_mutex_lock(&localColect->mutexsend);
    //qDebug("waiting_functionsend\n");
    while(localColect->conditionsend == 0) {
        // 使用条件变量等待，直到条件满足或超时
        if(pthread_cond_timedwait(&localColect->condsend, &localColect->mutexsend, &ts) == ETIMEDOUT) {
            qDebug("Timeout occurred.\n");
            ret = -1;
            break;
        }
    }
    pthread_mutex_unlock(&localColect->mutexsend);
    return ret;
}

void mqtthandle::dowork(QMqttClient* mqttClient, FRESysParam* localPara,FreqCollectPara* localColect, AdcHandle* adhanle_l,TCPParam * param)
{
    int countIndex=0;
    int countWave=0;
    int countLongWave=0;
    int sendflag = 0;
    int errcount = 0;
    int netcount = 0;
    int status = -1;
    int ret = 0;
    m_uploadTopic = localColect->Topic;
    qint64 currentSecsSinceEpoch = QDateTime::currentDateTime().toSecsSinceEpoch();
    qint64 startsec[3];
    qint64 netchecktime;
    for(int i = 0; i < 3; ++i) {
        startsec[i] = currentSecsSinceEpoch;
    }
    netchecktime = currentSecsSinceEpoch;
    qDebug("init startsec value:%lld\n",startsec[0]);
    //qint64 startsec = QDateTime::currentDateTime().toSecsSinceEpoch();
    //while(localColect->loop)
    while(1)
    {
        qint64 nowsec = QDateTime::currentDateTime().toSecsSinceEpoch();
        m_uploadTopic = localColect->Topic;
        countIndex++;
        countWave++;
        countLongWave++;
        if(startsec[0]>nowsec+60)
        {
            startsec[0]=nowsec;
            startsec[1]=nowsec;
            startsec[2]=nowsec;
        }
        if(localPara->UploadPeriod>300)
            localPara->UploadPeriod = 300;
        //if(localColect->updateflag == 0)
        {
            if(((nowsec-startsec[0]) >= localPara->IndexPeriod)||((nowsec-startsec[0]) >= localPara->UploadPeriod))
            //if(nowsec%localPara->IndexPeriod == 0)
            {
                //testPackMqttData(mqttClient,localColect, localPara);
                sendflag = 1;
                startsec[0] = nowsec;
                qDebug("now need to send Index:%d!!!\n",localPara->IndexPeriod);
                adhanle_l->getAdDataIndex(&m_IndexList);

                ret = sendMqttIndexData(mqttClient,localColect,localPara,startsec);
                if(ret==0)
                {
                    errcount++;
                    if(errcount*localPara->IndexPeriod>1800)
                    {
                        qDebug("now need to restart mcu board!!!!\n");
                        system("echo 0 > /sys/class/gpio/gpio101/value;sleep 2;echo 1 > /sys/class/gpio/gpio101/value");
                        errcount = 0;
                        param->status = 0;
                    }
                }
                else if(ret>0)
                {
                    errcount = 0;
                    param->status = 1;
                }
                if(ret<0)
                {
                    netcount++;
                    int mode;
                    if(netcount*localPara->IndexPeriod>3400)
                    {
                        qDebug("net error now need to restart rk board!!!!\n");
                        AppConfig::get("Device/update", mode);
                        if((mode>>1)&0x01 == 0x01)
                        {
                            qDebug("================11=================!!!\n");
                            system("reboot");
                        }
                        netcount =0;
                        //param->status = 0;
                    }
                }
                else if(ret>0)
                {
                    netcount = 0;
                    //param->status = 1;
                }
                qDebug("ret:%d,count:%d，netcount:%d,param->status:%d!!!\n",ret,errcount,netcount,param->status);

//                if(status != param->status)
//                {
//                    if(param->status==0)
//                        sendMqttERRORNO(localColect,0x01);
//                    status = param->status;
//                }
            }
            //if((nowsec-startsec[1]) >= localPara->IndexPeriod)
            if(((nowsec-startsec[1]) >= localPara->IndexPeriod)||((nowsec-startsec[1]) >= localPara->UploadPeriod))
            //if(nowsec%localPara->WavePeriod == 0)
            {
                sendflag = 1;
                startsec[1] = nowsec;
                qDebug("now need to send Wave:%d!!!\n",localPara->WavePeriod);

                int i,j;
                m_highwave.chn.clear();
                m_lowwave.chn.clear();
                qDebug("icount:%d\n",localColect->VibChannelList.count());
                for(i=0; i<localColect->VibChannelList.count();i++)
                {
                    for(j=0;j<localColect->VibChannelList[i].VibParaList.count();j++)
                    {
                        qDebug("i:%d,jcount:%d\n",i,localColect->VibChannelList[i].VibParaList.count());
                        if(localColect->VibChannelList[i].VibParaList[j].Code==1)
                        {
                            m_highwave.maxlen = localColect->VibChannelList[i].VibParaList[j].Length/4;
                            qDebug("maxlen:%d!!!\n",m_highwave.maxlen);
                        }
                        if(localColect->VibChannelList[i].VibParaList[j].Code==11)
                        {
                            m_highwave.len = localColect->VibChannelList[i].VibParaList[j].Length/4;
                            m_highwave.chn.append(localColect->VibChannelList[i].ChNo);
                            m_highwave.freq = localColect->VibChannelList[i].VibParaList[j].HighFreq;
                            qDebug("len:%d,chn:%d,freq:%d!!!\n",m_highwave.len,m_highwave.chn[i],m_highwave.freq);
                        }
                        if(localColect->VibChannelList[i].VibParaList[j].Code==12)
                        {
                            m_lowwave.len = localColect->VibChannelList[i].VibParaList[j].Length/4;
                            m_lowwave.chn.append(localColect->VibChannelList[i].ChNo);
                            m_lowwave.freq = localColect->VibChannelList[i].VibParaList[j].HighFreq;
                            m_lowwave.maxlen = localColect->VibChannelList[i].VibParaList[j].Length/4;
                            qDebug("len:%d,chn:%d,freq:%d,maxlen:%d!!!\n",m_lowwave.len,m_lowwave.chn[i],m_lowwave.freq,m_lowwave.maxlen);
                        }

                    }
                    if(m_highwave.maxlen == 0)
                        m_highwave.maxlen = m_highwave.len;
                }
                m_highwave.code = 11;
                m_lowwave.code = 12;
//                adhanle_l->setAdParam(m_highwave.freq);
//                adhanle_l->getAdDataWave(&m_highwave);
//                sendMqttWaveData(mqttClient,localColect,localPara, 0, startsec);
                adhanle_l->setAdParam(m_lowwave.freq);
                adhanle_l->getAdDataWave(&m_lowwave);
                sendMqttWaveData(mqttClient,localColect, localPara, 1, startsec);

    //            m_highwave.code = 1;
    //            sendMqttWaveData(mqttClient,localColect,localPara, 0, startsec);
    //            qDebug("send over Wave:%d!!!\n",localPara->WavePeriod);
            }
            //if((nowsec-startsec[2]) >= localPara->IndexPeriod)
            if(((nowsec-startsec[2]) >= localPara->IndexPeriod)||((nowsec-startsec[2]) >= localPara->UploadPeriod))
            //if(nowsec%localPara->LongWavePeriod == 0)
            {
                sendflag = 1;
                startsec[2] = nowsec;
                qDebug("now need to send LongWave:%d!!!\n",localPara->LongWavePeriod);
                int i,j;
                m_highwave.chn.clear();
                for(i=0; i<localColect->VibChannelList.count();i++)
                {
                    for(j=0;j<localColect->VibChannelList[i].VibParaList.count();j++)
                    {
                        if(localColect->VibChannelList[i].VibParaList[j].Code==1)
                        {
                            m_highwave.len = localColect->VibChannelList[i].VibParaList[j].Length/4;
                            m_highwave.chn.append(localColect->VibChannelList[i].ChNo);
                            m_highwave.freq = localColect->VibChannelList[i].VibParaList[j].HighFreq;
                            if(m_highwave.maxlen == 0)
                                m_highwave.maxlen = localColect->VibChannelList[i].VibParaList[j].Length/4;
                        }

                    }
                }
                m_highwave.code = 1;
                adhanle_l->setAdParam(m_highwave.freq);
                adhanle_l->getAdDataWave(&m_highwave);
                sendMqttWaveData(mqttClient,localColect, localPara, 0, startsec);
                qDebug("send over Wave:%d!!!\n",localPara->LongWavePeriod);
            }
            QThread::msleep(500);
        }
    }
    qDebug("mqtt handle over!!!");
    signal_function(localColect);
//    emit resultReady(parameter);
}
