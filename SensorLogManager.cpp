#include "SensorLogManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QProcess>

SensorLogManager::SensorLogManager(QObject *parent)
    : QObject(parent)
{
    QMutexLocker locker(&m_mutex);
    m_rootObj["IndexInfo"] = QJsonArray();
    m_rootObj["WaveInfo"] = QJsonArray();
}

SensorLogManager::~SensorLogManager()
{
    // 析构时，如果还有锁未释放，直接保存（但不调用公共 save，避免虚函数调用）
    QMutexLocker locker(&m_mutex);
    if (!m_filePath.isEmpty()) {
        QJsonObject wholeObj;
        wholeObj["SensorLog"] = m_rootObj;
        QJsonDocument doc(wholeObj);
        QFile file(m_filePath);
        if (file.open(QIODevice::WriteOnly))
            file.write(doc.toJson(QJsonDocument::Indented));
    }
}

bool SensorLogManager::load(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);
    m_filePath = filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_rootObj["IndexInfo"] = QJsonArray();
        m_rootObj["WaveInfo"] = QJsonArray();
        QJsonObject wholeObj;
        wholeObj["SensorLog"] = m_rootObj;
        QJsonDocument doc(wholeObj);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
        }
        return true;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }

    if (!doc.isObject()) return false;

    QJsonObject wholeObj = doc.object();
    if (!wholeObj.contains("SensorLog") || !wholeObj["SensorLog"].isObject()) {
        qWarning() << "Invalid format: missing SensorLog object";
        return false;
    }

    m_rootObj = wholeObj["SensorLog"].toObject();
    if (!m_rootObj.contains("IndexInfo")) m_rootObj["IndexInfo"] = QJsonArray();
    if (!m_rootObj.contains("WaveInfo")) m_rootObj["WaveInfo"] = QJsonArray();

    return true;
}

