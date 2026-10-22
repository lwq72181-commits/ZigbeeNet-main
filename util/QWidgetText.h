#ifndef QWIDGETTEXT_H
#define QWIDGETTEXT_H

#include <QWidget>
#include "TStringCast.h"
#include "QStringUtil.h"


class QWidgetText
{
public:
    QWidgetText();

    template <class Widget>
    static void setText(Widget* widget, const QString& v)
    {
        widget->setText(v);
    }

    template <class Widget, class V>
    static void setText(Widget* widget, const V& v)
    {
        std::string str = comn::StringCast::toString(v);
        QString text = QString::fromStdString(str);
        widget->setText(text);
    }

    template <class Widget, class V>
    static bool getText(Widget* widget, V& value)
    {
        if (!widget)
        {
            return false;
        }

        QString text = widget->text();
        comn::StringCast::toValue(text.toStdString(), value);
        return true;
    }

    template <class Widget>
    static bool getText(Widget* widget, QString& value)
    {
        if (!widget)
        {
            return false;
        }

        value = widget->text();
        return true;
    }


};

#endif // QWIDGETTEXT_H
