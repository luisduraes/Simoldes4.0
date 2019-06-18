#ifndef ADS1118_H
#define ADS1118_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QFile>

#include <abstractsensorinterface.h>
#include <abstractsensor.h>
#include <timing.h>

#define PRINT_DEBUG

class ADS_Data : public AbstractSensor{

    public:
        void addData(const float &value){
            dataVec.push_back(value);
        }
};

class ADS1118 : public AbstractSensorInterface{

    //Miscellaneous variables
    const float  fsrV = 7.8125;
    const float  vLT[8] = {0, 2.96, 5.9, 8.86, 11.8, 14.74, 17.7, 20.65};
    const float  tLT[8] = {0, 72.61, 144.12, 217.99, 290.15, 360.64, 430.78, 500};
    const size_t n_ofBytes = 4;
    const uint8_t ChipSelect, numOfSamples, numOfChannels = 2;
    const uint8_t ADS_MOSI[12] = {0x00, 0x00, 0x85, 0x9B, 0x00, 0x00, 0x8B, 0x8B, 0x00, 0x00, 0xBB, 0x8B};
    uint8_t ADS_MISO[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    //Own objects
    ADS_Data _Temp_Internal, _Temp_Thermo1, _Temp_Thermo2;

    //Qt containers   
    const QString format = "dd.MM.yyyy hh:mm:ss";
    const QString urlADS1118 = "ADS_1.txt";
    QVector <qint64> timeStamp;
    QFile file_ad1118;
    QTextStream textstream_file_ad1118;

    //Private functions
    float voltLT(float temp);
    float tempLT(float volt);
    void FileSetup();
    void FileWriteHeader();
    void storeDataToFile();

    public:
        explicit ADS1118(const QString &, const uint8_t &, const uint8_t &);
        ~ADS1118();
        void getBuffers(uint8_t **rBuf, uint8_t **tBuf) override;
        void getNumOfSlotsPerChannel(uint8_t&) override;
        uint8_t getChipSelect() override;
        size_t getNumBytesTransferred() override;
        std::vector< std::tuple<float, float, float> > getStatistics(std::vector<std::string> &);
        void thermoProcessingData();
};

#endif // ADS1118_H
