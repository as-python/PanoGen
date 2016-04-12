#include "cameracalibration.h"

bool CameraCalibration::goodInput = false;

CameraCalibration::CameraCalibration(QObject *parent) : QObject(parent)
{    

}

void CameraCalibration::setPath(const string &value)
{
    path = value;
}

void CameraCalibration::setSquareSize(float value)
{
    squareSize = value;
}

void CameraCalibration::setAspectRatio(float value)
{
    aspectRatio = value;
}

void CameraCalibration::setDelay(int value)
{
    delay = value;
}

void CameraCalibration::setBwriteExtrinsics(bool value)
{
    bwriteExtrinsics = value;
}

void CameraCalibration::setShowUndistorsed(bool value)
{
    showUndistorsed = value;
}

void CameraCalibration::setBwritePoints(bool value)
{
    bwritePoints = value;
}

void CameraCalibration::setInput(const string &value)
{
    input = value;
}

void CameraCalibration::setSaveUndistorsed(bool value)
{
    saveUndistorsed = value;
}

void CameraCalibration::setCalibZeroTangentDist(bool value)
{
    calibZeroTangentDist = value;
}

void CameraCalibration::setCalibFixPrincipalPoint(bool value)
{
    calibFixPrincipalPoint = value;
}

void CameraCalibration::setBoardSize(const Size &value)
{
    boardSize = value;
}

void CameraCalibration::setCalibrationPattern(const Pattern &value)
{
    calibrationPattern = value;
}

void CameraCalibration::setNrFrames(int value)
{
    nrFrames = value;
}

void CameraCalibration::setOutputFileName(const string &value)
{
    outputFileName = value;
}

void CameraCalibration::setOutput_mapfile_name(const string &value)
{
    output_mapfile_name = value;
}

void CameraCalibration::setInputType(const InputType &value)
{
    inputType = value;
}

