 /* SENSOR DISCRIPTION
 *
 * S1 ABREVIATION FOR SENSOR_1
 * -> INCLUDES THE TEMP AND RELATIVE HUMIDITY SENSORS
 * -> ITS FUNCTION WILL BE CALLED AS MANY TIMES AS SENSOR_1_READSPERDAY
 *
 */

#include "cprocessing.h"
#include "timing.h"
#include <wiringPi.h>
#include <bcm2835.h>
#include <QDateTime>
#include <sys/time.h>
#include <chrono>
#include <proximity_ind.h>
#include <jsoncpp/json/json.h>

#define SENSOR_1_READSPERDAY 2
#define CE0 25
#define CE1 8
#define CE2 7

const uint_fast32_t ADC_AQ_Time = 200;
const uint_fast32_t SPI_AQ_Time = 300;

/*
 *  Interruption Routine Related
 *  ISR_PIN -> Interruption PIN
 *  IGNORE_CHANGE_BELOW_USEC -> time passed since the last change
 *  (jitter filter), ignore if diff is below this value.
 */
#define ISR_PIN 21
#define IGNORE_CHANGE_BELOW_USEC 10000
static volatile int ISR_pinState;
static int cycle_counter = 0;
static volatile int nOfSensor = 0;
struct timeval last_change;
volatile qint64 openedMold_timer, closedMold_timer, auxMold_timer;

CProcessing *pCProcessing;

/*
 * FormatDatatoJsonStructure(*,*,*) formats and concatenates
 * data according to Json standards.
 * Receives the name of the sensor and its tuple with data
 * Concatenates all into the original output stream
 */
void FormatDatatoJsonStructure(std::stringstream &ssOutput, const std::string outName,
                               const std::tuple<float,float,float> outData){

    ssOutput << "\n\"Sensor_" << ++nOfSensor << "\" : {\n";
    ssOutput << "\t\"Name\": \"" << outName << "\",\n";                     //Name of sensor
    ssOutput << "\t\"MaxValue\": " << std::get<0> (outData) << ",\n";       //Max value of sensor
    ssOutput << "\t\"MinValue\": " << std::get<1> (outData) << ",\n";       //Min value of sensor
    ssOutput << "\t\"AvgValue\": " << std::get<2> (outData) << "\n";        //Avg value of sensor
    ssOutput << "\t}";
}

/*
 * Interruption Handler function
 * Sensor:  Contact sensor (Openning cycles)
 * Brief Description:
 * When the PIN state is '1' (The Mold is open), it starts
 * a thread which is responsible for the statisics.
 * It stores both opening and closing times for later analysis.
 */
void ISR_Handler(){

    struct timeval now;    
    unsigned long diff;
    ISRThread *pISR = new ISRThread(pCProcessing);

    gettimeofday(&now, NULL);

    // Time difference in usec
    diff = (now.tv_sec * 1000000 + now.tv_usec) -
            (last_change.tv_sec * 1000000 + last_change.tv_usec);

    // Filter jitter
    if (diff > IGNORE_CHANGE_BELOW_USEC) {
        if (ISR_pinState){
            auxMold_timer = QDateTime::currentDateTime().toMSecsSinceEpoch();
            pISR->start();
            qDebug() << "Mold Opened\n";
            ++cycle_counter;
        }else{
            closedMold_timer = QDateTime::currentDateTime().toMSecsSinceEpoch();
            qDebug() << "Mold Closed\n";
        }
        ISR_pinState = !ISR_pinState;
    }
    last_change = now;
}

/*
 * Interruption setup
 * Sensor:  Contact sensor (Openning cycles)
 * Brief Description: Assigns a pin to its behavior.
 * Defines the catch of the interruption itself
 * (EDGE_BOTH, since we want to know both rising and fall times).
 * It stores the time for later analysis.
 */
int SetupISR(int PIN){

    pinMode(PIN, INPUT);
    wiringPiISR(PIN, INT_EDGE_BOTH, &ISR_Handler);

    gettimeofday(&last_change, NULL);

    ISR_pinState = digitalRead(PIN);
    openedMold_timer = QDateTime::currentDateTime().toMSecsSinceEpoch();
    return ISR_pinState;
}

