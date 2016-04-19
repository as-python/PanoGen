#include "blend_and_play.h"
#include "ui_blend_and_play.h"

Blend_And_Play::Blend_And_Play(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Blend_And_Play)
{
    ui->setupUi(this);
}

Blend_And_Play::~Blend_And_Play()
{
    delete ui;
}

void Blend_And_Play::on_pushButton_BrowseLeftVideo_clicked()
{
    QString filter = "Vide files (*.png *.jpg *.bmp)";
    //QString filter = "Vide files (*.avi *.mp4 *.MP4)";
    QString image_Path = QFileDialog::getOpenFileName(this, tr("Select an image"), QString::fromStdString(pathToResource), filter);

    if( !image_Path.isEmpty())
        ui->lineEdit_LeftVideo->setText(image_Path);
}

void Blend_And_Play::on_pushButton_BrowseFrontVideo_clicked()
{
    QString filter = "Vide files (*.png *.jpg *.bmp)";
    //QString filter = "Vide files (*.avi *.mp4 *.MP4)";
    QString image_Path = QFileDialog::getOpenFileName(this, tr("Select an image"), QString::fromStdString(pathToResource), filter);

    if( !image_Path.isEmpty())
        ui->lineEdit_FrontVideo->setText(image_Path);
}

void Blend_And_Play::on_pushButton_BrowseRightVideo_clicked()
{
    QString filter = "Vide files (*.png *.jpg *.bmp)";
    //QString filter = "Vide files (*.avi *.mp4 *.MP4)";
    QString image_Path = QFileDialog::getOpenFileName(this, tr("Select an image"), QString::fromStdString(pathToResource), filter);

    if( !image_Path.isEmpty())
        ui->lineEdit_RightVideo->setText(image_Path);
}

void Blend_And_Play::on_pushButton_BrowseLFMap_clicked()
{
    QString filter = "Map files (*.xml)";
    QString image_Path = QFileDialog::getOpenFileName(this, tr("Select the Left-Front Map"), QString::fromStdString(pathToResource), filter);

    if( !image_Path.isEmpty())
        ui->lineEdit_LFMap->setText(image_Path);
}

void Blend_And_Play::on_pushButton_BrowseFPMap_clicked()
{
    QString filter = "Map files (*.xml)";
    QString image_Path = QFileDialog::getOpenFileName(this, tr("Select the Front-Right Map"), QString::fromStdString(pathToResource), filter);

    if( !image_Path.isEmpty())
        ui->lineEdit_FRMap->setText(image_Path);
}
void Blend_And_Play::setPathToResource(const string &value)
{
    pathToResource = value;
}

void Blend_And_Play::on_pushButton_Play_clicked()
{
    if(ui->lineEdit_LeftVideo->text().isEmpty() || ui->lineEdit_FrontVideo->text().isEmpty() || ui->lineEdit_RightVideo->text().isEmpty())
    {
        QMessageBox::information(0, "Error!", "Please select the three images");
        return;
    }
    if(ui->lineEdit_LFMap->text().isEmpty() || ui->lineEdit_FRMap->text().isEmpty())
    {
        QMessageBox::information(0, "Error!", "Please select the Map files");
        return;
    }
    if(ui->lineEdit_BlendWinWidth->text().isEmpty())
    {
        QMessageBox::information(0, "Error!", "Please enter the Blend window Size");
        return;
    }
    readMaps();

    for(int i=0; i<350; i++)
    {
        nextImage();
        applyMaps_LeftFront();
        applyMaps_FrontRight();
        blend_LeftFront();
        blend_FrontRight();
        combineAndSet();

//        double t = (double)getTickCount();
//        t = ((double)getTickCount() - t)/getTickFrequency();
//        std::cout << "Times passed in seconds: " << t << std::endl;
    }

}


bool Blend_And_Play::readMaps()
{
    string mapName_LF, mapName_FR;
    mapName_LF = ui->lineEdit_LFMap->text().toStdString();
    mapName_FR = ui->lineEdit_FRMap->text().toStdString();

    FileStorage fs_LF(mapName_LF, FileStorage::READ);
    if( !fs_LF.isOpened() )
    {
        QMessageBox::information(0, "Error!", "Error reading Left-Front Map!");
        return false;
    }
    fs_LF["src_img_size"] >> srcImg_Size;
    fs_LF["pad_Size"] >> padSize;
    fs_LF["pano_size"] >> panoLF_Size;

    fs_LF["Map_X"] >> MapX_LeftFront_32f;
    fs_LF["Map_Y"] >> MapY_LeftFront_32f;
    fs_LF.release();

    FileStorage fs_FR(mapName_FR, FileStorage::READ);
    if( !fs_FR.isOpened() )
    {
        QMessageBox::information(0, "Error!", "Error reading Front-Right Map!");
        return false;
    }
    fs_FR["pano_size"] >> panoFR_Size;

    fs_FR["Map_X"] >> MapX_FrontRight_32f;
    fs_FR["Map_Y"] >> MapY_FrontRight_32f;
    fs_FR.release();

    blendWindow_Size = Size( ui->lineEdit_BlendWinWidth->text().toInt(), srcImg_Size.height );

    videoCap_Left = VideoCapture( ui->lineEdit_LeftVideo->text().toStdString() );
    videoCap_Front = VideoCapture( ui->lineEdit_FrontVideo->text().toStdString() );
    videoCap_Right = VideoCapture( ui->lineEdit_RightVideo->text().toStdString() );
    if(!videoCap_Left.isOpened() || !videoCap_Front.isOpened() || !videoCap_Right.isOpened())
    {
        QMessageBox::information(0, "Error!", "Error reading Videos from the disc!");
        return false;
    }
    return true;
}

