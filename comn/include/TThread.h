/*
 * TThread.h
 *
 *  Created on: 2010-4-21
 *      Author: terry
 */

#ifndef TTHREAD_H_
#define TTHREAD_H_

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else //
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif //WIN32
#include <assert.h>
#include <atomic>
#include <utility>
#include "TCriticalSection.h"
#include <iostream>

////////////////////////////////////////////////////////////////////////////

#ifndef TIMEVAL_TO_TIMESPEC
#define TIMEVAL_TO_TIMESPEC(tv, ts) {                   \
        (ts)->tv_sec = (tv)->tv_sec;                    \
        (ts)->tv_nsec = (tv)->tv_usec * 1000;               \
}
#endif //TIMEVAL_TO_TIMESPEC

namespace comn
{

#ifdef WIN32

class Thread
{
public:
    typedef HANDLE          ThreadHandle;
    typedef unsigned int    ThreadID;

    Thread()
    :m_stackSize(0),m_threadID(0),m_handle(0),m_canExit(false)
    {
    }

    explicit Thread(unsigned int stack_size)
    :m_stackSize(stack_size),m_threadID(0),m_handle(0),m_canExit(false)
    {
    }

    virtual ~Thread()
    {
		if (isRunning())
		{
			tryStop();

			if (!join(1000))
			{
				char text[1024] = { 0 };
				sprintf_s(text, "thread failed to join. this: %p, thread: %d\n", this, m_threadID);
				OutputDebugStringA(text);
			}
		}
    }

    bool start()
    {
		if (isRunning())
		{
			char text[1024] = { 0 };
			sprintf_s(text, "thread already is running. this: %p, thread: %d\n", this, m_threadID);
			OutputDebugStringA(text);
			return false;
		}

        m_canExit = false;
        return createThread();
    }

    void stop()
    {
    	if (isRunning())
    	{
    		tryStop();
        	join(-1);
    	}
    }

    void tryStop()
    {
		m_canExit = true;
        doStop();
    }

    bool join(int millisecond = -1)
    {
        bool ok = true;
		ThreadHandle handle = getHandle();
        if (handle != 0)
        {
            DWORD dwMilliseconds = (millisecond >= 0) ? millisecond : INFINITE;
            DWORD res = ::WaitForSingleObject(handle, dwMilliseconds);

			comn::AutoCritSec lock(m_csThread);
            CloseHandle(m_handle);
			m_handle = 0;

            ok = (WAIT_OBJECT_0 == res);
        }
        return ok;
    }


    ThreadHandle getHandle()
    {
		comn::AutoCritSec lock(m_csThread);
        return m_handle;
    }

    ThreadID getThreadID()
    {
		comn::AutoCritSec lock(m_csThread);
        return m_threadID;
    }

    bool isRunning()
    {
		comn::AutoCritSec lock(m_csThread);
        return (m_handle != 0);
    }

    void setStackSize(unsigned int stack_size)
    {
        m_stackSize = stack_size;
    }

    static ThreadID getCurrentThreadId()
    {
        return GetCurrentThreadId();
    }

protected:
    virtual int run() =0;
    virtual bool startup() { return true; }
    virtual void cleanup() {}
    virtual void doStop() {}


    unsigned int    m_stackSize;
    ThreadID        m_threadID;
    ThreadHandle    m_handle;
	std::atomic_bool m_canExit;

	comn::CriticalSection	m_csThread;



private:
	unsigned threadProc()
	{
		int code = 0;
		if (!m_canExit)
		{
			if (startup())
			{
				code = run();
			}
			else
			{
				code = 1;
			}
			cleanup();
		}

		{
			comn::AutoCritSec lock(m_csThread);
			m_threadID = 0;
		}

		_endthreadex(code);
		return code;
	}

    static unsigned __stdcall ThreadFunction(void* param)
    {
        int code = 0;
        Thread* pThread = (Thread*)param;

		code = pThread->threadProc();

        return code;
    }

    bool createThread()
    {
        unsigned int flag = 0;
        void* param = (void*)this;
        m_handle = (HANDLE)_beginthreadex(NULL, m_stackSize, ThreadFunction,
                            param, flag, &m_threadID);
        return (m_handle != 0);
    }

};

#else //WIN32

class Thread
{
public:
    typedef pthread_t    ThreadHandle;
    typedef pthread_t    ThreadID;

    Thread()
    :m_stackSize(0),m_threadID(0),m_handle(-1),m_canExit(false)
    {
    }

