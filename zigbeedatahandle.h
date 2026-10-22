#ifndef ZIGBEEDATAHANDLE_H
#define ZIGBEEDATAHANDLE_H
#include <QObject>
#include "Acquisitiontype.h"
#include "ring_buffer.h"
class ZigBeeDataHandle : public QObject
{
    Q_OBJECT
public:
    ZigBeeDataHandle();
    int initBuffer();
    int unpackADCvalue();
public:
    //QVector<data_buf> m_bufVector;
    data_buf *m_bufVector[24];
    data_buf *m_rbuffer;
protected:
    int setSche();
    int initParam();
public slots:
    void dowork();
};

#endif // ZIGBEEDATAHANDLE_H