void CProcessing::getStatistics(){

    std::stringstream JsonOutputBuffer;
    std::vector <std::string> SensorsName;

    auto prox_Sensor_t_ON =  auxMold_timer - closedMold_timer;
    auto prox_Sensor_t_OFF = closedMold_timer - openedMold_timer;

    TCP_client workerTCP;
    uint16_t bytesRead;

    JsonOutputBuffer << "{\n";
    JsonOutputBuffer << "\"Mold\": \"TEST_UMINHO\",\n";

    JsonOutputBuffer << "\"Period of Reading\": {\n";
    JsonOutputBuffer << "\t\"Initial time\": \t\""    << QDateTime::fromMSecsSinceEpoch(openedMold_timer).toString("dd.MM.yyyy hh:mm:ss").toStdString() << "\",\n";
    JsonOutputBuffer << "\t\"Final time\": \t\""      << QDateTime::fromMSecsSinceEpoch(auxMold_timer).toString("dd.MM.yyyy hh:mm:ss").toStdString()  << "\"\n\t},\n";
    openedMold_timer = auxMold_timer;

    JsonOutputBuffer << "\"Cycle Counter\": {\n";
    JsonOutputBuffer << "\t\"N of Cycle\": "   << cycle_counter << ",\n";
    JsonOutputBuffer << "\t\"Time on\": "       << prox_Sensor_t_ON / 1000.0 << ",\n";
    JsonOutputBuffer << "\t\"Time off\": "      << prox_Sensor_t_OFF / 1000.0 << "\n\t},";

    //AD7608 Sensor
    auto ad7608_Stats =  pAd7608->getStatistics(SensorsName);
    for(size_t iter = 0; iter < ad7608_Stats.size(); iter++){
        FormatDatatoJsonStructure(JsonOutputBuffer, SensorsName.at(iter), ad7608_Stats.at(iter));
        JsonOutputBuffer << ",";
    }
    SensorsName.clear();

    //ADS1118
    auto ads1118_Stats = pAds1118_1[0]->getStatistics(SensorsName);
    for(size_t iter = 0; iter < ads1118_Stats.size(); iter++){
        FormatDatatoJsonStructure(JsonOutputBuffer, SensorsName.at(iter), ads1118_Stats.at(iter));
        //if(iter == 0)
            JsonOutputBuffer << ",";
    }
    SensorsName.clear();

    //Telaire
    auto telaire_Stats =  pTelaire->getStatistics(SensorsName);
    for(size_t iter = 0; iter < telaire_Stats.size(); iter++){
        FormatDatatoJsonStructure(JsonOutputBuffer, SensorsName.at(iter), telaire_Stats.at(iter));
        if(iter == 0)
            JsonOutputBuffer << ",";
    }

    JsonOutputBuffer << "\n}"; //End of message

    //If the connection is successfully established with the local server,
    //Send the dataframe to the cloud
    if(workerTCP.ConnectToHost("127.0.0.1", 5050) == 0){
        bytesRead = workerTCP.SendMessage(JsonOutputBuffer);
    }

    nOfSensor = 0;
}

CProcessing::CProcessing(QObject *parent) : QObject(parent){

    pCProcessing = this;

    for(auto iter = 0; iter < nOfSockets; iter++){
        try{
            pAds1118_1[iter] =  new ADS1118(urlDirectory, ADSNumSamples, chipSelect.at(iter));
        }catch (std::bad_alloc& ba){
            std::cerr << "bad_alloc caught: " << ba.what() << '\n';
        }
    }

    try{
        pTelaire =  new Telaire(urlDirectory, TelaireNumSamples);
    }catch (std::bad_alloc& ba){
        std::cerr << "bad_alloc caught: " << ba.what() << '\n';
    }

    try{
        pAd7608 =  new AD7608(urlDirectory, AD7608NumSamples);
    }catch (std::bad_alloc& ba){
        std::cerr << "bad_alloc caught: " << ba.what() << '\n';
    }

    if(!bcm2835_init()){
        std::cerr << "BCM2835 library was not initialized!\n"
                  << "Steps for fixing this issue:\n"
                  << "1ºStep - Run the program again.\n"
                  << "2ºStep - Restart Raspberry Pi (Only if the first step does not work.\n";
        //return -1;
    }

    wiringPiSetup();
    if(SetupISR(ISR_PIN)) qDebug() << "Started! Initial state is on\n";
    else qDebug() << "Started! Initial state is off\n";

    /*
     *  ADC Setup
     *  Construct timer with the aquisition time given (ADC_AQ_Time).
     *  Wait for timeout, then jump to its function (ADC_Start_Client()).
     */
    try{
        SPI_Aux_Timer = new QTimer(this);
    }catch (std::bad_alloc& ba){
        std::cerr << "bad_alloc caught: " << ba.what() << '\n';
    }
    connect(SPI_Aux_Timer, &QTimer::timeout, this, &CProcessing::SPI_Aux_Start_Client);
    SPI_Aux_Timer->setInterval(ADC_AQ_Time);
    SPI_Aux_Timer->start();


    /*
     *  SPI Timer Setup
     *  Construct timer with the aquisition time given (SPI_AQ_Time).
     *  Wait for timeout, then jump to its function (SPI_Start_Client()).
     */
    try{
        SPI_Timer = new QTimer(this);
    }catch (std::bad_alloc& ba){
        std::cerr << "bad_alloc caught: " << ba.what() << '\n';
    }
    connect(SPI_Timer, &QTimer::timeout, this, &CProcessing::SPI_Start_Client);
    SPI_Timer->setInterval(SPI_AQ_Time);

    SPI_Timer->start();

    /*
     *  I2C Timer Setup
     *  Construct timer with the aquisition time given.
     *  Less cumbersome timer, which is ideal for this operation,
     *  since its function will be called just a few times per day.
     *  When timeout occurs, it jumps for a specific event type function.
     */
    //startTimer((24 * 3600 * 1000) / SENSOR_1_READSPERDAY);
    startTimer(1*1000);
}

