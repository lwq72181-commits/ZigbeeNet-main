/*
 * TStringJoiner.h
 *
 *  Created on: 2017年3月5日
 *      Author: chuanjiang.zh
 */

#ifndef TSTRINGJOINER_H_
#define TSTRINGJOINER_H_

#include <string>
#include <sstream>
#include <cstring>

namespace comn
{

class StringJoiner
{
public:
	StringJoiner(const std::string& prefix, const std::string& seperator):
		m_oss(),
		m_size(),
		m_seperator(seperator),
		m_prefix(prefix)
	{
	}

	~StringJoiner()
	{
	}

	size_t add(const std::string& str)
	{
        if (str.empty())
        {
            return m_size;
        }
		
		if (m_size == 0)
		{
			m_oss << m_prefix << " ";
		}

		if (m_size > 0)
		{
			
			m_oss << m_seperator;
		}

		m_oss << str;

		m_size ++;
		return m_size;
	}

	size_t add(const std::string& prefix, const char* value)
	{
		if (m_size == 0)
		{
			m_oss << m_prefix << " ";
		}

		if (m_size > 0)
		{
			m_oss << m_seperator;
		}

		m_oss << prefix;
		m_oss << '\'' << value << '\'';

		m_size++;
		return m_size;
	}

	size_t add(const std::string& prefix, const std::string& value)
	{
		if (m_size == 0)
		{
			m_oss << m_prefix << " ";
		}

		if (m_size > 0)
		{
			m_oss << m_seperator;
		}

		m_oss << prefix;
		m_oss << '\'' << value << '\'';

		m_size++;
		return m_size;
	}

	template < class T >
	size_t add(const std::string& prefix, const T& t)
	{
		if (m_size == 0)
		{
			m_oss << m_prefix << " ";
		}

		if (m_size > 0)
		{
			m_oss << m_seperator;
		}

		m_oss << prefix;
		m_oss << t;

		m_size ++;
		return m_size;
	}

	size_t size() const
	{
		return m_size;
	}

	bool empty() const
	{
		return size() == 0;
	}

	std::string str()
	{
		return m_oss.str();
	}


protected:
	std::ostringstream m_oss;
	size_t	m_size;
	std::string	m_seperator;
	std::string m_prefix;
};


}


#endif /* TSTRINGJOINER_H_ */
