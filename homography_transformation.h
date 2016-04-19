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
    QImage Mat2QImage(const Mat3b &src);
    void drawPolygon();
    Point getIntersctionPoint( Point line1_Point1, Point line1_Point2, Point line2_Point1, Point line2_Point2);
    bool findNew_PerspectiveVertices();

    void estimateHomography_LeftFront();
    void estimatePerspective_LeftFront();
    void estimateHomography_FrontRight();
    void estimatePerspective_FrontRight();

private:
    Ui::Homography_Transformation *ui;
    string pathToResource;
    bool isTopRightSet, isBottomRightSet, isBottomLeftSet, isTopLeftSet;
    Point2f pt_TopRight, pt_BottomRight, pt_BottomLeft, pt_TopLeft;
    Point2f pt_NewTopRight, pt_NewBottomRight, pt_NewBottomLeft, pt_NewTopLeft;
    Mat sideImg_Src, frontImg_Src, sideImg_Padded;
    Mat rectifiedImg_ByHomography;
    cv::Mat_<float> homography_MapX_32f, homography_MapY_32f;
    Size mapSize, srcImg_Size, padSize;
    int newWidthPad, padToRemove;


private slots:
    void on_pushButton_BrowseSideImg_clicked();
    void on_pushButton_BrowseFrontImg_clicked();
    void on_pushButton_EstimateHomography_clicked();    
    void on_radioButton_LeftFront_toggled(bool checked);
    void on_radioButton_FrontRight_toggled(bool checked);
    void onMouseMove(QPointF mousePos);
    void onMouseClick(QPointF mousePos);
    void on_pushButton_PerfomPerspective_clicked();
};

#endif // HOMOGRAPHY_TRANSFORMATION_H