void Blend_And_Play::applyMaps_LeftFront()
{
    //Mat leftImg_Src = imread( ui->lineEdit_LeftVideo->text().toStdString() );
    //frontImg_Src = imread( ui->lineEdit_FrontVideo->text().toStdString() );

    if( !leftImg_Src.data || !frontImg_Src.data )
    {
        QMessageBox::information(0, "Error!", "Error reading images from disc. Check the image paths!");
        return;
    }

    leftBorder = MapX_LeftFront_32f.cols-srcImg_Size.width;
    rightBorder = MapX_LeftFront_32f.cols;

    Mat leftImg_Padded = Mat(srcImg_Size.height+padSize.height/2, srcImg_Size.width, leftImg_Src.type());
    leftImg_Src.copyTo(leftImg_Padded(Rect(0, padSize.height/2, srcImg_Size.width, srcImg_Size.height)) );

    Mat rectifiedImg_LF(MapX_LeftFront_32f.rows, MapX_LeftFront_32f.cols, leftImg_Src.type());
    cv::remap( leftImg_Padded, rectifiedImg_LF, MapX_LeftFront_32f, MapY_LeftFront_32f, cv::INTER_LINEAR );



    //Mat half( rectifiedImg_LF, cv::Rect( leftBorder, padSize.height/2, srcImg_Size.width, srcImg_Size.height ));
    //frontImg_Src.copyTo( half );


    rectifiedImg_LFCropped = Mat(srcImg_Size.height, rectifiedImg_LF.cols, rectifiedImg_LF.type());
    rectifiedImg_LF(Rect(0, padSize.height/2, rectifiedImg_LF.cols, srcImg_Size.height)).copyTo(rectifiedImg_LFCropped);

//    namedWindow("Left Front", WINDOW_NORMAL);
//    imshow( "Left Front", rectifiedImg_LFCropped );
}

void Blend_And_Play::applyMaps_FrontRight()
{
    //frontImg_Src = imread( ui->lineEdit_FrontVideo->text().toStdString() );
    //Mat rightImg_Src = imread( ui->lineEdit_RightVideo->text().toStdString() );

    if( !frontImg_Src.data || !rightImg_Src.data )
    {
        QMessageBox::information(0, "Error!", "Error reading images from disc. Check the image paths!");
        return;
    }

    Mat rightImg_Padded = Mat(srcImg_Size.height+padSize.height/2, srcImg_Size.width, rightImg_Src.type());
    rightImg_Src.copyTo(rightImg_Padded(Rect(0, padSize.height/2, srcImg_Size.width, srcImg_Size.height)) );

    Mat rectifiedImg_FR(MapX_FrontRight_32f.rows, MapX_FrontRight_32f.cols, frontImg_Src.type());
    cv::remap( rightImg_Padded, rectifiedImg_FR, MapX_FrontRight_32f, MapY_FrontRight_32f, cv::INTER_LINEAR );
    //Mat half_( rectifiedImg_FR, cv::Rect( 0, padSize.height/2, srcImg_Size.width, srcImg_Size.height ));
    //frontImg_Src.copyTo( half_ );

    rectifiedImg_FR_Cropped = Mat(srcImg_Size.height, rectifiedImg_FR.cols, rectifiedImg_FR.type());
    rectifiedImg_FR(Rect(0, padSize.height/2, rectifiedImg_FR.cols, srcImg_Size.height)).copyTo(rectifiedImg_FR_Cropped);

//    namedWindow("Front Right", WINDOW_NORMAL);
//    imshow( "Front Right", rectifiedImg_FR_Cropped );
}

