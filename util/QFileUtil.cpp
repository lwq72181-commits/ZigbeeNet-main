#include "QFileUtil.h"
#include <QDir>

#ifdef WIN32
#else
    #include <unistd.h>
#endif




QFileUtil::QFileUtil()
{

}

bool QFileUtil::copy(const QString& srcPath, const QString& toDir, bool coverFileIfExist)
{
    QFileInfo srcInfo(srcPath);
    QDir srcDir(srcPath);
    QString fileName = srcDir.dirName();

    QDir destDir(toDir);
    QString toPath = destDir.filePath(fileName);

    if (srcInfo.isDir())
    {
        destDir.mkdir(fileName);

        QFileInfoList fileInfoList = srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        for (int i = 0; i < fileInfoList.size(); i ++)
        {
            QFileInfo& fileInfo = fileInfoList[i];

            copy(fileInfo.filePath(), toPath, coverFileIfExist);
        }
    }
    else
    {
        if (QFile(toPath).exists())
        {
            if (coverFileIfExist)
            {
                QFile(toPath).remove();
            }
            else
            {
                return false;
            }
        }

        return QFile(srcPath).copy(toPath);
    }

    return true;
}



static bool endWithSeperator(const QString& filepath)
{
    if (filepath.isEmpty())
    {
        return false;
    }

    return (filepath[filepath.size()-1] == '/') || (filepath[filepath.size()-1] == '\\');
}

static bool startWithSeperator(const QString& filepath)
{
    if (filepath.isEmpty())
    {
        return false;
    }

    return (filepath[0] == '/') || (filepath[0] == '\\');
}

static QString join(const QString& dir, const QString& name)
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

bool QFileUtil::copy(const QString& srcPath, const QString& filename, const QString& toDir, bool coverFileIfExist)
{
    QString filepath = join(srcPath, filename);
    return copy(filepath, toDir, coverFileIfExist);
}

bool QFileUtil::copyPrefix(const QString& srcPath, const QString& prefix, const QString& toDir, bool coverFileIfExist)
{
    QDir srcDir(srcPath);
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::AllEntries;
    QStringList entryList = srcDir.entryList(filters);
    for (int i = 0; i < entryList.size(); ++ i)
    {
        QString& entry = entryList[i];
        if (!entry.startsWith(prefix))
        {
            continue;
        }

        QString srcFile = join(srcPath, entry);
        copy(srcFile, toDir, coverFileIfExist);
    }

    return true;
}

bool QFileUtil::remove(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (fileInfo.isDir())
    {
        return QDir(filePath).removeRecursively();
    }
    else
    {
        return QFile(filePath).remove();
    }
}

QString QFileUtil::basename(const QString& filePath)
{
    QDir dir(filePath);
    return dir.dirName();
}

QString QFileUtil::dirname(const QString& filePath)
{
     QDir dir(filePath);
     dir.cdUp();
     return dir.path();
}

QString QFileUtil::getFileExt(const QString& filePath)
{
    QString ext;
    int index = filePath.lastIndexOf('.');
    if (index >= 0)
    {
        ext = filePath.mid(index);
    }
    return ext;
}

bool QFileUtil::isSuchExt(const QString& filePath, const QStringList& extList)
{
    QString ext = getFileExt(filePath);
    return extList.contains(ext);
}

bool QFileUtil::load(const QString& filepath, QString& content)
{
    bool got = false;
    QFile file(filepath);
    file.open(QFile::ReadOnly | QFile::Text);
    if(file.isOpen())
    {
        QByteArray bytes = file.readAll();
        file.close();

        content = QLatin1String(bytes);

        file.close();

        got = true;
    }
    return got;
}

bool QFileUtil::load(const QString& filepath, QStringList& lines)
{
    bool got = false;
    QFile file(filepath);
    file.open(QFile::ReadOnly | QFile::Text);
    if(!file.isOpen())
    {
        return false;
    }

    char buf[1024] = {0};
    while (true)
    {
        qint64 lineLength = file.readLine(buf, sizeof(buf));
        if (lineLength == -1)
        {
            break;
        }

        QString line(buf);
        lines.push_back(line.trimmed());
    }

    file.close();

    return got;
}

bool QFileUtil::existFile(const QString& filepath)
{
    QFileInfo fileInfo(filepath);
    return fileInfo.isFile();
}



bool QFileUtil::copySub(const QString& srcPath, const QString& toDir, bool coverFileIfExist)
{
    QFileInfo srcInfo(srcPath);
    if (!srcInfo.exists())
    {
        return false;
    }

    if (srcInfo.isDir())
    {
        QDir srcDir(srcPath);

        QFileInfoList fileInfoList = srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        for (int i = 0; i < fileInfoList.size(); i ++)
        {
            QFileInfo& fileInfo = fileInfoList[i];

            copy(fileInfo.filePath(), toDir, coverFileIfExist);
        }
        return true;
    }
    return copy(srcPath, toDir, coverFileIfExist);
}


void QFileUtil::execSync()
{
#ifdef WIN32
#else
    sync();
#endif //
}

bool QFileUtil::save(const QString& filepath, QString& content)
{
    bool got = false;
    QFile file(filepath);
    file.open(QFile::WriteOnly | QFile::Text);
    if(file.isOpen())
    {
        file.write(content.toUtf8());
        file.close();
        got = true;
    }
    return got;
}
