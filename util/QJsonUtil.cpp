#include "QJsonUtil.h"
#include <QVariant>
#include <QFile>


QJsonUtil::QJsonUtil()
{

}


QStringList QJsonUtil::toStringList(const QJsonArray& jsonArray)
{
    QStringList strList;
    for (int i = 0; i < jsonArray.size(); ++ i)
    {
        strList.append(jsonArray[i].toString());
    }
    return strList;
}

QStringList QJsonUtil::toStringList(const QJsonObject& jsonObject, const char* key)
{
    if (jsonObject.contains(key))
    {
        QJsonValue value = jsonObject[key];
        if (value.isArray())
        {
            return toStringList(value.toArray());
        }
    }
    return QStringList();
}

QJsonArray QJsonUtil::toArray(const QStringList& strList)
{
    QJsonArray jsonArray;

    for (int i = 0; i < strList.size(); ++ i)
    {
        QJsonValue value(strList[i]);
        jsonArray.push_back(value);
    }

    return jsonArray;
}

qint64 QJsonUtil::getInt64(const QJsonObject& jsonObject, const char* key)
{
    QJsonValue jsonValue = jsonObject.value(key);
    return jsonValue.toVariant().toLongLong();
}

QString QJsonUtil::toString(const QJsonArray& jsonArray)
{
    QJsonDocument doc(jsonArray);
    return doc.toJson(QJsonDocument::Compact);
}

QString QJsonUtil::toString(const QJsonObject& json)
{
    QJsonDocument doc(json);
    return doc.toJson();
}

bool QJsonUtil::fromString(QJsonArray& jsonArray, const QString& str)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &error);
    jsonArray = doc.array();
    return error.error == QJsonParseError::NoError;
}

bool QJsonUtil::fromString(QJsonArray& jsonArray, const std::string& str)
{
	QByteArray ba(str.c_str(), str.size());
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(ba, &error);
	jsonArray = doc.array();
	return error.error == QJsonParseError::NoError;
}

bool QJsonUtil::fromString(QJsonObject& jsonObject, const QString& str)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &error);
    jsonObject = doc.object();
    return error.error == QJsonParseError::NoError;
}

bool QJsonUtil::fromString(QJsonObject& jsonObject, const std::string& str)
{
	QByteArray ba(str.c_str(), str.size());
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(ba, &error);
	jsonObject = doc.object();
	return error.error == QJsonParseError::NoError;
}

bool QJsonUtil::load(QJsonObject& json, const QString& filepath)
{
    QFile loadFile(filepath);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray ba = loadFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(ba);
    json = doc.object();
    return true;
}

bool QJsonUtil::load(QJsonArray& json, const QString& filepath)
{
    QFile loadFile(filepath);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray ba = loadFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(ba);
    json = doc.array();
    return true;
}

bool QJsonUtil::save(const QJsonObject& jsonObject, const QString& filepath)
{
    QFile saveFile(filepath);
    if (!saveFile.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QJsonDocument saveDoc(jsonObject);
    saveFile.write(saveDoc.toJson());
    saveFile.close();
    return true;
}

bool QJsonUtil::save(const QJsonArray& jsonArray, const QString& filepath)
{
    QFile saveFile(filepath);
    if (!saveFile.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QJsonDocument saveDoc(jsonArray);
    saveFile.write(saveDoc.toJson());
    saveFile.close();
    return true;
}

