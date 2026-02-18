#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Connection& c = Connection::getInstance();

    if(c.createconnect())
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
    else
    {
        QMessageBox::critical(nullptr, "Database",
                              "Connection failed.");
        return 1;
    }
}
