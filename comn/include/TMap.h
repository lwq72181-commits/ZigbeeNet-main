/*
 * TMap.h
 *
 *  Created on: 2015年7月23日
 *      Author: chuanjiang.zh@qq.com
 */

#ifndef TEST_TMAP_H_
#define TEST_TMAP_H_

#include "TCriticalSection.h"
#include <map>
#include <functional>

namespace comn
{


/**
 * 线程安全Map
 */
template < typename _Key, typename _Tp, typename _Compare = std::less<_Key>,
            typename _Alloc = std::allocator<std::pair<const _Key, _Tp> > >
class Map
{
public:
    typedef typename std::map< _Key, _Tp, _Compare, _Alloc >    BasicMap;

    typedef typename BasicMap::key_type     key_type;
    typedef typename BasicMap::mapped_type  mapped_type;
    typedef typename BasicMap::value_type   value_type;

    typedef typename BasicMap::iterator iterator;

    enum Triple
	{
    	kNone = -1,
    	kFalse = 0,
    	kTrue,
	};
protected:
    comn::CriticalSection   m_cs;
    BasicMap    m_map;


public:
    Map()
    {

    }

    ~Map()
    {

    }

    bool empty()
    {
        comn::AutoCritSec lock(m_cs);
        return m_map.empty();
    }

    size_t size()
    {
        comn::AutoCritSec lock(m_cs);
        return m_map.size();
    }

    bool exist(const key_type& key)
    {
        comn::AutoCritSec lock(m_cs);
        iterator it = m_map.find(key);
        return (it != m_map.end());
    }

    mapped_type& get(const key_type& key)
    {
        comn::AutoCritSec lock(m_cs);
        return m_map[key];
    }

    mapped_type find(const key_type& key)
    {
        mapped_type value = mapped_type();
    	comn::AutoCritSec lock(m_cs);
    	iterator it = m_map.find(key);
		if (it != m_map.end())
		{
			value = it->second;
		}
		return value;
    }

    bool find(const key_type& key, mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        bool found = false;
        iterator it = m_map.find(key);
        if (it != m_map.end())
        {
            value = it->second;
            found = true;
        }
        return found;
    }

    bool findByValue(const mapped_type& value, key_type& key)
    {
        comn::AutoCritSec lock(m_cs);
        bool found = false;
        iterator it = m_map.begin();
        for (; it != m_map.end(); ++ it)
        {
            if (value == it->second)
            {
                key = it->first;
                found = true;
                break;
            }
        }
        return found;
    }

    bool existValue(const mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        bool found = false;
        iterator it = m_map.begin();
        for (; it != m_map.end(); ++ it)
        {
            if (value == it->second)
            {
                found = true;
                break;
            }
        }
        return found;
    }


    /**
     * 不存在就添加到映射表中
     * @param key
     * @param value
     * @return 是否已经插入
     */
    bool insert(const key_type& key, const mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        std::pair<iterator, bool> result = m_map.insert(value_type(key, value));
        return result.second;
    }

    /**
     * 添加到映射表中, 如果key已经存在,对应的value会被替换
     * @param key
     * @param value
     */
    void set(const key_type& key, const mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        m_map[key] = value;
    }

    void assign(Map& map)
    {
    	comn::AutoCritSec lock(map.m_cs);
    	iterator it = map.m_map.begin();
    	for (; it != map.m_map.end(); ++ it)
    	{
    		set(it->first, it->second);
    	}
    }

    void replace(Map& map)
    {
    	clear();
    	assign(map);
    }

    void swap(Map& map)
    {
    	comn::AutoCritSec lock(m_cs);
    	comn::AutoCritSec lock2(map.m_cs);
    	std::swap(m_map, map.m_map);
    }

    /**
     * 添加到映射表中, 并返回原来的值
     * @param key
     * @param value
     * @return
     */
    mapped_type put(const key_type& key, const mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        std::pair<iterator, bool> result = m_map.insert(value_type(key, value));
        if (result.second)
        {
			return result.first->second;
        }
        else
        {
			mapped_type ret(result.first->second);
            result.first->second = value;
			return ret;
        }
    }

    bool remove(const key_type& key)
    {
        comn::AutoCritSec lock(m_cs);
        bool found = false;
        iterator it = m_map.find(key);
        if (it != m_map.end())
        {
            m_map.erase(it);
            found = true;
        }
        return found;
    }

