#ifndef CAMERACALIBRATION_H
#define CAMERACALIBRATION_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <QTime>
#include <QMessageBox>
#include <QCoreApplication>

#include <stdio.h>
#include <string>
#include <sstream>


using namespace cv;
using namespace std;

class CameraCalibration : public QObject
{
    Q_OBJECT

public:
    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
    enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

private:
    enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };
    Size boardSize;										// The size of the board -> Number of items by width and height
    Pattern calibrationPattern;							// One of the Chessboard, circles, or asymmetric circle pattern
    float squareSize;									// The size of a square in your defined unit (point, millimeter,etc).
    int nrFrames;										// The number of frames to use from the input for calibration
    float aspectRatio;									// The aspect ratio
    int delay;											// In case of a video input
    bool bwritePoints;									//  Write detected feature points
    bool bwriteExtrinsics;								// Write extrinsic parameters
    bool calibZeroTangentDist;							// Assume zero tangential distortion
    bool calibFixPrincipalPoint;						// Fix the principal point at the center
    string outputFileName;								// The name of the file where to write
    bool showUndistorsed;								// Show undistorted images after calibration
    bool saveUndistorsed;								// save undistorted images after calibration
    string input;										// The input ->
    string output_mapfile_name;							// The name of the output Map file
    static bool goodInput;

    int cameraID;
    vector<string> imageList;
    int atImageList;
    VideoCapture inputCapture;
    InputType inputType;
    int flag;
    string path;    

public:
    explicit CameraCalibration(QObject *parent = 0);

    void write(FileStorage& fs) const;					//Write serialization for this class
    bool calibrate();
    void interprate();
    bool readStringList( const string& filename, vector<string>& l );
    Mat nextImage();
    void addDelay(int mSec);

    bool runCalibrationAndSave(Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,vector<vector<Point2f> > imagePoints );
    bool runCalibration( Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs, vector<vector<Point2f> > imagePoints,
                                        vector<Mat>& rvecs, vector<Mat>& tvecs, vector<float>& reprojErrs,  double& totalAvgErr);
    static void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners, CameraCalibration::Pattern patternType );
    static double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints, const vector<vector<Point2f> >& imagePoints, const vector<Mat>& rvecs,
                                             const vector<Mat>& tvecs, const Mat& cameraMatrix , const Mat& distCoeffs, vector<float>& perViewErrors);
    void saveCameraParams( Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs, const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                        const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints, double totalAvgErr );
    bool save_camera_maps(Size imageSize, Mat map1, Mat map2);
    bool performCalibAndSave(string inputVideoFile, string mapFile, string outputVideoFile);

    /*  void read(const FileNode& node, CameraCalibration& x, const CameraCalibration& default_value = CameraCalibration());
        bool onlineCalibAndSaveVideo(string path);*/

    /*** Setter methods ***/
    void setBoardSize(const Size &value);
    void setCalibrationPattern(const Pattern &value);
    void setNrFrames(int value);
    void setOutputFileName(const string &value);
    void setOutput_mapfile_name(const string &value);
    void setInputType(const InputType &value);
    void setSquareSize(float value);
    void setAspectRatio(float value);
    void setDelay(int value);
    void setBwritePoints(bool value);
    void setBwriteExtrinsics(bool value);
    void setShowUndistorsed(bool value);
    void setInput(const string &value);
    void setSaveUndistorsed(bool value);
    void setCalibZeroTangentDist(bool value);
    void setCalibFixPrincipalPoint(bool value);
    void setPath(const string &value);

signals:
    void signal_ChangeImage(Mat);
    void signal_ChangeTitleText(QString);
    void signal_ChangeImageCounterText(QString);    

};

#endif // CAMERACALIBRATION_H
