#include "direader.h"

DIReader::DIReader(QObject *parent) : QThread(parent)
{
    this->moveToThread(this);

}

DIReader::~DIReader()
{
    if (d_file >= 0)
        close( d_file );
}

void DIReader::run()
{

    d_file = open("/dev/hidraw2", O_RDONLY|O_NONBLOCK);
    if (d_file >= 0) {
        notifier = new QSocketNotifier(d_file, QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, &DIReader::handleRead);
    }
    else
        qFatal("Could not open");

    exec();
}

void DIReader::handleRead()
{
    ssize_t count;
    char block[128];
    notifier->setEnabled(false);

    count = read(d_file, block, 128);
    //qDebug() << "\nsize =" << count;
    //QByteArray buff_tmp;
    //buff_tmp.append(block, count);
    //qDebug() << "Buffer:" << buff_tmp;


    for (int i = 0; i < 7; i++)
    {
        std::cout.width(4);
        std::cout << (int)block[i] << " | ";
        //qDebug() << "#" << i << "  " << (int)block[i];
    }
    std::cout << "\n";
    std::cout.flush();

    notifier->setEnabled(true);
}
