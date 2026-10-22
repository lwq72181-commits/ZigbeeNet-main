#include "QPath.h"
#include <QFile>
#include <QDir>
#include "Path.h"
#include <QTemporaryDir>
#include <QApplication>
#include <QFileInfo>


QPath::QPath()
{
}

bool QPath::exist(const QString& filepath)
{
    return QFile(filepath).exists();
}

bool QPath::createDirectories(const QString& filepath)
{
    return QDir().mkpath(filepath);
}

QString QPath::getWorkDir()
{
    return QApplication::applicationDirPath();
}

QString QPath::basename(const QString& filepath)
{
    int idx = lastIndexOfSeperator(filepath);
    if (idx < 0)
    {
        return filepath;
    }
    return filepath.mid(idx+1);
}

QString QPath::dirname(const QString& filepath)
{
    int idx = lastIndexOfSeperator(filepath);
    if (idx < 0)
    {
        return "";
    }
    return filepath.left(idx);
}

QString QPath::title(const QString& filepath)
{
    QString filename = basename(filepath);
    int idx = filename.lastIndexOf('.');
    if (idx < 0)
    {
        return filename;
    }
    return filename.mid(0, idx);
}

QString QPath::join(const QString& dir, const QString& name)
{
    QString filepath(dir);
    bool hasEnd = endWithSeperator(dir);
    if (!hasEnd)
    {
        filepath += '/';
    }

    if (startWithSeperator(name))
    {
        filepath.remove(filepath.size()-1, 1);
    }
    filepath += name;
    return filepath;
}

QString QPath::getFileName(const QString& filepath)
{
    int idx = filepath.lastIndexOf('.');
    if (idx < 0)
    {
        return filepath;
    }
    return filepath.left(idx);
}

QString QPath::getFileTitle(const QString& filepath)
{
    int idx = filepath.lastIndexOf('.');
    if (idx < 0)
    {
        return filepath;
    }
    return filepath.left(idx);
}

QString QPath::getFileExt(const QString& filepath)
{
    int idx = filepath.lastIndexOf('.');
    if (idx < 0)
    {
        return "";
    }
    return filepath.mid(idx+1);
}

int QPath::lastIndexOfSeperator(const QString& filepath)
{
    for (int i = filepath.size() - 1; i > 0; i --)
    {
        if (filepath[i] == '/' || filepath[i] == '\\')
        {
            return i;
        }
    }
    return -1;
}

bool QPath::endWithSeperator(const QString& filepath)
{
    if (filepath.isEmpty())
    {
        return false;
    }

    return (filepath[filepath.size()-1] == '/') || (filepath[filepath.size()-1] == '\\');
}

bool QPath::startWithSeperator(const QString& filepath)
{
    if (filepath.isEmpty())
    {
        return false;
    }

    return (filepath[0] == '/') || (filepath[0] == '\\');
}

QString QPath::abspath(QString& filepath)
{
    filepath = QDir(filepath).absolutePath();
    return filepath;
}

QString QPath::getTmpDir()
{
    return QDir::tempPath();
}

bool QPath::remove(const QString& filepath)
{
	QFileInfo info(filepath);
	if (info.isDir())
	{
		QDir dir(filepath);
		return dir.removeRecursively();
	}
	else
	{
		QFile file(filepath);
		return file.remove();
	}
}