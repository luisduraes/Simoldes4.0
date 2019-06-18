#ifndef TELAIRE_H
#define TELAIRE_H

#include <new>
#include <exception>
#include <numeric>

#include <QDebug>
#include <QVector>
#include <QDateTime>
#include <QFile>

#include <abstractsensorinterface.h>
#include <abstractsensor.h>

class Telaire_Data : public AbstractSensor{

    public:
        void addData(const float &value){
            dataVec.push_back(value);
        }
};

class Telaire : public AbstractSensorInterface{

    //Miscellaneous variables
    const uint8_t telaireAddr = 0x28;
    const size_t memory_size = 4;
    const uint8_t numOfSamples;
    const uint8_t numOfChannels = 2;
    uint8_t *pdataBuf; 

    //Own objects
    Telaire_Data _Temp, _Hum;

    //Qt containers
    const QString format = "hh:mm:ss";
    const QString urlTelaire = "Telaire.txt";
    QVector <qint64> timeStamp;
    QFile file_telaire;
    QTextStream textstream_file_telaire;

    //Private functions
    void FileSetup();
    void FileWriteHeader();
    void storeDataToFile();

    signals:
        void filled();

    public:
        explicit Telaire(const QString &dirUrl, const uint8_t &num);
        ~Telaire();
        void getBuffers(uint8_t **rBuf, uint8_t **tBuf) override;
        size_t getNumBytesTransferred() override;
        uint8_t getChipSelect() override {return 0;}
        void getNumOfSlotsPerChannel(uint8_t&) override {}
        const std::vector< std::tuple <float, float, float> > getStatistics(std::vector<std::string> &);
        void humTempProcessingData();
};

#endif // TELAIRE_H
