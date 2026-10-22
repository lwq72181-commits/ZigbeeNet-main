#ifndef QFILEUTIL_H
#define QFILEUTIL_H

#include <QString>
#include <QDir>
#include <QFile>


class QFileUtil
{
public:
    QFileUtil();

    static bool copy(const QString& srcPath, const QString& toDir, bool coverFileIfExist);

    static bool copy(const QString& srcPath, const QString& filename, const QString& toDir, bool coverFileIfExist);

    static bool copyPrefix(const QString& srcPath, const QString& prefix, const QString& toDir, bool coverFileIfExist);


    static bool remove(const QString& filePath);

    static QString basename(const QString& filePath);

    static QString dirname(const QString& filePath);

    static QString getFileExt(const QString& filePath);

    static bool isSuchExt(const QString& filePath, const QStringList& extList);

    static bool load(const QString& filepath, QString& content);

    static bool load(const QString& filepath, QStringList& lines);

    static bool existFile(const QString& filepath);

    static void execSync();

    static bool save(const QString& filepath, QString& content);

    static bool copySub(const QString& srcPath, const QString& toDir, bool coverFileIfExist);

};

#endif // QFILEUTIL_H
