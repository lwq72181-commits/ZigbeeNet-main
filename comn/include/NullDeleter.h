/*
 * NullDeleter.h
 *
 *  Created on: 2018年8月13日
 *      Author: zhengboyuan
 */

#ifndef NULLDELETER_H_
#define NULLDELETER_H_

namespace comn
{

struct NullDeleter
{
    //! Function object result type
    typedef void result_type;
    /*!
     * Does nothing
     */
    template< typename T >
    void operator() (T*) const
    {
    }
};


} /* namespace comn */

#endif /* NULLDELETER_H_ */
