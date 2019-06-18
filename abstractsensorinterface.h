#ifndef ABSTRACTSENSORINTERFACE_H
#define ABSTRACTSENSORINTERFACE_H

#include <iostream>

class AbstractSensorInterface{
    
    public:
        virtual void getNumOfSlotsPerChannel(uint8_t& numSlots) = 0;
        virtual void getBuffers(uint8_t **rBuf, uint8_t **tBuf) = 0;
        virtual uint8_t getChipSelect() = 0;
        virtual size_t getNumBytesTransferred() = 0;
};

#endif // ABSTRACTSENSORINTERFACE_H
