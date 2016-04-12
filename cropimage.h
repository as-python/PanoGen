#ifndef CROPIMAGE_H
#define CROPIMAGE_H

#include <QDialog>
#include <QFileDialog>
#include <string>
#include <QPainter>
#include <QMessageBox>
#include <iostream>
#include <QPoint>
#include "mousetrack_qlabel.h"

using namespace std;
//using namespace cv;

namespace Ui {
class CropImage;
}

class CropImage : public QDialog
{
    Q_OBJECT

public:
    explicit CropImage(QWidget *parent = 0);
    ~CropImage();

    void setPathToResources(const string &value);

private slots:
    void on_pushButton_OpenImage_clicked();
    void on_MouseMove();
    void on_MousePressed();
    void on_DrawRectangle();

    void on_radioButton_Top_clicked();

    void on_radioButton_Right_clicked();

    void on_radioButton_Bottom_clicked();

    void on_radioButton_Left_clicked();

signals:
    void drawRectangle();
    void newFrameSize(QPoint leftTop, QPoint rightBottom);

private:
    Ui::CropImage *ui;
    string pathToResources;
    QImage imgToCrop;
    bool isImageLoaded, isTopSet, isBottomSet, isLeftSet, isRightSet;
    int yTop, yBottom, xLeft, xRight;
};

#endif // CROPIMAGE_H
