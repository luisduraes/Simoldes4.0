#include "spi_client.h"
#include "bcm2835.h"
#include <timing.h>

#include <QDebug>

SPI_client::SPI_client(AbstractSensorInterface *obj)
    : kChipSelect(obj->getChipSelect())
{
    obj->getBuffers(&pMiso, &pMosi);
    num_bytes = obj->getNumBytesTransferred();
    obj->getNumOfSlotsPerChannel(kNumReadings);
    SPISetup();
}

SPI_client::~SPI_client(){

    bcm2835_spi_end();
}

void SPI_client::SPI_ReadRoutine(){    

    if(!spi_setup_Ready){
        qDebug() << "Colocar warning, setup didn't run.";
        //exit(0);
    }

    uint8_t auxMult = kNumReadings * kNumDumpReadings;

    for(int j = 1; j <= kNumReadings; j++){
        for(int i = 1; i <= kNumDumpReadings; i++){
            bcm2835_spi_transfernb((char*) pMosi, (char*) pMiso, num_bytes);
            if( j * i  != auxMult) usleep(kMillisSecondsDelay * 1000);
        }
        pMosi += num_bytes;
        pMiso += num_bytes;
    }

    emit finished();
}

void SPI_client::SPISetup(){

    bcm2835_gpio_fsel(kChipSelect, BCM2835_GPIO_FSEL_OUTP);

    switch(kChipSelect){
        case 7:
            bcm2835_gpio_write(25, HIGH);
            bcm2835_gpio_write(8, HIGH);
            break;
        case 8:
            bcm2835_gpio_write(25, HIGH);
            bcm2835_gpio_write(7, HIGH);
            break;
        default:
            bcm2835_gpio_write(7, HIGH);
            bcm2835_gpio_write(8, HIGH);
    }
    bcm2835_gpio_write(kChipSelect, LOW);

    if(!bcm2835_spi_begin())
        qDebug() << "SPI-BCM2835 didn't start!\n";

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);

    spi_setup_Ready = 1;
}


