#ifndef QTHELPER_H
#define QTHELPER_H

#include <QString>
#include <QComboBox>

class QtHelper
{
public:
    QtHelper();

    static size_t copy(char* buf, size_t size, const QString& str);

    template < class T>
    static size_t copy(T& t, const QString &str)
    {
        return QtHelper::copy(t, sizeof(t), str);
    }

    static int setCurrentIndex(QComboBox* cmb, const QVariant& data);

    static bool openExplorer(const QString& path);

    static QString chooseDir(QWidget *parent, const QString& dirpath);

};

#endif // QTHELPER_H