bool CameraCalibration::calibrate()
{
    interprate();

    //MainWindow mainWin;
    if (!goodInput)
    {
        cout << "ERROR: Invalid input detected. Application stopping. " << endl;
        return false;
    }

    vector<vector<Point2f> > imagePoints;
    Mat cameraMatrix, distCoeffs;
    Size imageSize;
    int mode = inputType == IMAGE_LIST ? CAPTURING : DETECTION;

    clock_t prevTimestamp = 0;
    //const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
    const char ESC_KEY = 27;

    for (int i = 0;; ++i)
    {        
        Mat view;
        bool blinkOutput = false;

        view = nextImage();

        //-----  If no more image, or got enough, then stop calibration and show result -------------
        if (mode == CAPTURING && imagePoints.size() >= (unsigned) nrFrames)
        {
            if (runCalibrationAndSave(imageSize, cameraMatrix, distCoeffs, imagePoints))
                mode = CALIBRATED;
            else
                mode = DETECTION;
        }
        if (view.empty()) // If no more images then run calibration, save and stop loop.
        {
            if (imagePoints.size() > 0 && mode != CALIBRATED)
                runCalibrationAndSave(imageSize, cameraMatrix, distCoeffs, imagePoints);
            break;
        }

        imageSize = view.size();  // Format input image.

        vector<Point2f> pointBuf;

        bool found;
        switch (calibrationPattern) // Find feature points on the input format
        {
        case CHESSBOARD:
            found = findChessboardCorners(view, boardSize, pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case CIRCLES_GRID:
            found = findCirclesGrid(view, boardSize, pointBuf);
            break;
        case ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid(view, boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID);
            break;
        default:
            found = false;
            break;
        }

        if (found)                // If done with success,
        {
            // improve the found corners' coordinate accuracy for chessboard
            if (calibrationPattern == CHESSBOARD)
            {
                Mat viewGray;
                cvtColor(view, viewGray, COLOR_BGR2GRAY);
                cornerSubPix(viewGray, pointBuf, Size(11, 11), Size(-1, -1), TermCriteria( CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            }

            if (mode == CAPTURING && // For camera only take new samples after delay time
                    (!inputCapture.isOpened() || clock() - prevTimestamp > this->delay * 1e-3 * CLOCKS_PER_SEC))
            {
                imagePoints.push_back(pointBuf);
                prevTimestamp = clock();
                blinkOutput = inputCapture.isOpened();
            }

            // Draw the corners.
            drawChessboardCorners(view, boardSize, Mat(pointBuf), found);            
        }

        //----------------------------- Output Text ------------------------------------------------
        string msg = (mode == CAPTURING) ? "100/100" : mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
        //int baseLine = 0;
        //Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
        //Point textOrigin(view.cols - 2 * textSize.width - 10, view.rows - 2 * baseLine - 10);

        if (mode == CAPTURING)
            msg = format("%d/%d", (int) imagePoints.size(), nrFrames);

        //putText(view, msg, textOrigin, 1, 1, mode == CALIBRATED ? GREEN : RED);

        if (blinkOutput)
            bitwise_not(view, view);

        //------------------------- Video capture  output  undistorted ------------------------------
        if (mode == CALIBRATED && showUndistorsed)
        {
            Mat temp = view.clone();
            undistort(temp, view, cameraMatrix, distCoeffs);
        }

        //------------------------------ Show image and check for input commands -------------------
        //imshow("Image View", view);
        emit signal_ChangeImage(view);
        emit signal_ChangeImageCounterText(QString::fromStdString(msg));

        char key = (char) waitKey(inputCapture.isOpened() ? 50 : delay);

        if (key == ESC_KEY)
            break;

        if (key == 'u' && mode == CALIBRATED)
            showUndistorsed = !showUndistorsed;

        if (inputCapture.isOpened() && key == 'g')
        {
            mode = CAPTURING;
            imagePoints.clear();
        }

        addDelay(0); //This delay is added so that the user can see the detected Chessboard. This can be removed....
    }

    Mat view, rview, map1, map2;
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize,1, imageSize, 0),
                            imageSize, CV_16SC2, map1, map2);

    bool success = save_camera_maps(imageSize, map1, map2);    
    if(success)
    {
        string text = "Calibration succeeded!\n";
        text +=  "\nThe camera parameters are saved at: " + path + outputFileName;
        text += "\n\nThe joint undistortion and rectification transformation have been calculated and saved as maps at: " + path + "maps/" + output_mapfile_name;
        QMessageBox::information(0, "Parameters saved!", QString::fromStdString(text));
    }
    else
        QMessageBox::information(0, "Error!", "ERROR! Couldn't save the maps!");

    // -----------------------Show the undistorted image for the image list ------------------------
    if (inputType == IMAGE_LIST && showUndistorsed)
    {
        bool should_ChangeTitleText = true;
        for (int i = 0; i < (int) imageList.size(); i++)
        {
            view = imread(imageList[i], 1);
            if (view.empty())
                continue;
            if(should_ChangeTitleText)
            {
                emit signal_ChangeTitleText("Undistorted image");
                should_ChangeTitleText = !should_ChangeTitleText;
            }
            string msg = format("%d/%d", i+1, nrFrames);

            remap(view, rview, map1, map2, INTER_LINEAR);
            emit signal_ChangeImage(rview);
            emit signal_ChangeImageCounterText(QString::fromStdString(msg));

            if(saveUndistorsed)
            {
                stringstream ss;
                ss << (i+1);
                imwrite( path + "calibration_output/" + ss.str() + ".png", rview );
            }
            addDelay(300);
        }
    }
    else if(inputType == IMAGE_LIST && saveUndistorsed)
    {
        for (int i = 0; i < (int) imageList.size(); i++)
        {
            view = imread(imageList[i], 1);
            if (view.empty())
                continue;

            remap(view, rview, map1, map2, INTER_LINEAR);
            string msg = format("Saving %d/%d", i+1, nrFrames);
            emit signal_ChangeImageCounterText(QString::fromStdString(msg));
            stringstream ss;
            ss << (i+1);
            imwrite( path + "calibration_output/" + ss.str() + ".png", rview );

        }

    }
    return true;
}

void CameraCalibration::interprate()
{
    goodInput = true;
    if (boardSize.width <= 0 || boardSize.height <= 0)
    {
        cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
        goodInput = false;
    }
    if (squareSize <= 10e-6)
    {
        cerr << "Invalid square size " << squareSize << endl;
        goodInput = false;
    }
    if (nrFrames <= 0)
    {
        cerr << "Invalid number of frames " << nrFrames << endl;
        goodInput = false;
    }

    if (input.empty())      // Check for valid input
            inputType = INVALID;
    else
    {
        if (input[0] >= '0' && input[0] <= '9')
        {
            stringstream ss(input);
            ss >> cameraID;
            inputType = CAMERA;
        }
        else
        {
            if (readStringList(input, imageList))
            {
                inputType = IMAGE_LIST;
                nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
            }
            else
                inputType = VIDEO_FILE;
        }
        if (inputType == CAMERA)
            inputCapture.open(cameraID);
        if (inputType == VIDEO_FILE)
            inputCapture.open(input);
        if (inputType != IMAGE_LIST && !inputCapture.isOpened())
                inputType = INVALID;
    }
    if (inputType == INVALID)
    {
        cerr << " Inexistent input: " << input;
        goodInput = false;
    }

    flag = 0;
    if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
    if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;

    if (calibrationPattern == NOT_EXISTING)
    {
        cerr << " Inexistent camera calibration mode: " << calibrationPattern << endl;
        goodInput = false;
    }
    atImageList = 0;

}

bool CameraCalibration::readStringList( const string& filename, vector<string>& l )
{
    l.clear();
    FileStorage fs( filename, FileStorage::READ);
    if( !fs.isOpened() )
        return false;
    FileNode n = fs.getFirstTopLevelNode();
    if( n.type() != FileNode::SEQ )
        return false;
    FileNodeIterator it = n.begin(), it_end = n.end();
    for( ; it != it_end; ++it )
        l.push_back( path + (string)*it);
    return true;
}

Mat CameraCalibration::nextImage()
{
    Mat result;
    if( inputCapture.isOpened() )
    {
        Mat view0;
        inputCapture >> view0;
        view0.copyTo(result);
    }
    else if( atImageList < (int)imageList.size() )
        result = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);

    return result;
}

