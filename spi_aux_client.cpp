#include "spi_aux_client.h"

#include "portablesleep.h"
#include <bcm2835.h>
#include <QDebug>
#include <QThread>

SPI_aux_client::SPI_aux_client(AbstractSensorInterface *obj){

    obj->getBuffers(&pMiso, &pMosi);
    n_ofBytes = obj->getNumBytesTransferred();
    SPISetup();
}

SPI_aux_client::~SPI_aux_client(){

    bcm2835_aux_spi_end();
}

void SPI_aux_client::SPI_Aux_ReadRoutine(){

    if(!spi_setup_Ready){
        qDebug() << "Colocar warning, setup didn't run.";
        //exit(0);
    }

    bcm2835_gpio_write(STNDBY, HIGH);
    bcm2835_gpio_write(RESET, HIGH);
    usleep(20);
    bcm2835_gpio_write(RESET, LOW);

    bcm2835_gpio_write(CNVST, LOW);
    usleep(2);
    bcm2835_gpio_write(CNVST, HIGH);
    usleep(5);

    bcm2835_aux_spi_transfernb((char*) pMosi,(char*) pMiso, n_ofBytes);

    emit finished();
}

void SPI_aux_client::SPISetup(){

    if(!bcm2835_aux_spi_begin()){
        qDebug()<<"SPI_AUX-BCM2835 didn't start!";
    }

    bcm2835_gpio_fsel(CNVST, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(BUSY, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(STNDBY, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RESET, BCM2835_GPIO_FSEL_OUTP);

    spi_setup_Ready = 1;
}
