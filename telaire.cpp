#include "telaire.h"
#include <iomanip>

Telaire::Telaire(const QString &dirUrl, const uint8_t &num):
    file_telaire(dirUrl + urlTelaire), numOfSamples(num)
{   
    try{
        pdataBuf = new uint8_t [memory_size];
    }catch (std::bad_alloc& ba){
        std::cerr << "Telaire pdataBuf, bad_alloc caught: " << ba.what() << std::endl;
    }
    for(size_t i = 0; i < memory_size; i++){
        *(pdataBuf++) = 0;
    }
}

Telaire::~Telaire(){

    delete[] pdataBuf;
}

void Telaire::FileSetup(){

    setlocale(LC_NUMERIC, "C");

    textstream_file_telaire.setDevice(&file_telaire);
    textstream_file_telaire.setFieldAlignment(QTextStream::AlignLeft);
    textstream_file_telaire.setFieldWidth(15);

    FileWriteHeader();
}

void Telaire::FileWriteHeader(){

    if(!file_telaire.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    textstream_file_telaire << "TELAIRE - ROOM TEMPERATURE/HUMIDITY\n"
                            << "TEMPERATURE" << "HUMIDITY" << "TIME OF SAMPLING"
                            << qSetFieldWidth(0) << "\n";
    file_telaire.close();
}

void Telaire::getBuffers(uint8_t **rBuf, uint8_t **tBuf){

    *rBuf = (uint8_t *) pdataBuf;
    **tBuf = telaireAddr;
}

size_t Telaire::getNumBytesTransferred(){

    return memory_size;
}

void Telaire::humTempProcessingData(){

    timeStamp.push_back(QDateTime::currentDateTime().toMSecsSinceEpoch());

    float Hum = (((pdataBuf[0] & 0x3F ) << 8) + pdataBuf[1]) / 16384.0 * 100.0;
    float Temp = ((unsigned)(pdataBuf[2] * 64) + (unsigned)(pdataBuf[3] >> 2 )) / 16384.0 * 165.0 - 40.0;

//    qDebug() << "Telaire" << "\n"
//             << "Temperature:" << Temp << "ºC" << "\n"
//             << "Humidity:" << Hum << "%" << "\n";

    _Temp.addData(Temp);
    _Hum.addData(Hum);

}

const std::vector< std::tuple <float, float, float> > Telaire::getStatistics(std::vector<std::string> &outSensoresName){

    std::vector< std::tuple <float, float, float> > statVector;

    statVector.reserve(numOfChannels);
    outSensoresName.reserve(numOfChannels);

    outSensoresName.push_back("Room Temperature");
    statVector.push_back(_Temp.getStatistics());

    outSensoresName.push_back("Relative Humidity");
    statVector.push_back(_Hum.getStatistics());

    timeStamp.clear();

    return statVector;
}

void Telaire::storeDataToFile(){

    QVector <float> auxVec_1, auxVec_2;

    QTextStream out(&file_telaire);
    out.setFieldAlignment(QTextStream::AlignLeft);
    out.setFieldWidth(15);

    auxVec_1 = _Temp.readData();
    auxVec_2 = _Hum.readData();

    if(!file_telaire.open(QIODevice::Append | QIODevice::Text))
        return;

    for(auto iter = 0; iter < timeStamp.size(); iter++){
        out << auxVec_1.at(iter) << auxVec_2.at(iter)
            << QDateTime::fromMSecsSinceEpoch(timeStamp.at(iter)).toString(format)
            << qSetFieldWidth(0) << "\n" << qSetFieldWidth(14);
    }

    file_telaire.close();
}
