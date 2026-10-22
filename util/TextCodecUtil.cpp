#include "TextCodecUtil.h"
#include <QTextCodec>
#include "TSafeStr.h"


TextCodecUtil::TextCodecUtil()
{

}

std::string TextCodecUtil::convert(const QString& str)
{
#ifdef WIN32
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    QByteArray ba = codec->fromUnicode(str);
    return std::string(ba.data(), ba.size());
#else
    return str.toStdString();
#endif //
}



QString TextCodecUtil::convert(const std::string& str)
{
    return convert(str.c_str());
}

QString TextCodecUtil::convert(const char* str)
{
#ifdef WIN32
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    return codec->toUnicode(str);
#else
    return QString::fromStdString(str);
#endif //
}

void TextCodecUtil::toGbk(const QString &str, char* buf, int size)
{
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    QByteArray ba = codec->fromUnicode(str);
    comn::copyStr(buf, size, ba.data(), ba.size());
}

std::string TextCodecUtil::toGbk(const QString &str)
{
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    QByteArray ba = codec->fromUnicode(str);
    return std::string(ba.data(), ba.size());
}

std::string TextCodecUtil::toGbk(const QByteArray &ba)
{
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    QByteArray gbkByteArray = codec->fromUnicode((const QChar*)ba.data(), ba.size());
    return std::string(gbkByteArray.data(), gbkByteArray.size());
}

QString TextCodecUtil::fromGbk(const char* str)
{
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    return codec->toUnicode(str);
}

QString TextCodecUtil::fromGbk(const std::string& str)
{
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    return codec->toUnicode(str.c_str());
}

QString TextCodecUtil::fromGbk(const QByteArray& ba)
{
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    return codec->toUnicode(ba);
}
