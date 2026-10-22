/*
 * TSortedList.h
 *
 *  Created on: 2013年7月24日
 *      Author: zhengchuanjiang
 */

#ifndef RTP_TSORTEDLIST_H_
#define RTP_TSORTEDLIST_H_


#include <list>
#include <algorithm>
#include <functional>

namespace comn
{

template < class T, class Comparator = typename std::less< T > >
class SortedList
{
public:
    typedef typename std::list< T >     List;
    typedef typename List::value_type    value_type;
    typedef typename List::size_type     size_type;
    typedef typename List::reference     reference;
    typedef typename List::const_reference     const_reference;

    typedef typename List::iterator      iterator;
    typedef typename List::const_iterator    const_iterator;

private:
    List    m_list;
    Comparator  m_cmp;

public:
    SortedList()
    {
    }

    explicit SortedList(size_type size):
            m_list(size)
    {
    }

    ~SortedList()
    {

    }

    bool empty() const
    {
        return m_list.empty();
    }

    size_t  size() const
    {
        return m_list.size();
    }

    size_type max_size() const
    {
        return m_list.max_size();
    }

    const_iterator begin() const
    {
        return m_list.begin();
    }

    iterator begin()
    {
        return m_list.begin();
    }

    const_iterator end() const
    {
        return m_list.end();
    }

    iterator end()
    {
        return m_list.end();
    }


    void insert(const value_type& x)
    {
        iterator it = upper_bound(begin(), end(), x, m_cmp);
        m_list.insert(it, x);
    }

    bool pop(reference ref)
    {
        if (m_list.empty())
        {
            return false;
        }

        ref = m_list.front();
        m_list.pop_front();
        return true;
    }

    bool pop()
    {
        if (m_list.empty())
        {
            return false;
        }

        m_list.pop_front();
        return true;
    }

    void remove(const value_type& x)
    {
        m_list.remove(x);
    }

    template<class Predicate>
    void remove_if(Predicate p)
    {
        m_list.remove_if(p);
    }


    const_reference front() const
    {
        return m_list.front();
    }

    reference front()
    {
        return m_list.front();
    }

    reference back()
    {
        return m_list.back();
    }

    const_reference back() const
    {
        return m_list.back();
    }

    void pop_front()
    {
        m_list.pop_front();
    }

    void pop_back()
    {
        m_list.pop_back();
    }

    void clear()
    {
        m_list.clear();
    }

    void unique()
    {
        m_list.unique();
    }

    iterator erase(iterator it)
    {
        return m_list.erase(it);
    }

    iterator erase(iterator first, iterator last)
    {
        return m_list.erase(first, last);
    }

	bool replace(const value_type& x)
	{
		bool found = false;
		iterator it = upper_bound(begin(), end(), x, m_cmp);
		if (it != end())
		{
			if (equal(*it, x))
			{
				*it = x;
				found = true;
			}
		}

		if (!found)
		{
			m_list.insert(it, x);
		}
		return found;
	}

private:

	bool equal(const value_type& element, const value_type& v) const
	{
		return !m_cmp(element, v) && !m_cmp(v, element);
	}

};



}

#endif /* RTP_TSORTEDLIST_H_ */
