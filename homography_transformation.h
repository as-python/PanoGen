#ifndef HOMOGRAPHY_TRANSFORMATION_H
#define HOMOGRAPHY_TRANSFORMATION_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "scaleimage_graphicsview.h"

using namespace std;
using namespace cv;

namespace Ui {
class Homography_Transformation;
}

class Homography_Transformation : public QWidget
{
    Q_OBJECT

public:
    explicit Homography_Transformation(QWidget *parent = 0);
    ~Homography_Transformation();

    void setPathToResource(const string &value);

private:
    Ui::Homography_Transformation *ui;
    string pathToResource;

public slots:
    void onMouseMove(QPointF mousePos);
private slots:
    void on_pushButton_BrowseSideImg_clicked();
    void on_pushButton_BrowseFrontImg_clicked();
    void on_pushButton_EstimateHomography_clicked();    
    void on_radioButton_LeftFront_toggled(bool checked);
    void on_radioButton_FrontRight_toggled(bool checked);
};

#endif // HOMOGRAPHY_TRANSFORMATION_H
