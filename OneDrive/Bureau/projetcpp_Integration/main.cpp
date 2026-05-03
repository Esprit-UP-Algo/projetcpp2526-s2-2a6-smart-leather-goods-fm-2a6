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

    // --- Connexion Singleton ---
    Connexion *cnx = Connexion::getInstance();

    if (!cnx->ouvrir()) {
        QMessageBox::critical(nullptr, "Erreur Base de Données",
            "❌ Impossible de se connecter à Oracle.\n\n"
            "Vérifiez que :\n"
            "• Oracle XE est démarré\n"
            "• Le listener est actif\n"
            "• Les identifiants sont corrects\n\n"
            "Erreur : " + cnx->getDatabase().lastError().text());
        return -1;
    }

    MainWindow w;
    w.showMaximized();

    int result = a.exec();

    // Fermer la connexion à la fin
    cnx->fermer();

    return result;
}
