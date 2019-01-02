#include "recogdemo.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RecogDemo w;
    w.show();

    return a.exec();
}
