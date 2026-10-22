/*
 * JsonUtil.cpp
 *
 *  Created on: 2018年3月4日
 *      Author: zhengboyuan
 */

#include "JsonUtil.h"

#include <cstring>

JsonUtil::JsonUtil()
{
}

JsonUtil::~JsonUtil()
{
}

std::ostream& JsonUtil::write(std::ostream& os, const Json::Value& json, char quote)
{
	Json::ValueType valueType = json.type();
	if (valueType == Json::arrayValue)
	{
		// pass
	}
	else if (valueType == Json::objectValue)
	{
		// pass
	}
	else if (valueType == Json::stringValue)
	{
		if (quote != 0)
		{
			os << '\'';
			os << json.asString();
			os << '\'';
		}
		else
		{
			os << json.asString();
		}
	}
	else if (valueType == Json::nullValue)
	{
		os << "null";
	}
	else if (valueType == Json::realValue)
	{
		os << json.asDouble();
	}
	else if (valueType == Json::intValue)
	{
		if (json.isInt())
		{
			os << json.asInt();
		}
		else
		{
			os << json.asInt64();
		}
	}
	else if (valueType == Json::uintValue)
	{
		if (json.isUInt())
		{
			os << json.asUInt();
		}
		else
		{
			os << json.asUInt64();
		}
	}
	else if (valueType == Json::booleanValue)
	{
        int val = json.asBool() ? 1 : 0;
        os << val;
	}
	return os;
}

std::string JsonUtil::joinKey(const Json::Value& json, const std::string& delimiter,
				char quote,
				const std::string& prefix, const std::string& postfix)
{
	std::ostringstream ss;
	if (json.isObject())
	{
		ss << prefix;

		Json::Value::Members members = json.getMemberNames();
		for (size_t i = 0; i < members.size(); ++ i)
		{
			if (i > 0)
			{
				ss << delimiter;
			}

			if (quote == 0)
			{
				ss << members[i];
			}
			else
			{
				ss << quote;
				ss << members[i];
				ss << quote;
			}
		}

		ss << postfix;
	}
	return ss.str();
}

std::string JsonUtil::joinKey(const Json::Value& json, const std::string& delimiter,
		char quote,
		const std::string& prefix, const std::string& postfix, int flags)
{
	std::ostringstream ss;
	if (json.isObject())
	{
		ss << prefix;

		bool ignoreNull = ((flags & kIgnoreNull) != 0);
        //FormatFlag writeFlag = ((flags & kQuoteString) != 0) ? kQuoteString : kNone;

		size_t idx = 0;
		Json::Value::Members members = json.getMemberNames();
		for (size_t i = 0; i < members.size(); ++ i)
		{
			Json::Value value = json.get(members[i], Json::Value::null);
			if (value.isNull() && ignoreNull)
			{
				// pass
			}
			else
			{
				if (idx > 0)
				{
					ss << delimiter;
				}

				if (quote == 0)
				{
					ss << members[i];
				}
				else
				{
					ss << quote;
					ss << members[i];
					ss << quote;
				}

				idx ++;
			}
		}

		ss << postfix;
	}
	return ss.str();
}

std::string JsonUtil::joinValue(const Json::Value& json, const std::string& delimiter,
		const std::string& prefix, const std::string& postfix, int flags)
{
	std::ostringstream ss;
	if (json.isObject())
	{
		ss << prefix;

		bool ignoreNull = ((flags & kIgnoreNull) != 0);
		FormatFlag writeFlag = ((flags & kQuoteString) != 0) ? kQuoteString : kNone;

		size_t idx = 0;
		Json::Value::Members members = json.getMemberNames();
		for (size_t i = 0; i < members.size(); ++ i)
		{
			Json::Value value = json.get(members[i], Json::Value::null);
			if (value.isNull() && ignoreNull)
			{
				// pass
			}
			else
			{
				if (idx > 0)
				{
					ss << delimiter;
				}

				write(ss, json[members[i]], writeFlag);
				idx ++;
			}
		}

		ss << postfix;
	}
	return ss.str();
}

std::string JsonUtil::join(const Json::Value& json, const std::string& equalSign, const std::string& delimiter,
		char quote,
		const std::string& prefix, const std::string& postfix, int flags)
{
	std::ostringstream ss;
	if (json.isObject())
	{
		ss << prefix;

		bool ignoreNull = ((flags & kIgnoreNull) != 0);
		FormatFlag writeFlag = ((flags & kQuoteString) != 0) ? kQuoteString : kNone;

		size_t idx = 0;
		Json::Value::Members members = json.getMemberNames();
		for (size_t i = 0; i < members.size(); ++ i)
		{
			Json::Value value = json.get(members[i], Json::Value::null);
			if (value.isNull() && ignoreNull)
			{
				// pass
			}
			else
			{
				if (idx > 0)
				{
					ss << delimiter;
				}

				if (quote == 0)
				{
					ss << members[i];
				}
				else
				{
					ss << quote;
					ss << members[i];
					ss << quote;
				}

				ss << equalSign;
				write(ss, json[members[i]], writeFlag);

				idx ++;
			}
		}

		ss << postfix;
	}
	return ss.str();
}



bool JsonUtil::optGet(const Json::Value& json, const char* name, int& value)
{
	if (!json.isMember(name))
	{
		return false;
	}

	value = json[name].asInt();
	return true;
}


bool JsonUtil::optGet(const Json::Value& json, const char* name, double& value)
{
	if (!json.isMember(name))
	{
		return false;
	}

	value = json[name].asDouble();
	return true;
}


bool JsonUtil::optGet(const Json::Value& json, const char* name, std::string& value)
{
	if (!json.isMember(name))
	{
		return false;
	}

	value = json[name].asString();
	return true;
}


bool JsonUtil::optGet(const Json::Value& json, const char* name, bool& value)
{
	if (!json.isMember(name))
	{
		return false;
	}

	value = json[name].asBool();
	return true;
}



bool JsonUtil::optGet(const Json::Value& json, const char* name, int64_t& value)
{
	if (!json.isMember(name))
	{
		return false;
	}

	value = json[name].asInt64();
	return true;
}

