#include "LimitFileDialog.h"
#include <QDir>


LimitFileDialog::LimitFileDialog(QWidget *parent,
                                 const QString &caption,
                                 const QString &directory,
                                 const QString &filter):
    QFileDialog(parent,caption, directory, filter)
{
    connect(this, &QFileDialog::directoryEntered, this, &LimitFileDialog::on_directoryEntered);

}

void LimitFileDialog::setRootDir(const QString& rootDir)
{
    QDir dir(rootDir);
    m_rootDir = dir.path();

    if (rootDir.size() > 0)
    {
        QList<QUrl> urls;
        setSidebarUrls(urls);

        setDirectory(m_rootDir);
    }
}

void LimitFileDialog::setDisabledDirectory(const QStringList& dirs, const QString& defDir)
{
    m_disabledDirs = dirs;
    m_defDir = defDir;
}

void LimitFileDialog::on_directoryEntered(const QString &directory)
{
    //qDebug("directoryEntered: %s", qPrintable(directory));

    if (m_rootDir.size() > 0)
    {
        if (isSubDir(directory, m_rootDir))
        {
            //
        }
        else
        {
            setDirectory(m_rootDir);
        }
    }
    else if (isSubDir(directory, m_disabledDirs))
    {
        setDirectory(m_defDir);
    }
}

bool LimitFileDialog::isSubDir(const QString& dir, const QString& root)
{
    return (dir.size() >= root.size() && (dir.indexOf(root) == 0));
}

bool LimitFileDialog::isSubDir(const QString& dir, const QStringList& roots)
{
    for (const QString& root :roots)
    {
        if (isSubDir(dir, root))
        {
            return true;
        }
    }
    return false;
}
