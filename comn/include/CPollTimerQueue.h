/*
 * CPollTimerQueue.h
 *
 *  Created on: 2018年5月31日
 *      Author: zhengboyuan
 */

#ifndef CPOLLTIMERQUEUE_H_
#define CPOLLTIMERQUEUE_H_

#include "PollTimerQueue.h"
#include <list>
#include <atomic>
#include <mutex>


namespace comn
{

struct PollTimerTask
{
	int m_id;
	PollTimerHandler   m_handler;
    uint32_t  m_interval;
    bool      m_isRepeat;
    int64_t   m_expire;

    PollTimerTask():
        m_id(),
        m_handler(),
        m_interval(0),
        m_isRepeat(false),
        m_expire(0)
    {
    }

    PollTimerTask(int id, PollTimerHandler handler, uint32_t interval, bool repeat):
        m_id(id),
        m_handler(handler),
        m_interval(interval),
        m_isRepeat(repeat),
        m_expire(interval)
    {
    }

    int64_t updateExpire()
    {
        m_expire += m_interval;
        return m_expire;
    }

    int64_t updateExpire(int64_t timeBase)
    {
        m_expire = m_interval + timeBase;
        return m_expire;
    }

    bool operator < (const PollTimerTask& task) const
    {
        return (m_expire < task.m_expire);
    }

};

class CPollTimerQueue : public PollTimerQueue
{
public:
    static const int TIMER_UNIT = 1000;

    static const uint32_t DEVIATION = 10;   /// 计时器误差, 毫秒.

public:
    CPollTimerQueue();
    virtual ~CPollTimerQueue();

    virtual int poll();

    virtual int addTimer(uint32_t ms, PollTimerHandler handler);

    virtual int addElapse(uint32_t ms, PollTimerHandler handler);

    virtual bool cancel(int id);

    /**
     * cancel all timer
     */
    virtual void clear();

protected:
    typedef std::list< PollTimerTask >  TimerTaskList;
    typedef TimerTaskList::iterator iterator;

    iterator find(int id);
    bool remove(int id);
    iterator findWithExpire(int64_t expire);

    int64_t getCurTime();
    bool add(PollTimerTask& task);
    void insertTask(PollTimerTask& task);
    bool runTask(PollTimerTask& task);

    int64_t checkExpire();

    bool checkoutFront(PollTimerTask& task, int64_t& timeout);

	int makeID();

protected:
	std::mutex	m_mutex;

    TimerTaskList     m_taskList;

    std::atomic_int	m_nextID;

};


} /* namespace util */

#endif /* CPOLLTIMERQUEUE_H_ */
