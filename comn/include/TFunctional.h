/*
 * TFunctional.h
 *
 *  Created on: 2018年6月20日
 *      Author: zhengboyuan
 */

#ifndef TFUNCTIONAL_H_
#define TFUNCTIONAL_H_

#include <functional>

#if __cplusplus >= 201103L
	// c++ 11
#else
	
	#if defined(__GNUC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)
	#include <tr1/functional>
	namespace std
	{
		using std::tr1::function;
		using std::tr1::bind;

		namespace placeholders
		{
			using std::tr1::placeholders::_1;
			using std::tr1::placeholders::_2;
			using std::tr1::placeholders::_3;
		}
	}
	#endif //

#endif //

#endif /* TFUNCTIONAL_H_ */
