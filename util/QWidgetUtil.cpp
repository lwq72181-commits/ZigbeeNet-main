#include "QWidgetUtil.h"
#include <QApplication>
#include <QDesktopWidget>


QWidgetUtil::QWidgetUtil()
{

}

QRect QWidgetUtil::mapToGlobal(QWidget* widget, const QRect& rc)
{
    QPoint pt = widget->mapToGlobal(rc.topLeft());
    return QRect(pt, rc.size());
}

void QWidgetUtil::move(QWidget* widget, const QRect& rc)
{
    widget->move(rc.topLeft());
    widget->resize(rc.size());
}

QString QWidgetUtil::makeLinkText(const QString& url, const QString& name)
{
    return QString("<a href=\"%1\">%2").arg(url, name);
}

void QWidgetUtil::center(QWidget* widget)
{
    widget->move(QApplication::desktop()->screen()->rect().center() - widget->rect().center());
}
