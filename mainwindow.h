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
    void on_btn_tri_points_clicked();   // Trier par Points Fidélité
    void on_btn_mailing_clicked();      // Mailing Ciblé

    // Sélection dans la table
    void on_tableClients_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    Client clientTmp;  // ← Maintenant Client est connu grâce à l'include

    // Méthodes utilitaires
    void refreshTableView();           // Rafraîchir l'affichage
    void clearInputFields();           // Effacer les champs
    void loadClientToForm(int row);    // Charger un client dans le formulaire
    bool validateInputs();             // Valider les champs
    void enableInputFields(bool enable);   // Activer/désactiver les champs
    void setFormMode(bool isNewMode);      // Mode ajout ou modification
    void updateClientStatus();         // Mettre à jour le statut de fidélité
    void updateIAPrediction();         // Mettre à jour la prévision IA
};

#endif // MAINWINDOW_H
