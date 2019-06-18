#ifndef AD7608_H
#define AD7608_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QDateTime>
#include <QFile>

#include <sstream>
#include <stdio.h>
#include <locale.h>
#include <memory>
#include <vector>
#include <tuple>
#include <bits/stdc++.h>
#include <string.h>

#include <abstractsensorinterface.h>
#include <abstractsensor.h>

#define BUFFER_SIZE 1024

class Lvdt : public AbstractSensor{

        int count = 0;
        float aux_lvdt = 0;
        float disp_diff = 0;

    public:

        void addData(const float &value){
            aux_lvdt += value;
            count++;
            if(count == 5)
            {
                disp_diff = 15.05 - (-1.5881 * (aux_lvdt / 5) + 11.783);
                dataVec.push_back(disp_diff);
                //qDebug() << "LVDT Displacement:" << disp_diff << "mm. Voltage: "<< (aux_lvdt / 5);
                aux_lvdt = 0;
                count = 0;
            }
        }
};

class LoadCell : public AbstractSensor{

        int count = 0;
        float aux_loadCell = 0;
        float load_cell = 0;

    public:

        void addData(const float &value){
            aux_loadCell += value;
            count++;
            dataVec.push_back(value);
            //qDebug() << "Voltage: " << value;

//            if(count == 5)
//            {
//                load_cell = (aux_loadCell / 5 / 1027) * 1000 * 900 / 10;
//                dataVec.push_back(load_cell);
//                qDebug() << "Load Cell - Applied force:" << load_cell << "Kg. Voltage:" << (aux_loadCell / 5);
//                count = 0;
//                aux_loadCell = 0;
//            }
        }
};

class AD7608 : public AbstractSensorInterface{

    const uint8_t numOfChannels = 8;
    const uint8_t ResolutionADC = 18;
    const uint16_t numOfSamples;
    float *ADC_channels;
    size_t n_ofBytes, i = 0;
    uint8_t *ADC_MOSI, *ADC_MISO;
    uint16_t inc = 0;
    uint16_t numOfMsgs = 0;

    const QString format = "dd.MM.yyyy hh:mm:ss";
    const QString urlAD7608 = "AD7608.txt";
    QVector <qint64> timeStamp;
    QFile file_ad7608;
    QTextStream textstream_file_ad7608;

    Lvdt _lvdt_1, _lvdt_2;
    LoadCell _loadcell_1;

    inline size_t getNumOfBytes();
    void FileSetup();
    void FileWriteHeader();
    void initializeVecData();
    void storeDataToFile();

    public:
        explicit AD7608(const QString&, const uint16_t &);
        ~AD7608();
        void getBuffers(uint8_t **, uint8_t **) override;
        size_t getNumBytesTransferred() override;
        uint8_t getChipSelect() override {return 0;}
        void getNumOfSlotsPerChannel(uint8_t&) override {}
        const std::vector<std::tuple<float, float, float>> getStatistics(std::vector<std::string> &);
        void processDataReceived();        

    signals:
        void vectorsfilled();
};


class FileStream: public QTextStream{

    public:
        QTextStream *outFile;
        explicit FileStream(QFile *device){
            outFile = new QTextStream(device);
            outFile->setFieldAlignment(QTextStream::AlignLeft);
            outFile->setFieldWidth(15);
        }
};

#endif // AD7608_H
