#include "mousetrack_qlabel.h"

MouseTrack_QLabel::MouseTrack_QLabel(QWidget *parent) : QLabel(parent)
{
    setMouseTracking(true);
}

void MouseTrack_QLabel::mouseMoveEvent(QMouseEvent *ev)
{
    this->x = ev->x();
    this->y = ev->y();

    emit mouse_Position();
}
void MouseTrack_QLabel::mousePressEvent(QMouseEvent *ev)
{
    emit mouse_Pressed();
}
