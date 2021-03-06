#include "homography_transformation.h"
#include "ui_homography_transformation.h"

Homography_Transformation::Homography_Transformation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Homography_Transformation)
{
    ui->setupUi(this);
    connect(ui->graphicsView_Img, SIGNAL(mouseMoved(QPointF)), this, SLOT(onMouseMove(QPointF)));
    connect(ui->graphicsView_Img, SIGNAL(mousePressed(QPointF)), this, SLOT(onMouseClick(QPointF)));
    ui->graphicsView_Img->setImage(QImage("/home/arjun/workspace/Qt/Delete/qt.jpg"));

    ui->radioButton_LeftFront->setChecked(true);
    ui->label_SideImg->setText("Left Image");
    isTopRightSet = false;
    isBottomRightSet = false;
    isBottomLeftSet = false;
    isTopLeftSet =false;
    mapSize = Size(-1, -1);
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

    ui->label_MousePos->setText(QString("X = %1, Y = %2").arg( round(mousePos.x()) ).arg( round(mousePos.y()) ));
}

void Homography_Transformation::onMouseClick(QPointF mousePos)
{
    if(ui->radioButton_TopRight->isChecked())
    {
        pt_TopRight = Point2f( (float)mousePos.x(), (float)mousePos.y() );
        ui->label_TopRight->setText(QString("X = %1, Y = %2").arg( round(mousePos.x()) ).arg( round(mousePos.y())));
        isTopRightSet = true;
        if(isTopLeftSet && isTopRightSet && isBottomLeftSet && isBottomRightSet)
            drawPolygon();
    }
    else if(ui->radioButton_BottomRight->isChecked())
    {
        ui->label_BottomRight->setText(QString("X = %1, Y = %2").arg( round(mousePos.x()) ).arg( round(mousePos.y())));
        isBottomRightSet = true;
        pt_BottomRight = Point2f( (float)mousePos.x(), (float)mousePos.y() );
        if(isTopLeftSet && isTopRightSet && isBottomLeftSet && isBottomRightSet)
            drawPolygon();
    }
    else if(ui->radioButton_BottomLeft->isChecked())
    {
        ui->label_BottomLeft->setText(QString("X = %1, Y = %2").arg( round(mousePos.x()) ).arg( round(mousePos.y())));
        isBottomLeftSet = true;
        pt_BottomLeft = Point2f( (float)mousePos.x(), (float)mousePos.y() );
        if(isTopLeftSet && isTopRightSet && isBottomLeftSet && isBottomRightSet)
            drawPolygon();
    }
    else if(ui->radioButton_TopLeft->isChecked())
    {
        ui->label_TopLeft->setText(QString("X = %1, Y = %2").arg( round(mousePos.x()) ).arg( round(mousePos.y())));
        isTopLeftSet = true;
        pt_TopLeft = Point2f( (float)mousePos.x(), (float)mousePos.y() );
        if(isTopLeftSet && isTopRightSet && isBottomLeftSet && isBottomRightSet)
            drawPolygon();
    }

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

    if(ui->radioButton_LeftFront->isChecked())
        estimateHomography_LeftFront();
    else if(ui->radioButton_FrontRight->isChecked())
        estimateHomography_FrontRight();

}


void Homography_Transformation::on_pushButton_PerfomPerspective_clicked()
{
    if( !isTopLeftSet || !isTopRightSet || !isBottomLeftSet || !isBottomRightSet)
    {
        QMessageBox::information(0, "Error!", "Please select the four corners before proceeding");
        return;
    }
    if(ui->radioButton_LeftFront->isChecked())
        estimatePerspective_LeftFront();
    else if(ui->radioButton_FrontRight->isChecked())
        estimatePerspective_FrontRight();

}


QImage Homography_Transformation::Mat2QImage(const Mat3b &src)
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

void Homography_Transformation::drawPolygon()
{
    if( rectifiedImg_ByHomography.data )
    {
        vector<Point> contour;
        contour.push_back(pt_TopRight);
        contour.push_back(pt_BottomRight);
        contour.push_back(pt_BottomLeft);
        contour.push_back(pt_TopLeft);

        const cv::Point *pts = (const cv::Point*) Mat(contour).data;
        int npts = Mat(contour).rows;

        Mat copyImg;
        rectifiedImg_ByHomography.copyTo(copyImg);
        polylines(copyImg, &pts, &npts, 1, true, Scalar(255, 0, 0), 3, CV_AA, 0);
        ui->graphicsView_Img->setImage(Mat2QImage(copyImg));
    }
}

