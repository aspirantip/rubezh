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


class DIReader : public QThread
{
    Q_OBJECT

    int d_file = 0;
    QSocketNotifier *notifier;

public:
    explicit DIReader(QObject *parent = nullptr);
    ~DIReader();

    void run() override;

signals:

private slots:
    void handleRead();

};

#endif // DIREADER_H
