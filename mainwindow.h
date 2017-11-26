#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt library
// ================================
#include <QMainWindow>
#include <QMessageBox>
#include <QGridLayout>
#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QKeyEvent>
#include <QStackedWidget>



// Opencv library
// ================================
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
typedef cv::Point3_<uint8_t> Pixel;


//
#include <sys/time.h>

#include "case_thread.h"
#include "direader.h"


using namespace std;
using namespace cv;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    uint8_t indSelectObject = 1;
    uint8_t stateShowObject = 0;
    uint8_t indShowCamera   = 0;



    bool f_stream = true;
    bool f_LowVoltage = true;
    bool f_CriticalVoltage = true;
    bool f_Camera = true;

    QStackedWidget *stackedWidgets;


    CaseThread thread;

    QMessageBox msgboxBlockSystem;
    QTimer timerBlockSystem;
    QVector<QLabel*> arrCamera;
    QList<QString> lstAccessKeys;

    DIReader DevReader;

    void CreateConnection();
    void CreateWindowBlockSystem();
    void getAccessKeys();

private slots:
    void slStartCamera();
    void slLowVoltage();
    void slCriticalVoltage();
    void slBlockAccess();
    void slReceiveAccessKey(QString key);
    void slCloseApplication();
    void slSelectObject(int ind_object);
    void slSelectCamera(int ind_camera);
    void slChangeModeShowObject();


protected:
    void closeEvent(QCloseEvent *event);
    virtual bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