void CameraCalibration::addDelay(int mSec)
{
    QTime dieTime= QTime::currentTime().addMSecs(mSec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

bool CameraCalibration::runCalibrationAndSave(Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,vector<vector<Point2f> > imagePoints )
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;

    bool ok = runCalibration(imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs, reprojErrs, totalAvgErr);

    cout << (ok ? "Calibration succeeded" : "Calibration failed") << ". Avg Re-projection error = "  << totalAvgErr << endl;;

    if( ok )
        saveCameraParams( imageSize, cameraMatrix, distCoeffs, rvecs ,tvecs, reprojErrs, imagePoints, totalAvgErr);

    return ok;
}

bool CameraCalibration::runCalibration( Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs, vector<vector<Point2f> > imagePoints,
                                        vector<Mat>& rvecs, vector<Mat>& tvecs, vector<float>& reprojErrs,  double& totalAvgErr)
{
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( flag & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = 1.0;

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(boardSize, squareSize, objectPoints[0], calibrationPattern);

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints, rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}

void CameraCalibration::calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners, CameraCalibration::Pattern patternType )
{
    corners.clear();

    switch(patternType)
    {
    case CameraCalibration::CHESSBOARD:
    case CameraCalibration::CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; ++i )
            for( int j = 0; j < boardSize.width; ++j )
                corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));
        break;

    case CameraCalibration::ASYMMETRIC_CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
            for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float((2*j + i % 2)*squareSize), float(i*squareSize), 0));
        break;
    default:
        break;
    }
}

double CameraCalibration::computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints, const vector<vector<Point2f> >& imagePoints,
                                                     const vector<Mat>& rvecs, const vector<Mat>& tvecs, const Mat& cameraMatrix ,
                                                     const Mat& distCoeffs, vector<float>& perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i )
    {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);

        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

// Print camera parameters to the output file
void CameraCalibration::saveCameraParams( Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs, const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                          const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints, double totalAvgErr )
{
    FileStorage fs( path + "calibration_output/" + outputFileName, FileStorage::WRITE );

    time_t tm;
    time( &tm );
    struct tm *t2 = localtime( &tm );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_Time" << buf;

    if( !rvecs.empty() || !reprojErrs.empty() )
        fs << "nrOfFrames" << (int)std::max(rvecs.size(), reprojErrs.size());
    fs << "image_Width" << imageSize.width;
    fs << "image_Height" << imageSize.height;
    fs << "board_Width" << boardSize.width;
    fs << "board_Height" << boardSize.height;
    fs << "square_Size" << squareSize;

    if( flag & CV_CALIB_FIX_ASPECT_RATIO )
        fs << "FixAspectRatio" << aspectRatio;

    if( flag )
    {
        sprintf( buf, "flags: %s%s%s%s",
            flag & CV_CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "",
            flag & CV_CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "",
            flag & CV_CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "",
            flag & CV_CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "" );
        cvWriteComment( *fs, buf, 0 );

    }

    fs << "flagValue" << flag;

    fs << "Camera_Matrix" << cameraMatrix;
    fs << "Distortion_Coefficients" << distCoeffs;

    fs << "Avg_Reprojection_Error" << totalAvgErr;
    if( !reprojErrs.empty() )
        fs << "Per_View_Reprojection_Errors" << Mat(reprojErrs);

    if( !rvecs.empty() && !tvecs.empty() )
    {
        CV_Assert(rvecs[0].type() == tvecs[0].type());
        Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
        for( int i = 0; i < (int)rvecs.size(); i++ )
        {
            Mat r = bigmat(Range(i, i+1), Range(0,3));
            Mat t = bigmat(Range(i, i+1), Range(3,6));

            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
            //*.t() is MatExpr (not Mat) so we can use assignment operator
            r = rvecs[i].t();
            t = tvecs[i].t();
        }
        cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
        fs << "Extrinsic_Parameters" << bigmat;
    }

    if( !imagePoints.empty() )
    {
        Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
        for( int i = 0; i < (int)imagePoints.size(); i++ )
        {
            Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
            Mat imgpti(imagePoints[i]);
            imgpti.copyTo(r);
        }
        fs << "Image_points" << imagePtMat;
    }
}

