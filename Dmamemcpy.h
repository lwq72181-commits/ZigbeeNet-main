#ifndef DMAMEMCPY_H
#define DMAMEMCPY_H
#include <QObject>
#include "Acquisitiontype.h"
#include "ring_buffer.h"

class DmaMemcpy:public QObject
{
    Q_OBJECT
public:
    DmaMemcpy();
public:
    //QVector<data_buf> m_bufVector;
    data_buf *m_bufVector[24];
    data_buf *m_rbuffer;
protected:
    void neon_copy(volatile void *dst, volatile void *src, int sz);
    int dma_init(struct DMAInfo *dma_info);
    void dma_destroy(struct DMAInfo *dma_info);
    int dma_write(struct DMAInfo *dma_info);
    int dma_read(struct DMAInfo *dma_info);
    bool init_parameter();
    int initBuffer();
    int unpackADCvalue();
    int convert24To32Signed(int input);
    int setSche();
protected:
    struct DMAInfo *m_DMAInfo;
    //int *m_rbuffer = NULL;

public slots:
     // doWork定义了线程要执行的操作
    void dowork();
    //void handleData();
signals:
    //void dmasig();
};

#endif // DMAMEMCPY_H
