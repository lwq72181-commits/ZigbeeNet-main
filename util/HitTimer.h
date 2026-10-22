/*
 * HitTimer.h
 *
 *  Created on: 2019年7月24日
 *      Author: terry
 */

#ifndef HITTIMER_H_
#define HITTIMER_H_

#include "BasicType.h"


class HitTimer
{
public:
    /// msec
    static const uint32_t   MIN_DURATION = 500;

public:
    HitTimer();
    virtual ~HitTimer();

    void reset();

    bool hitTest();

    int64_t getStart() const;

    void setMinDuration(uint32_t ms);

protected:
    /// get clock in msec
    int64_t clock();

protected:
    int64_t     m_start;
    uint32_t    m_duration;

};

#endif /* HITTIMER_H_ */
