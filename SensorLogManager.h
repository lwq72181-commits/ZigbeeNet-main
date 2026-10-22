#ifndef SENSORLOGMANAGER_H
#define SENSORLOGMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QRecursiveMutex>

class SensorLogManager : public QObject
{
    Q_OBJECT
public:
    explicit SensorLogManager(QObject *parent = nullptr);
    ~SensorLogManager();

    bool load(const QString &filePath);
    bool save();

    // IndexInfo
    bool addIndexInfo(int sn);
    bool removeIndexInfo(int sn);
    bool containsIndexInfo(int sn) const;
    QList<int> getIndexInfoSnList() const;

    bool addIndexSend(int sn, const QString &name);
    bool removeIndexSend(int sn, const QString &name);
    QList<QString> getIndexSendList(int sn) const;
    void setMaxIndexSendCount(int sn, int maxCount);
    int maxIndexSendCount(int sn) const;

    bool addIndexRecv(int sn, const QString &name);
    bool removeIndexRecv(int sn, const QString &name);
    QList<QString> getIndexRecvList(int sn) const;
    void setMaxIndexRecvCount(int sn, int maxCount);
    int maxIndexRecvCount(int sn) const;

    // WaveInfo
    bool addWaveInfo(int sn);
    bool removeWaveInfo(int sn);
    bool containsWaveInfo(int sn) const;
    QList<int> getWaveInfoSnList() const;

    bool addWaveSend(int sn, const QString &name);
    bool removeWaveSend(int sn, const QString &name);
    QList<QString> getWaveSendList(int sn) const;
    void setMaxWaveSendCount(int sn, int maxCount);
    int maxWaveSendCount(int sn) const;

    bool addWaveRecv(int sn, const QString &name);
    bool removeWaveRecv(int sn, const QString &name);
    QList<QString> getWaveRecvList(int sn) const;
    void setMaxWaveRecvCount(int sn, int maxCount);
    int maxWaveRecvCount(int sn) const;
    QMap<int, QList<QString>> getAllIndexRecv() const;
    QMap<int, QList<QString>> getAllIndexSend() const;
    QByteArray readFileContent(QString filePath) const;
private:
    // 内部辅助函数（不加锁，调用者必须已持有锁）
    bool ensureIndexInfoExists(int sn);
    bool ensureWaveInfoExists(int sn);
    QJsonObject findIndexInfoObject(int sn) const;
    QJsonObject findWaveInfoObject(int sn) const;
    bool updateIndexInfoObject(int sn, const QJsonObject &newObj);
    bool updateWaveInfoObject(int sn, const QJsonObject &newObj);
    bool addWithCircularOverwrite(QJsonArray &array, const QString &name, int maxCount, int type);
    bool removeByName(QJsonArray &array, const QString &name);
    int SuperSystem(const QString& cmd);
    QString m_filePath;
    QJsonObject m_rootObj;
    QMap<int, int> m_maxIndexSend;
    QMap<int, int> m_maxIndexRecv;
    QMap<int, int> m_maxWaveSend;
    QMap<int, int> m_maxWaveRecv;

    mutable QRecursiveMutex m_mutex;   // 递归互斥锁，支持同一线程多次加锁
};

#endif // SENSORLOGMANAGER_H
