#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include "calibrationwindow.h"

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

private:
    Ui::Gui *ui;
    CalibrationWindow* calibWindow;
    string pathToResources;
};

#endif // GUI_H
