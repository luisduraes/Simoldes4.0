#ifndef SPI_CLIENT_H
#define SPI_CLIENT_H

#include <QObject>
#include <portablesleep.h>
#include <abstractsensorinterface.h>

class SPI_client : public QObject
{
    Q_OBJECT

    uint8_t *pMiso = nullptr;
    uint8_t *pMosi = nullptr;
    const uint8_t kNumDumpReadings = 2;
    const uint8_t kMillisSecondsDelay = 28;
    const uint8_t kChipSelect;
    uint8_t kNumReadings;
    size_t num_bytes;
    bool spi_setup_Ready = 0;

    void SPISetup();

    public:
        explicit SPI_client(AbstractSensorInterface*);
        ~SPI_client();
        void SPI_ReadRoutine();

    signals:
        void finished();
};

#endif // SPI_CLIENT_H
