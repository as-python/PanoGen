#include "gui.h"
#include "ui_gui.h"

Gui::Gui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Gui)
{
    ui->setupUi(this);
    pathToResources = "/home/arjun/workspace/PanoGen/resources/";
}

Gui::~Gui()
{
    delete ui;
}

void Gui::on_pushButton_Calibrate_clicked()
{
    calibWindow = new CalibrationWindow();
    calibWindow->setPathToResources(this->pathToResources);
    calibWindow->show();
}

void Gui::on_pushButton_Calibrate_2_clicked()
{
    reduceFR = new Reduce_FrameRate();
    reduceFR->setPathToResource(this->pathToResources);
    reduceFR->show();
}

void Gui::on_pushButton_HomographyTrans_clicked()
{
    homographyTrans = new Homography_Transformation();
    homographyTrans->setPathToResource(this->pathToResources);
    homographyTrans->show();
}

void Gui::on_pushButton_HomographyTrans_BlendAndPlay_clicked()
{
    blendNPlay = new Blend_And_Play();
    blendNPlay->setPathToResource(this->pathToResources);
    blendNPlay->show();

}