bool SensorLogManager::save()
{
    QMutexLocker locker(&m_mutex);
    if (m_filePath.isEmpty()) {
        qWarning() << "No file path specified for saving";
        return false;
    }

    QJsonObject wholeObj;
    wholeObj["SensorLog"] = m_rootObj;

    QJsonDocument doc(wholeObj);
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << m_filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

// ---------- 私有辅助函数（调用者必须已持有 m_mutex）----------
bool SensorLogManager::ensureIndexInfoExists(int sn)
{
    if (containsIndexInfo(sn))
        return true;
    return addIndexInfo(sn);
}

bool SensorLogManager::ensureWaveInfoExists(int sn)
{
    if (containsWaveInfo(sn))
        return true;
    return addWaveInfo(sn);
}

QJsonObject SensorLogManager::findIndexInfoObject(int sn) const
{
    QJsonArray array = m_rootObj["IndexInfo"].toArray();
    for (const QJsonValue &val : array) {
        if (val.isObject() && val.toObject()["sn"].toInt() == sn)
            return val.toObject();
    }
    return QJsonObject();
}

QJsonObject SensorLogManager::findWaveInfoObject(int sn) const
{
    QJsonArray array = m_rootObj["WaveInfo"].toArray();
    for (const QJsonValue &val : array) {
        if (val.isObject() && val.toObject()["sn"].toInt() == sn)
            return val.toObject();
    }
    return QJsonObject();
}

bool SensorLogManager::updateIndexInfoObject(int sn, const QJsonObject &newObj)
{
    QJsonArray array = m_rootObj["IndexInfo"].toArray();
    for (int i = 0; i < array.size(); ++i) {
        if (array[i].isObject() && array[i].toObject()["sn"].toInt() == sn) {
            array[i] = newObj;
            m_rootObj["IndexInfo"] = array;
            return true;
        }
    }
    return false;
}

bool SensorLogManager::updateWaveInfoObject(int sn, const QJsonObject &newObj)
{
    QJsonArray array = m_rootObj["WaveInfo"].toArray();
    for (int i = 0; i < array.size(); ++i) {
        if (array[i].isObject() && array[i].toObject()["sn"].toInt() == sn) {
            array[i] = newObj;
            m_rootObj["WaveInfo"] = array;
            return true;
        }
    }
    return false;
}

bool SensorLogManager::addWithCircularOverwrite(QJsonArray &array, const QString &name, int maxCount, int type)
{
    QString cmd;
    if (maxCount == 0) {
        qWarning() << "Maximum count is 0, cannot add entry:" << name;
        return false;
    }
    if (maxCount > 0 && array.size() >= maxCount) {
        if(type==0)
        {
            QJsonObject obj = array[0].toObject();
            QString name = obj["name"].toString();
            cmd = QString("rm /userdata/uartdata/%1").arg(name);
            SuperSystem(cmd);
        }
        array.removeAt(0);
    }
    QJsonObject newEntry;
    newEntry["name"] = name;
    array.append(newEntry);
    if(type==0)
    {
        cmd = QString("cp /userdata/data/%1 /userdata/uartdata/").arg(name);
        SuperSystem(cmd);
    }
    return true;
}

QByteArray SensorLogManager::readFileContent(QString filePath) const
{
    QMutexLocker locker(&m_mutex);
    if (filePath.isEmpty()) {
        qWarning() << "No file path associated with manager";
        return QByteArray();
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for reading:" << filePath;
        return QByteArray();
    }
    QByteArray data = file.readAll();
    file.close();
    return data;
}

bool SensorLogManager::removeByName(QJsonArray &array, const QString &name)
{
    for (int i = 0; i < array.size(); ++i) {
        if (array[i].isObject() && array[i].toObject()["name"].toString() == name) {
            array.removeAt(i);
            return true;
        }
    }
    return false;
}

QMap<int, QList<QString>> SensorLogManager::getAllIndexRecv() const
{
    QMutexLocker locker(&m_mutex);
    QMap<int, QList<QString>> result;
    QJsonArray indexInfoArray = m_rootObj["IndexInfo"].toArray();
    for (const QJsonValue &val : indexInfoArray) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        int sn = obj["sn"].toInt();
        QList<QString> recvList;
        QJsonArray recvArray = obj["indexRecv"].toArray();
        for (const QJsonValue &recvVal : recvArray) {
            if (recvVal.isObject()) {
                QString name = recvVal.toObject()["name"].toString();
                if (!name.isEmpty())
                    recvList.append(name);
            }
        }
        if (!recvList.isEmpty() || obj.contains("indexRecv")) // 即使空列表也保存，便于遍历所有 SN
            result.insert(sn, recvList);
    }
    return result;
}

//QMap<int, QList<QString>> SensorLogManager::getAllIndexSend() const
//{
//    QMutexLocker locker(&m_mutex);
//    QMap<int, QList<QString>> result;
//    QJsonArray indexInfoArray = m_rootObj["IndexInfo"].toArray();
//    for (const QJsonValue &val : indexInfoArray) {
//        if (!val.isObject()) continue;
//        QJsonObject obj = val.toObject();
//        int sn = obj["sn"].toInt();
//        QList<QString> recvList;
//        QJsonArray recvArray = obj["IndexSend"].toArray();
//        for (const QJsonValue &recvVal : recvArray) {
//            if (recvVal.isObject()) {
//                QString name = recvVal.toObject()["name"].toString();
//                if (!name.isEmpty())
//                    recvList.append(name);
//            }
//        }
//        if (!recvList.isEmpty() || obj.contains("IndexSend")) // 即使空列表也保存，便于遍历所有 SN
//            result.insert(sn, recvList);
//    }
//    return result;
//}

QMap<int, QList<QString>> SensorLogManager::getAllIndexSend() const
{
    QMutexLocker locker(&m_mutex);
    QMap<int, QList<QString>> result;
    QJsonArray indexInfoArray = m_rootObj["IndexInfo"].toArray();
    for (const QJsonValue &val : indexInfoArray) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        int sn = obj["sn"].toInt();
        QList<QString> sendList;
        QJsonArray sendArray = obj["IndexSend"].toArray();
        for (const QJsonValue &sendVal : sendArray) {
            if (sendVal.isObject()) {
                QString name = sendVal.toObject()["name"].toString();
                if (!name.isEmpty())
                    sendList.append(name);
            }
        }
        // 即使空列表也记录，方便知道该 SN 存在但无发送索引
        result.insert(sn, sendList);
    }
    return result;
}

