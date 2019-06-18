#ifndef CPROCESSING_H
#define CPROCESSING_H

#include <QTimer>
#include <QEvent>
#include <QThread>

#include <exception>
#include <new>

#include <i2c_client.h>
#include <spi_aux_client.h>
#include <spi_client.h>
#include <ads1118.h>
#include <telaire.h>
#include <ad7608.h>
#include <tcp_client.h>

class CProcessing : public QObject
{
    Q_OBJECT

    //Miscellaneous variables
    const static uint8_t nOfSockets = 3;
    const uint8_t ADSNumSamples = 20;
    const uint8_t TelaireNumSamples = 5;
    const uint16_t AD7608NumSamples = 30;
    bool b_workerSPI = false;

    //Qt containers
    QTimer *SPI_Aux_Timer, *SPI_Timer;
    const QVector <uint8_t> chipSelect = {25, 8, 7, 9};
    const QString urlDirectory = "/home/generated_Files/";

    //Own objects
    SPI_aux_client  *workerSPI_Aux;
    SPI_client      *workerSPI;
    I2C_client      *workerI2C;
    ADS1118     *pAds1118_1[nOfSockets];
    Telaire     *pTelaire;
    AD7608      *pAd7608;

    void SPI_Start_Client();
    void SPI_Aux_Start_Client();

    protected:
        void timerEvent(QTimerEvent *) override;

    public:
        explicit CProcessing(QObject *parent = nullptr);
        ~CProcessing();
        void getStatistics();

};


class ISRThread : public QThread
{
    Q_OBJECT

    CProcessing *pAux;

    public:
        void run(){
            pAux->getStatistics();
            emit doneStat();
        }
        explicit ISRThread(CProcessing *pIN){

            pAux = pIN;
            connect(this, &ISRThread::doneStat, this, &QThread::quit);
            connect(this, &QThread::finished, this, &QThread::deleteLater);
        }

    signals:
        void doneStat();
};

#endif // CPROCESSING_H
