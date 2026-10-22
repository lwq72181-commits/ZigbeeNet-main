#include "QtGlobalMutex.h"


QtGlobalMutex::QtGlobalMutex():
    m_sharedMemory()
{
}

QtGlobalMutex::~QtGlobalMutex()
{
}

bool QtGlobalMutex::lock()
{
    return m_sharedMemory.lock();
}

bool QtGlobalMutex::unlock()
{
    return m_sharedMemory.unlock();
}

bool QtGlobalMutex::open(const QString& name)
{
    m_sharedMemory.setKey(name);
    return m_sharedMemory.create(1) && (m_sharedMemory.error() != QSharedMemory::AlreadyExists);
}

void QtGlobalMutex::close()
{
    //
}
