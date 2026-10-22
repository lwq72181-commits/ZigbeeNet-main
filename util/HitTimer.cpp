/*
 * HitTimer.cpp
 *
 *  Created on: 2019年7月24日
 *      Author: terry
 */

#include "HitTimer.h"
#include <QDateTime>


HitTimer::HitTimer():
    m_start(),
    m_duration(MIN_DURATION)
{
    reset();
}

HitTimer::~HitTimer()
{
}

void HitTimer::reset()
{
    m_start = clock();
}

bool HitTimer::hitTest()
{
    int64_t curTime = clock();
    int64_t elapse = (curTime - m_start);
    if (elapse >= m_duration)
    {
        m_start = curTime;
        return true;
    }
    return false;
}

int64_t HitTimer::getStart() const
{
    return m_start;
}

void HitTimer::setMinDuration(uint32_t ms)
{
    m_duration = ms;
}

int64_t HitTimer::clock()
{
    return QDateTime::currentMSecsSinceEpoch();
}
