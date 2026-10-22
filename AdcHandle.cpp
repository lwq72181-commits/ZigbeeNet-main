#include "AdcHandle.h"
#include<QThread>
#include <math.h>
#include "AppService.h"



AdcHandle::AdcHandle()
{
    initADC();
}

emxArray_real_T *AdcHandle::argInit_1xUnbounded_real_T()
{
  emxArray_real_T *result;
  int idx1;

  /* Set the size of the array.
     Change this size to the value that the application requires. */
  result = emxCreate_real_T(1, 4096);

  /* Loop over the array to initialize each element. */
  for (idx1 = 0; idx1 < result->size[1U]; idx1++) {
    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result->data[idx1] = 0.0;
  }

  return result;
}

int AdcHandle::initADC()
{
    emxInitArray_real_T(&m_FFTData, 1);

    m_ADData = argInit_1xUnbounded_real_T();
    return 0;
}

int AdcHandle::setAdParam(int freq)
{
    return 0;
}

double AdcHandle::calculateRMS(double *data, int n) {
    double sumOfSquares = 0.0; // 用于累加平方和
    for(int i = 0; i < n; i++) {
        sumOfSquares += pow(data[i], 2); // 计算每个数值的平方并累加
    }
    double meanOfSquares = sumOfSquares / n; // 计算平方的平均值
    double rmsValue = sqrt(meanOfSquares); // 对平均值开平方根得到RMS

    return rmsValue;
}

int AdcHandle::getAdDataIndex(QList<FREIndexValue> *data)
{
    int count = 0;
    int sizecopy = DMA_BLOCK_SIZE*4;
    QDateTime currentTime = QDateTime::currentDateTime();
    QString isoFormat = currentTime.toString("yyyy-MM-dd'T'hh:mm:ss");
    //QString isoFormat = currentTime.toString(Qt::ISODate);
    QList<FREIndexValue>::iterator it;
    FREIndexValue mid;
    mid.speed.append(1.1);
    mid.temp.append(44.4);
    mid.HFAcceleration.append(22.2);
    mid.LFSquare.append(33.3);
    mid.speed.append(1.1);
    mid.Time.append(isoFormat);
    data->append(mid);
    return 0;
    for (it = data->begin(); it != data->end(); ++it) {
        it->speed.append(1.1);
        it->temp.append(44.4);
        it->HFAcceleration.append(22.2);
        it->LFSquare.append(33.3);
        it->speed.append(1.1);
        it->Time.append(isoFormat);
    }
    return 0;

    DmaMemcpy *l_dma = AppService::dmaMemcpy();
    while(true)
    {
        data_buf::s_data *p_data = l_dma->m_rbuffer->get_data();
        if(p_data == NULL)
        {
            qDebug("get waveDate empty error!!!\n");
            count++;
            if(count<10)
            {
                qDebug("count:%d need to reget value!!!\n",count);
                //l_dma->m_bufVector[i]->put_buffer(p_data);
                QThread::usleep(1000);
                continue;
            }
            else
            {
                //l_dma->m_bufVector[i]->put_buffer(p_data);
                break;
            }

        }
        QList<FREIndexValue>::iterator it;
        for (it = data->begin(); it != data->end(); ++it) {
            memcpy(m_data,&p_data->pdata[DMA_BLOCK_SIZE*(it->chn)],sizecopy);
            Integral(m_ADData, 5000, 1/**/, 10, 1000, m_FFTData);//speed:1 length:2

        }
        l_dma->m_rbuffer->put_buffer(p_data);
    }

    return 0;
}

