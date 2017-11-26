#ifndef DIREADER_H
#define DIREADER_H

#include <QThread>
#include <QObject>
#include <QDebug>
#include <QSocketNotifier>


/* Unix */
#include <fcntl.h>
#include <unistd.h>

/* standart library */
#include <iostream>

const uint8_t sz_buff = 5;

class DIReader : public QThread
{
    Q_OBJECT

    int d_file = 0;
    QSocketNotifier *notifier;

    bool f_button = false;
    std::vector<bool> vFlagsPressButtons;

    int8_t buff[sz_buff];


    inline void buffParser();


public:
    explicit DIReader(QObject *parent = nullptr);
    ~DIReader();

    void run() override;

signals:
    void sgSelectObject(int i_object);
    void sgSelectCamera(int i_camera);

private slots:
    void handleRead();

};

#endif // DIREADER_H
