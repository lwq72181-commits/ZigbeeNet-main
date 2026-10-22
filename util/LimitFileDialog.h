#ifndef LIMITFILEDIALOG_H
#define LIMITFILEDIALOG_H

#include <QWidget>
#include <QFileDialog>

class LimitFileDialog : public QFileDialog
{
    Q_OBJECT

public:
    LimitFileDialog(QWidget *parent = Q_NULLPTR,
                    const QString &caption = QString(),
                    const QString &directory = QString(),
                    const QString &filter = QString());


    void setRootDir(const QString& rootDir);

    void setDisabledDirectory(const QStringList& dirs, const QString& defDir);

public:
    static bool isSubDir(const QString& dir, const QString& root);
    static bool isSubDir(const QString& dir, const QStringList& roots);

protected slots:
    void on_directoryEntered(const QString &directory);



protected:
    QString m_rootDir;

    QStringList m_disabledDirs;
    QString m_defDir;

};

#endif // LIMITFILEDIALOG_H
