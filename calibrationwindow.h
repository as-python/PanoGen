#ifndef CALIBRATIONWINDOW_H
#define CALIBRATIONWINDOW_H

#include <QWidget>
#include <QFileDialog>
#include <QPoint>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

#include <stdio.h>
#include <string>

#include "cameracalibration.h"
#include "cropimage.h"

using namespace std;
using namespace cv;

namespace Ui {
class CalibrationWindow;
}

class CalibrationWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CalibrationWindow(QWidget *parent = 0);
    ~CalibrationWindow();

    CameraCalibration* camCalib;
    QImage Mat2QImage(const cv::Mat3b &src);
    void setPathToResources(const string &value);

private slots:
    void on_pushButton_FindCalibParams_clicked();
    void on_pushButton_ChooseImgList_clicked();
    void setImage(Mat img);
    void setImageLabelText(QString text);
    void setImageCounterText(QString text);    

    void on_pushButton_CropImage_clicked();
    void save_NewFrameSize(QPoint topLeft, QPoint bottomRight);    

    void on_pushButton_BrowseInputVid_clicked();

    void on_pushButton_BrowseCalibMap_clicked();

    void on_pushButton_PerformCalibration_clicked();

private:
    Ui::CalibrationWindow *ui;
    string pathToResources;
    CropImage* cropImgWindow;
};

#endif // CALIBRATIONWINDOW_H
