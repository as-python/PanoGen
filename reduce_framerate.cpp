#include "reduce_framerate.h"
#include "ui_reduce_framerate.h"

Reduce_FrameRate::Reduce_FrameRate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Reduce_FrameRate)
{
    ui->setupUi(this);
    ui->label_Status->setVisible(false);

    ui->comboBox_RequiredFR->addItem("20", 19.94);
    ui->comboBox_RequiredFR->addItem("30", 29.97);
    //ui->comboBox_RequiredFR->addItem("40", 40);
    //ui->comboBox_RequiredFR->addItem("50", 50);
    ui->comboBox_RequiredFR->addItem("60", 59.94);
    ui->comboBox_RequiredFR->setCurrentIndex(1);

}

Reduce_FrameRate::~Reduce_FrameRate()
{
    delete ui;
}

void Reduce_FrameRate::on_pushButton_ReduceFR_clicked()
{
    if(ui->lineEdit_OutputVidName->text().isEmpty() || ui->lineEdit_InputVideo->text().isEmpty())
    {
        QMessageBox::information(0, "Error!", "Input and Output video names are required");
        return;
    }

    VideoCapture inputCap( inputVideoFile );
    if(!inputCap.isOpened())
    {
        QMessageBox::information(0, "ERROR!", QString("Could not open the Input Video: %1").arg(QString::fromStdString(inputVideoFile)));
        return;
    }

    string outputName = ui->lineEdit_OutputVidName->text().toStdString();
    size_t dotLocation = outputName.find_last_of(".");
    if(dotLocation == string::npos || outputName.substr(dotLocation+1).empty())
    {
        QMessageBox::information(0, "ERROR!", "Output video extention missing (.avi)");
        return;
    }
    else if(outputName.substr(dotLocation+1) != "avi" && outputName.substr(dotLocation+1) != "AVI")
    {
        QMessageBox::information(0, "ERROR!", "Only avi format supported");
        return;
    }

    Size frameSize( inputCap.get(CV_CAP_PROP_FRAME_WIDTH), inputCap.get(CV_CAP_PROP_FRAME_HEIGHT) );
    string outputVideoFile = pathToResource + "videos/" + ui->lineEdit_OutputVidName->text().toStdString();
    double requiredFR = ui->comboBox_RequiredFR->itemData(ui->comboBox_RequiredFR->currentIndex()).toDouble();

    try
    {
        VideoWriter outputVideoWriter( outputVideoFile,  CV_FOURCC('D','I','V','X'), requiredFR, frameSize, true); //initialize the VideoWriter object
        //VideoWriter outputVideoWriter ( outputVideoFile,  CV_FOURCC('P','I','M','1'), requiredFR, frameSize, true); //initialize the VideoWriter object
        if( !outputVideoWriter.isOpened() )
        {
            QMessageBox::information(0, "ERROR!", QString("Failed to initialize the output video writer: %1").arg(QString::fromStdString(outputVideoFile)));
            //return false;
        }

        Mat camFrame;
        int frameSkipFreq = currentFR / requiredFR;
        int frameCount = 0;

        ui->label_Status->setVisible(true);
        QCoreApplication::processEvents();
        while(1)
        {
            bool bSuccess = inputCap.read(camFrame); // read a new frame from video

            if ( !bSuccess )
                break;

            if(frameCount % frameSkipFreq == 0)
                outputVideoWriter.write(camFrame);

            frameCount ++;
        }
        ui->label_Status->setVisible(false);
        QMessageBox::information(0, "Finished!", QString("Finished writing the video to the disk at a lower frame rate!\n%1").arg(QString::fromStdString(outputVideoFile)));
    }
    catch( const exception &ex)
    {
       string msg = "Error in function: on_pushButton_ReduceFR_clicked() in file:reduce_framerate.cpp\n";
       msg += ex.what();
       QMessageBox::information(0, "Error!", QString::fromStdString(msg));
    }
}

void Reduce_FrameRate::on_pushButton_InputVideo_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Input video file"), QString::fromStdString(pathToResource), tr("Videos (*.MP4 *.avi *.mp4)"));
    if(!fileName.isEmpty())
    {
        ui->lineEdit_InputVideo->setText(fileName);
        inputVideoFile = fileName.toStdString();

        VideoCapture inputCap( inputVideoFile );
        if(!inputCap.isOpened())
        {
            cout << "ERROR!! Could not open the Input Video: " << inputVideoFile << endl;
            return;
        }        
        currentFR = round(inputCap.get(CV_CAP_PROP_FPS));
        ui->lineEdit_CurrentFR->setText(QString::number(currentFR));
    }
}

void Reduce_FrameRate::setPathToResource(const string &value)
{
    pathToResource = value;
}

