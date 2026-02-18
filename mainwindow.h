#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ordrefabrication.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QVector>
#include <QDate>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPrinter>
#include <QFileDialog>

// --- INCLUDES ---
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QHeaderView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ColorDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        QVariant bg = index.data(Qt::BackgroundRole);
        if (bg.isValid() && bg.canConvert<QBrush>()) {
            painter->fillRect(opt.rect, bg.value<QBrush>());
            opt.backgroundBrush = QBrush(Qt::NoBrush);
        }
        QStyledItemDelegate::paint(painter, opt, index);
    }
};

struct ProduitInfo { QString ref; QString nom; double coutMatiere; QString collection; QString cuir; int temps; };
struct CommandeInfo { QString id; QString produit; int quantite; QString matiere; QDate dateDebut; QString dateFinEstimee; QString statut; QString etapeAuditee; int etatEtape; };
struct EmployeInfo { QString id; QString nom; QString prenom; QString poste; QString departement; QDate dateEmbauche; double salaire; QString rfid; };
struct MatiereInfo { QString code; QString categorie; QString etat; QString couleur; QString qualite; double quantite; QString unite; QString zone; QString allee; QString typeStock; QDate dateRec; };
struct ClientInfo { QString id; QString nom; QString telephone; QString adresse; QString email; int pointsFidelite; };
struct DepotInfo { QString id; QString emplacement; QString etagere; double capaciteMax; double quantiteActuelle; QString typeStockage; };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    OrdreFabrication tmpOrdre; // <--- AJOUTER CETTE LIGNE

    QVector<CommandeInfo> mesCommandes;
    QVector<ProduitInfo> mesProduits;
    QVector<EmployeInfo> mesEmployes;
    QVector<MatiereInfo> mesMatieres;
    QVector<ClientInfo> mesClients;
    QVector<DepotInfo> mesDepots;

    ColorDelegate *myColorDelegate;

    int indexCommandeSelectionnee = -1;
    bool modeModification = false; int indexModification = -1;
    bool modeModifProd = false; int indexModifProd = -1;
    bool modeModifEmp = false; int indexModifEmp = -1;
    bool modeModifStock = false; int indexModifStock = -1;
    bool modeModifClient = false; int indexModifClient = -1;
    bool modeModifDepot = false; int indexModifDepot = -1;

    // Fonctions Communes
    void exporterPDF(QTableWidget *table, QString titreDocument);
    void exporterCSV(QTableWidget *table, const QString &titreDocument);

    // Module Planif
    void rafraichirListeCommandes();
    void configurerTimelineGantt();
    void dessinerBarre(int ligne, int colDebut, int duree, QString texte, QColor bgCol, QColor textCol);
    void calculerEtAfficherStats();

    // Module Produits
    void rafraichirListeProduits();
    void calculerStatsProduits();
    void showProdSimDialog(); // (déjà existant)
    void showProduitCoutDialog();
    void showHistoriqueModeDialog();
    void showPlanifIaDialog(); // [NOUVEAU] Pop-up IA Planification

    // Module RH
    void rafraichirListeEmployes();
    void calculerStatsRH();
    void reponseChatbot();
    void showEmpEvalDialog(); // [NOUVEAU]

    // Module Clients
    void rafraichirListeClients();
    void calculerStatsClients();
    void exporterFactureClient();
    void showClientIaDialog();
    void showFideliteDialog();

    // Module Dépôt
    void rafraichirListeDepots();
    void calculerStatsDepots();
    void showOptimizeSpaceDialog();
    void showRavitaillementDialog();

    // Module Stock (EXISTANT)
    void rafraichirListeMatieres();
    void calculerStatsStock();
    void showCompareDialog();
    void showBesoinDialog();
    // AJOUTER CETTE FONCTION :
    void ouvrirDialoguePlanif(bool estModification);
    void ouvrirDialogueProduit(bool estModif);
    void ouvrirDialogueEmploye(bool estModif);
    void ouvrirDialogueStock(bool estModif);
    void ouvrirDialogueClient(bool estModif);
    void ouvrirDialogueDepot(bool estModif);

    // ... (vos variables existantes) ...

    // AJOUTER CES LIGNES POUR LES STATS POP-UP :
    void ouvrirStatsProduits();
    void ouvrirStatsRH();
    void ouvrirStatsStock();
    void ouvrirStatsClients();
    void ouvrirStatsDepot();

    // Une petite fonction utilitaire pour le design des cartes KPI
    QFrame* creerCarteStat(QString icone, QString val, QString titre, QString couleurFond);
};

#endif // MAINWINDOW_H