int AdcHandle::getAdDataWave(FREWaveValue *waveData)//TODO :malloc base on flag to malloc only once only if length changed then malloc new space
{
    return 0;
    //long and high wave use same param to store
    if(waveData->data == NULL)//TODO when channel count changed we need to remalloc space
    {
        int rows = waveData->chn.count();
        int cols = waveData->maxlen;
        qDebug("need to malloc wavedata,rows:%d,cols:%d!!!\n",rows,cols);
        //waveData->data = (int**)malloc(rows * sizeof(int*)); // 分配行指针数组
        while((waveData->data = (int**)malloc(rows * sizeof(int*))) == NULL)
        {
            qDebug("malloc waveDate array error!!!\n");
            QThread::sleep(1);
        }
        for(int i = 0; i < rows; ++i)
        {
            while((waveData->data[i] = (int*)malloc(cols * sizeof(int)))==NULL) // 分配每行的空间
            {
                qDebug("malloc waveDate space error!!!\n");
                QThread::sleep(1);
            }
        }
    }
    QDateTime currentTime = QDateTime::currentDateTime();
    waveData->time = currentTime.toString("yyyy-MM-dd'T'hh:mm:ss");//currentTime.toString(Qt::ISODate);
    return 0;
    DmaMemcpy *l_dma = AppService::dmaMemcpy();
    int left,count,j;
    left=waveData->maxlen;
    count = 0;
    j=0;
    int sizecopy = DMA_BLOCK_SIZE*4;
    while(left>0)
    {
        data_buf::s_data *p_data = l_dma->m_rbuffer->get_data();
        if(p_data == NULL)
        {
            qDebug("get waveDate empty error!!!\n");
            count++;
            if(count<10)
            {
                qDebug("count:%d need to reget value!!!\n",count);
                //l_dma->m_bufVector[i]->put_buffer(p_data);
                QThread::usleep(1000);
                continue;
            }
            else
            {
                //l_dma->m_bufVector[i]->put_buffer(p_data);
                break;
            }

        }
//        FILE *fp = fopen("sav.dat", "ab");
//        fwrite(p_data->pdata, 1, p_data->data_len, fp);
//        //fflush(fp);
//        fclose(fp);
        if(left>DMA_BLOCK_SIZE)
        {
            for(int i=0;i<waveData->chn.count();i++)
            {
                memcpy(&waveData->data[i][DMA_BLOCK_SIZE*j],&p_data->pdata[DMA_BLOCK_SIZE*(waveData->chn[i])],sizecopy);//i row 1
            }

//            for(int i=0;i<24;i++)
//            {
//                qDebug("i:%d",i);
//                if(waveData->chn.contains(i))
//                {
//                    memcpy(&waveData->data[i][512*j],&p_data->pdata[512*i],2048);//i row 1
//                }
//            }

            left-=DMA_BLOCK_SIZE;
            j++;
            //qDebug("j:%d,left:%d\n",j,left);
        }
        else
        {
            for(int i=0;i<waveData->chn.count();i++)
            {
                memcpy(&waveData->data[i][DMA_BLOCK_SIZE*j],&p_data->pdata[DMA_BLOCK_SIZE*(waveData->chn[i])],left*4);//i row 1
            }
//            for(int i=0;i<24;i++)
//            {
//                if(waveData->chn.contains(i))
//                memcpy(&waveData->data[i][512*j],&p_data->pdata[512*i],left*4);
//            }
            //memcpy(&waveData->data[i][512*j],&p_data->pdata[512*i],left);
            //memcpy(waveData->data[i],p_data->pdata,left);
            left=0;
            //qDebug("j:%d,left:%d\n",j,left);
        }
        QDateTime currentTime = QDateTime::currentDateTime();
        waveData->time = currentTime.toString("yyyy-MM-dd'T'hh:mm:ss");//currentTime.toString(Qt::ISODate);
        l_dma->m_rbuffer->put_buffer(p_data);
    }

//    for(int i=0;i<24;i++)
//    {
//        left=waveData->maxlen;
//        count = 0;
//        qDebug("i:%d,left:%d\n",i,left);
//        while(left>0)
//        {
//            data_buf::s_data *p_data = l_dma->m_bufVector[i]->get_data();
//            if(p_data == NULL)
//            {
//                qDebug("get waveDate empty error!!!\n");
//                count++;
//                if(count<10)
//                {
//                    qDebug("count:%d need to reget value!!!\n",count);
//                    l_dma->m_bufVector[i]->put_buffer(p_data);
//                    QThread::usleep(1000);
//                    continue;
//                }
//                else
//                {
//                    l_dma->m_bufVector[i]->put_buffer(p_data);
//                    break;
//                }

//            }
//            if(left>p_data->data_len)
//            {
//                memcpy(waveData->data[i],p_data->pdata,p_data->data_len);
//                left-=p_data->data_len;
//                qDebug("i:%d,left:%d\n",i,left);
//            }
//            else
//            {
//                memcpy(waveData->data[i],p_data->pdata,left);
//                left=0;
//                qDebug("i:%d,left:%d\n",i,left);
//            }

//            l_dma->m_bufVector[i]->put_buffer(p_data);
//        }
//    }

    // 初始化和访问

    // 释放内存
//    for(int i = 0; i < rows; ++i)
//        free(waveData->data[i]);
//    free(waveData->data);
    return 0;
}

int AdcHandle::getAdDataLongWave(char *data)
{
    return 0;
}
