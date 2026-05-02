#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "connexion.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Connexion *cnx = Connexion::getInstance();
    if(cnx->estConnecte()) {
        QSqlQuery q;
        if(q.exec("SELECT column_name FROM user_tab_columns WHERE table_name = 'DEPOTS'")) {
            while(q.next()) {
                qDebug() << "Column:" << q.value(0).toString();
            }
        } else {
            qDebug() << "Query failed:" << q.lastError().text();
        }
    } else {
        qDebug() << "Connection failed";
    }
    return 0;
}
