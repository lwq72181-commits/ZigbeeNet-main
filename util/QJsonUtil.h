#ifndef QJSONUTIL_H
#define QJSONUTIL_H

#include "BasicType.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include "QVariantCast.h"


class QJsonUtil
{
public:
    QJsonUtil();

    static QStringList toStringList(const QJsonArray& jsonArray);
    static QStringList toStringList(const QJsonObject& jsonObject, const char* key);

    static QJsonArray toArray(const QStringList& strList);

    static qint64 getInt64(const QJsonObject& jsonObject, const char* key);

    static QString toString(const QJsonArray& jsonArray);
    static QString toString(const QJsonObject& json);

    static bool fromString(QJsonArray& jsonArray, const QString& str);
    static bool fromString(QJsonObject& jsonObject, const QString& str);
	static bool fromString(QJsonArray& jsonArray, const std::string& str);
	static bool fromString(QJsonObject& jsonObject, const std::string& str);

    static bool load(QJsonObject& json, const QString& filepath);
    static bool load(QJsonArray& json, const QString& filepath);

    static bool save(const QJsonObject& jsonObject, const QString& filepath);
    static bool save(const QJsonArray& jsonArray, const QString& filepath);

    template < class T>
    static bool get(const QJsonObject& jsonObject, const char* key, T& value)
    {
        bool got = false;
        QJsonValue jsonValue = jsonObject[key];
        if (!jsonValue.isUndefined())
        {
            got = QVariantCast::toValue(jsonValue.toVariant(), value);
        }
        return got;
    }



};

#endif // QJSONUTIL_H
