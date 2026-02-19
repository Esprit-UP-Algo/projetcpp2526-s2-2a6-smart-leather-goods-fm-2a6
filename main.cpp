#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "connexion.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // TEST DE CONNEXION DIRECTtt
    qDebug() << "\n=== TEST DE CONNEXION AVANT LANCEMENT ===\n";

    Connexion* db = Connexion::getInstance();
    if (db->etablirConnexion()) {
        qDebug() << "\n✅ CONNEXION RÉUSSIE !\n";
    } else {
        qDebug() << "\n❌ ÉCHEC DE CONNEXION !\n";
    }

    // Fermer la connexion (optionnel)
    // db->fermerConnexion();

    MainWindow w;
    w.show();
    return a.exec();
}