    bool remove(const key_type& key, mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        bool found = false;
        iterator it = m_map.find(key);
        if (it != m_map.end())
        {
            value = it->second;
            m_map.erase(it);

            found = true;
        }
        return found;
    }

    bool removeValue(const mapped_type& value, key_type& key)
	{
		comn::AutoCritSec lock(m_cs);
		bool found = false;
		iterator it = m_map.begin();
		for (; it != m_map.end(); ++ it)
		{
			if (value == it->second)
			{
				key = it->first;
				m_map.erase(it);
				found = true;
				break;
			}
		}
		return found;
	}

    /**
     * 移除头部元素
     * @param key
     * @param value
     * @return true 表示已移除并返回对应的key-value
     */
    bool pop_front(key_type& key, mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        bool found = false;
        iterator it = m_map.begin();
        if (it != m_map.end())
        {
            key = it->first;
            value = it->second;

            m_map.erase(it);
            found = true;
        }
        return found;
    }


    void clear()
    {
        comn::AutoCritSec lock(m_cs);
        m_map.clear();
    }

    bool getAt(size_t idx, key_type& key, mapped_type& value)
    {
    	bool found = false;
    	comn::AutoCritSec lock(m_cs);
    	if (idx < m_map.size())
    	{
    		iterator it = m_map.begin();
    		std::advance(it, idx);
    		if (it != m_map.end())
    		{
    			key = it->first;
    			value = it->second;
    			found = true;
    		}
    	}
    	return found;
    }

    bool getAt(size_t idx, key_type& key)
    {
        bool found = false;
        comn::AutoCritSec lock(m_cs);
        if (idx < m_map.size())
        {
            iterator it = m_map.begin();
            std::advance(it, idx);
            if (it != m_map.end())
            {
                key = it->first;
                found = true;
            }
        }
        return found;
    }

    template < class Functor >
	bool existIf(Functor& func)
	{
		comn::AutoCritSec lock(m_cs);
		bool found = false;
		iterator it = m_map.begin();
		for (; it != m_map.end(); ++ it)
		{
			if (func(it->first, it->second))
			{
				found = true;
				break;
			}
		}
		return found;
	}

    template < class Functor >
    bool findIf(Functor& func, key_type& key, mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        bool found = false;
        iterator it = m_map.begin();
        for (; it != m_map.end(); ++ it)
        {
            if (func(it->first, it->second))
            {
                key = it->first;
                value = it->second;
                found = true;
                break;
            }
        }
        return found;
    }

    template < class Functor >
    bool removeIf(Functor& func, key_type& key, mapped_type& value)
    {
        comn::AutoCritSec lock(m_cs);
        bool found = false;
        iterator it = m_map.begin();
        for (; it != m_map.end(); ++ it)
        {
            if (func(it->first, it->second))
            {
                key = it->first;
                value = it->second;
                m_map.erase(it);

                found = true;
                break;
            }
        }
        return found;
    }

    template < class Functor >
    size_t forEach(Functor& func)
    {
    	size_t count = 0;
        comn::AutoCritSec lock(m_cs);
        iterator it = m_map.begin();
        for (; it != m_map.end(); ++ it)
        {
            if (func(it->first, it->second))
            {
            	count ++;
            }
            else
            {
            	break;
            }
        }
        return count;
    }

    template < class Functor >
    size_t removeAll(Functor& func)
    {
        comn::AutoCritSec lock(m_cs);
        size_t count = 0;
        iterator it = m_map.begin();
        while (it != m_map.end())
        {
            if (func(it->first, it->second))
            {
                count ++;
                m_map.erase(it ++);
            }
            else
            {
                it ++;
            }
        }
        return count;
    }

    template < class Functor, class Container >
    size_t removeAll(Functor& func, Container& cont)
    {
        comn::AutoCritSec lock(m_cs);
        size_t count = 0;
        iterator it = m_map.begin();
        while (it != m_map.end())
        {
            if (func(it->first, it->second))
            {
            	cont.push_back(it->second);
                count ++;
                m_map.erase(it ++);
            }
            else
            {
                it ++;
            }
        }
        return count;
    }

    template < class Container >
    size_t values(Container& cont)
    {
    	comn::AutoCritSec lock(m_cs);
		iterator it = m_map.begin();
		for (; it != m_map.end(); ++ it)
		{
			cont.push_back(it->second);
		}
		return m_map.size();
    }

