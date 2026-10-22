#include "ResizeWidget.h"

ResizeWidget::ResizeWidget(QWidget *parent) : QWidget(parent)
{
}

void ResizeWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    emit sizeChanged();
}
