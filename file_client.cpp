#include "file_client.h"
#include <QDebug>

#include <QString>

File_client::File_client(const QString &url, const std::stringstream &ssHeader) :
    report(url)
{
    setlocale(LC_NUMERIC, "C");

    out.setDevice(&report);
    out.setFieldAlignment(QTextStream::AlignLeft);
    out.setFieldWidth(20);
    qDebug() << url;
    qDebug() << ssHeader.str().c_str();

    writeContent(ssHeader, QIODevice::WriteOnly | QIODevice::Text);
}

void File_client::writeContent(const std::stringstream &ssOutput, QIODevice::OpenMode mode){

    const QString strOutput = QString::fromStdString(ssOutput.str());

    if(!report.open(mode))
        return;

    out << strOutput;
    report.close();
}
