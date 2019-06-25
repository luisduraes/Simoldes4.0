#include "ads1118.h"
#include <iomanip>

ADS1118::ADS1118(const QString &dirUrl, const uint8_t &num, const uint8_t &CS):
    file_ad1118(dirUrl + urlADS1118), numOfSamples(num), ChipSelect(CS)
{
    _Temp_Internal.setName("Internal_1");
    _Temp_Thermo1.setName("Thermo_1a");
    _Temp_Thermo2.setName("Thermo_1b");

    FileSetup();
}

void ADS1118::FileSetup(){

    setlocale(LC_NUMERIC, "C");

    textstream_file_ad1118.setDevice(&file_ad1118);
    textstream_file_ad1118.setFieldAlignment(QTextStream::AlignLeft);
    textstream_file_ad1118.setFieldWidth(15);

    FileWriteHeader();
}

void ADS1118::FileWriteHeader(){

    if(!file_ad1118.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    textstream_file_ad1118 << "ADS TEMPERATURE READINGS\n"
                           << "THERMOCOUPLE 1" << "THERMOCOUPLE 2" << "TIME OF SAMPLING"
                           << qSetFieldWidth(0) << "\n";
    file_ad1118.close();
}



void ADS1118::getBuffers(uint8_t **rBuf, uint8_t **tBuf){

    *rBuf = (uint8_t *) ADS_MISO;
    *tBuf = (uint8_t *) ADS_MOSI;
}

uint8_t ADS1118::getChipSelect(){

    return ChipSelect;
}

size_t ADS1118::getNumBytesTransferred(){

    return n_ofBytes;
}

void ADS1118::getNumOfSlotsPerChannel(uint8_t& numSlots){

    numSlots = 3;
}

float ADS1118::voltLT(float temp){

    int n = 0;
    float voltage = 0;

    for(int i = 0; i < 8; i++)
        if(temp < tLT[i]){
            n = i;
            break;
        }

    voltage = vLT[n - 1] + (vLT[n] - vLT[n - 1]) * ((temp - tLT[n - 1]) / (tLT[n] - tLT[n - 1]));
    return voltage;
}

float ADS1118::tempLT(float volt){

    int n = 0;
    float temperature = 0;

    for(int i = 0; i < 8; i++)
        if(volt < vLT[i]){
            n = i;
            break;
        }

    temperature = tLT[n - 1] + (tLT[n] - tLT[n - 1]) * ((volt - vLT[n - 1]) / (vLT[n] - vLT[n - 1]));
    return temperature;
}

void ADS1118::thermoProcessingData(){

    timeStamp.push_back(QDateTime::currentDateTime().toMSecsSinceEpoch());

    float  ADS_Voltage_Thermo1, ADS_Voltage_Thermo2,
           ADS_Temp_Thermo1, ADS_Temp_Thermo2,
           ADS_Internal_Temp, ADS_Internal_Voltage;

    ADS_Internal_Temp = ((ADS_MISO[0] >> 2) * 256 + (ADS_MISO[1] >> 2) + ((ADS_MISO[0] & 0x03) << 6)) * 0.03125;
    ADS_Internal_Voltage = voltLT(ADS_Internal_Temp);

    if(ADS_MISO[4]>=128)
        ADS_Voltage_Thermo1 = (0 - (0x10000 - (ADS_MISO[4] * 256 + ADS_MISO[5])))* fsrV /1000;
    else
        ADS_Voltage_Thermo1 = (ADS_MISO[4] * 256 + ADS_MISO[5]) * fsrV /1000;

    if(ADS_MISO[8]>=128)
        ADS_Voltage_Thermo2 = (0 - (0x10000 - (ADS_MISO[8] * 256 + ADS_MISO[9])))* fsrV /1000;
    else
        ADS_Voltage_Thermo2 = (ADS_MISO[8] * 256 + ADS_MISO[9]) * fsrV /1000;

    ADS_Temp_Thermo1 = tempLT(ADS_Internal_Voltage + ADS_Voltage_Thermo1);
    ADS_Temp_Thermo2 = tempLT(ADS_Internal_Voltage + ADS_Voltage_Thermo2);

    qDebug() << "CE" << ChipSelect;

#ifdef PRINT_DEBUG
    qDebug() << "Internal Temp: " << qSetRealNumberPrecision(4) << ADS_Internal_Temp << "ºC";
    qDebug() << "Thermocouple 1: " << qSetRealNumberPrecision(4)<< ADS_Temp_Thermo1 << "ºC";
    //qDebug() << "Thermocouple 2: " << qSetRealNumberPrecision(4) << ADS_Temp_Thermo2 << "ºC\n";
#endif

    _Temp_Internal.addData(ADS_Internal_Temp);
    _Temp_Thermo1.addData(ADS_Temp_Thermo1);
    _Temp_Thermo2.addData(ADS_Temp_Thermo2);
}

std::vector< std::tuple <float, float, float> >
ADS1118::getStatistics(std::vector<std::string> &outSensoresName){

    std::vector< std::tuple <float, float, float> > statVector;

    storeDataToFile();

    statVector.reserve(numOfChannels);
    outSensoresName.reserve(numOfChannels);

    outSensoresName.push_back(_Temp_Thermo1.getName().toStdString());
    statVector.push_back(_Temp_Thermo1.getStatistics());

    outSensoresName.push_back(_Temp_Thermo2.getName().toStdString());
    statVector.push_back(_Temp_Thermo2.getStatistics());

    timeStamp.clear();

    return statVector;
}

void ADS1118::storeDataToFile(){

    QVector <float> auxVec_1, auxVec_2;

    QTextStream out(&file_ad1118);
    out.setFieldAlignment(QTextStream::AlignLeft);
    out.setFieldWidth(15);

    auxVec_1 = _Temp_Thermo1.readData();
    auxVec_2 = _Temp_Thermo2.readData();

    if(!file_ad1118.open(QIODevice::Append | QIODevice::Text))
        return;

    for(auto iter = 0; iter < timeStamp.size(); iter++){
        out << auxVec_1.at(iter) << auxVec_2.at(iter)
            << QDateTime::fromMSecsSinceEpoch(timeStamp.at(iter)).toString(format)
            << qSetFieldWidth(0) << "\n" << qSetFieldWidth(15);
    }

    file_ad1118.close();
}
