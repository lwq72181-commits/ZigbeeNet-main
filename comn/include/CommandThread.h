/*
 * CommandThread.h
 *
 *  Created on: 2016年3月18日
 *      Author: terry
 */

#ifndef COMMANDTHREAD_H_
#define COMMANDTHREAD_H_

#include "BasicType.h"
#include "TThread.h"
#include "TEvent.h"
#include "TCriticalSection.h"
#include <queue>
#include <string>

namespace comn
{


struct Param
{
	enum Type
	{
		kBool,
		kInt,
		kLong,
		kFloat,
		kDouble,
		kString,
		kPointer,
	};

	Type type;

	union Value
	{
		bool b;
		int i32;
		int64_t i64;
		float f;
		double d;
		void* ptr;
	} value;

	std::string      str;

	Param();
	explicit Param(bool value);
	explicit Param(int32_t value);
	explicit Param(int64_t value);
	explicit Param(float value);
	explicit Param(double value);
	explicit Param(const char* value);
	explicit Param(const std::string& value);
	explicit Param(void* value);

	bool toBool() const;
	int  toInt() const;
	int64_t toLong() const;
	float toFloat() const;
	double toDouble() const;
	std::string toString() const;
	void* toPtr() const;


};


class CommandThread : public comn::Thread
{
public:
	enum CommandType
	{
		kQuit = -1,
		kNone = 0,
	};

	static const int MAX_PARAMS = 4;
	static Param s_param;

	struct Command
	{
		int type;
		int size;
		Param params[MAX_PARAMS];

		explicit Command(int t=kNone):
			type(t),
			size()
		{
		}

		Command(int t, const Param& p1):
			type(t),
			size()
		{
			add(p1);
		}

		Command(int t, const Param& p1, const Param& p2):
			type(t),
			size()
		{
			add(p1);
			add(p2);
		}

		void add(const Param& param)
		{
			if (size < MAX_PARAMS)
			{
				params[size] = param;
				size ++;
			}
		}

		void clear()
		{
			size = 0;
		}

		Param& at(int idx)
		{
			if (idx >= 0 && idx < size)
			{
				return params[idx];
			}
			return CommandThread::s_param;
		}
	};

	typedef std::queue< Command >	CommandQueue;

	enum Constant
	{
		IDLE_DURATION = 1000 * 10,	/// millisecond
	};

public:
	CommandThread();
	virtual ~CommandThread();

	void postCommand(const Command& cmd);

	void postCommand(int cmdType);

	template < class T >
	void postCommand(int cmdType, T t)
	{
		Param param(t);
		Command cmd(cmdType, param);
		postCommand(cmd);
	}

	template < class T , class E >
	void postCommand(int cmdType, T t, E e)
	{
		Param param(t);
		Param ext(e);
		Command cmd(cmdType, param, ext);
		postCommand(cmd);
	}


	bool getCommand(Command& cmd);

	void clearQueue();

	void postEvent();

	bool timedwait(int ms);

	void stop();

protected:

	virtual void onCommand(Command& cmd) =0;
	virtual void onIdle();

	bool fetchAndHandle();

protected:
	virtual int run();
	virtual void doStop();

protected:
	comn::CriticalSection	m_csCmd;
	comn::Event		m_eventCmd;
	CommandQueue	m_cmdQueue;


};



} /* namespace util */

#endif /* COMMANDTHREAD_H_ */