/*
 * Cprocessing destructor
 * When the destructor is called, it tries to close the library.
 */
CProcessing::~CProcessing(){

    if(!bcm2835_close()){
        qDebug() << "BCM2835 cannot finish!";
    }
}

/*
 * I2C Timer Event
 * Brief Description: Creates a new thread and a new I2C Client, then,
 * moves the I2C Client to the thread as it will run inside of it.
 * Assigns multiple signals to its slots, for control and redirecting purposes.
 * Starts the thread.
 */
void CProcessing::timerEvent(QTimerEvent *){

    QThread *workerThread;
    try{
        workerThread = new QThread;
    }catch (std::bad_alloc& ba){
        std::cerr << "I2C Thread, bad_alloc caught: " << ba.what() << std::endl;
    }

    try{
        workerI2C = new I2C_client(pTelaire);
    }catch (std::bad_alloc& ba){
        std::cerr << "I2C_client, bad_alloc caught: " << ba.what() << std::endl;
    }
    workerI2C->I2C_Setup();
    workerI2C->moveToThread(workerThread);

    connect(workerThread, &QThread::started, workerI2C, &I2C_client::I2CReadRoutine);
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
    connect(workerI2C, &I2C_client::finished, workerThread, &QThread::quit);
    connect(workerI2C, &I2C_client::finished, workerI2C, &QThread::deleteLater);
    connect(workerI2C, &I2C_client::finished, [=]{
        pTelaire->humTempProcessingData();
    });

    workerThread->start();
}


/*
 * SPI Auxiliar Timer Event
 * Brief Description: Creates a new thread and a new SPI_Aux_Client, then,
 * moves the I2C Client to the thread as it will run inside of it.
 * Assigns multiple signals to its slots, for control and redirecting purposes.
 * Starts the thread.
 */
void CProcessing::SPI_Aux_Start_Client(){

    QThread *workerThread;
    try{
        workerThread = new QThread;
    }catch (std::bad_alloc& ba){
        std::cerr << "ADC Thread, bad_alloc caught: " << ba.what() << '\n';
    }

    try{
        workerSPI_Aux = new SPI_aux_client(pAd7608);
    }catch (std::bad_alloc& ba){
        std::cerr << "ADC_client, bad_alloc caught: " << ba.what() << '\n';
    }
    workerSPI_Aux->moveToThread(workerThread);

    connect(workerThread, &QThread::started, workerSPI_Aux, &SPI_aux_client::SPI_Aux_ReadRoutine);
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
    connect(workerSPI_Aux, &SPI_aux_client::finished, [=]{
        pAd7608->processDataReceived();
    });
    connect(workerSPI_Aux, &SPI_aux_client::finished, workerThread, &QThread::quit);
    connect(workerSPI_Aux, &SPI_aux_client::finished, workerSPI_Aux, &QThread::deleteLater);
    workerThread->start();
}

void CProcessing::SPI_Start_Client(){

    QThread *workerThread;

    for(auto iter = 0; iter < nOfSockets; iter++){        

        try{
            workerThread = new QThread;
        }catch (std::bad_alloc& ba){
            std::cerr << "SPI Thread, bad_alloc caught: " << ba.what() << '\n';
        }

        try{
            workerSPI = new SPI_client(pAds1118_1[iter]);
        }catch (std::bad_alloc& ba){
            std::cerr << "SPI_client, bad_alloc caught: " << ba.what() << '\n';
        }

        workerSPI->moveToThread(workerThread);

        connect(workerThread, &QThread::started, workerSPI, &SPI_client::SPI_ReadRoutine);
        connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
        connect(workerSPI, &SPI_client::finished, workerThread, &QThread::quit);
        connect(workerSPI, &SPI_client::finished, workerSPI, &QThread::deleteLater);
        connect(workerSPI, &SPI_client::finished, [=]{
            pAds1118_1[iter]->thermoProcessingData();
        });

        /*
         * Connect the destroyed signal of each thread to a lambda function which activates
         * a flag, meaning that the thread itself has been destroyed.
         */
        connect(workerSPI, &SPI_client::destroyed, [=]{
            b_workerSPI = true;
        });

        workerThread->start();

        /*
         * Stop the execution until the flag is activated.
         * It does't stop the execution of the program itself because most of the threads are still running.
         * It avoids running all Adcs simultaneously, as it is't possible, it works sequentially which is what is intended.
         */
        while(b_workerSPI != true)
            ;
        b_workerSPI = false;
    }
}

