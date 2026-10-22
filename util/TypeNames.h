#ifndef TYPENAMES_H
#define TYPENAMES_H

#include <stddef.h>
#include <string.h>
#include <string>


template < class T >
class TypeNames
{
public:
    struct Entry
    {
        T t;
        const char* name;
    };

    typedef Entry*  iterator;
    typedef const Entry*  const_iterator;

    TypeNames():
        m_entryList(),
        m_size()
    {
    }

    TypeNames(Entry* entryList, size_t size):
        m_entryList(entryList),
        m_size(size)
    {
    }

    size_t size() const
    {
        return m_size;
    }

    const Entry& at(size_t idx) const
    {
        return m_entryList[idx];
    }

    const_iterator begin() const
    {
        return m_entryList;
    }

    const_iterator end() const
    {
        return m_entryList + m_size;
    }

    T getType(size_t idx) const
    {
        return m_entryList[idx].t;
    }

    const char* getName(size_t idx) const
    {
        return m_entryList[idx].name;
    }



    std::string toString(T t) const
    {
        return toName(t);
    }

    const char* toName(T t) const
    {
        for (size_t i = 0; i < m_size; i ++)
        {
            if (t == m_entryList[i].t)
            {
                return m_entryList[i].name;
            }
        }
        return "";
    }

    bool parse(const char* name, T& t) const
    {
        bool found = false;
        for (size_t i = 0; i < m_size; i ++)
        {
            if (strcmp(name, m_entryList[i].name) == 0)
            {
                t = m_entryList[i].t;
                found = true;
                break;
            }
        }
        return found;
    }

    bool parse(const std::string& name, T& t) const
    {
        bool found = false;
        for (size_t i = 0; i < m_size; i ++)
        {
            if (name == m_entryList[i].name)
            {
                t = m_entryList[i].t;
                found = true;
                break;
            }
        }
        return found;
    }

protected:
    Entry*  m_entryList;
    size_t  m_size;

};


typedef TypeNames<int>      IntTypeNames;


#endif // TYPENAMES_H