void Blend_And_Play::blend_LeftFront()
{
    blendedImg_LeftFront = Mat( blendWindow_Size, frontImg_Src.type());

    Mat_<Vec3f> blendMaskLeft( blendWindow_Size, CV_32FC3 );
    Mat_<Vec3f> blendMaskFront( blendWindow_Size, CV_32FC3 );

    getBlendMask(blendMaskLeft, blendMaskFront, blendWindow_Size);

    Mat blendWindowLeft, blendWindowFront;

    rectifiedImg_LFCropped(Rect(leftBorder, 0, blendWindow_Size.width, srcImg_Size.height)).copyTo(blendWindowLeft);
    frontImg_Src(Rect(0, 0, blendWindow_Size.width, srcImg_Size.height)).copyTo(blendWindowFront);

//    imshow("left", blendWindowLeft);
//    imshow("right", blendWindowFront);

    int channels = blendWindowLeft.channels();
    int nRows = blendWindowLeft.rows;
    int nCols = blendWindowLeft.cols * channels;

    Mat_<float> m(blendWindow_Size, 0.0);
    m(Range::all(),Range(0,m.cols/2)) = 1.0;


    applyBlending(blendWindowLeft, blendWindowFront, blendedImg_LeftFront, blendMaskLeft, blendMaskFront, nRows, nCols);

//    namedWindow("window", CV_WINDOW_NORMAL);
//    Mat leftPano( rectifiedImg_LFCropped.size(), frontImg_Src.type());
//    rectifiedImg_LFCropped(Rect( 0, 0, leftBorder, srcImg_Size.height )).copyTo(leftPano(Rect( 0, 0, leftBorder, srcImg_Size.height )));
//    blendedImg_LeftFront(Rect( 0, 0, blendWindow_Size.width, srcImg_Size.height )).copyTo(leftPano(Rect( leftBorder, 0, blendWindow_Size.width, srcImg_Size.height )));
//    frontImg_Src(Rect( blendWindow_Size.width, 0, srcImg_Size.width-blendWindow_Size.width, srcImg_Size.height )).
//            copyTo(leftPano(Rect( leftBorder+blendWindow_Size.width, 0, srcImg_Size.width-blendWindow_Size.width, srcImg_Size.height )));
//    imshow("window", leftPano);

}

void Blend_And_Play::blend_FrontRight()
{
    blendedImg_FrontRight = Mat( blendWindow_Size, frontImg_Src.type());

    Mat_<Vec3f> blendMaskFront( blendWindow_Size, CV_32FC3 );
    Mat_<Vec3f> blendMaskRight( blendWindow_Size, CV_32FC3 );

    getBlendMask(blendMaskFront, blendMaskRight, blendWindow_Size);

    Mat blendWindowFront, blendWindowRight;

    frontImg_Src( Rect( srcImg_Size.width-blendWindow_Size.width, 0, blendWindow_Size.width, srcImg_Size.height ) ).copyTo(blendWindowFront);
    rectifiedImg_FR_Cropped( Rect( srcImg_Size.width-blendWindow_Size.width, 0, blendWindow_Size.width, srcImg_Size.height)).copyTo(blendWindowRight);

//    imshow("left", blendWindowFront);
//    imshow("right", blendWindowRight);

    int channels = blendWindowFront.channels();
    int nRows = blendWindowFront.rows;
    int nCols = blendWindowFront.cols * channels;

    Mat_<float> m(blendWindow_Size, 0.0);
    m(Range::all(),Range(0,m.cols/2)) = 1.0;

    //namedWindow("rightPano", CV_WINDOW_NORMAL);
    applyBlending(blendWindowFront, blendWindowRight, blendedImg_FrontRight, blendMaskFront, blendMaskRight, nRows, nCols);

    Mat rightPano( rectifiedImg_FR_Cropped.size(), frontImg_Src.type());

    frontImg_Src(Rect( 0, 0, srcImg_Size.width-blendWindow_Size.width, srcImg_Size.height )).copyTo(rightPano(Rect( 0, 0, srcImg_Size.width-blendWindow_Size.width, srcImg_Size.height )));

    blendedImg_FrontRight( Rect( 0, 0, blendWindow_Size.width, srcImg_Size.height )).
            copyTo(rightPano(Rect( srcImg_Size.width-blendWindow_Size.width, 0, blendWindow_Size.width, srcImg_Size.height )));

    rectifiedImg_FR_Cropped( Rect( srcImg_Size.width, 0, rectifiedImg_FR_Cropped.cols-srcImg_Size.width, srcImg_Size.height )).
            copyTo( rightPano( Rect( srcImg_Size.width, 0, rectifiedImg_FR_Cropped.cols-srcImg_Size.width, srcImg_Size.height )));
    //imshow("rightPano", rightPano);

}

