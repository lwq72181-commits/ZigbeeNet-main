#ifndef TEXTCODECUTIL_H
#define TEXTCODECUTIL_H

#include <string>
#include <QString>

class TextCodecUtil
{
public:
    TextCodecUtil();

    static std::string convert(const QString& str);

    static QString convert(const std::string& str);

    static QString convert(const char* str);


    static void toGbk(const QString &str, char* buf, int size);
    static std::string toGbk(const QString &str);
    static std::string toGbk(const QByteArray &ba);

    static QString fromGbk(const char* str);
    static QString fromGbk(const std::string& str);
    static QString fromGbk(const QByteArray& ba);

};

#endif // TEXTCODECUTIL_H
