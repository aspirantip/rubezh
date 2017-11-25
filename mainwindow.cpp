#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("RuBEZH");
    this->setGeometry(0, 0, 1024, 768);
    QApplication::instance()->installEventFilter(this);


    ui->lblVideo_0->setAlignment(Qt::AlignCenter);
    ui->lblVideo_1->setAlignment(Qt::AlignCenter);
    ui->lblVideo_2->setAlignment(Qt::AlignCenter);
    ui->lblVideo_3->setAlignment(Qt::AlignCenter);
    ui->wdgCamera->setAlignment(Qt::AlignCenter);
    /*
    if (f_Camera)
        this->setCentralWidget( ui->wdgCamera );
    else
        this->setCentralWidget( ui->widget );
    */

/*
    QLabel *lblFirst  = new QLabel;
    QLabel *lblSecond = new QLabel;
    lblFirst->setAlignment(Qt::AlignCenter);
    lblFirst->setText("First page");
    lblFirst->setMaximumSize(500, 400);
    lblSecond->setAlignment(Qt::AlignCenter);
    lblSecond->setText("Second page");
*/
    stackedWidgets = new QStackedWidget;
    stackedWidgets->addWidget(ui->widget);
    stackedWidgets->addWidget(ui->wdgCamera);
    this->setCentralWidget( stackedWidgets );
    stackedWidgets->setCurrentIndex(0);




    CreateConnection();
    CreateWindowBlockSystem();

    getAccessKeys();
    thread.start();
}

MainWindow::~MainWindow()
{
    f_stream = true;
    delete ui;
}

void MainWindow::CreateConnection()
{
    connect(ui->actionCamera,   &QAction::triggered, this, &MainWindow::slStartCamera);
    connect(&timerBlockSystem,  &QTimer::timeout, this, &MainWindow::slBlockAccess);
    connect(&thread,            &CaseThread::sgLowVoltage,      this, &MainWindow::slLowVoltage);
    connect(&thread,            &CaseThread::sgCriticalVoltage, this, &MainWindow::slCriticalVoltage);

}

void MainWindow::CreateWindowBlockSystem()
{
    msgboxBlockSystem.setWindowTitle(" ");
    msgboxBlockSystem.setText(tr("Система заблокирована."));
    msgboxBlockSystem.setInformativeText(tr("Активируйте доступ."));
    msgboxBlockSystem.setStandardButtons(QMessageBox::NoButton);

    timerBlockSystem.setInterval( 60 * 1000);
    //timerBlockSystem.start();
}

void MainWindow::getAccessKeys()
{
    QFile fileAccessKeys("access_keys");
    if(!fileAccessKeys.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "File" << fileAccessKeys.fileName() << "was not opened";

        // выводить модульное окно с предупреждением

        return;
    }

    QTextStream in(&fileAccessKeys);
    while (!in.atEnd()){
        lstAccessKeys.append( in.readLine() );
    }

    fileAccessKeys.close();
/*
    qDebug() << "Keys:";
    foreach (auto str_key, lstAccessKeys) {
        qDebug() << str_key;
    }
*/
}

