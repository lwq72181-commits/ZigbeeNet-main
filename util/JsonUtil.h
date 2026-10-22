/*
 * JsonUtil.h
 *
 *  Created on: 2018年3月4日
 *      Author: chuanjiang.zh@qq.com
 */

#ifndef UTIL_JSONUTIL_H_
#define UTIL_JSONUTIL_H_

#include "BasicType.h"
#include "json/json.h"
#include <memory>
#include <sstream>

typedef std::shared_ptr<Json::StreamWriter>  StreamWriterPtr;


class JsonUtil
{
public:
	enum FormatFlag
	{
		kNone = 0,
		kIgnoreNull  = 0x01,
		kQuoteString = 0x02
	};

public:
	JsonUtil();
	virtual ~JsonUtil();

	template <class JsonList>
	static std::string toString(const JsonList& jsonList)
	{
		std::ostringstream sout;
		Json::StreamWriterBuilder builder;
		StreamWriterPtr const writer(builder.newStreamWriter());

		sout << "[";

		int idx = 0;
		typename JsonList::const_iterator it = jsonList.begin();
		for (; it != jsonList.end(); ++ it)
		{
			if (idx > 0)
			{
				sout << ",";
			}

			writer->write(*it, &sout);
			idx ++;
		}

		sout << "]";

		return sout.str();
	}

	static std::ostream& write(std::ostream& os, const Json::Value& json, char quote);

	static std::string joinKey(const Json::Value& json, const std::string& delimiter,
				char quote,
				const std::string& prefix, const std::string& postfix);

	static std::string joinKey(const Json::Value& json, const std::string& delimiter,
			char quote,
			const std::string& prefix, const std::string& postfix, int flags);

	static std::string joinValue(const Json::Value& json, const std::string& delimiter,
			const std::string& prefix, const std::string& postfix, int flags);

	static std::string join(const Json::Value& json, const std::string& equalSign, const std::string& delimiter,
			char quote,
			const std::string& prefix, const std::string& postfix, int flags);

	static bool optGet(const Json::Value& json, const char* name, int& value);
	static bool optGet(const Json::Value& json, const char* name, double& value);
	static bool optGet(const Json::Value& json, const char* name, std::string& value);
	static bool optGet(const Json::Value& json, const char* name, bool& value);
	static bool optGet(const Json::Value& json, const char* name, int64_t& value);


};

#endif /* UTIL_JSONUTIL_H_ */
