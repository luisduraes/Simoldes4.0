#include <proximity_ind.h>
#include <QDebug>

Proximity_Ind::Proximity_Ind(size_t size){

    connect(this, &Proximity_Ind::VecFilled , [=]{
        this->printStuff();
    });
    openingCycles = 0;
    timeStamp.reserve(size);
}

Proximity_Ind::~Proximity_Ind(){
    timeStamp.clear();
}

void Proximity_Ind::addData(qint64 value){

    if(openingCycles++ == 10){
        emit VecFilled();
    }
    timeStamp.push_back(value);
}

uint16_t Proximity_Ind::getNumOfCycles(){

    return openingCycles;
}

void Proximity_Ind::printStuff(){

    qDebug() << "Filled\n";
}
