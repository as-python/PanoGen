#ifndef REDUCE_FRAMERATE_H
#define REDUCE_FRAMERATE_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <string>
#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

namespace Ui {
class Reduce_FrameRate;
}

class Reduce_FrameRate : public QWidget
{
    Q_OBJECT

public:
    explicit Reduce_FrameRate(QWidget *parent = 0);
    ~Reduce_FrameRate();

    void setPathToResource(const string &value);

private slots:
    void on_pushButton_ReduceFR_clicked();

    void on_pushButton_InputVideo_clicked();

private:
    Ui::Reduce_FrameRate *ui;
    string pathToResource;
    string inputVideoFile;
    int currentFR;
};

#endif // REDUCE_FRAMERATE_H
