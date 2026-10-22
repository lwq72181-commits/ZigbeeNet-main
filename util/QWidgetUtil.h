#ifndef QWIDGETUTIL_H
#define QWIDGETUTIL_H

#include <QWidget>

class QWidgetUtil
{
public:
    QWidgetUtil();

    static QRect mapToGlobal(QWidget* widget, const QRect& rc);

    static void move(QWidget* widget, const QRect& rc);

    static QString makeLinkText(const QString& url, const QString& name);

    static void center(QWidget* widget);

};

#endif // QWIDGETUTIL_H
