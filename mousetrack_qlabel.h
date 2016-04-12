#ifndef MOUSETRACK_QLABEL_H
#define MOUSETRACK_QLABEL_H

#include <QLabel>
//#include <QWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QEvent>

class MouseTrack_QLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MouseTrack_QLabel(QWidget *parent = 0);

    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);

    int x, y;

signals:
    void mouse_Position();
    void mouse_Pressed();

};

#endif // MOUSETRACK_QLABEL_H
