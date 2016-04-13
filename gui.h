#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include "calibrationwindow.h"
#include "reduce_framerate.h"
#include "homography_transformation.h"

namespace Ui {
class Gui;
}

class Gui : public QMainWindow
{
    Q_OBJECT

public:
    explicit Gui(QWidget *parent = 0);
    ~Gui();

private slots:
    void on_pushButton_Calibrate_clicked();

    void on_pushButton_Calibrate_2_clicked();

    void on_pushButton_HomographyTrans_clicked();

private:
    Ui::Gui *ui;
    CalibrationWindow* calibWindow;
    Reduce_FrameRate *reduceFR;
    Homography_Transformation *homographyTrans;
    string pathToResources;
};

#endif // GUI_H
