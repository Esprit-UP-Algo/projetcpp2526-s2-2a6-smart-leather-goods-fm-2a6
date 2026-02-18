#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include "connexion.h" // <--- Ajout

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ProjetCpp_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // --- CONNEXION BDD ---
    Connexion c;
    bool etat = c.createconnect();
    if (!etat) {
        QMessageBox::critical(nullptr, "Erreur Base de Données", "Impossible de se connecter à Oracle.\nVérifiez que la base est lancée.");
        // On continue quand même pour voir l'interface, ou return -1 pour quitter
    }
    // ---------------------

    MainWindow w;
    w.show();
    return a.exec();
}
