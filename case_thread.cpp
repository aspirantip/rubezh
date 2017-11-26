#include <QTcpSocket>
#include <QDataStream>

#include "case_thread.h"
//----------------------------------------------------------------------------------------
QString ipAddress = "192.168.0.248";
int     ipPort    = 5001;
unsigned char   inData[64];
const int       Timeout = 5000;

//предупреждение - 10.7В, выключение - 10.2В
const double    low_v = 11.7;
const double    critical_v = 10.2;

const uint8_t   nm_elemV = 10;
double          arrU[nm_elemV];
double          currentU = 0.0;
uint8_t         ind_arrU = 0;

//----------------------------------------------------------------------------------------
CaseThread::CaseThread(QObject *parent) : QThread(parent), quit(false),
    strLastAccessKey("")
{
}
//----------------------------------------------------------------------------------------
CaseThread::~CaseThread()
{
    quit = true;
    wait();

    qDebug() << "CaseThread::~CaseThread()";
}
//----------------------------------------------------------------------------------------
void CaseThread::run()
{
    qDebug() << "Connect to hardware";

    QString serverName = ipAddress;
    quint16 serverPort = ipPort;

    uint8_t   ConState = 0;

    QTcpSocket socket;
    QDataStream strm(&socket);
    strm.setVersion(QDataStream::Qt_5_9);

    uint32_t cnt_tick = 0;
    while (!quit) {
        if(ConState == 2){
            socket.disconnect();
            ConState = 0;
        }
        if(ConState == 0){
            socket.connectToHost(serverName, serverPort); //IP: 192.168.0.248  Port: 5001;
            if (socket.waitForConnected(Timeout)) {
                ConState = 1;
            }
            else{
                sleep(1);
                continue;
            }
        }

        strm.writeRawData("EXW",3);     //опрос устройства
        if(!socket.waitForBytesWritten(Timeout)){
            ConState = 2;
        }

        if (socket.waitForReadyRead(Timeout)) {
            strm.readRawData( (char*)inData, 32);
            /*
                bytes:
                    0 - 'A'
                    1 - нажатие FIRE
                    2 - сервисный режим
                    3 - напряжение В (x10) предупреждение - 10.7В, выключение - 10.2В
                    4 - 'B'
                    5-12 - коды радиобрелка
            */
            //QString tmpS = QString("F=%1   M=%2   U=%3V  S=").arg(uint(inData[1])).arg(uint(inData[2])).arg(double(inData[3]/10.0));

            if (uint(inData[1]) == 1)
                emit sgPressedFire();

            QString strAccessKey;
            for(int i = 0; i < 8; i++){
                strAccessKey += QString("%1 ").arg(uint(inData[5+i]),3,16);
            }
            //tmpS += strAccessKey;
            //emit sgState(tmpS);


            if (strLastAccessKey != strAccessKey){
                strLastAccessKey = strAccessKey;
                emit sgSendAccessKey(strAccessKey);
            }
        }
        msleep(20);


        if (cnt_tick++ > 50){
            cnt_tick = 0;
            StateVoltage();
        }
    }

    if(ConState == 1){
        qDebug() << "Команда выключения устройства";
/*
        strm.writeRawData("EXQ", 3);     //команда выключения устройства (желательно дублировать командой SHUTDOWN системе)
        bool f_wr (false);
        f_wr = socket.waitForBytesWritten(Timeout);
        qDebug() << "f_wr =" << f_wr;
        sleep(1);
*/
        socket.disconnect();
    }
}
//----------------------------------------------------------------------------------------
void CaseThread::StateVoltage()
{
    if (!f_initArrVoltage)
    {
        if (ind_arrU == nm_elemV)
            ind_arrU = 0;

        if (double(inData[3]/10.0) != 0)
            arrU[ind_arrU] = double(inData[3]/10.0);
        ind_arrU++;


        currentU = 0;
        for (uint8_t ind = 0; ind < nm_elemV; ind++) {
            currentU += arrU[ind];
            //std::cout << arrU[ind] << " ";
        }
        //std::cout << std::endl;

        currentU /= nm_elemV;
        if (0) {
            qDebug() << "Current state:";
            qDebug() << "        voltage =" << currentU;
            qDebug() << "        low_v ="   << low_v;
            qDebug() << "        critical_v =" << critical_v;
            qDebug() << "";
        }

        if (currentU <= critical_v){
            emit sgCriticalVoltage();
            quit = true;
            return;
        }

        if (currentU <= low_v)
            emit sgLowVoltage();


    }
    else {
        double crVoltage = double(inData[3]/10.0);
        for (uint8_t ind = 0; ind < nm_elemV; ind++) {
            arrU[ind] = crVoltage;
        }
        f_initArrVoltage = false;
    }
}
//----------------------------------------------------------------------------------------
