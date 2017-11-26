#include "direader.h"

DIReader::DIReader(QObject *parent) : QThread(parent)
{
    this->moveToThread(this);
    vFlagsPressButtons.resize(16, false);

}

DIReader::~DIReader()
{
    disconnect(notifier, &QSocketNotifier::activated, this, &DIReader::handleRead);
    if (d_file >= 0)
        close( d_file );


    this->wait();
}

void DIReader::run()
{

    d_file = open("/dev/hidraw3", O_RDONLY|O_NONBLOCK);
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
    notifier->setEnabled(false);
    uint8_t count = read(d_file, buff, sz_buff);
    if (count != sz_buff)
        qDebug() << "ERROR | read data" << count << " bytes from" << sz_buff;
/*
 * // for debug output
 * // =======================================
    for (int i = 0; i < sz_buff; i++)
    {
        std::cout.width(4);
        std::cout << (int)buff[i] << " | ";

    }
    std::cout << "\n";
    std::cout.flush();
    // =======================================
*/

    buffParser();

    notifier->setEnabled(true);
}

inline void DIReader::buffParser()
{
    // first row of buttons
    // ===============================
    if (buff[4] == 8 && !vFlagsPressButtons[0]){     // 1 button
        vFlagsPressButtons[0] = true;
        emit sgSelectCamera( 0 );
    }
    else {
        if (buff[4] != 8 && vFlagsPressButtons[0]){
            vFlagsPressButtons[0] = false;
        }
    }
    if (buff[4] == 4 && !vFlagsPressButtons[1]){     // 2 button
        vFlagsPressButtons[1] = true;
        emit sgSelectCamera( 1 );
    }
    else {
        if (buff[4] != 4 && vFlagsPressButtons[1]){
            vFlagsPressButtons[1] = false;
        }
    }
    if (buff[4] == 2 && !vFlagsPressButtons[2]){     // 3 button
        vFlagsPressButtons[2] = true;
        emit sgSelectCamera( 2 );
    }
    else {
        if (buff[4] != 2 && vFlagsPressButtons[2])
            vFlagsPressButtons[2] = false;
    }
    if (buff[4] == 1 && !vFlagsPressButtons[3]){     // 4 button
        vFlagsPressButtons[3] = true;
        emit sgSelectCamera( 3 );
    }
    else {
        if (buff[4] != 1 && vFlagsPressButtons[3])
            vFlagsPressButtons[3] = false;
    }
    /*
    if (buff[3] == -128  && !vFlagsPressButtons[4]){ // 5 button
        vFlagsPressButtons[4] = true;
        emit sgSelectObject( 25 );
    }
    else {
        if (buff[3] != -128 && vFlagsPressButtons[4])
            vFlagsPressButtons[4] = false;
    }
    if (buff[3] == 64  && !vFlagsPressButtons[5]){   // 6 button
        vFlagsPressButtons[5] = true;
        emit sgSelectObject( 26 );
    }
    else {
        if (buff[3] != 64 && vFlagsPressButtons[5]){
            vFlagsPressButtons[5] = false;
        }
    }
    */


    // second and third row of buttons
    // ===============================
    if (buff[3] == 32 && !vFlagsPressButtons[6]){    // 1 button
        vFlagsPressButtons[6] = true;
        emit sgSelectObject( 1 );
    }
    else {
        if (buff[3] != 32 && vFlagsPressButtons[6]){
            vFlagsPressButtons[6] = false;
        }
    }
    if (buff[3] == 16 && !vFlagsPressButtons[7]){    // 2 button
        vFlagsPressButtons[7] = true;
        emit sgSelectObject( 2 );
    }
    else {
        if (buff[3] != 16 && vFlagsPressButtons[7]){
            vFlagsPressButtons[7] = false;
        }
    }
    if (buff[3] == 8  && !vFlagsPressButtons[8]){    // 3 button
        vFlagsPressButtons[8] = true;
        emit sgSelectObject( 3 );
    }
    else {
        if (buff[3] != 8 && vFlagsPressButtons[8])
            vFlagsPressButtons[8] = false;
    }
    if (buff[3] == 4  && !vFlagsPressButtons[9]){    // 4 button
        vFlagsPressButtons[9] = true;
        emit sgSelectObject( 4 );
    }
    else {
        if (buff[3] != 4 && vFlagsPressButtons[9])
            vFlagsPressButtons[9] = false;
    }
    if (buff[3] == 2  && !vFlagsPressButtons[10]){   // 5 button
        vFlagsPressButtons[10] = true;
        emit sgSelectObject( 5 );
    }
    else {
        if (buff[3] != 2 && vFlagsPressButtons[10])
            vFlagsPressButtons[10] = false;
    }
    if (buff[3] == 1  && !vFlagsPressButtons[11]){   // 6 button
        vFlagsPressButtons[11] = true;
        emit sgSelectObject( 6 );
    }
    else {
        if (buff[3] != 1 && vFlagsPressButtons[11]){
            vFlagsPressButtons[11] = false;
        }
    }
    if (buff[4] == -128 && !vFlagsPressButtons[12]){ // 7 button
        vFlagsPressButtons[12] = true;
        emit sgSelectObject( 7 );
    }
    else {
        if (buff[4] != -128 && vFlagsPressButtons[12]){
            vFlagsPressButtons[12] = false;
        }
    }
    if (buff[4] == 64  && !vFlagsPressButtons[13]){  // 8 button
        vFlagsPressButtons[13] = true;
        emit sgSelectObject( 8 );
    }
    else {
        if (buff[4] != 64 && vFlagsPressButtons[13])
            vFlagsPressButtons[13] = false;
    }
    if (buff[4] == 32  && !vFlagsPressButtons[14]){  // 9 button
        vFlagsPressButtons[14] = true;
        emit sgSelectObject( 9 );
    }
    else {
        if (buff[4] != 32 && vFlagsPressButtons[14])
            vFlagsPressButtons[14] = false;
    }
    if (buff[4] == 16  && !vFlagsPressButtons[15]){  // 10 button
        vFlagsPressButtons[15] = true;
        emit sgSelectObject( 10 );
    }
    else {
        if (buff[4] != 16 && vFlagsPressButtons[15])
            vFlagsPressButtons[15] = false;
    }

}
