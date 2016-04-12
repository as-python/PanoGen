#include "cropimage.h"
#include "ui_cropimage.h"

CropImage::CropImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CropImage)
{
    ui->setupUi(this);
    //ui->label_CalibSample->setVisible(false);
    ui->frame_Crop->setEnabled(false);
    ui->label_ImgExample->setVisible(false);

    isImageLoaded = false;
    isTopSet =false;
    isBottomSet = false;
    isLeftSet = false;
    isRightSet = false;

    connect(ui->label_Image, SIGNAL(mouse_Position()), this, SLOT(on_MouseMove()));
    connect(ui->label_Image, SIGNAL(mouse_Pressed()), this, SLOT(on_MousePressed()));
    connect(this, SIGNAL(drawRectangle()), this, SLOT(on_DrawRectangle()));
}

CropImage::~CropImage()
{
    delete ui;
}

void CropImage::on_pushButton_OpenImage_clicked()
{
    QString filter = "Image files (*.png *.jpg *.bmp)";
    QString image_Path = QFileDialog::getOpenFileName(this, tr("Select an image"), QString::fromStdString(pathToResources), filter);

    if( !image_Path.isEmpty())
    {
        QPixmap pixmap(image_Path);
        ui->label_Image->setPixmap(pixmap);
        imgToCrop = pixmap.toImage();
        isImageLoaded = true;
        if(!ui->frame_Crop->isEnabled())
            ui->frame_Crop->setEnabled(true);
    }

    /*ui->label_CalibSample->setVisible(true);
    QPixmap pixmap(QString::fromStdString( pathToResources+ "calibSample.png" ));
    int w = ui->label_CalibSample->width();
    int h = ui->label_CalibSample->height();
    ui->label_CalibSample->setPixmap(pixmap.scaled(w,h,Qt::KeepAspectRatio));*/
}
void CropImage::setPathToResources(const string &value)
{
    pathToResources = value;
}

void CropImage::on_MouseMove()
{
    //const QPixmap* pixmap = ui->label_Image->pixmap();
    //QImage img = pixmap->toImage();
    if( isImageLoaded )
    {
        QColor pixelValue = imgToCrop.pixel(ui->label_Image->x,ui->label_Image->y);
        ui->label_MousePosition->setText(QString("X = %1, Y = %2").arg(ui->label_Image->x).arg(ui->label_Image->y));
        ui->label_RgbValue->setText(QString("RGB: (%1, %2, %3)").arg(pixelValue.red()).arg(pixelValue.green()).arg(pixelValue.blue()));
    }
    else
    {
        ui->label_MousePosition->setText(QString("X = %1, Y = %2").arg(ui->label_Image->x).arg(ui->label_Image->y));
    }

}

void CropImage::on_MousePressed()
{
    if(ui->radioButton_Top->isChecked())
    {
        ui->label_Top->setText(QString("X = %1, Y = %2").arg(ui->label_Image->x).arg(ui->label_Image->y));
        isTopSet = true;
        yTop = ui->label_Image->y;
        if(isTopSet && isBottomSet && isLeftSet && isRightSet)
            emit drawRectangle();
    }
    else if(ui->radioButton_Right->isChecked())
    {
        ui->label_Right->setText(QString("X = %1, Y = %2").arg(ui->label_Image->x).arg(ui->label_Image->y));
        isRightSet = true;
        xRight = ui->label_Image->x;
        if(isTopSet && isBottomSet && isLeftSet && isRightSet)
            emit drawRectangle();
    }
    else if(ui->radioButton_Bottom->isChecked())
    {
        ui->label_Bottom->setText(QString("X = %1, Y = %2").arg(ui->label_Image->x).arg(ui->label_Image->y));
        isBottomSet = true;
        yBottom = ui->label_Image->y;
        if(isTopSet && isBottomSet && isLeftSet && isRightSet)
            emit drawRectangle();
    }
    else if(ui->radioButton_Left->isChecked())
    {
        ui->label_Left->setText(QString("X = %1, Y = %2").arg(ui->label_Image->x).arg(ui->label_Image->y));
        isLeftSet = true;
        xLeft = ui->label_Image->x;
        if(isTopSet && isBottomSet && isLeftSet && isRightSet)
        {
            emit drawRectangle();
            QMessageBox msgBox;
            msgBox.setText("The new frame size has been saved in the Calibration Map file");
            msgBox.exec();
        }
    }
    //ui->label_MousePosition->setText("mouse pressed!");
}

void CropImage::on_DrawRectangle()
{
    QPixmap pixmap = QPixmap::fromImage(imgToCrop);
    QPainter painter (&pixmap);
    QPen pen( Qt::red );
    pen.setWidth(1);
    painter.setPen (pen);

    painter.drawRect(xLeft, yTop, (xRight-xLeft), (yBottom-yTop));
    painter.end ();

    ui->label_Image->setPixmap(pixmap);
    emit newFrameSize(QPoint(xLeft,yTop), QPoint(xRight, yBottom));
}

void CropImage::on_radioButton_Top_clicked()
{
    if(!ui->label_ImgExample->isVisible())
        ui->label_ImgExample->setVisible(true);

    int w = ui->label_ImgExample->width();
    int h = ui->label_ImgExample->height();
    QPixmap pixmap(QString::fromStdString(pathToResources + "gui_pictures/top.png"));
    ui->label_ImgExample->setPixmap(pixmap.scaled(w,h,Qt::KeepAspectRatio));

}

void CropImage::on_radioButton_Right_clicked()
{
    if(!ui->label_ImgExample->isVisible())
        ui->label_ImgExample->setVisible(true);

    int w = ui->label_ImgExample->width();
    int h = ui->label_ImgExample->height();
    QPixmap pixmap(QString::fromStdString(pathToResources + "gui_pictures/right.png"));
    ui->label_ImgExample->setPixmap(pixmap.scaled(w,h,Qt::KeepAspectRatio));
}

void CropImage::on_radioButton_Bottom_clicked()
{
    if(!ui->label_ImgExample->isVisible())
        ui->label_ImgExample->setVisible(true);

    int w = ui->label_ImgExample->width();
    int h = ui->label_ImgExample->height();
    QPixmap pixmap(QString::fromStdString(pathToResources + "gui_pictures/bottom.png"));
    ui->label_ImgExample->setPixmap(pixmap.scaled(w,h,Qt::KeepAspectRatio));
}

void CropImage::on_radioButton_Left_clicked()
{
    if(!ui->label_ImgExample->isVisible())
        ui->label_ImgExample->setVisible(true);

    int w = ui->label_ImgExample->width();
    int h = ui->label_ImgExample->height();
    QPixmap pixmap(QString::fromStdString(pathToResources + "gui_pictures/left.png"));
    ui->label_ImgExample->setPixmap(pixmap.scaled(w,h,Qt::KeepAspectRatio));
}
