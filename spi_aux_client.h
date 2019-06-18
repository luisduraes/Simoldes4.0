#ifndef SPI_AUX_CLIENT_H
#define SPI_AUX_CLIENT_H

#include <QObject>
#include <abstractsensorinterface.h>

#define CNVST 4
#define STNDBY 13
#define BUSY 23
#define RESET 12

class SPI_aux_client : public QObject
{
    Q_OBJECT

    uint8_t *pMiso = nullptr;
    uint8_t *pMosi = nullptr;
    size_t n_ofBytes = 0;
    bool spi_setup_Ready = 0;

    void SPISetup();

    public:
        explicit SPI_aux_client(AbstractSensorInterface *);
        ~SPI_aux_client();        
        void SPI_Aux_ReadRoutine();

    signals:
        void finished();
};

#endif // SPI_AUX_CLIENT_H
