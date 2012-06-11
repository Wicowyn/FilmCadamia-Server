#include <QtGui/QApplication>
#include <QtTest/qbenchmark.h>
#include "mafenetre.h"

int main(int argc, char *argv[])
{
    initSerialize();
    QApplication a(argc, argv);
    //QBENCHMARK{
    MaFenetre w;
    w.show();
    //}

    return a.exec();
}
