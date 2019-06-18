#ifndef ABSTRACTSENSOR_H
#define ABSTRACTSENSOR_H

#include <QVector>
#include <QString>
#include <tuple>

class AbstractSensor
{
    protected:
        QVector <float> dataVec;
        QString name;

    public:        
        virtual void addData(const float &) = 0;        
        void setName(const QString &str){
            name = str;
        }
        const QVector <float> readData(){
            return dataVec;
        }
        const QString getName(){
            return name;
        }
        const std::tuple <float, float, float> getStatistics(){

            //Do the statistics
            float max = *std::max_element(dataVec.begin(), dataVec.end());
            float min = *std::min_element(dataVec.begin(), dataVec.end());
            float avg = std::accumulate(dataVec.begin(), dataVec.end(), 0.0) / dataVec.size();

            //Clear the vector
            dataVec.clear();

            //Return the tuple created
            return std::make_tuple(max, min, avg);
        }
};

#endif // ABSTRACTSENSOR_H