void MainWindow::slStartCamera()
{
    struct timeval tv0;
    int fps = 0;
    int fps_sec = 0;
    int now_sec = 0;

    std::vector<cv::Point> vcObjPoint = {cv::Point(50, 50), cv::Point(50, 160), cv::Point(210, 210)};
    cv::Scalar clrRectObject (255, 0, 0);
    cv::Scalar clrRectObjectSelected (0, 0, 255);


    //-- Выбираем первую попавшуюся камеру
    VideoCapture cap(0);

    //-- Проверяем, удалось ли подключиться
    if (!cap.isOpened()) {
        qDebug()<<"Camera not opened!";
        return;
    }

    //-- Выставляем параметры камеры ширину и высоту кадра в пикселях
//    bool f_wframe = cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
//    bool f_hframe = cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    bool f_wframe = cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    bool f_hframe = cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    //bool f_fps    = cap.set(CV_CAP_PROP_FPS, 30);
    std::cout << "f_wframe = "  << f_wframe << " f_hframe = " << f_hframe << std::endl;
    std::cout << "FPS    = " << cap.get(CV_CAP_PROP_FPS) << std::endl;
    std::cout << "WIDTH  = " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << std::endl;
    std::cout << "HEIGHT = " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << std::endl;



    // Destination Buffer and JPEG Encode Parameter
    size_t width    = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    size_t height   = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    size_t cam_fps  = cap.get(CV_CAP_PROP_FPS);

    std::vector<uchar> buf( width * height );
    std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, 95 };
    std::vector<float> convToJpeg;
    cv::Mat buff;

    TickMeter tm;       // Timer
    float_t time_convert(0.0);

    Mat frame;

    while ( f_stream ) {


        cap >> frame;
        if (frame.empty()) {
            qDebug() << "frame is empty!";
            return;
        }


        // select objects
        // ================================================
        if(stateShowObject){

            // отображение выделенного объекта на весь виджет
            // ================================================
            if (stateShowObject == 3){
                cv::Rect rctObject (vcObjPoint[indSelectObject-1].x, vcObjPoint[indSelectObject-1].y, 150, 100);
                cv::Mat matSelectedObject = frame( rctObject );
                frame = matSelectedObject;
            }
            // ================================================
            else {
                uint8_t nm_object = 0;
                cv::Scalar clrObjRect;

                // затемнение вокруг выделенного объекта
                // ================================================
                if (stateShowObject == 2){
                    cv::Point pntRect = vcObjPoint[indSelectObject-1];
                    for(int y = 0; y < frame.rows; y++){
                        Pixel *Mi = frame.ptr<Pixel>(y);
                        for(int x = 0; x < frame.cols; x++){
                            if( x <= pntRect.x || x >= pntRect.x+150 ||
                                y <= pntRect.y || y >= pntRect.y+100   ){
                                Mi[x].x >>= 1;
                                Mi[x].y >>= 1;
                                Mi[x].z >>= 1;
                            }
                        }
                    }
                // Parallel execution using C++11 lambda.
                /*
                 if (nm_object == indSelectObject) {
                    frame.forEach<Pixel>([](Pixel &p, const int *position) -> void {
                            p.x = p.x >> 1;
                    });
                }
                */
                }
                // ================================================


                // draw borders around objects
                // ================================================
                foreach (auto pointRect, vcObjPoint) {
                    if (nm_object < vcObjPoint.size())
                        nm_object++;
                    else
                        nm_object = 0;

                    if (nm_object != indSelectObject)
                        clrObjRect = clrRectObject;
                    else {
                        clrObjRect = clrRectObjectSelected;
                    }


                    const uint8_t thickLine = 2;
                    const int font = cv::FONT_HERSHEY_SIMPLEX;
                    std::string strObjNumber = QString::number(nm_object).toStdString();
                    cv::Size txtSize = getTextSize(strObjNumber, font, 1, thickLine, 0);

                    cv::Rect rctObject (pointRect.x, pointRect.y, 150, 100);
                    cv::Rect rctLable (rctObject.x, rctObject.y, txtSize.width+6, txtSize.height+6);
                    cv::rectangle(frame, rctObject, clrObjRect, 2);
                    cv::rectangle(frame, rctLable,  clrObjRect, cv::FILLED);

                    cv::Point pntText(rctLable.x + (rctLable.width - txtSize.width)/2, rctLable.y + txtSize.height + (rctLable.height - txtSize.height)/2);
                    putText(frame, strObjNumber, pntText, font, 1, cv::Scalar(255, 255, 255), thickLine);

                }
                // ================================================
            }
        }
/*
 * нажатие красной кнопки:
 * 1 - затемнение области кроме выделенного объекта
 * 2 - отображаем только выделенный объект
 * 3 - сброс к первоначальному состоянию
*/

        // output image on the widget
        // ================================================
        cv::cvtColor(frame, frame, CV_BGR2RGB);
        QImage qImgFrame((uchar*)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        //std::cout << "f_Camera =" << f_Camera << std::endl;
        if (f_Camera){
            qImgFrame = qImgFrame.scaled(ui->wdgCamera->size(), Qt::KeepAspectRatio);
            ui->wdgCamera->setPixmap( QPixmap::fromImage( qImgFrame ) );
        }
        else {
            qImgFrame = qImgFrame.scaled(ui->lblVideo_0->size(), Qt::KeepAspectRatio);
            ui->lblVideo_0->setPixmap( QPixmap::fromImage(qImgFrame) );
            //ui->lblVideo_1->setPixmap( QPixmap::fromImage(qImgFrame) );
            //ui->lblVideo_2->setPixmap( QPixmap::fromImage(qImgFrame) );
            ui->lblVideo_3->setPixmap( QPixmap::fromImage(qImgFrame) );
        }
        // ================================================

        // подсчитываем количество кадров
        if (0){
            tm.start();
            cv::imencode( ".jpg", frame, buf, params );
            tm.stop();
            convToJpeg.push_back( tm.getTimeMilli() / tm.getCounter() );

            gettimeofday( &tv0, 0);
            now_sec = tv0.tv_sec;
            if (fps_sec == now_sec)
                fps++;
            else
            {
                std::cout << "fps: " << fps+1 << std::endl;
                fps_sec = now_sec;


                foreach (auto time, convToJpeg) {
                    //std::cout << "time = " << time << std::endl;
                    time_convert += time;
                }
                time_convert /= fps;
                std::cout << "Time of convert to JPEG: " << time_convert << " [ms]" << std::endl;

                convToJpeg.clear();
                time_convert = 0.0;
                fps = 0;
            }
        }

        waitKey(1);
    }

    qDebug() << "Finished stream_video";
}

