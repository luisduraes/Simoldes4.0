#include <QCoreApplication>
#include <cprocessing.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    CProcessing test1;
    return a.exec();
}
