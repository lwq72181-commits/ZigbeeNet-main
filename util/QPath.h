#ifndef QPATH_H
#define QPATH_H

#include <QString>

class QPath
{
public:
    QPath();

    static bool exist(const QString& filepath);
    static bool createDirectories(const QString& filepath);

    static QString getWorkDir();

    static QString basename(const QString& filepath);
    static QString dirname(const QString& filepath);
    static QString title(const QString& filepath);

    static QString join(const QString& dir, const QString& name);

    static QString getFileName(const QString& filepath);
    static QString getFileTitle(const QString& filepath);
    static QString getFileExt(const QString& filepath);

    static bool endWithSeperator(const QString& filepath);
    static bool startWithSeperator(const QString& filepath);

    static QString abspath(QString& filepath);
    static QString getTmpDir();

	static bool remove(const QString& filepath);

protected:
    static int lastIndexOfSeperator(const QString& filepath);

};

#endif // QPATH_H
