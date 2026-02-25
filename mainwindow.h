#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ordrefabrication.h"
#include "matierepremiere.h"
#include "depot.h"

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

struct ProduitInfo { QString id_produit; QString designation; double cout; QString collection; QString typeCuir; int tempsFab; QString idClient; QString idEmplacement; };
struct CommandeInfo { QString id; QString idProduit; int quantite; QString idMatiere; QDate dateDebut; QString dateFinEstimee; QString statut; QString idEmploye; int etatEtape; };
struct EmployeInfo { QString id; QString nom; QString prenom; QString poste; QString email; QString telephone; QString departement; QDate dateEmbauche; double salaire; QString rfid; };
struct MatiereInfo { QString id; QString code; QString categorie; QString numLot; QString etat; QString couleur; double quantite; QString typeStockage; QString qualite; };
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
    OrdreFabrication tmpOrdre;
    MatierePremiere tmpMatiere;
    Depot tmpDepot;


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
    void preparerFormulaireEmploye(bool estModif, int idx = -1);
    void showEmpEvalTab();
    void showEmpAncienneteTab();
    void showEmpAssistantTab();

    // Module Clients
    void rafraichirListeClients();
    void calculerStatsClients();
    void exporterFactureClient();
    void showClientFideliteTab();
    void showClientIaTab();


    // Module Dépôt
    void rafraichirListeDepots();
    void calculerStatsDepots();
    void preparerFormulaireDepot(bool estModif, int idx = -1);
    void showDepotOptimizeTab();
    void showDepotRavitaillementTab();

    // Module Stock (NOUVEAU - SPA avec onglets)
    void rafraichirListeMatieres();
    void calculerStatsStock();
    void preparerFormulaireStock(bool estModif, int idx = -1);
    void showStockCompareTab();
    void showStockCalculTab();
    void preparerFormulairePlanif(bool estModification);
    void preparerFormulaireProduit(bool estModif, int idx = -1);
    void ouvrirDialogueClient(bool estModif);

    // ... (vos variables existantes) ...

    // AJOUTER CES LIGNES POUR LES STATS POP-UP :
    void ouvrirStatsProduits();
    void ouvrirStatsRH();
    void ouvrirStatsStock();
    void ouvrirStatsClients();
    void ouvrirStatsDepot();
    void ouvrirStatsPlanification();
    void preparerFormulaireModif(int idx);
    void ouvrirIAPrediction();
    // Dashboard global (Page d'Accueil)
    void construireDashboardAccueil();
    // Pages dynamiques modernisées
    void construirePageAccueil();
    void construirePageLogin();

    // --- Alertes personnalisées FIL D'OR ---
    void alerteSucces(const QString &titre, const QString &message);
    void alerteErreur(const QString &titre, const QString &message);
    void alerteWarning(const QString &titre, const QString &message);
    void alerteInfo(const QString &titre, const QString &message);

    // Une petite fonction utilitaire pour le design des cartes KPI
    QFrame* creerCarteStat(QString icone, QString val, QString titre, QString couleurFond);
};

#endif // MAINWINDOW_H
