#include "i2c_client.h"
#include <QDebug>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

I2C_client::I2C_client(AbstractSensorInterface *obj){

    obj->getBuffers(&pBuf, &pAddress);
    num_bytes = obj->getNumBytesTransferred();
}

I2C_client::~I2C_client(){

     file_i2c = close(file_i2c);
}

void I2C_client::I2C_Setup(){

    if((file_i2c = open(filename, O_RDWR)) < 0)
        qDebug() << "I2C ERROR: Cannot open file " << filename << "! \n";

    if(ioctl(file_i2c, I2C_SLAVE, slaveAddress) < 0)
        qDebug() << "I2C ERROR: Address " << slaveAddress << " not found! \n";
}

void I2C_client::I2CReadRoutine(){

    if(read(file_i2c, pBuf, num_bytes) != (uint8_t) num_bytes)
        qDebug() << "I2C ERROR: Failed to read from the interface! \n";

    emit finished();
}
