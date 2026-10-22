#ifndef CLICKSLIDER_H
#define CLICKSLIDER_H

#include <QSlider>
#include <QMouseEvent>

class ClickSlider : public QSlider
{
public:
    ClickSlider(QWidget *parent = 0);

    bool isPressed() const;

    void setValueNoSignals(int value);

protected:
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);

protected:
    bool    m_pressed;

};

#endif // CLICKSLIDER_H
