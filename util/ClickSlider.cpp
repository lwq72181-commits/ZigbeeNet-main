#include "ClickSlider.h"

ClickSlider::ClickSlider(QWidget *parent):
    QSlider(parent),
    m_pressed()
{

}

void ClickSlider::mousePressEvent(QMouseEvent *ev)
{
    QSlider::mousePressEvent(ev);

    m_pressed = true;

    //获取鼠标的位置，这里并不能直接从ev中取值（因为如果是拖动的话，鼠标开始点击的位置没有意义了）.
    double pos = 0;
    if (orientation() == Qt::Horizontal)
    {
        pos = ev->pos().x() / (double)width();
    }
    else
    {
        pos = (height() - ev->pos().y()) / (double)height();
    }

    int value = (int)(pos * (maximum() - minimum()) + minimum());
    setValue(value);

    setSliderDown(true);

    //qDebug("mousePressEvent value:%d, value: %d, pos: %d", value, this->value(), sliderPosition());
}

void ClickSlider::mouseReleaseEvent(QMouseEvent *ev)
{
    //qDebug("ClickSlider::mouseReleaseEvent m_pressed:%d, value: %d, pos: %d", m_pressed, this->value(), sliderPosition());

    if (m_pressed)
    {
        setValue(sliderPosition());
        setSliderDown(false);
        m_pressed = false;
    }

    QSlider::mouseReleaseEvent(ev);
}

bool ClickSlider::isPressed() const
{
    return m_pressed;
}

void ClickSlider::setValueNoSignals(int value)
{
    blockSignals(true);
    setValue(value);
    blockSignals(false);
}
