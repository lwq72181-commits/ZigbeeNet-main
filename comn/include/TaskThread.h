/*
 * TaskThread.h
 *
 *  Created on: 2020年8月22日
 *      Author: chuanjiang.zh
 */

#ifndef TASKTHREAD_H_
#define TASKTHREAD_H_

#include "BasicType.h"
#include "TThread.h"
#include <functional>
#include <deque>
#include <mutex>
#include "TEvent.h"
#include "PollTimerQueue.h"

namespace comn
{


class TaskThread : public comn::Thread
{
public:
	typedef std::function< void () >	Task;
	typedef std::deque< Task >			TaskQueue;

	typedef std::function< void (int) >	TimerTask;

	enum Constant
	{
		IDLE_DURATION = 1000 * 10,	/// millisecond
	};

public:
	TaskThread();
	virtual ~TaskThread();

	void post(Task task);
	void clearTask();

	int addTimer(uint32_t ms, TimerTask task);
	int addElapse(uint32_t ms, TimerTask task);

	void cancelTimer(int tid);
	void clearTimer();

	void postEvent();

protected:
	virtual void onIdle();

	virtual int run();
	virtual void doStop();

private:
	TaskThread(const TaskThread&) = delete;
	TaskThread& operator = (const TaskThread&) = delete;

protected:
	bool fetchAndHandle();
	bool fetchTask(Task& task);

	int poll();

	bool timedwait(int ms);

protected:
	std::mutex		m_mutex;
	comn::Event		m_eventTask;
	TaskQueue		m_taskQueue;

	PollTimerQueue*	m_timerQueue;


};


}

#endif /* TASKTHREAD_H_ */
