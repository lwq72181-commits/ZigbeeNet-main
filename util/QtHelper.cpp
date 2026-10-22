#include "QtHelper.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>


QtHelper::QtHelper()
{
}

size_t QtHelper::copy(char* buf, size_t size, const QString& str)
{
    if (!buf || size == 0)
    {
        return 0;
    }

    std::string medium = str.toStdString();
    size_t count = std::min((size_t)medium.size(), size);
    if (count == 0)
    {
        buf[0] = 0;
    }
    else
    {
        strncpy(buf, medium.c_str(), count);
    }
    return count;
}

int QtHelper::setCurrentIndex(QComboBox* cmb, const QVariant& data)
{
    int index = cmb->findData(data);
    cmb->setCurrentIndex(index);
    return index;
}

bool QtHelper::openExplorer(const QString& path)
{
    QString url("file:///");
    url += path;

    return QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
}

QString QtHelper::chooseDir(QWidget *parent, const QString& dirpath)
{
    QString filePath;
    QFileDialog fileDialog(parent);
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);

    if (dirpath.size() > 0)
    {
        fileDialog.setDirectory(dirpath);
    }

    if (fileDialog.exec() != QDialog::Accepted)
    {
        return filePath;
    }

    QStringList fileList = fileDialog.selectedFiles();
    if (fileList.size())
    {
        filePath = fileList[0];
    }
    return filePath;
}
