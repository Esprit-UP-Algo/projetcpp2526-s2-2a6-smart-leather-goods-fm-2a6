#include "mainwindow.h"
#include <QApplication>
#include "connexion.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Connexion c;
    if(!c.createconnect())
        return -1;

    MainWindow w;
    w.show();
    return a.exec();
}
