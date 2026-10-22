#ifndef THREADSAFEMULTIVECTOR_H
#define THREADSAFEMULTIVECTOR_H


#include <QVector>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QMap>
#include <QString>
#include <QDebug>
class ThreadSafeMultiVector {
public:
    // 添加一个新的 QVector 到容器中
    void addVectorfull(const QString& name, const QVector<float>& initialData = {}) {
        QWriteLocker locker(&rwLock);
        if (vectors.contains(name)) {
            qWarning() << "Vector with name" << name << "already exists.";
            return;
        }
        vectors[name] = {initialData, new QReadWriteLock};
    }

    void addVector(const QString& name) {
        QWriteLocker locker(&rwLock);
        if (vectors.contains(name)) {
            qWarning() << "Vector with name" << name << "already exists.";
            return;
        }
        vectors[name] = {QVector<float>(), new QReadWriteLock};
    }

    // 写入元素到指定的 QVector
    bool writeElement(const QString& name, int index, float value) {
        QReadLocker locker(&rwLock); // 读取 vectors 容器时需要锁
        auto it = vectors.find(name);
        if (it == vectors.end()) {
            qWarning() << "Vector with name" << name << "does not exist.";
            return false;
        }

        QWriteLocker vectorLocker(it.value().second); // 写入特定 QVector 时需要写锁
        if (index >= 0 && index < it.value().first.size()) {
            it.value().first[index] = value;
            return true;
        } else {
            qWarning() << "Index out of bounds in writeElement for vector" << name;
            return false;
        }
    }

    // 读取元素从指定的 QVector
    float readElement(const QString& name, int index) const {
        QReadLocker locker(&rwLock); // 读取 vectors 容器时需要锁
        auto it = vectors.find(name);
        if (it == vectors.end()) {
            qWarning() << "Vector with name" << name << "does not exist.";
            return 0; // 返回默认值
        }

        QReadLocker vectorLocker(it.value().second); // 读取特定 QVector 时需要读锁
        if (index >= 0 && index < it.value().first.size()) {
            return it.value().first[index];
        } else {
            qWarning() << "Index out of bounds in readElement for vector" << name;
            return 0; // 返回默认值
        }
    }

    // 获取指定 QVector 的大小
    int size(const QString& name) const {
        QReadLocker locker(&rwLock); // 读取 vectors 容器时需要锁
        auto it = vectors.find(name);
        if (it == vectors.end()) {
            qWarning() << "Vector with name" << name << "does not exist.";
            return 0;
        }

        QReadLocker vectorLocker(it.value().second); // 读取特定 QVector 时需要读锁
        return it.value().first.size();
    }

    // 向指定的 QVector 添加元素
    void append(const QString& name, float value) {
        QReadLocker locker(&rwLock); // 读取 vectors 容器时需要锁
        auto it = vectors.find(name);
        if (it == vectors.end()) {
            qWarning() << "Vector with name" << name << "does not exist.";
            return;
        }

        QWriteLocker vectorLocker(it.value().second); // 修改特定 QVector 时需要写锁
        it.value().first.append(value);
    }

    // 清空指定的 QVector
    void clear(const QString& name) {
        QReadLocker locker(&rwLock); // 读取 vectors 容器时需要锁
        auto it = vectors.find(name);
        if (it == vectors.end()) {
            qWarning() << "Vector with name" << name << "does not exist.";
            return;
        }

        QWriteLocker vectorLocker(it.value().second); // 修改特定 QVector 时需要写锁
        it.value().first.clear();
    }

private:
    mutable QReadWriteLock rwLock; // 保护整个 vectors 容器
    QMap<QString, std::pair<QVector<float>, QReadWriteLock*>> vectors; // 存储 QVector 和其对应的锁
};

#endif // THREADSAFEMULTIVECTOR_H
