#include "homography_transformation.h"
#include "ui_homography_transformation.h"

Homography_Transformation::Homography_Transformation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Homography_Transformation)
{
    ui->setupUi(this);
    connect(ui->graphicsView_Img, SIGNAL(mouseMoved(QPointF)), this, SLOT(onMouseMove(QPointF)));
    ui->graphicsView_Img->setImage(QImage("/home/arjun/workspace/Qt/Delete/qt.jpg"));

    ui->radioButton_LeftFront->setChecked(true);
    ui->label_SideImg->setText("Left Image");
}

Homography_Transformation::~Homography_Transformation()
{
    delete ui;
}
void Homography_Transformation::setPathToResource(const string &value)
{
    pathToResource = value;
}

void Homography_Transformation::onMouseMove(QPointF mousePos)
{
    ui->label_MousePos->setText(QString("X = %1, Y = %2").arg(mousePos.x()).arg(mousePos.y()));
}


void Homography_Transformation::on_pushButton_BrowseSideImg_clicked()
{
    QString filter = "Image files (*.png *.jpg *.bmp)";
    QString image_Path = QFileDialog::getOpenFileName(this, tr("Select an image"), QString::fromStdString(pathToResource), filter);

    if( !image_Path.isEmpty())
        ui->lineEdit_SideImg->setText(image_Path);
}

void Homography_Transformation::on_pushButton_BrowseFrontImg_clicked()
{
    QString filter = "Image files (*.png *.jpg *.bmp)";
    QString image_Path = QFileDialog::getOpenFileName(this, tr("Select an image"), QString::fromStdString(pathToResource), filter);

    if( !image_Path.isEmpty())
        ui->lineEdit_FrontImg->setText(image_Path);
}