    template < class Container >
	size_t values(Container& cont, size_t offset, size_t limit)
	{
		comn::AutoCritSec lock(m_cs);
		if (m_map.size() <= offset)
		{
			return 0;
		}

		size_t count = 0;
		iterator it = m_map.begin();
		std::advance(it, offset);
		while (it != m_map.end() && (count < limit))
		{
			cont.push_back(it->second);

			it ++;
			count ++;
		}
		return count;
	}

    template < class Container, class Functor >
	size_t values(Container& cont, size_t offset, size_t limit, Functor& pred)
	{
		comn::AutoCritSec lock(m_cs);
		if (m_map.size() <= offset)
		{
			return 0;
		}

		size_t count = 0;
		iterator it = m_map.begin();
		std::advance(it, offset);
		while (it != m_map.end() && (count < limit))
		{
			if (pred(it->first, it->second))
			{
				cont.push_back(it->second);
				count ++;
			}

			it ++;
		}
		return count;
	}

    /**
     * 分页搜索
     * @param cont	结果容器
     * @param offset	起始索引(满足条件的索引范围内)
     * @param limit		最多获取个数
     * @param pred		判断条件
     * @return		满足条件的总数
     */
    template < class Container, class Functor >
	size_t search(Container& cont, size_t offset, size_t limit, Functor& pred)
	{
		comn::AutoCritSec lock(m_cs);
		if (m_map.size() <= offset)
		{
			return 0;
		}

		size_t total = 0;
		size_t index = 0;

		for (iterator it = m_map.begin(); it != m_map.end(); ++ it)
		{
			if (pred(it->first, it->second))
			{
				if ((index >= offset) && index < (offset + limit))
				{
					cont.push_back(it->second);
				}

				index++;
				total++;
			}
		}

		return total;
	}

    template < class Container >
    size_t keys(Container& cont)
    {
    	comn::AutoCritSec lock(m_cs);
		iterator it = m_map.begin();
		for (; it != m_map.end(); ++ it)
		{
			cont.push_back(it->first);
		}
		return m_map.size();
    }

    size_t keys(key_type* cont, size_t maxSize)
    {
        comn::AutoCritSec lock(m_cs);
        size_t index = 0;
        iterator it = m_map.begin();
        for (; it != m_map.end(); ++it)
        {
            if (index < maxSize)
            {
                cont[index] = it->first;
                index ++;
            }
            else
            {
                break;
            }
        }
        return index;
    }

    template < class Functor, class Container >
    size_t findAll(Functor& func, Container& cont)
    {
        comn::AutoCritSec lock(m_cs);
        size_t count = 0;
        iterator it = m_map.begin();
        for (; it != m_map.end(); ++ it)
        {
            if (func(it->first, it->second))
            {
            	cont.push_back(it->second);
            	count ++;
            }
        }
        return count;
    }

    template < class Functor >
    Triple checkRemove(const key_type& key, mapped_type& value, Functor& check)
    {
    	Triple state = kFalse;
    	comn::AutoCritSec lock(m_cs);
		iterator it = m_map.find(key);
		if (it != m_map.end())
		{
			if (check(it->first, it->second))
			{
				value = it->second;
				state = kTrue;
			}
			else
			{
				m_map.erase(it);
				state = kNone;
			}
		}
		return state;
    }

    template < class Functor >
    bool replace(mapped_type& value, Functor& check)
    {
    	bool found = false;
    	comn::AutoCritSec lock(m_cs);
    	iterator it = m_map.begin();
		for (; it != m_map.end(); ++ it)
		{
			if (check(it->first, it->second))
			{
				it->second = value;
				found = true;
				break;
			}
		}
		return found;
    }

    template < class Functor >
    bool findDo(const key_type& key, Functor& func)
    {
        bool found = false;
        comn::AutoCritSec lock(m_cs);
        iterator it = m_map.find(key);
        if (it != m_map.end())
        {
            func(it->second);
            found = true;
        }
        return found;
    }

    template < class Functor, class Container >
    size_t findAllKey(Functor& func, Container& cont)
    {
        comn::AutoCritSec lock(m_cs);
        size_t count = 0;
        iterator it = m_map.begin();
        for (; it != m_map.end(); ++ it)
        {
            if (func(it->first, it->second))
            {
            	cont.push_back(it->first);
            	count ++;
            }
        }
        return count;
    }

};






}

#endif /* TEST_TMAP_H_ */
