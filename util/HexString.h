#ifndef HEXSTRING_H
#define HEXSTRING_H

#include <QString>
#include <utility>
#include <cctype>

class HexString
{
public:

    static QString toHex(const QByteArray& value)
    {
        return value.toHex();
    }

    static QByteArray fromHex(const QString& str)
    {
        QByteArray ba;

        quint32 data = 0;
        int count = 0;
        for (int i = 0; i < str.size(); ++ i)
        {
            char ch = str[i].toLatin1();

            if (std::isxdigit(ch))
            {
                int num = 0;
                if (std::isdigit(ch))
                {
                    num = (ch - '0');
                }
                else if (ch >= 'a' && ch <= 'f')
                {
                    num = ch - 'a' + 10;
                }
                else
                {
                    num = ch - 'A' + 10;
                }

                if (count == 0)
                {
                    data = num;
                    count ++;
                }
                else
                {
                    data = (data << 4) + num;

                    ba.append((quint8)data);
                    count = 0;
                }
            }
            else
            {
                if (count > 0)
                {
                    ba.append((quint8)data);
                    count = 0;
                }

                if (ch != ' ')
                {
                    break;
                }
            }
        }

        if (count > 0)
        {
            ba.append((quint8)data);
            count = 0;
        }

        return ba;
    }
};


#endif // HEXSTRING_H
