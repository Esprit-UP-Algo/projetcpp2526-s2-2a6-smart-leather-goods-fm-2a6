#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.h"  // ← AJOUTER CETTE LIGNE

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slots pour les boutons de la sidebar
    void on_btn_ajouter_clicked();      // AJOUTER
    void on_btn_modifier_clicked();     // MODIFIER
    void on_btn_supprimer_clicked();    // SUPPRIMER
    void on_btn_rechercher_clicked();   // RECHERCHER
    void on_btn_pdf_clicked();          // PDF / Factures
    void on_btn_stat_clicked();         // Statistiques
    void on_btn_facture_clicked();      // Générer Facture PDF
    void on_btn_tri_points_clicked();   // Trier par Points Fidélité  ← GARDE ICI
    void on_btn_mailing_clicked();      // Mailing Ciblé

    // Sélection dans la table
    void on_tableClients_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    Client clientTmp;

    // Méthodes utilitaires
    void refreshTableView();
    void clearInputFields();
    void loadClientToForm(int row);
    bool validateInputs();
    void enableInputFields(bool enable);
    void setFormMode(bool isNewMode);
    void updateClientStatus();
    void updateIAPrediction();
    void verifierPlanifications();
   void envoyerMail(QString to, QString subject, QString body);
    QString getProduitsFromDB();
   void askChatbot(QString question);
    void on_sendButton_clicked();
    // ← SUPPRIME on_btn_tri_points_clicked() D'ICI
};

#endif // MAINWINDOW_H
