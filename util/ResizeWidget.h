#ifndef RESIZEWIDGET_H
#define RESIZEWIDGET_H

#include <QWidget>

class ResizeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResizeWidget(QWidget *parent = 0);

signals:
    void sizeChanged();

public slots:

protected:
    virtual void resizeEvent(QResizeEvent *event);

};

#endif // RESIZEWIDGET_H