int SensorLogManager::SuperSystem(const QString& cmd) {
    QProcess process;
    process.start("sh", {"-c", cmd});  // 强制通过 shell 执行
    qDebug()<<cmd;
    if (!process.waitForStarted()) return -1;
    if (!process.waitForFinished()) return -1;
    qDebug() << "Output:" << process.readAllStandardOutput();
    qDebug() << "Error:" << process.readAllStandardError();
    return 0;
}

// ---------- 公共接口：都加锁（递归锁允许重入）----------
bool SensorLogManager::addIndexInfo(int sn)
{
    QMutexLocker locker(&m_mutex);
    if (containsIndexInfo(sn)) {
        qWarning() << "IndexInfo for sn" << sn << "already exists";
        return false;
    }
    QJsonObject newItem;
    newItem["sn"] = sn;
    newItem["IndexSend"] = QJsonArray();
    newItem["indexRecv"] = QJsonArray();
    QJsonArray array = m_rootObj["IndexInfo"].toArray();
    array.append(newItem);
    m_rootObj["IndexInfo"] = array;
    return save();
}

bool SensorLogManager::removeIndexInfo(int sn)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray array = m_rootObj["IndexInfo"].toArray();
    for (int i = 0; i < array.size(); ++i) {
        if (array[i].isObject() && array[i].toObject()["sn"].toInt() == sn) {
            array.removeAt(i);
            m_rootObj["IndexInfo"] = array;
            m_maxIndexSend.remove(sn);
            m_maxIndexRecv.remove(sn);
            return save();
        }
    }
    qWarning() << "IndexInfo with sn" << sn << "not found";
    return false;
}

bool SensorLogManager::containsIndexInfo(int sn) const
{
    QMutexLocker locker(&m_mutex);
    QJsonArray array = m_rootObj["IndexInfo"].toArray();
    for (const QJsonValue &val : array) {
        if (val.isObject() && val.toObject()["sn"].toInt() == sn)
            return true;
    }
    return false;
}

QList<int> SensorLogManager::getIndexInfoSnList() const
{
    QMutexLocker locker(&m_mutex);
    QList<int> snList;
    QJsonArray array = m_rootObj["IndexInfo"].toArray();
    for (const QJsonValue &val : array) {
        if (val.isObject())
            snList.append(val.toObject()["sn"].toInt());
    }
    return snList;
}

bool SensorLogManager::addIndexSend(int sn, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureIndexInfoExists(sn)) return false;
    QJsonObject obj = findIndexInfoObject(sn);
    if (obj.isEmpty()) return false;
    QJsonArray sendArray = obj["IndexSend"].toArray();
    int maxCount = m_maxIndexSend.value(sn, -1);
    if (!addWithCircularOverwrite(sendArray, name, maxCount, 1)) return false;
    obj["IndexSend"] = sendArray;
    if (!updateIndexInfoObject(sn, obj)) return false;
    return save();
}

bool SensorLogManager::removeIndexSend(int sn, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (!containsIndexInfo(sn)) return false;
    QJsonObject obj = findIndexInfoObject(sn);
    if (obj.isEmpty()) return false;
    QJsonArray sendArray = obj["IndexSend"].toArray();
    if (!removeByName(sendArray, name)) {
        qWarning() << "IndexSend entry with name" << name << "not found for sn" << sn;
        return false;
    }
    obj["IndexSend"] = sendArray;
    if (!updateIndexInfoObject(sn, obj)) return false;
    return save();
}

QList<QString> SensorLogManager::getIndexSendList(int sn) const
{
    QMutexLocker locker(&m_mutex);
    QList<QString> list;
    QJsonObject obj = findIndexInfoObject(sn);
    if (obj.isEmpty()) return list;
    QJsonArray sendArray = obj["IndexSend"].toArray();
    for (const QJsonValue &val : sendArray) {
        if (val.isObject())
            list.append(val.toObject()["name"].toString());
    }
    return list;
}

