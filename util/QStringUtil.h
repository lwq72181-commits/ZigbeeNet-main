#ifndef QSTRINGUTIL_H
#define QSTRINGUTIL_H

#include <QString>
#include <string>
#include <sstream>

class QStringUtil
{
public:
    QStringUtil()
    {
    }

    template < class T >
    static void toValue( const QString& str, T& t )
    {
        std::istringstream iss(str.toStdString());
        iss >> t;
    }

    static void toValue(const QString& str, bool& value)
    {
        value = toBool(str);
    }

    static bool toBool(const QString& str)
    {
        if (str.isEmpty())
        {
            return false;
        }

        if (str == "0" || str == "false" || str == "False" || str == "FALSE")
        {
            return false;
        }
        return true;
    }


    static QString getHead(const QString& src, char ch)
    {
        int index = src.indexOf(ch);
        if (index >= 0)
        {
            return src.mid(0, index);
        }
        return src;
    }

    static QString getTail(const QString& src, char ch)
    {
        int pos = src.indexOf(ch);
        if (pos >= 0)
        {
            return src.mid(pos+1);
        }
        return QString();
    }

    static QString getHead(const QString& src, const QString& seperator)
    {
        int pos = src.indexOf(seperator);
        if (pos >= 0)
        {
            return src.mid(0, pos);
        }
        return src;
    }

    static QString getTail(const QString& src, const QString& seperator)
    {
        int pos = src.indexOf(seperator);
        if (pos >= 0)
        {
            return src.mid(pos+seperator.size());
        }
        return QString();
    }

    static bool split(const QString& src, char ch, QString& head, QString& tail)
    {
        bool found = false;
        int pos = src.indexOf(ch);
        if (pos >= 0)
        {
            head = src.mid(0, pos);
            tail = src.mid(pos+1);
            found = true;
        }
        else
        {
            head = src;
            tail.clear();
        }
        return found;
    }

    template < class Head, class Tail >
    static bool split(const QString& src, char ch, Head& h, Tail& t)
    {
        QString head;
        QString tail;
        if (!split(src, ch, head, tail))
        {
            return false;
        }

        toValue(head, h);
        toValue(tail, t);

        return true;
    }

    static bool split(const QString& src, const QString& seperator, QString& head, QString& tail)
    {
        bool found = false;
        int pos = src.indexOf(seperator);
        if (pos >= 0)
        {
            head = src.mid(0, pos);
            tail = src.mid(pos+1);
            found = true;
        }
        else
        {
            head = src;
            tail.clear();
        }
        return found;
    }

    template < class Head, class Tail >
    static bool split(const QString& src, const QString& seperator, Head& h, Tail& t)
    {
        QString head;
        QString tail;
        if (!split(src, seperator, head, tail))
        {
            return false;
        }

        toValue(head, h);
        toValue(tail, t);

        return true;
    }

    static bool rsplit(const QString& src, char ch, QString& head, QString& tail)
    {
        bool found = false;
        int pos = src.lastIndexOf(ch);
        if (pos >= 0)
        {
            head = src.mid(0, pos);
            tail = src.mid(pos+1);
            found = true;
        }
        else
        {
            head = src;
            tail.clear();
        }
        return found;
    }

    static bool rsplit(const QString& src, const QString& seperator, QString& head, QString& tail)
    {
        bool found = false;
        int pos = src.lastIndexOf(seperator);
        if (pos >= 0)
        {
            head = src.mid(0, pos);
            tail = src.mid(pos+1);
            found = true;
        }
        else
        {
            head = src;
            tail.clear();
        }
        return found;
    }



    template<class Container>
    static int split(const QString& src, char ch, Container& vect)
    {
        if (src.isEmpty())
            return 0;

        int count = 0;

        int begin = 0;
        int pos = src.indexOf(ch, begin);
        while (pos >= 0)
        {
            QString substr;
            if (pos - begin > 0)
            {
                substr = src.mid(begin, pos - begin);
            }
            vect.push_back(substr);
            count++;

            begin = pos + 1;
            pos = src.indexOf(ch, begin);
        }

        if (begin < src.size())
        {
            vect.push_back(src.mid(begin));
            count++;
        }
        return count;
    }

    template <class Container>
    static int split(const QString& src, const QString& sp, Container& cont)
    {
        if (src.isEmpty())
            return 0;

        int count = 0;

        int begin = 0;
        int pos = src.indexOf(sp, begin);
        while (pos >= 0)
        {
            QString substr;
            if (pos - begin > 0)
            {
                substr = src.mid(begin, pos - begin);
            }
            cont.push_back(substr);
            count++;

            begin = pos + sp.size();
            pos = src.indexOf(sp, begin);
        }

        if (begin < src.size())
        {
            cont.push_back(src.mid(begin));
            count++;
        }
        return count;
    }


    static QString extract(const QString& src, int start, char chBegin, char chEnd, bool fullMatch)
    {
        QString substr;
        int posBegin = src.indexOf(chBegin, start);
        if (posBegin < 0)
        {
            return substr;
        }

        int posEnd = src.indexOf(chEnd, posBegin + 1);
        if (posEnd < 0)
        {
            if (!fullMatch)
            {
                substr = src.mid(posBegin+1);
            }
        }
        else
        {
            substr = src.mid(posBegin+1, posEnd - posBegin - 1);
        }
        return substr;
    }

    static QString extract(const QString& src, int start, const char* strBegin,
        const char* strEnd, bool fullMatch)
    {
        QString substr;
        int posBegin = src.indexOf(strBegin, start);
        if (posBegin < 0)
        {
            return substr;
        }

        int beginSize = strlen(strBegin);

        int posEnd = src.indexOf(strEnd, posBegin + beginSize);
        if (posEnd < 0)
        {
            if (!fullMatch)
            {
                substr = src.mid(posBegin+beginSize);
            }
        }
        else
        {
            substr = src.mid(posBegin+beginSize, posEnd - posBegin - beginSize);
        }
        return substr;
    }


};

#endif // QSTRINGUTIL_H
