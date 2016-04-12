#include "calibrationwindow.h"
#include "ui_calibrationwindow.h"

CalibrationWindow::CalibrationWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibrationWindow)
{
    ui->setupUi(this);
    camCalib = new CameraCalibration();
    connect(camCalib, SIGNAL(signal_ChangeImage(Mat)), this, SLOT(setImage(Mat)));
    connect( camCalib, SIGNAL(signal_ChangeTitleText(QString)), this, SLOT(setImageLabelText(QString)) );
    connect( camCalib, SIGNAL(signal_ChangeImageCounterText(QString)), this, SLOT(setImageCounterText(QString)) );
    connect( camCalib, SIGNAL(signal_setStatus(QString)), this, SLOT(setStatus(QString)) );

    cropImgWindow = new CropImage();
    connect(cropImgWindow, SIGNAL(newFrameSize(QPoint,QPoint)), this, SLOT(save_NewFrameSize(QPoint,QPoint)));

    ui->lineEdit_NrFrames->setValidator(new QRegExpValidator( QRegExp("[0-9]+e[0-9]+")));
    ui->lineEdit_NrCols->setValidator(new QRegExpValidator( QRegExp("[0-9]+e[0-9]+")));
    ui->lineEdit_NrRows->setValidator(new QRegExpValidator( QRegExp("[0-9]+e[0-9]+")));

    ui->comboBox_CalibPattern->addItem("Chessboard", CameraCalibration::CHESSBOARD);
    ui->comboBox_CalibPattern->addItem("Circles grid", CameraCalibration::CIRCLES_GRID);
    ui->comboBox_CalibPattern->addItem("Asymmetrical circles grid", CameraCalibration::ASYMMETRIC_CIRCLES_GRID);


    ui->comboBox_InputType->addItem("Image list", CameraCalibration::IMAGE_LIST);
    ui->comboBox_InputType->addItem("Video file", CameraCalibration::VIDEO_FILE);
    ui->comboBox_InputType->addItem("Camera", CameraCalibration::CAMERA);

    ui->label_ImageCounter->setVisible(false);
    ui->label_Status->setVisible(false);
}

CalibrationWindow::~CalibrationWindow()
{
    delete ui;
}

QImage CalibrationWindow::Mat2QImage(const Mat3b &src)
{
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
    for (int y = 0; y < src.rows; ++y) {
            const cv::Vec3b *srcrow = src[y];
            QRgb *destrow = (QRgb*)dest.scanLine(y);
            for (int x = 0; x < src.cols; ++x) {
                    destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
            }
    }
    return dest;
}

void CalibrationWindow::on_pushButton_Calibrate_clicked()
{
    ui->groupBox_CalibSettings->setEnabled(false);
    Size boardSize;
    boardSize.height = ui->lineEdit_NrRows->text().toInt()-1;
    boardSize.width = ui->lineEdit_NrCols->text().toInt()-1;
    camCalib->setBoardSize(boardSize);
    camCalib->setCalibrationPattern((CameraCalibration::Pattern)ui->comboBox_CalibPattern->itemData(ui->comboBox_CalibPattern->currentIndex()).toInt());
    camCalib->setInputType((CameraCalibration::InputType)ui->comboBox_InputType->itemData(ui->comboBox_InputType->currentIndex()).toInt());
    camCalib->setNrFrames(ui->lineEdit_NrFrames->text().toInt());
    camCalib->setOutputFileName(ui->lineEdit_LogName->text().toStdString());
    camCalib->setOutput_mapfile_name(ui->lineEdit_MapName->text().toStdString());
    camCalib->setInput(ui->lineEdit_ImgList->text().toStdString());
    camCalib->setShowUndistorsed(ui->checkBox_ShowUndistort->isChecked());
    camCalib->setSaveUndistorsed(ui->checkBox_SaveUndistort->isChecked());

    camCalib->setSquareSize(35);
    camCalib->setAspectRatio(1);
    camCalib->setBwritePoints(false);
    camCalib->setBwriteExtrinsics(true);
    camCalib->setDelay(100);
    camCalib->setCalibZeroTangentDist(true);
    camCalib->setCalibFixPrincipalPoint(true);
    camCalib->setPath(pathToResources);

    bool success = camCalib->calibrate();

    if(success)
        cout << "Successfully calibrated the camera and saved the maps" << endl;
    else
        cout << "ERROR: Camera calibration failed! Refer cameracalibration.cpp" << endl;

    ui->groupBox_CalibSettings->setEnabled(true);
}

void CalibrationWindow::on_pushButton_ChooseImgList_clicked()
{
    QString filter = "File Description (*.xml)";
    QString image_List = QFileDialog::getOpenFileName(this, tr("Select Image list"), QString::fromStdString(pathToResources), filter);

    ui->lineEdit_ImgList->setText(image_List);
}

void CalibrationWindow::setImage(Mat cv_Img)
{
    QPixmap pixmap(QPixmap::fromImage(Mat2QImage(cv_Img)));
    int w = ui->label_Image->width();
    int h = ui->label_Image->height();
    ui->label_Image->setPixmap(pixmap.scaled(w,h,Qt::KeepAspectRatio));
}

void CalibrationWindow::setImageLabelText(QString text)
{
    ui->label_ImageTitle->setText(text);
}

void CalibrationWindow::setImageCounterText(QString text)
{
    if(!ui->label_ImageCounter->isVisible())
        ui->label_ImageCounter->setVisible(true);
    ui->label_ImageCounter->setText(text);
}

void CalibrationWindow::setStatus(QString text)
{
    if(!ui->label_Status->isVisible())
        ui->label_Status->setVisible(true);

    ui->label_Status->setText(text);
}
void CalibrationWindow::setPathToResources(const string &value)
{
    pathToResources = value;
}


void CalibrationWindow::on_pushButton_CropImage_clicked()
{
    cropImgWindow->setPathToResources(this->pathToResources);
    cropImgWindow->setModal(true);
    cropImgWindow->exec();
}

void CalibrationWindow::save_NewFrameSize(QPoint topLeft, QPoint bottomRight)
{
    cout << "Herrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"<<endl;
    FileStorage fs( pathToResources + "maps/" + ui->lineEdit_MapName->text().toStdString(), FileStorage::APPEND );

    fs << "new_Left_X" << topLeft.x();
    fs << "new_Top_Y" << topLeft.y();
    fs << "new_Right_X" << bottomRight.x();
    fs << "new_Bottom_Y" << bottomRight.y();
    fs << "cropImage_Width" << ( bottomRight.x() - topLeft.x());
    fs << "cropImage_Height" << ( bottomRight.y() - topLeft.y());

    fs.release();


}
