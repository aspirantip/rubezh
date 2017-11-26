#ifndef CASE_THREAD_H
#define CASE_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <QTimer>
#include <QDebug>


#include <iostream>

//===================================================================================
class CaseThread : public QThread
{
    Q_OBJECT

public:
    CaseThread(QObject *parent = 0);
    ~CaseThread();

    void requestNewCase(const QString &hostName, quint16 port);
    void run() override;

signals:
    void sgState(const QString &message);
    void sgLowVoltage();
    void sgCriticalVoltage();
    void sgSendAccessKey(QString key);
    void sgPressedFire();

private slots:
    void StateVoltage();

private:
    bool quit;
    bool f_initArrVoltage = true;
    QString strLastAccessKey;
};
//===================================================================================
extern  QString ipAddress;
extern  int     ipPort;
//===================================================================================
#endif
