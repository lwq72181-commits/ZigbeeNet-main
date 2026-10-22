/*    file: TNamedMutex.h
 *    desc:
 *   
 * created: 2009-09-02 09:44:15
 *  author: terry
 * version: 1.0
 * company: 
 */

#if !defined TNAMEDMUTEX_H_
#define TNAMEDMUTEX_H_

////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <windows.h>
#else //
#include <pthread.h>
#endif //WIN32
/////////////////////////////////////////////////////////////////////////////

namespace comn
{

#ifdef WIN32

class NamedMutex
{
public:
    typedef HANDLE MutexType;
    typedef HANDLE MutexHandle;

    NamedMutex()
    {
    }

    ~NamedMutex()
    {
        close();
    }

    bool open(const char* name)
    {
        m_object = ::CreateMutexA( NULL, false, name );
    }

    void close()
    {
        if (m_object != NULL)
        {
            ::CloseHandle( m_object );
            m_object = NULL;
        }
    }

    bool isOpen() const
    {
        return (m_object != NULL);
    }

    bool lock()
    {
        if (!isOpen())
        {
            return false;
        }

        ::WaitForSingleObject( m_object, INFINITE );
        return true;
    }

    void unlock()
    {
        if (!isOpen())
        {
            return ;
        }

        ::ReleaseMutex( m_object );
    }

    bool trylock()
    {
        DWORD ret = ::WaitForSingleObject( m_object, 0 );
        return ( WAIT_OBJECT_0 == ret );
    }

private:
    MutexType m_object;

    /// disable copy and assign
    NamedMutex( const NamedMutex& mtx );
    NamedMutex& operator = ( const NamedMutex& mtx );

};

#else

/// cross platform mutex
class NamedMutex
{
public:
    typedef pthread_mutex_t MutexType;
    typedef pthread_mutex_t* MutexHandle;

    explicit NamedMutex(bool recursive = true)
    {
        create(recursive);
    }

    ~NamedMutex()
    {
        destroy();
    }

    void lock()
    {
        ::pthread_mutex_lock(&m_object);
    }

    bool trylock()
    {
        return (0 == ::pthread_mutex_trylock(&m_object));
    }

    void unlock()
    {
        ::pthread_mutex_unlock(&m_object);
    }

    MutexHandle getHandle()
    {
        return &m_object;
    }

private:
    /// disable copy and assign
    NamedMutex(const NamedMutex& mtx);
    NamedMutex& operator =(const NamedMutex& mtx);

protected:
    pthread_mutexattr_t m_attr;
    MutexType m_object;

    int create(bool recursive)
    {
        int ret = ::pthread_mutexattr_init(&m_attr);
        if (0 == ret)
        {
            if (recursive)
            {
                ::pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
            }
            ret = ::pthread_mutex_init(&m_object, &m_attr);
        }
        return ret;
    }

    void destroy()
    {
        ::pthread_mutex_destroy(&m_object);
        ::pthread_mutexattr_destroy(&m_attr);
    }

};

#endif //WIN32

/// auto mutex
class AutoNamedMutex
{
public:
    explicit AutoNamedMutex(NamedMutex& mtx) :
        m_locked(false), m_mutex(mtx)
    {
        m_mutex.lock();
        m_locked = true;
    }

    ~AutoNamedMutex()
    {
        unlock();
    }

    void unlock()
    {
        if (m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    bool m_locked;
    NamedMutex& m_mutex;

    AutoNamedMutex(const AutoNamedMutex& mtx);
    AutoNamedMutex& operator =(const AutoNamedMutex& mtx);

};

} // end of namespace
////////////////////////////////////////////////////////////////////////////
#endif //TNAMEDMUTEX_H_