void Blend_And_Play::combineAndSet()
{
    int width = rectifiedImg_LFCropped.cols + (rectifiedImg_FR_Cropped.cols - srcImg_Size.width);
    int height = srcImg_Size.height;
    Mat panorama( height, width, frontImg_Src.type() );

    rectifiedImg_LFCropped(Rect( 0, 0, leftBorder, srcImg_Size.height )).copyTo(panorama(Rect( 0, 0, leftBorder, srcImg_Size.height )));
    blendedImg_LeftFront(Rect( 0, 0, blendWindow_Size.width, srcImg_Size.height )).copyTo(panorama(Rect( leftBorder, 0, blendWindow_Size.width, srcImg_Size.height )));
    frontImg_Src(Rect( blendWindow_Size.width, 0, srcImg_Size.width-blendWindow_Size.width, srcImg_Size.height )).
                copyTo(panorama(Rect( leftBorder+blendWindow_Size.width, 0, srcImg_Size.width-blendWindow_Size.width, srcImg_Size.height )));

    blendedImg_FrontRight( Rect( 0, 0, blendWindow_Size.width, srcImg_Size.height )).
            copyTo(panorama(Rect( rightBorder-blendWindow_Size.width, 0, blendWindow_Size.width, srcImg_Size.height )));

    rectifiedImg_FR_Cropped( Rect( srcImg_Size.width, 0, rectifiedImg_FR_Cropped.cols-srcImg_Size.width, srcImg_Size.height )).
            copyTo( panorama( Rect( rightBorder, 0, rectifiedImg_FR_Cropped.cols-srcImg_Size.width, srcImg_Size.height )));


    QPixmap pixmap( QPixmap::fromImage(Mat2QImage(panorama)));
    int w = ui->label_Panorama->width();
    int h = ui->label_Panorama->height();
    ui->label_Panorama->setPixmap(pixmap.scaled(w,h,Qt::KeepAspectRatio));
    //ui->label_Panorama->setPixmap(pixmap);

    QCoreApplication::processEvents();

    //namedWindow("Panorama", CV_WINDOW_NORMAL);
    //imshow("Panorama", panorama );
    //waitKey(1);
//    cout << panorama.size() << endl;


}

bool Blend_And_Play::nextImage()
{
    leftImg_Src = imread(ui->lineEdit_LeftVideo->text().toStdString());
    frontImg_Src = imread( ui->lineEdit_FrontVideo->text().toStdString() );
    rightImg_Src = imread( ui->lineEdit_RightVideo->text().toStdString() );

//    videoCap_Left.read(leftImg_Src);
//    videoCap_Front.read(frontImg_Src);
//    videoCap_Right.read(rightImg_Src);
}

void Blend_And_Play::getBlendMask(Mat_<Vec3f> &blendMaskLeft, Mat_<Vec3f> &blendMaskRight, Size blendWindowSize)
{
    int channels = blendMaskLeft.channels();
    int nRows = blendMaskLeft.rows;
    int nCols = blendMaskLeft.cols * channels;

    float* ptrBlendMaskLeft;
    float* ptrBlendMaskRight;

    for(int i = 0; i < nRows; ++i)
    {
        float maskParam = 0.0;
        int counter = 0;
        ptrBlendMaskLeft = blendMaskLeft.ptr<float>(i);
        ptrBlendMaskRight = blendMaskRight.ptr<float>(i);
        for (int j = 0; j < nCols; ++j)
        {
            ptrBlendMaskLeft[j] = (blendWindowSize.width-maskParam)/blendWindowSize.width;
            ptrBlendMaskRight[j] = maskParam/blendWindowSize.width;
            counter++;
            if(counter%3 == 0)
            {
                counter = 0;
                maskParam++;
            }
        }
    }
}

void Blend_And_Play::applyBlending(Mat &blendSrcLeft, Mat &blendSrcRight, Mat &blendedImg, Mat_<Vec3f> blendMaskLeft, Mat_<Vec3f> blendMaskRight, int nRows, int nCols)
{
    uchar* ptrBlendSrcL;
    uchar* ptrBlendSrcR;
    uchar* ptrBlendedImg;
    float* ptrBlendMaskLeft;
    float* ptrBlendMaskRight;


    for(int i = 0; i < nRows; ++i)
    {
        ptrBlendSrcL = blendSrcLeft.ptr<uchar>(i);
        ptrBlendSrcR = blendSrcRight.ptr<uchar>(i);
        ptrBlendedImg = blendedImg.ptr<uchar>(i);

        ptrBlendMaskLeft = blendMaskLeft.ptr<float>(i);
        ptrBlendMaskRight = blendMaskRight.ptr<float>(i);
        for (int j = 0; j < nCols; ++j)
        {
            ptrBlendedImg[j] = (ptrBlendSrcL[j] * ptrBlendMaskLeft[j] + ptrBlendSrcR[j] * ptrBlendMaskRight[j]);
        }
    }
}

QImage Blend_And_Play::Mat2QImage(const Mat3b &src)
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




