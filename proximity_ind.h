#ifndef PROXIMITY_IND_H
#define PROXIMITY_IND_H

#include <QVector>
#include <QObject>

class Proximity_Ind : public QObject
{
    Q_OBJECT

    uint16_t openingCycles;
    QVector<qint64> timeStamp;

    public:
        explicit Proximity_Ind(size_t);
        ~Proximity_Ind();
        void addData(qint64);
        uint16_t getNumOfCycles();
        void printStuff();

    signals:
        void VecFilled();
};

#endif // PROXIMITY_IND_H
