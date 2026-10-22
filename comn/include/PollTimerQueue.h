/*
 * PollTimerQueue.h
 *
 *  Created on: 2020年4月15日
 *      Author: chuanjiang.zh
 */

#ifndef POLLTIMERQUEUE_H_
#define POLLTIMERQUEUE_H_

#include "BasicType.h"
#include <string>
#include <time.h>
#include <functional>


namespace comn
{


typedef std::function<void(int tid)>  PollTimerHandler;


class PollTimerQueue
{
public:

	static const int INVALID_ID = -1;

public:
    virtual ~PollTimerQueue() {}

    /**
     * 返回下一个计时器到达的时间, 单位为毫秒.
     * @return
     */
    virtual int poll() =0;

    virtual int addTimer(uint32_t ms, PollTimerHandler handler) =0;

    virtual int addElapse(uint32_t ms, PollTimerHandler handler) =0;

    virtual bool cancel(int id) =0;

    /**
     * cancel all timer
     */
    virtual void clear() =0;

};


} /* namespace comn */


#endif /* POLLTIMERQUEUE_H_ */