void SensorLogManager::setMaxIndexSendCount(int sn, int maxCount)
{
    QMutexLocker locker(&m_mutex);
    if (maxCount < -1) maxCount = -1;
    m_maxIndexSend[sn] = maxCount;
}

int SensorLogManager::maxIndexSendCount(int sn) const
{
    QMutexLocker locker(&m_mutex);
    return m_maxIndexSend.value(sn, -1);
}

bool SensorLogManager::addIndexRecv(int sn, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureIndexInfoExists(sn)) return false;
    QJsonObject obj = findIndexInfoObject(sn);
    if (obj.isEmpty()) return false;
    QJsonArray recvArray = obj["indexRecv"].toArray();
    int maxCount = m_maxIndexRecv.value(sn, -1);
    if (!addWithCircularOverwrite(recvArray, name, maxCount, 0)) return false;
    obj["indexRecv"] = recvArray;
    if (!updateIndexInfoObject(sn, obj)) return false;
    return save();
}

bool SensorLogManager::removeIndexRecv(int sn, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (!containsIndexInfo(sn)) return false;
    QJsonObject obj = findIndexInfoObject(sn);
    if (obj.isEmpty()) return false;
    QJsonArray recvArray = obj["indexRecv"].toArray();
    if (!removeByName(recvArray, name)) {
        qWarning() << "IndexRecv entry with name" << name << "not found for sn" << sn;
        return false;
    }
    obj["indexRecv"] = recvArray;
    if (!updateIndexInfoObject(sn, obj)) return false;
    return save();
}

QList<QString> SensorLogManager::getIndexRecvList(int sn) const
{
    QMutexLocker locker(&m_mutex);
    QList<QString> list;
    QJsonObject obj = findIndexInfoObject(sn);
    if (obj.isEmpty()) return list;
    QJsonArray recvArray = obj["indexRecv"].toArray();
    for (const QJsonValue &val : recvArray) {
        if (val.isObject())
            list.append(val.toObject()["name"].toString());
    }
    return list;
}

void SensorLogManager::setMaxIndexRecvCount(int sn, int maxCount)
{
    QMutexLocker locker(&m_mutex);
    if (maxCount < -1) maxCount = -1;
    m_maxIndexRecv[sn] = maxCount;
}

int SensorLogManager::maxIndexRecvCount(int sn) const
{
    QMutexLocker locker(&m_mutex);
    return m_maxIndexRecv.value(sn, -1);
}

// WaveInfo 实现
bool SensorLogManager::addWaveInfo(int sn)
{
    QMutexLocker locker(&m_mutex);
    if (containsWaveInfo(sn)) {
        qWarning() << "WaveInfo for sn" << sn << "already exists";
        return false;
    }
    QJsonObject newItem;
    newItem["sn"] = sn;
    newItem["WaveSend"] = QJsonArray();
    newItem["WaveRecv"] = QJsonArray();
    QJsonArray array = m_rootObj["WaveInfo"].toArray();
    array.append(newItem);
    m_rootObj["WaveInfo"] = array;
    return save();
}

bool SensorLogManager::removeWaveInfo(int sn)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray array = m_rootObj["WaveInfo"].toArray();
    for (int i = 0; i < array.size(); ++i) {
        if (array[i].isObject() && array[i].toObject()["sn"].toInt() == sn) {
            array.removeAt(i);
            m_rootObj["WaveInfo"] = array;
            m_maxWaveSend.remove(sn);
            m_maxWaveRecv.remove(sn);
            return save();
        }
    }
    qWarning() << "WaveInfo with sn" << sn << "not found";
    return false;
}

bool SensorLogManager::containsWaveInfo(int sn) const
{
    QMutexLocker locker(&m_mutex);
    QJsonArray array = m_rootObj["WaveInfo"].toArray();
    for (const QJsonValue &val : array) {
        if (val.isObject() && val.toObject()["sn"].toInt() == sn)
            return true;
    }
    return false;
}

QList<int> SensorLogManager::getWaveInfoSnList() const
{
    QMutexLocker locker(&m_mutex);
    QList<int> snList;
    QJsonArray array = m_rootObj["WaveInfo"].toArray();
    for (const QJsonValue &val : array) {
        if (val.isObject())
            snList.append(val.toObject()["sn"].toInt());
    }
    return snList;
}