void Homography_Transformation::on_pushButton_EstimateHomography_clicked()
{
    if(ui->lineEdit_FrontImg->text().toStdString().empty() || ui->lineEdit_SideImg->text().toStdString().empty())
    {
        QMessageBox::information(0, "Error!", "Please select both the images to estimate the homography");
        return;
    }
    if(ui->lineEdit_KeypointNum->text().toStdString().empty())
    {
        QMessageBox::information(0, "Error!", "Please enter the number of keypoints to be deteced in the images");
        return;
    }

    Mat sideImg_Src = imread( ui->lineEdit_SideImg->text().toStdString() );
    Mat frontImg_Src = imread( ui->lineEdit_FrontImg->text().toStdString() );

    if( !sideImg_Src.data || !frontImg_Src.data )
    {
        QMessageBox::information(0, "Error!", "Error reading images from disc. Check the image paths!");
        return;
    }

    if(sideImg_Src.size != frontImg_Src.size)
    {
        QMessageBox::information(0, "Error!", "Both the images should be of the same size!");
        return;
    }
    Size srcImg_Size = frontImg_Src.size;


//    namedWindow("Side Image", WINDOW_NORMAL);
//    imshow("Side Image", sideImg_Src);
//    namedWindow("Front Image", WINDOW_NORMAL);
//    imshow("Front Image", frontImg_Src);

    int height_pad = 1500, width_pad = 2000;//3000 2500
    //int height_pad = 2500, width_pad = 3000;//3000 2500

    Mat sideImg_Padded(sideImg_Src.rows+height_pad, sideImg_Src.cols, sideImg_Src.type());
    Mat frontImg_Padded(frontImg_Src.rows+height_pad, frontImg_Src.cols+width_pad, sideImg_Src.type());

    sideImg_Src.copyTo(sideImg_Padded(Rect(0, height_pad/2, sideImg_Src.cols,sideImg_Src.rows)) );
    frontImg_Src.copyTo(frontImg_Padded(Rect(width_pad, height_pad/2, frontImg_Src.cols,frontImg_Src.rows)) );

    namedWindow("Side Image Padded", WINDOW_NORMAL);
    imshow("Side Image Padded", sideImg_Padded);
    namedWindow("Front Image Padded", WINDOW_NORMAL);
    imshow("Front Image Padded", frontImg_Padded);
    waitKey(0);

    Mat frontImg_Gray, sideImg_Gray;

    // Convert to Grayscale
    cvtColor( sideImg_Padded, sideImg_Gray, CV_RGB2GRAY );
    cvtColor( frontImg_Padded, frontImg_Gray, CV_RGB2GRAY );

    if( !sideImg_Gray.data || !frontImg_Gray.data )
    {
        QMessageBox::information(0, "Error!", "Problem converting color image to gray image");
        return;
    }

    Mat frontImg_Mask = Mat::zeros(frontImg_Gray.size(), CV_8UC1);
    frontImg_Mask(Rect(width_pad, height_pad/2, srcImg_Size.width/2, srcImg_Size.height)) = 1;

    Mat sideImg_Mask = Mat::zeros(sideImg_Gray.size(), CV_8UC1);
    sideImg_Mask( Rect( srcImg_Size.width/2, height_pad/2, srcImg_Size.width/2, srcImg_Size.height)) = 1;
//    Mat left_color= imread( ui->lineEdit_SideImg->text().toStdString() );
//    Mat right_color= imread( ui->lineEdit_FrontImg->text().toStdString() );

//    namedWindow("Left Image", WINDOW_NORMAL);
//    imshow("Left Image", left_color);
//    namedWindow("Right Image", WINDOW_NORMAL);
//    imshow("Right Image", right_color);

//    int height_pad = 1500, width_pad = 2100;//3000 2500
//    //int height_pad = 2500, width_pad = 3000;//3000 2500

//    Mat left_color_image(left_color.rows+height_pad, left_color.cols, left_color.type());
//    Mat right_color_image(right_color.rows+height_pad, right_color.cols+width_pad, left_color.type());
//    left_color.copyTo(left_color_image(Rect(0, height_pad/2, left_color.cols,left_color.rows)) );
//    right_color.copyTo(right_color_image(Rect(width_pad, height_pad/2, left_color.cols,left_color.rows)) );

//    /*namedWindow("left", WINDOW_NORMAL);
//    imshow("left", left_color_image);
//    namedWindow("right", WINDOW_NORMAL);
//    imshow("right", right_color_image);
//    waitKey(0);*/

//    Mat right_gray_image, left_gray_image;

//    // Convert to Grayscale
//    cvtColor( left_color_image, left_gray_image, CV_RGB2GRAY );
//    cvtColor( right_color_image, right_gray_image, CV_RGB2GRAY );

//    if( !left_gray_image.data || !right_gray_image.data )
//    {
//        std::cout<< " --(!) Error reading images " << std::endl; //return -1;
//    }

//    Mat right_image_mask = Mat::zeros(right_gray_image.size(), CV_8UC1);
//    right_image_mask(Rect(width_pad, height_pad/2,right_color.cols/2, right_color.rows)) = 1;

//    Mat left_image_mask = Mat::zeros(left_gray_image.size(), CV_8UC1);
//    left_image_mask(Rect(left_color.cols/2, height_pad/2, left_color.cols/2, left_color.rows)) = 1;

//    //-- Step 1: Detect the keypoints using SIFT Detector

//    SiftFeatureDetector detector(500);//400
//    std::vector< KeyPoint > keypoints_right_image, keypoints_left_image;

//    detector.detect( right_gray_image, keypoints_right_image, right_image_mask );
//    detector.detect( left_gray_image, keypoints_left_image, left_image_mask );

//    /*cv::Mat left_image_with_keypoint, right_image_with_keypoint;
//    cv::drawKeypoints( left_gray_image, keypoints_left_image, left_image_with_keypoint );
//    cv::drawKeypoints( right_gray_image, keypoints_right_image, right_image_with_keypoint );
//    namedWindow("Left image with Keypoints", WINDOW_NORMAL);
//    cv::imshow( "Left image with Keypoints", left_image_with_keypoint );
//    namedWindow("Right image with Keypoints", WINDOW_NORMAL);
//    cv::imshow( "Right image with Keypoints", right_image_with_keypoint );*/

//    //-- Step 2: Calculate descriptors (feature vectors)
//    SiftDescriptorExtractor extractor;

//    Mat descriptors_left_image, descriptors_right_image;

//    extractor.compute( left_gray_image, keypoints_left_image, descriptors_left_image );
//    extractor.compute( right_gray_image, keypoints_right_image, descriptors_right_image );

//    //-- Step 3: Matching descriptor vectors using FLANN matcher
//    FlannBasedMatcher matcher;
//    std::vector< DMatch > matches;
//    matcher.match( descriptors_right_image, descriptors_left_image, matches );

//    double max_dist = 0; double min_dist = 100;

//    //-- Quick calculation of max and min distances between keypoints
//    for( int i = 0; i < descriptors_right_image.rows; i++ )
//    {
//        double dist = matches[i].distance;
//        if( dist < min_dist ) min_dist = dist;
//        if( dist > max_dist ) max_dist = dist;
//    }

//    cout << "-- Max dist : " << max_dist << endl;
//    cout << "-- Min dist : " << min_dist << endl;

//    //-- Use only "good" matches (i.e. whose distance is less than 3*min_dist )
//    std::vector< DMatch > good_matches;

//    for( int i = 0; i < descriptors_right_image.rows; i++ )
//    {
//        if( matches[i].distance < 3*min_dist )
//        {
//            good_matches.push_back( matches[i] );
//        }
//    }

//    cout << "good matches size:" << good_matches.size() << endl;
//    Mat img_matches;
//    drawMatches( right_gray_image, keypoints_right_image, left_gray_image, keypoints_left_image,
//                       good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
//                       vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
//    namedWindow("Good Matches", WINDOW_NORMAL);
//    imshow( "Good Matches", img_matches );

//    std::vector< Point2f > good_keypoints_right_image;
//    std::vector< Point2f > good_keypoints_left_image;

//    for( std::size_t i = 0; i < good_matches.size(); i++ )
//    {
//        //-- Get the keypoints from the good matches
//        good_keypoints_right_image.push_back( keypoints_right_image[ good_matches[i].queryIdx ].pt );
//        good_keypoints_left_image.push_back( keypoints_left_image[ good_matches[i].trainIdx ].pt );
//    }

//    Mat Homography = findHomography( good_keypoints_right_image, good_keypoints_left_image, CV_RANSAC, 2 );
//    int height = right_color_image.rows;
//    int width = (right_color_image.cols);

//    Homography.convertTo(Homography, CV_32FC1);
//    cv::Mat_<float> homo_mapx_32f(height, width), homo_mapy_32f(height, width);
//    for(int y = 0; y<height; ++y)
//    {
//        float *buff_mapx = ((float*) homo_mapx_32f.data)+y*width;
//        float *buff_mapy = ((float*) homo_mapy_32f.data)+y*width;
//        for(int x = 0; x<width; ++x)
//        {
//            cv::Mat_<float> pt(3, 1);
//            pt(0) = x;
//            pt(1) = y;
//            pt(2) = 1;
//            pt = Homography*pt;
//            pt /= pt(2);
//            buff_mapx[x] = pt(0);
//            buff_mapy[x] = pt(1);
//        }
//    }
}

void Homography_Transformation::on_radioButton_LeftFront_toggled(bool checked)
{
    if(checked)
    {
        ui->label_SideImg->setText("Left Image");
        //ui->lineEdit_SideImg->clear();
    }
}

void Homography_Transformation::on_radioButton_FrontRight_toggled(bool checked)
{
    if(checked)
    {
        ui->label_SideImg->setText("Right Image");
        //ui->lineEdit_SideImg->clear();
    }

}
