#ifndef I2C_CLIENT_H
#define I2C_CLIENT_H

#include <QObject>
#include <abstractsensorinterface.h>

class I2C_client : public QObject
{
    Q_OBJECT

    int file_i2c;
    uint8_t slaveAddress = 0;
    uint8_t *pAddress = &slaveAddress;
    uint8_t *pBuf = nullptr;
    const char *filename = (const char*) "/dev/i2c-1";
    size_t num_bytes;

    public:
        explicit I2C_client(AbstractSensorInterface *obj);
        ~I2C_client();
        void I2CReadRoutine();
        void I2C_Setup();

    signals:
        void finished();        
};

#endif // I2C_CLIENT_H