Point Homography_Transformation::getIntersctionPoint(Point line1_Point1, Point line1_Point2, Point line2_Point1, Point line2_Point2)
{
    float m1, c1, m2, c2;
    float dx, dy;
    float intersection_X, intersection_Y;

    dx = line1_Point2.x - line1_Point1.x;
    dy = line1_Point2.y - line1_Point1.y;

    if( dx==0 )
        dx = 1;
    m1 = dy / dx;
    c1 = line1_Point1.y - m1 * line1_Point1.x;

    dx = line2_Point2.x - line2_Point1.x;
    dy = line2_Point2.y - line2_Point1.y;

    if( dx==0 )
        dx = 1;
    m2 = dy / dx;
    c2 = line2_Point1.y - m2 * line2_Point1.x;

    if( (m1 -m2) == 0 )
    {
        cout << "No intersection" << endl;
        return Point(4,4);
    }
    else
    {
        intersection_X = (c2 - c1) / (m1 - m2);
        intersection_Y = m1 * intersection_X + c1;
        cout << "Intersecting Point: = " << intersection_X << " , " << intersection_Y << endl;
        return Point2f(intersection_X, intersection_Y);
    }
}

bool Homography_Transformation::findNew_PerspectiveVertices()
{
    if(ui->radioButton_LeftFront->isChecked())
    {
        if( (pt_TopRight.x - pt_TopLeft.x) > (srcImg_Size.width * 1.3))
        {
            newWidthPad = srcImg_Size.width;
            padToRemove = padSize.width - newWidthPad;

            Point TopHLine_Point1( round(pt_TopLeft.x), round(pt_TopLeft.y) );
            Point TopHLine_Point2( round(pt_TopRight.x), round(pt_TopRight.y) );

            Point intersectingVLine_Point1( padToRemove, 0 );
            Point intersectingVLine_Point2( padToRemove, mapSize.height );

            Point BottomHLine_Point1( round(pt_BottomLeft.x), round(pt_BottomLeft.y) );
            Point BottomHLine_Point2( round(pt_BottomRight.x), round(pt_BottomRight.y) );

            pt_NewTopLeft = getIntersctionPoint(TopHLine_Point1, TopHLine_Point2, intersectingVLine_Point1, intersectingVLine_Point2);
            pt_NewBottomLeft = getIntersctionPoint(BottomHLine_Point1, BottomHLine_Point2, intersectingVLine_Point1, intersectingVLine_Point2);

            return true;
            //line(copyImg, pt_NewTopLeft, pt_NewBottomLeft, Scalar(0,0,255), 2);

        }
        else
            return false;
    }
    else if(ui->radioButton_FrontRight->isChecked())
    {
        if( (pt_TopRight.x - pt_TopLeft.x) > (srcImg_Size.width * 1.3))
        {
            newWidthPad = srcImg_Size.width;
            padToRemove = padSize.width - newWidthPad;

            Point TopHLine_Point1( round(pt_TopLeft.x), round(pt_TopLeft.y) );
            Point TopHLine_Point2( round(pt_TopRight.x), round(pt_TopRight.y) );

            Point intersectingVLine_Point1( mapSize.width - padToRemove, 0 );
            Point intersectingVLine_Point2( mapSize.width - padToRemove, mapSize.height );

            Point BottomHLine_Point1( round(pt_BottomLeft.x), round(pt_BottomLeft.y) );
            Point BottomHLine_Point2( round(pt_BottomRight.x), round(pt_BottomRight.y) );

            pt_NewTopRight = getIntersctionPoint(TopHLine_Point1, TopHLine_Point2, intersectingVLine_Point1, intersectingVLine_Point2);
            pt_NewBottomRight = getIntersctionPoint(BottomHLine_Point1, BottomHLine_Point2, intersectingVLine_Point1, intersectingVLine_Point2);

            return true;
            //line(copyImg, pt_NewTopLeft, pt_NewBottomLeft, Scalar(0,0,255), 2);

        }
        else
            return false;
    }
    return false;

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


void Homography_Transformation::estimateHomography_LeftFront()
{
    sideImg_Src = imread( ui->lineEdit_SideImg->text().toStdString() );
    frontImg_Src = imread( ui->lineEdit_FrontImg->text().toStdString() );

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
    srcImg_Size = frontImg_Src.size();


//    namedWindow("Side Image", WINDOW_NORMAL);
//    imshow("Side Image", sideImg_Src);
//    namedWindow("Front Image", WINDOW_NORMAL);
//    imshow("Front Image", frontImg_Src);

    padSize = Size( 2000, 1500);//3000 2500
    sideImg_Padded = Mat(sideImg_Src.rows+padSize.height, sideImg_Src.cols, sideImg_Src.type());
    Mat frontImg_Padded(frontImg_Src.rows+padSize.height, frontImg_Src.cols+padSize.width, sideImg_Src.type());

    sideImg_Src.copyTo(sideImg_Padded(Rect(0, padSize.height/2, sideImg_Src.cols,sideImg_Src.rows)) );
    frontImg_Src.copyTo(frontImg_Padded(Rect(padSize.width, padSize.height/2, frontImg_Src.cols,frontImg_Src.rows)) );

//    namedWindow("Side Image Padded", WINDOW_NORMAL);
//    imshow("Side Image Padded", sideImg_Padded);
//    namedWindow("Front Image Padded", WINDOW_NORMAL);
//    imshow("Front Image Padded", frontImg_Padded);

    Mat frontImg_Gray, sideImg_Gray;
    // Convert to Grayscale
    cvtColor( sideImg_Padded, sideImg_Gray, CV_RGB2GRAY );
    cvtColor( frontImg_Padded, frontImg_Gray, CV_RGB2GRAY );

    if( !sideImg_Gray.data || !frontImg_Gray.data )
    {
        QMessageBox::information(0, "Error!", "Problem converting color image to gray image");
        return;
    }

    Mat mask_FrontImg = Mat::zeros(frontImg_Gray.size(), CV_8UC1);
    mask_FrontImg(Rect(padSize.width, padSize.height/2, srcImg_Size.width/2, srcImg_Size.height)) = 1;

    Mat mask_SideImg = Mat::zeros(sideImg_Gray.size(), CV_8UC1);
    mask_SideImg( Rect( srcImg_Size.width/2, padSize.height/2, srcImg_Size.width/2, srcImg_Size.height)) = 1;

    //-- Step 1: Detect the keypoints using SIFT Detector

    SiftFeatureDetector featureDetector(ui->lineEdit_KeypointNum->text().toInt());
    std::vector< KeyPoint > keypoints_SideImg, keypoints_FrontImg;

    featureDetector.detect( frontImg_Gray, keypoints_FrontImg, mask_FrontImg );
    featureDetector.detect( sideImg_Gray, keypoints_SideImg, mask_SideImg );

//    cv::Mat sideImg_WithKeypoints, frontImg_WithKeypoints;
//    cv::drawKeypoints( sideImg_Gray, keypoints_SideImg, sideImg_WithKeypoints );
//    cv::drawKeypoints( frontImg_Gray, keypoints_FrontImg, frontImg_WithKeypoints );
//    namedWindow("Side image with Keypoints", WINDOW_NORMAL);
//    cv::imshow( "Side image with Keypoints", sideImg_WithKeypoints );
//    namedWindow("Front image with Keypoints", WINDOW_NORMAL);
//    cv::imshow( "Front image with Keypoints", frontImg_WithKeypoints );

    //-- Step 2: Calculate descriptors (feature vectors)
    SiftDescriptorExtractor extractor;
    Mat descriptors_SideImg, descriptors_FrontImg;

    extractor.compute( sideImg_Gray, keypoints_SideImg, descriptors_SideImg );
    extractor.compute( frontImg_Gray, keypoints_FrontImg, descriptors_FrontImg );

    //-- Step 3: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( descriptors_FrontImg, descriptors_SideImg, matches );

    double max_dist = 0; double min_dist = 100;
    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_FrontImg.rows; i++ )
    {
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    cout << "-- Max dist : " << max_dist << endl;
    cout << "-- Min dist : " << min_dist << endl;

    //-- Use only "good" matches (i.e. whose distance is less than 3*min_dist )
    std::vector< DMatch > good_matches;

    for( int i = 0; i < descriptors_FrontImg.rows; i++ )
        if( matches[i].distance < 3*min_dist )
            good_matches.push_back( matches[i] );

    cout << "good matches size:" << good_matches.size() << endl;

    Mat img_matches;
    drawMatches( frontImg_Gray, keypoints_FrontImg, sideImg_Gray, keypoints_SideImg, good_matches, img_matches,
                 Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    namedWindow("Good Matches", WINDOW_NORMAL);
    imshow( "Good Matches", img_matches );

    std::vector< Point2f > goodKeypoints_SideImg, goodKeypoints_FrontImg;

    for( std::size_t i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        goodKeypoints_FrontImg.push_back( keypoints_FrontImg[ good_matches[i].queryIdx ].pt );
        goodKeypoints_SideImg.push_back( keypoints_SideImg[ good_matches[i].trainIdx ].pt );
    }

    Mat homography = findHomography( goodKeypoints_FrontImg, goodKeypoints_SideImg, CV_RANSAC, 2 );
    mapSize = frontImg_Padded.size();

    homography.convertTo(homography, CV_32FC1);
    //cv::Mat_<float> homography_MapX_32f(height, width), homography_MapY_32f(height, width);
    homography_MapX_32f = Mat_<float>(mapSize.height, mapSize.width);
    homography_MapY_32f = Mat_<float>(mapSize.height, mapSize.width);
    for(int y = 0; y < mapSize.height; ++y)
    {
        float *buff_MapX = ((float*) homography_MapX_32f.data) + y*mapSize.width;
        float *buff_MapY = ((float*) homography_MapY_32f.data) + y*mapSize.width;
        for(int x = 0; x < mapSize.width; ++x)
        {
            cv::Mat_<float> pt(3, 1);
            pt(0) = x;
            pt(1) = y;
            pt(2) = 1;
            pt = homography*pt;
            pt /= pt(2);
            buff_MapX[x] = pt(0);
            buff_MapY[x] = pt(1);
        }
    }

    rectifiedImg_ByHomography = Mat(frontImg_Padded.rows, frontImg_Padded.cols , sideImg_Src.type());
    cv::remap( sideImg_Padded, rectifiedImg_ByHomography, homography_MapX_32f, homography_MapY_32f, cv::INTER_LINEAR );
    Mat half( rectifiedImg_ByHomography, cv::Rect( padSize.width, padSize.height/2, srcImg_Size.width, srcImg_Size.height ) );
    frontImg_Src.copyTo( half );
    line(rectifiedImg_ByHomography, Point(padSize.width, 0), Point(padSize.width, frontImg_Padded.rows), Scalar(0,0,255), 2);
    //namedWindow("Rectified by Homography", WINDOW_NORMAL);
    //imshow( "Rectified by Homography", rectifiedImg_ByHomography );
    ui->graphicsView_Img->setImage(Mat2QImage(rectifiedImg_ByHomography), 2.7);

}

void Homography_Transformation::estimatePerspective_LeftFront()
{
    Point2f inputQuad[4], outputQuad[4];
    Mat perspective( 3, 3, CV_32FC1 );

    inputQuad[0] = pt_TopRight;
    inputQuad[1] = pt_BottomRight;
    inputQuad[2] = pt_BottomLeft;
    inputQuad[3] = pt_TopLeft;

    if( !findNew_PerspectiveVertices() )
        return;

    outputQuad[0] = pt_TopRight;
    outputQuad[1] = pt_BottomRight;
    outputQuad[2] = pt_NewBottomLeft;
    outputQuad[3] = pt_NewTopLeft;

    perspective = getPerspectiveTransform( outputQuad, inputQuad );
    perspective.convertTo(perspective, CV_32FC1);

    if( mapSize.height == -1 || mapSize.width == -1 )
    {
        QMessageBox::information(0, "Error!", "Map size is not set. Complete Homography estimation first!");
        return;
    }
    cv::Mat_<float> perspective_MapX_32f(mapSize.height, mapSize.width), perspective_MapY_32f(mapSize.height, mapSize.width);
    for(int y = 0; y < mapSize.height; ++y)
    {
        float *buff_MapX = ((float*) perspective_MapX_32f.data)+y*mapSize.width;
        float *buff_MapY = ((float*) perspective_MapY_32f.data)+y*mapSize.width;
        for(int x = 0; x < mapSize.width; ++x)
        {
            cv::Mat_<float> pt(3, 1);
            pt(0) = x;
            pt(1) = y;
            pt(2) = 1;
            pt = perspective*pt;
            pt /= pt(2);
            buff_MapX[x] = pt(0);
            buff_MapY[x] = pt(1);
        }
    }

    Mat_<float> homographyPerspectiveCombined_MapX_32f( mapSize.height, mapSize.width ), homographyPerspectiveCombined_MapY_32f( mapSize.height, mapSize.width );

    cv::remap(homography_MapX_32f, homographyPerspectiveCombined_MapX_32f, perspective_MapX_32f, perspective_MapY_32f, cv::INTER_LINEAR);
    cv::remap(homography_MapY_32f, homographyPerspectiveCombined_MapY_32f, perspective_MapX_32f, perspective_MapY_32f, cv::INTER_LINEAR);

    for( int j = 0; j < homography_MapX_32f.rows; j++ )
    {
        for( int i = padSize.width; i < homography_MapX_32f.cols; i++ )
        {
            homographyPerspectiveCombined_MapX_32f.at<float>(j,i) = homography_MapX_32f.at<float>(j,i);
            homographyPerspectiveCombined_MapY_32f.at<float>(j,i) = homography_MapY_32f.at<float>(j,i);
        }

    }

    Mat rectified_final_img(mapSize.height, mapSize.width, frontImg_Src.type());
    cv::remap( sideImg_Padded, rectified_final_img, homographyPerspectiveCombined_MapX_32f, homographyPerspectiveCombined_MapY_32f, cv::INTER_LINEAR );
    Mat half( rectified_final_img, cv::Rect( padSize.width, padSize.height/2, srcImg_Size.width, srcImg_Size.height ));
    frontImg_Src.copyTo( half );

    //namedWindow("After Homography and Perspective transformation", WINDOW_NORMAL);
    //imshow( "After Homography and Perspective transformation", rectified_final_img );

    /***********************************************
     * Reducing map size
     *
     */

    Mat_<float> combinedReduced_MapX_32f( srcImg_Size.height+padSize.height/2, mapSize.width - padToRemove ), combinedReduced_MapY_32f( srcImg_Size.height+padSize.height/2, mapSize.width - padToRemove );

    for( int j = 0; j < combinedReduced_MapX_32f.rows; j++ )
    {
        int col = padToRemove;
        for( int i = 0; i < combinedReduced_MapX_32f.cols; i++ )
        {
            combinedReduced_MapX_32f.at<float>(j,i) = homographyPerspectiveCombined_MapX_32f.at<float>(j,col);
            combinedReduced_MapY_32f.at<float>(j,i) = homographyPerspectiveCombined_MapY_32f.at<float>(j,col);
            col++;
        }
    }

    Mat sideImg_HalfPadded = Mat(srcImg_Size.height+padSize.height/2, srcImg_Size.width, sideImg_Src.type());
    sideImg_Src.copyTo(sideImg_HalfPadded(Rect(0, padSize.height/2, srcImg_Size.width, srcImg_Size.height)) );

    Mat rectifiedSmall(combinedReduced_MapX_32f.rows, combinedReduced_MapX_32f.cols, frontImg_Src.type());
    cv::remap( sideImg_HalfPadded, rectifiedSmall, combinedReduced_MapX_32f, combinedReduced_MapY_32f, cv::INTER_LINEAR );
    Mat half_( rectifiedSmall, cv::Rect( rectifiedSmall.cols-srcImg_Size.width, padSize.height/2, srcImg_Size.width, srcImg_Size.height ));
    frontImg_Src.copyTo( half_ );

    Mat rectifiedImg_Cropped(srcImg_Size.height, rectifiedSmall.cols, rectifiedSmall.type());
    rectifiedSmall(Rect(0, padSize.height/2, rectifiedSmall.cols, srcImg_Size.height)).copyTo(rectifiedImg_Cropped);

    //namedWindow("After Homography and Perspective + CROP", WINDOW_NORMAL);
    //imshow( "After Homography and Perspective + CROP", rectifiedImg_Cropped );

    string filename = pathToResource + "maps/map_LeftFront_float.xml";
    FileStorage fs_write(filename, FileStorage::WRITE);

    fs_write << "src_img_size" << srcImg_Size;
    fs_write << "pad_Size" << padSize;
    fs_write << "pano_size" << rectifiedImg_Cropped.size();

    fs_write << "Map_X" << combinedReduced_MapX_32f;
    fs_write << "Map_Y" << combinedReduced_MapY_32f;
    fs_write.release();

    ui->graphicsView_Img->setImage(Mat2QImage(rectifiedImg_Cropped));

    cout << "finished perspective" << endl;
}

void Homography_Transformation::estimateHomography_FrontRight()
{
    sideImg_Src = imread( ui->lineEdit_SideImg->text().toStdString() );
    frontImg_Src = imread( ui->lineEdit_FrontImg->text().toStdString() );
    srcImg_Size = frontImg_Src.size();

//    namedWindow("Side Image", WINDOW_NORMAL);
//    imshow("Side Image", sideImg_Src);
//    namedWindow("Front Image", WINDOW_NORMAL);
//    imshow("Front Image", frontImg_Src);

    padSize = Size( 2000, 1500);//3000 2500
    //int height_pad = 1500, width_pad = 0;//3500
    //int height_pad = 7000, width_pad = 5000;


    Mat frontImg_Padded( srcImg_Size.height+padSize.height, srcImg_Size.width, frontImg_Src.type());
    sideImg_Padded = Mat(srcImg_Size.height+padSize.height, srcImg_Size.width, frontImg_Src.type());
    //left_color.copyTo(left_color_image(Rect(width_pad, height_pad/2, left_color.cols,left_color.rows)) );
    //right_color.copyTo(right_color_image(Rect(0, height_pad/2, left_color.cols,left_color.rows)) );
    frontImg_Src.copyTo(frontImg_Padded(Rect(0, padSize.height/2, srcImg_Size.width, srcImg_Size.height)) );
    sideImg_Src.copyTo(sideImg_Padded(Rect(0, padSize.height/2, srcImg_Size.width, srcImg_Size.height)) );


    Mat frontImg_Gray, sideImg_Gray;
    // Convert to Grayscale
    cvtColor( sideImg_Padded, sideImg_Gray, CV_RGB2GRAY );
    cvtColor( frontImg_Padded, frontImg_Gray, CV_RGB2GRAY );

    if( !sideImg_Gray.data || !frontImg_Gray.data )
    {
        QMessageBox::information(0, "Error!", "Problem converting color image to gray image");
        return;
    }

    Mat mask_FrontImg = Mat::zeros(frontImg_Gray.size(), CV_8UC1);
    mask_FrontImg(Rect(srcImg_Size.width/2, padSize.height/2, srcImg_Size.width/2, srcImg_Size.height)) = 1;

    Mat mask_SideImg = Mat::zeros(sideImg_Gray.size(), CV_8UC1);
    mask_SideImg( Rect( 0, padSize.height/2, srcImg_Size.width/2, srcImg_Size.height)) = 1;

    //-- Step 1: Detect the keypoints using SIFT Detector

    SiftFeatureDetector detector(400);//400
    std::vector< KeyPoint > keypoints_SideImg, keypoints_FrontImg;

    detector.detect( sideImg_Gray, keypoints_SideImg, mask_SideImg );
    detector.detect( frontImg_Gray, keypoints_FrontImg, mask_FrontImg );

//    cv::Mat frontImg_WithKeypoints, sideImg_WithKeypoints;
//    cv::drawKeypoints( frontImg_Gray, keypoints_FrontImg, frontImg_WithKeypoints );
//    cv::drawKeypoints( sideImg_Gray, keypoints_SideImg, sideImg_WithKeypoints );
//    namedWindow("Front image with Keypoints", WINDOW_NORMAL);
//    cv::imshow( "Front image with Keypoints", frontImg_WithKeypoints );
//    namedWindow("Side image with Keypoints", WINDOW_NORMAL);
//    cv::imshow( "Side image with Keypoints", sideImg_WithKeypoints );

    //-- Step 2: Calculate descriptors (feature vectors)
    SiftDescriptorExtractor extractor;

    Mat descriptors_FrontImg, descriptors_SideImg;

    extractor.compute( frontImg_Gray, keypoints_FrontImg, descriptors_FrontImg );
    extractor.compute( sideImg_Gray, keypoints_SideImg, descriptors_SideImg );

    //-- Step 3: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( descriptors_SideImg, descriptors_FrontImg, matches );

    double max_dist = 0; double min_dist = 100;

    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_SideImg.rows; i++ )
    {
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );

    //-- Use only "good" matches (i.e. whose distance is less than 3*min_dist )
    std::vector< DMatch > good_matches;

    for( int i = 0; i < descriptors_SideImg.rows; i++ )
    {
        if( matches[i].distance < 3*min_dist )
        {
            good_matches.push_back( matches[i] );
        }
    }

    Mat img_matches;
    drawMatches( sideImg_Gray, keypoints_SideImg, frontImg_Gray, keypoints_FrontImg, good_matches, img_matches,
                        Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    namedWindow("Good Matches", WINDOW_NORMAL);
    imshow( "Good Matches", img_matches );

    std::vector< Point2f > goodKeypoints_SideImg, goodKeypoints_FrontImg;

    for( std::size_t i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        goodKeypoints_SideImg.push_back( keypoints_SideImg[ good_matches[i].queryIdx ].pt );
        goodKeypoints_FrontImg.push_back( keypoints_FrontImg[ good_matches[i].trainIdx ].pt );
    }

    Mat homography = findHomography( goodKeypoints_FrontImg, goodKeypoints_SideImg, CV_RANSAC, 3 );
    homography.convertTo(homography, CV_32FC1);

    mapSize = Size(srcImg_Size.width+padSize.width, srcImg_Size.height+padSize.height );
    homography_MapX_32f = Mat_<float>(mapSize.height, mapSize.width);
    homography_MapY_32f = Mat_<float>(mapSize.height, mapSize.width);
    for(int y = 0; y < mapSize.height; ++y)
    {
        float *buff_MapX = ((float*) homography_MapX_32f.data) + y * mapSize.width;
        float *buff_MapY = ((float*) homography_MapY_32f.data) + y * mapSize.width;
        for(int x = 0; x<mapSize.width; ++x)
        {
            cv::Mat_<float> pt(3, 1);
            pt(0) = x;
            pt(1) = y;
            pt(2) = 1;
            pt = homography*pt;
            pt /= pt(2);
            buff_MapX[x] = pt(0);
            buff_MapY[x] = pt(1);
        }
    }

    rectifiedImg_ByHomography = Mat(mapSize.height, mapSize.width , frontImg_Src.type());
    cv::remap( sideImg_Padded, rectifiedImg_ByHomography, homography_MapX_32f, homography_MapY_32f, cv::INTER_LINEAR );

    Mat half( rectifiedImg_ByHomography, cv::Rect( 0, padSize.height/2, srcImg_Size.width, srcImg_Size.height ));
    frontImg_Src.copyTo( half );
    line(rectifiedImg_ByHomography, Point(srcImg_Size.width, 0), Point(srcImg_Size.width, mapSize.height), Scalar(0,0,255), 2);
    //namedWindow("Rectified by Homography", WINDOW_NORMAL);
    //imshow( "Rectified by Homography", rectifiedImg_ByHomography );
    ui->graphicsView_Img->setImage(Mat2QImage(rectifiedImg_ByHomography), 2.7);
}

void Homography_Transformation::estimatePerspective_FrontRight()
{
    Point2f inputQuad[4], outputQuad[4];
    Mat perspective( 3, 3, CV_32FC1 );

    inputQuad[0] = pt_TopRight;
    inputQuad[1] = pt_BottomRight;
    inputQuad[2] = pt_BottomLeft;
    inputQuad[3] = pt_TopLeft;

    if( !findNew_PerspectiveVertices() )
        return;

    outputQuad[0] = pt_NewTopRight;
    outputQuad[1] = pt_NewBottomRight;
    outputQuad[2] = pt_BottomLeft;
    outputQuad[3] = pt_TopLeft;

    perspective = getPerspectiveTransform( outputQuad, inputQuad );
    perspective.convertTo(perspective, CV_32FC1);

    if( mapSize.height == -1 || mapSize.width == -1 )
    {
        QMessageBox::information(0, "Error!", "Map size is not set. Complete Homography estimation first!");
        return;
    }
    cv::Mat_<float> perspective_MapX_32f(mapSize.height, mapSize.width), perspective_MapY_32f(mapSize.height, mapSize.width);
    for(int y = 0; y < mapSize.height; ++y)
    {
        float *buff_MapX = ((float*) perspective_MapX_32f.data) + y * mapSize.width;
        float *buff_MapY = ((float*) perspective_MapY_32f.data) + y * mapSize.width;
        for(int x = 0; x < mapSize.width; ++x)
        {
            cv::Mat_<float> pt(3, 1);
            pt(0) = x;
            pt(1) = y;
            pt(2) = 1;
            pt = perspective*pt;
            pt /= pt(2);
            buff_MapX[x] = pt(0);
            buff_MapY[x] = pt(1);
        }
    }

    Mat_<float> homographyPerspectiveCombined_MapX_32f( mapSize.height, mapSize.width ), homographyPerspectiveCombined_MapY_32f( mapSize.height, mapSize.width );

    cv::remap(homography_MapX_32f, homographyPerspectiveCombined_MapX_32f, perspective_MapX_32f, perspective_MapY_32f, cv::INTER_LINEAR);
    cv::remap(homography_MapY_32f, homographyPerspectiveCombined_MapY_32f, perspective_MapX_32f, perspective_MapY_32f, cv::INTER_LINEAR);

    for( int j = 0; j < homography_MapX_32f.rows; j++ )
    {
        for( int i = 0; i < srcImg_Size.width; i++ )
        {
            homographyPerspectiveCombined_MapX_32f.at<float>(j,i) = homography_MapX_32f.at<float>(j,i);
            homographyPerspectiveCombined_MapY_32f.at<float>(j,i) = homography_MapY_32f.at<float>(j,i);
        }

    }

    Mat rectified_final_img(mapSize.height, mapSize.width, frontImg_Src.type());
    cv::remap( sideImg_Padded, rectified_final_img, homographyPerspectiveCombined_MapX_32f, homographyPerspectiveCombined_MapY_32f, cv::INTER_LINEAR );
    Mat half( rectified_final_img, cv::Rect( 0, padSize.height/2, srcImg_Size.width, srcImg_Size.height ));
    frontImg_Src.copyTo( half );

    namedWindow("Rectified by Perspective transformation (Front - Right)", WINDOW_NORMAL);
    imshow( "Rectified by Perspective transformation (Front - Right)", rectified_final_img );

    /***********************************************
     * Reducing map size
     *
     */

    Mat_<float> combinedReduced_MapX_32f( srcImg_Size.height+padSize.height/2, mapSize.width - padToRemove ), combinedReduced_MapY_32f( srcImg_Size.height+padSize.height/2, mapSize.width - padToRemove );

    for( int j = 0; j < combinedReduced_MapX_32f.rows; j++ )
    {
        for( int i = 0; i < combinedReduced_MapX_32f.cols; i++ )
        {
            combinedReduced_MapX_32f.at<float>(j,i) = homographyPerspectiveCombined_MapX_32f.at<float>(j,i);
            combinedReduced_MapY_32f.at<float>(j,i) = homographyPerspectiveCombined_MapY_32f.at<float>(j,i);
        }
    }

    Mat sideImg_HalfPadded = Mat(srcImg_Size.height+padSize.height/2, srcImg_Size.width, sideImg_Src.type());
    sideImg_Src.copyTo(sideImg_HalfPadded(Rect(0, padSize.height/2, srcImg_Size.width, srcImg_Size.height)) );

    Mat rectifiedSmall(combinedReduced_MapX_32f.rows, combinedReduced_MapX_32f.cols, frontImg_Src.type());
    cv::remap( sideImg_HalfPadded, rectifiedSmall, combinedReduced_MapX_32f, combinedReduced_MapY_32f, cv::INTER_LINEAR );
    Mat half_( rectifiedSmall, cv::Rect( 0, padSize.height/2, srcImg_Size.width, srcImg_Size.height ));
    frontImg_Src.copyTo( half_ );

    Mat rectifiedImg_Cropped(srcImg_Size.height, rectifiedSmall.cols, rectifiedSmall.type());
    rectifiedSmall(Rect(0, padSize.height/2, rectifiedSmall.cols, srcImg_Size.height)).copyTo(rectifiedImg_Cropped);

    //namedWindow("After Homography and Perspective + CROP", WINDOW_NORMAL);
    //imshow( "After Homography and Perspective + CROP", rectifiedImg_Cropped );

    string filename = pathToResource + "maps/map_FrontRight_float.xml";
    FileStorage fs_write(filename, FileStorage::WRITE);

    fs_write << "src_img_size" << srcImg_Size;
    fs_write << "pad_Size" << padSize;
    fs_write << "pano_size" << rectifiedImg_Cropped.size();

    fs_write << "Map_X" << combinedReduced_MapX_32f;
    fs_write << "Map_Y" << combinedReduced_MapY_32f;
    fs_write.release();

    ui->graphicsView_Img->setImage(Mat2QImage(rectifiedImg_Cropped));

    cout << "finished perspective" << endl;

}