    explicit Thread(unsigned int stack_size)
    :m_stackSize(stack_size),m_threadID(0),m_handle(-1),m_canExit(false)
    {
    }

    virtual ~Thread()
    {
		if (isRunning())
		{
			tryStop();
			if (!join(1000))
			{
				std::cerr << "failed to join. this: " << this << ",thread id: " << m_threadID;
			}
		}
    }

    bool start()
    {
		if (isRunning())
		{
			std::cerr << "thread is already runnig. this: " << this << ",thread id: " << m_threadID;
			return false;
		}

        m_canExit = false;
        return createThread();
    }

    void stop()
    {
        if (isRunning())
        {
            tryStop();
            join(-1);
        }
    }

    void tryStop()
    {
        m_canExit = true;
        doStop();
    }

    bool join(int millisecond = -1)
    {
		ThreadHandle handle = getHandle();
        if (handle == (pthread_t)-1)
        {
            return true;
        }

		bool done = false;
        if (millisecond <= 0)
        {
            done = ( 0 == ::pthread_join(handle, NULL));
        }
		else
		{
#ifdef ANDROID
			done = (0 == ::pthread_join(handle, NULL));
#else
			struct timespec     ts;
			getTimeout(&ts, millisecond);
			done = (0 == ::pthread_timedjoin_np(handle, NULL, &ts));
#endif //
		}

		{
			comn::AutoCritSec   lock(m_csThread);
			m_handle = (pthread_t)-1;
		}

		return done;
    }

    ThreadHandle getHandle()
    {
		comn::AutoCritSec   lock(m_csThread);
        return m_handle;
    }

    ThreadID getThreadID() const
    {
        return m_threadID;
    }

    bool isRunning()
    {
		comn::AutoCritSec lock(m_csThread);
        return (m_handle != (pthread_t)-1);
    }

    void setStackSize(unsigned int stack_size)
    {
        m_stackSize = stack_size;
    }

    int kill(int sig)
    {
        int ret = 0;
        if (isRunning())
        {
            ret = ::pthread_kill(m_handle, sig);
        }
        return ret;
    }

    static ThreadID getCurrentThreadId()
    {
#ifdef ANDROID
		return pthread_self();
#else
		return syscall(SYS_gettid);
#endif //
    }

protected:
    virtual int run() =0;
    virtual bool startup() { return true; }
    virtual void cleanup() {}
    virtual void doStop() {}


    unsigned int    m_stackSize;
    ThreadID        m_threadID;
    ThreadHandle    m_handle;
    std::atomic_bool m_canExit;

	comn::CriticalSection	m_csThread;

private:

	void threadProc()
	{
		int code = 0;
		#ifdef ANDROID
		m_threadID = pthread_self();
		#else
        m_threadID = syscall( SYS_gettid );
		#endif //
		
        if ( !m_canExit )
        {
            if (startup())
            {
                code = run();
            }
            else
            {
                code = 1;
            }
            cleanup();
        }
        
        pthread_detach(pthread_self());

		{
			comn::AutoCritSec lock(m_csThread);
			m_threadID = 0;
			m_handle = (pthread_t)-1;
		}
		
        ::pthread_exit(&code);
	}
	
    static void* ThreadFunction(void* param)
    {
        Thread* pThread = (Thread*)param;
		
		pThread->threadProc();

        return NULL;
    }

    bool createThread()
    {
        pthread_attr_t attr;
        ::pthread_attr_init( &attr );
        pthread_attr_t* pAttr = NULL;
        if ( 0 != m_stackSize )
        {
            ::pthread_attr_setstacksize( &attr, m_stackSize );
            pAttr = &attr;
        }
        int ret = ::pthread_create( &m_handle, pAttr, ThreadFunction, (void*)this );
        ::pthread_attr_destroy( &attr );
        return ( 0 == ret );
    }

    static void getTimeout(struct timespec *spec, int timer)
    {
        struct timeval current;
        gettimeofday( &current, NULL );
        TIMEVAL_TO_TIMESPEC( &current, spec );
        spec->tv_sec = current.tv_sec + ((timer + current.tv_usec / 1000) / 1000);
        spec->tv_nsec = ((current.tv_usec / 1000 + timer) % 1000) * 1000000;
    }

};

#endif //WIN32

} // end of namespace
////////////////////////////////////////////////////////////////////////////

#endif /* TTHREAD_H_ */
