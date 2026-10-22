#ifndef QTYPENAMES_H
#define QTYPENAMES_H

#include <stddef.h>
#include <string.h>
#include <string>
#include <QString>


template < class T >
class QTypeNames
{
public:
    struct Entry
    {
        T t;
        QString name;
    };

    typedef Entry*  iterator;
    typedef const Entry*  const_iterator;

    QTypeNames():
        m_entryList(),
        m_size()
    {
    }

    QTypeNames(Entry* entryList, size_t size):
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

    const QString& getName(size_t idx) const
    {
        return m_entryList[idx].name;
    }


    QString toString(T t) const
    {
        return toName(t);
    }

    QString toName(T t) const
    {
        for (size_t i = 0; i < m_size; i ++)
        {
            if (t == m_entryList[i].t)
            {
                return m_entryList[i].name;
            }
        }
        return QString();
    }

    bool find(T t, QString& name)
    {
        for (size_t i = 0; i < m_size; i ++)
        {
            if (t == m_entryList[i].t)
            {
                name = m_entryList[i].name;
                return true;
            }
        }
        return false;
    }

    bool parse(const QString& name, T& t) const
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


typedef QTypeNames<int>      QIntTypeNames;


#endif // TYPENAMES_H