bool CameraCalibration::save_camera_maps(Size imageSize, Mat map1, Mat map2)
{
    FileStorage fs( path + "maps/" + output_mapfile_name, FileStorage::WRITE );

    time_t tm;
    time( &tm );
    struct tm *t2 = localtime( &tm );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_Time" << buf;

    fs << "image_Width" << imageSize.width;
    fs << "image_Height" << imageSize.height;
    fs << "board_Width" << boardSize.width;
    fs << "board_Height" << boardSize.height;
    fs << "square_Size" << squareSize;

    fs << "map1" << map1;
    fs << "map2" << map2;

    fs.release();
    return true;

}

void CameraCalibration::write(FileStorage& fs) const                        //Write serialization for this class
{
    fs << "{" << "BoardSize_Width"  << boardSize.width
              << "BoardSize_Height" << boardSize.height
              << "Square_Size"         << squareSize
              << "Calibrate_Pattern" << calibrationPattern
              << "Calibrate_NrOfFrameToUse" << nrFrames
              << "Calibrate_FixAspectRatio" << aspectRatio
              << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
              << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

              << "Write_DetectedFeaturePoints" << bwritePoints
              << "Write_extrinsicParameters"   << bwriteExtrinsics
              << "Write_outputFileName"  << outputFileName

              << "Show_UndistortedImage" << showUndistorsed

              << "Input_Delay" << delay
              << "Input" << input
       << "}";
}

/*
void read(const FileNode& node, CameraCalibration& x, const CameraCalibration& default_value = CameraCalibration())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

bool CameraCalibration::onlineCalibAndSaveVideo(string path)
{
    Mat cam_frame;
    VideoCapture cap( path );
    string fileName = path.substr( path.find_last_of("/")+1, path.find_last_of(".")-(path.find_last_of("/")+1));

    Mat calibrated_frame;
    Mat map1, map2;

    FileStorage fs( path + "maps/maps_afterCalibration_GoPro_720p_60fps_Medium.xml", FileStorage::READ); // Read the settings
    if (!fs.isOpened())
    {
        cout << "ERROR: Could not open the Calibration Map file:" << endl;
        return false;
    }
    fs["map1"] >> map1;
    fs["map2"] >> map2;

    VideoWriter out_video_writer ( path + videos/calibrated_" + fileName + ".avi", CV_FOURCC('P','I','M','1'), 60, Size(1136, 552), true); //initialize the VideoWriter object

    if ( !out_video_writer.isOpened() ) //if not initialize the VideoWriter successfully, exit the program
    {
        cout << "ERROR: Failed to write the video" << endl;
        return false;
    }

    while(1)
    {
        bool bSuccess = cap.read(cam_frame); // read a new frame from video

        if ( !bSuccess )
        {
            cout << "ERROR: Cannot read the frame from video file" << endl;
            break;
        }

        remap(cam_frame, calibrated_frame, map1, map2, INTER_LINEAR);

        //line(calibrated_frame1, Point(58,83), Point(1220,83), Scalar(255, 255, 255), 1); //Top    -
        //line(calibrated_frame1, Point(58,83), Point(57,635), Scalar(255, 255, 255), 1);    //Left   |
        //line(calibrated_frame1, Point(58,635), Point(1220, 635), Scalar(255, 255, 255), 1);  //Bottom _
        //line(calibrated_frame1, Point(1220,635), Point(1220,83), Scalar(255, 255, 255), 1);  //Right  |


        Rect roi_ToCrop_Calibrated( 58, 83, 1136, 552 ); // Crop size for GoPro 720p_60fps_Medium
        Mat calib_croped_frame = calibrated_frame( roi_ToCrop_Calibrated );
        out_video_writer.write(calib_croped_frame);

        //imshow("CameraStream", calib_croped_frame); //show the frame in "MyVideo" window

        if(waitKey(1) == 27)
        {
            cout << "Esc key is pressed by user" << endl;
            return false;
        }
    }
    cout << "Finished writing the video to the disk!" << endl;
    return true;
}*/
