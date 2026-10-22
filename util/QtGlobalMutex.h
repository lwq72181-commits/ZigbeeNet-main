#ifndef QTGLOBALMUTEX_H
#define QTGLOBALMUTEX_H

#include <QObject>
#include <QString>
#include <QSharedMemory>

class QtGlobalMutex: public QObject
{
   Q_OBJECT

public:
   QtGlobalMutex();
   ~QtGlobalMutex();

   bool open(const QString& name);
   void close();

   bool lock();
   bool unlock();

private:
   QSharedMemory m_sharedMemory; /*!< Used to provide the locking and unlocking
                                     implementation*/
};

#endif // QTGLOBALMUTEX_H
