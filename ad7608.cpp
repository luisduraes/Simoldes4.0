#include "ad7608.h"
#include <iomanip>

AD7608::AD7608(const QString &dirUrl, const uint16_t &num):
    file_ad7608(dirUrl + urlAD7608), numOfSamples(num)
{
    _lvdt_1.setName("LVDT_1");
    _loadcell_1.setName("Cell_1");
    n_ofBytes = getNumOfBytes();

    FileSetup();

    {
        try{
            ADC_MOSI = new uint8_t [n_ofBytes];
        }catch (std::bad_alloc& ba){
            std::cerr << "Telaire pdataBuf, bad_alloc caught: " << ba.what() << "\n";
        }

        try{
            ADC_MISO = new uint8_t [n_ofBytes];
        }catch (std::bad_alloc& ba){
            std::cerr << "Telaire pdataBuf, bad_alloc caught: " << ba.what() << "\n";
        }

        try{
            ADC_channels = new float [numOfChannels];
        }catch (std::bad_alloc& ba){
            std::cerr << "Telaire pdataBuf, bad_alloc caught: " << ba.what() << "\n";
        }
    }

    initializeVecData();   
}

AD7608::~AD7608(){

    delete[] ADC_MISO;
    delete[] ADC_MOSI;
    delete[] ADC_channels;
}

void AD7608::FileSetup(){

    setlocale(LC_NUMERIC, "C");

    textstream_file_ad7608.setDevice(&file_ad7608);
    textstream_file_ad7608.setFieldAlignment(QTextStream::AlignLeft);
    textstream_file_ad7608.setFieldWidth(15);

    FileWriteHeader();
}

void AD7608::FileWriteHeader(){

    if(!file_ad7608.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    textstream_file_ad7608 << "ADC GENERAL READINGS\n"
                           << "LVDT_1" << "LOADCELL" << "TIME OF SAMPLING"
                           << qSetFieldWidth(0) << "\n";
    file_ad7608.close();
}

void AD7608::initializeVecData(){

    for(size_t i = 0; i < n_ofBytes; i++){
        ADC_MISO[i] = 0;
        ADC_MOSI[i] = 0;
    }

    for(size_t i = 0; i < numOfChannels; i++){
        ADC_channels[i] = 0;
    }
}

size_t AD7608::getNumOfBytes(){

    return numOfChannels * ResolutionADC / 8;
}

void AD7608::getBuffers(uint8_t **rBuf, uint8_t **tBuf){

    *rBuf = ADC_MISO;
    *tBuf = ADC_MOSI;
}

size_t AD7608::getNumBytesTransferred(){

    return n_ofBytes;
}

void AD7608::processDataReceived(){

    timeStamp.push_back(QDateTime::currentDateTime().toMSecsSinceEpoch());

    ADC_channels[0] = ((ADC_MISO[0] << 10) | (ADC_MISO[1] << 2) | (ADC_MISO[2] >> 6)) & 0x3ffff;
    ADC_channels[1] = ((ADC_MISO[2] << 12) | (ADC_MISO[3] << 4) | (ADC_MISO[4] >> 4)) & 0x3ffff;
    ADC_channels[2] = ((ADC_MISO[4] << 14) | (ADC_MISO[5] << 6) | (ADC_MISO[6] >> 2)) & 0x3ffff;
    ADC_channels[3] = ((ADC_MISO[6] << 16) | (ADC_MISO[7] << 8) | (ADC_MISO[8] >> 0)) & 0x3ffff;

    ADC_channels[4] = ((ADC_MISO[ 9] << 10) | (ADC_MISO[10] << 2) | (ADC_MISO[11] >> 6)) & 0x3ffff;
    ADC_channels[5] = ((ADC_MISO[11] << 12) | (ADC_MISO[12] << 4) | (ADC_MISO[13] >> 4)) & 0x3ffff;
    ADC_channels[6] = ((ADC_MISO[13] << 14) | (ADC_MISO[14] << 6) | (ADC_MISO[15] >> 2)) & 0x3ffff;
    ADC_channels[7] = ((ADC_MISO[15] << 16) | (ADC_MISO[16] << 8) | (ADC_MISO[17] >> 0)) & 0x3ffff;

    for(int i = 0; i < numOfChannels; i++){
        if(ADC_channels[i] >= 0x20000)  ADC_channels[i] = 0 - (0x40000 - ADC_channels[i]);
        ADC_channels[i] = ADC_channels[i] * 10.0 / 0x20000;

    }
    _lvdt_1.addData(ADC_channels[0]);
    _loadcell_1.addData(ADC_channels[1]);
}

const std::vector< std::tuple <float, float, float> > AD7608::getStatistics(std::vector<std::string> &outSensoresName){

    std::vector< std::tuple <float, float, float> > statVector;

    storeDataToFile();

    statVector.reserve(numOfChannels);
    outSensoresName.reserve(numOfChannels);

    outSensoresName.push_back(_lvdt_1.getName().toStdString());
    statVector.push_back(_lvdt_1.getStatistics());

    outSensoresName.push_back(_loadcell_1.getName().toStdString());
    statVector.push_back(_loadcell_1.getStatistics());

    timeStamp.clear();
    return statVector;
}

void AD7608::storeDataToFile(){

    QVector <float> auxVec_1, auxVec_2;

    QTextStream out(&file_ad7608);
    out.setFieldAlignment(QTextStream::AlignLeft);
    out.setFieldWidth(15);

    auxVec_1 = _lvdt_1.readData();
    auxVec_2 = _loadcell_1.readData();

    if (!file_ad7608.open(QIODevice::Append | QIODevice::Text))
        return;

    for(auto iter = 0; iter < auxVec_1.size(); iter++){
        out << auxVec_1.at(iter) << auxVec_2.at(iter)
            << QDateTime::fromMSecsSinceEpoch(timeStamp.at(iter)).toString(format)
            << qSetFieldWidth(0) << "\n" << qSetFieldWidth(15);
    }

    file_ad7608.close();
}
