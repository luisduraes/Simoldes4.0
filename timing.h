#ifndef TIMING_H
#define TIMING_H

#include <chrono>
#include <QDebug>
#include <QString>

class Timing{

    std::chrono::time_point<std::chrono::system_clock> start, end;
    const QString str;

    public:
        Timing(const QString &var):
            str(var)
        {
            start = std::chrono::high_resolution_clock::now();
        }

        ~Timing(){
            end = std::chrono::high_resolution_clock::now();
            qDebug() << str
                     << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
                     << " us\n";
        }
};


#endif // TIMING_H