void MainWindow::slLowVoltage()
{
    qDebug() << "slLowVoltage [...]";

    if (f_LowVoltage){
        f_LowVoltage = false;

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Внимание!"));
        msgBox.setText(tr("Низкий заряд аккамулятора."));
        msgBox.setInformativeText(tr("Подключите питание!"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

void MainWindow::slCriticalVoltage()
{
    qDebug() << "slCriticalVoltage [...]";

    if (f_CriticalVoltage){
        f_CriticalVoltage = false;

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr(" "));
        msgBox.setText(tr("Критический заряд аккамулятора!"));
        msgBox.setInformativeText(tr("Выключение устройства ..."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();

        close();
        // тут посылаем сигнал в case_thread на отключение "EXQ"
    }
}

void MainWindow::slBlockAccess()
{
    //qDebug() << "slBlockAccess [...]";

    connect(&thread, &CaseThread::sgSendAccessKey, this, &MainWindow::slReceiveAccessKey);
    timerBlockSystem.stop();
    msgboxBlockSystem.exec();
}

void MainWindow::slReceiveAccessKey(QString key)
{
    qDebug() << "slreceiveAccessKey [...] | key =" << key;

    if( lstAccessKeys.contains(key) ){
        qDebug() << "Доступ разрешен!";

        msgboxBlockSystem.accept();
        timerBlockSystem.start();
        disconnect(&thread, &CaseThread::sgSendAccessKey, this, &MainWindow::slReceiveAccessKey);

        ui->statusBar->showMessage("key: " + key);
    }
    else {
        // уведомляем, что в доступе отказано!
        qDebug() << "В доступе отказано!";

    }
}

void MainWindow::slCloseApplication()
{

}

void MainWindow::slSelectObject(uint8_t ind_object)
{
    indSelectObject = ind_object;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    f_stream = false;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease){
        //qDebug() << "eventFilter() |" << event->type();

        if (timerBlockSystem.isActive())
            timerBlockSystem.start();
    }

    return watched->event( event );
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9){
        uint8_t nm_key = event->key() - 0x30;
        qDebug() << "number =" << nm_key;
        slSelectObject( nm_key );
    }

    if (event->key() == Qt::Key_W){
        stateShowObject++;
        if (stateShowObject == 4)
            stateShowObject = 0;
    }


    if (event->key() == Qt::Key_A){          // отображаем виджет со всеми камерами
        f_Camera = false;
        stackedWidgets->setCurrentIndex(0);
    }
    if (event->key() == Qt::Key_S){          // отображаем камеру 1
        f_Camera = true;
        stackedWidgets->setCurrentIndex(1);
    }
    if (event->key() == Qt::Key_D){          // отображаем камеру 2
        f_Camera = true;
                stackedWidgets->setCurrentIndex(1);
    }
    if (event->key() == Qt::Key_F){          // отображаем камеру 3
        f_Camera = true;
        stackedWidgets->setCurrentIndex(1);
    }
    if (event->key() == Qt::Key_G){          // отображаем камеру 4
        f_Camera = true;
        stackedWidgets->setCurrentIndex(1);
    }

}
