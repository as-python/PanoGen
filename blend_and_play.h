#ifndef BLEND_AND_PLAY_H
#define BLEND_AND_PLAY_H

#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

namespace Ui {
class Blend_And_Play;
}

class Blend_And_Play : public QWidget
{
    Q_OBJECT

public:
    explicit Blend_And_Play(QWidget *parent = 0);
    ~Blend_And_Play();
    void setPathToResource(const string &value);
    void getBlendMask(Mat_<Vec3f>& blendMaskLeft, Mat_<Vec3f>& blendMaskRight, Size blendWindowSize);
    void applyBlending(Mat& blendSrcLeft, Mat& blendSrcRight, Mat& blendedImg, Mat_<Vec3f> blendMaskLeft, Mat_<Vec3f> blendMaskRight, int nRows, int nCols);
    QImage Mat2QImage(const Mat3b &src);
    bool readMaps();
    void applyMaps_LeftFront();
    void applyMaps_FrontRight();
    void blend_LeftFront();
    void blend_FrontRight();
    void combineAndSet();
    bool nextImage();

private slots:
    void on_pushButton_BrowseLeftVideo_clicked();
    void on_pushButton_BrowseFrontVideo_clicked();
    void on_pushButton_BrowseRightVideo_clicked();
    void on_pushButton_BrowseLFMap_clicked();
    void on_pushButton_BrowseFPMap_clicked();
    void on_pushButton_Play_clicked();

private:
    Ui::Blend_And_Play *ui;
    string pathToResource;
    Size srcImg_Size, panoLF_Size, panoFR_Size, padSize, blendWindow_Size;
    int leftBorder, rightBorder;
    Mat frontImg_Src, leftImg_Src, rightImg_Src;
    Mat rectifiedImg_FR_Cropped, rectifiedImg_LFCropped;
    Mat_<float> MapX_LeftFront_32f, MapY_LeftFront_32f, MapX_FrontRight_32f, MapY_FrontRight_32f;
    Mat blendedImg_LeftFront, blendedImg_FrontRight;
    VideoCapture videoCap_Left, videoCap_Front, videoCap_Right;
};

#endif // BLEND_AND_PLAY_H