bool SensorLogManager::addWaveSend(int sn, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureWaveInfoExists(sn)) return false;
    QJsonObject obj = findWaveInfoObject(sn);
    if (obj.isEmpty()) return false;
    QJsonArray sendArray = obj["WaveSend"].toArray();
    int maxCount = m_maxWaveSend.value(sn, -1);
    if (!addWithCircularOverwrite(sendArray, name, maxCount, 3)) return false;
    obj["WaveSend"] = sendArray;
    if (!updateWaveInfoObject(sn, obj)) return false;
    return save();
}

bool SensorLogManager::removeWaveSend(int sn, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (!containsWaveInfo(sn)) return false;
    QJsonObject obj = findWaveInfoObject(sn);
    if (obj.isEmpty()) return false;
    QJsonArray sendArray = obj["WaveSend"].toArray();
    if (!removeByName(sendArray, name)) {
        qWarning() << "WaveSend entry with name" << name << "not found for sn" << sn;
        return false;
    }
    obj["WaveSend"] = sendArray;
    if (!updateWaveInfoObject(sn, obj)) return false;
    return save();
}

QList<QString> SensorLogManager::getWaveSendList(int sn) const
{
    QMutexLocker locker(&m_mutex);
    QList<QString> list;
    QJsonObject obj = findWaveInfoObject(sn);
    if (obj.isEmpty()) return list;
    QJsonArray sendArray = obj["WaveSend"].toArray();
    for (const QJsonValue &val : sendArray) {
        if (val.isObject())
            list.append(val.toObject()["name"].toString());
    }
    return list;
}

void SensorLogManager::setMaxWaveSendCount(int sn, int maxCount)
{
    QMutexLocker locker(&m_mutex);
    if (maxCount < -1) maxCount = -1;
    m_maxWaveSend[sn] = maxCount;
}

int SensorLogManager::maxWaveSendCount(int sn) const
{
    QMutexLocker locker(&m_mutex);
    return m_maxWaveSend.value(sn, -1);
}

bool SensorLogManager::addWaveRecv(int sn, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureWaveInfoExists(sn)) return false;
    QJsonObject obj = findWaveInfoObject(sn);
    if (obj.isEmpty()) return false;
    QJsonArray recvArray = obj["WaveRecv"].toArray();
    int maxCount = m_maxWaveRecv.value(sn, -1);
    if (!addWithCircularOverwrite(recvArray, name, maxCount, 2)) return false;
    obj["WaveRecv"] = recvArray;
    if (!updateWaveInfoObject(sn, obj)) return false;
    return save();
}

bool SensorLogManager::removeWaveRecv(int sn, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    if (!containsWaveInfo(sn)) return false;
    QJsonObject obj = findWaveInfoObject(sn);
    if (obj.isEmpty()) return false;
    QJsonArray recvArray = obj["WaveRecv"].toArray();
    if (!removeByName(recvArray, name)) {
        qWarning() << "WaveRecv entry with name" << name << "not found for sn" << sn;
        return false;
    }
    obj["WaveRecv"] = recvArray;
    if (!updateWaveInfoObject(sn, obj)) return false;
    return save();
}

QList<QString> SensorLogManager::getWaveRecvList(int sn) const
{
    QMutexLocker locker(&m_mutex);
    QList<QString> list;
    QJsonObject obj = findWaveInfoObject(sn);
    if (obj.isEmpty()) return list;
    QJsonArray recvArray = obj["WaveRecv"].toArray();
    for (const QJsonValue &val : recvArray) {
        if (val.isObject())
            list.append(val.toObject()["name"].toString());
    }
    return list;
}

void SensorLogManager::setMaxWaveRecvCount(int sn, int maxCount)
{
    QMutexLocker locker(&m_mutex);
    if (maxCount < -1) maxCount = -1;
    m_maxWaveRecv[sn] = maxCount;
}

int SensorLogManager::maxWaveRecvCount(int sn) const
{
    QMutexLocker locker(&m_mutex);
    return m_maxWaveRecv.value(sn, -1);
}
