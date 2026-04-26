#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ordrefabrication.h"
#include "matierepremiere.h"
#include "etape.h"
#include "depot.h"
#include "produit.h"
#include "client.h"
#include <QSqlQueryModel>
#include <QMainWindow>
#include <QTableWidget>
#include <QVector>
#include <QDate>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPrinter>
#include <QFileDialog>
#include <QProcess>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QHeaderView>
#include <QPointer>

#include "smartmotorcontroller.h"

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

    struct FashionOracleConcept {
        int conceptIndex = 1;
        QString productTypeEn;
        QString categoryLabel;
        QString style;
        QString palette;
        QString material;
        int targetYear = 2026;
    };
    using FashionConcept = FashionOracleConcept;

    static QJsonObject buildGenerateVisualsPostJson(const FashionOracleConcept &concept);
    static QByteArray jsonPayloadForFashionOracleGenerateVisuals(const FashionOracleConcept &concept);
    /**
     * Requete asynchrone vers /generate-visuals (GET query).
     * Beaucoup d instances locales n enregistrent que GET sur cette route : un POST provoque HTTP 405.
     * Le corps logique (type, style, …) est encode en query string ; le backend POST reste disponible pour d autres clients.
     */
    static QNetworkReply *sendFashionOracleGenerateVisualRequest(
        QNetworkAccessManager *nam,
        const FashionOracleConcept &concept,
        int transferTimeoutMs,
        QByteArray *outSentJson = nullptr);

    static QString buildPromptForConcept(const FashionOracleConcept &concept);

    QFrame *creerCarteStat(QString icone, QString val, QString titre, QString couleurFond);

private slots:
    void on_btn_valider_emp_clicked();
    void on_btn_valider_modif_emp_clicked();
    void on_btn_delete_emp_clicked();
    void on_btn_edit_emp_clicked();
    void on_btn_sort_alpha_emp_clicked();

    void on_btn_valider_produit_clicked();
    void on_btn_valider_modif_produit_clicked();
    void on_btn_delete_produit_clicked();
    void on_btn_edit_produit_clicked();
    void on_tableProduits_cellClicked(int row, int column);

    void on_btn_valider_stock_clicked();

    void on_btn_valider_client_clicked();
    void on_btn_valider_modif_client_clicked();

private:
    Ui::MainWindow *ui;
    OrdreFabrication tmpOrdre;
    MatierePremiere tmpMatiere;
    Produit tmpProduit;
    Depot tmpDepot;
    Etape tmpEtape;


    QVector<CommandeInfo> mesCommandes;
    QVector<ProduitInfo> mesProduits;
    QVector<EmployeInfo> mesEmployes;
    QVector<MatiereInfo> mesMatieres;
    QVector<ClientInfo> mesClients;
    QVector<DepotInfo> mesDepots;

    ColorDelegate *myColorDelegate;

    int selectedProdId = -1;
    int rowToEdit = -1;
    bool m_triProduitDesignationDescendant = false;

    int indexCommandeSelectionnee = -1;
    bool modeModification = false; int indexModification = -1;
    bool modeModifProd = false; int indexModifProd = -1;
    bool modeModifEmp = false; int indexModifEmp = -1;
    bool modeModifStock = false; int indexModifStock = -1;
    bool modeModifClient = false; int indexModifClient = -1;
    bool modeModifDepot = false; int indexModifDepot = -1;

    void exporterPDF(QTableWidget *table, QString titreDocument);
    void exporterCSV(QTableWidget *table, const QString &titreDocument);

    void rafraichirListeCommandes();
    void configurerTimelineGantt();
    void dessinerBarre(int ligne, int colDebut, int duree, QString texte, QColor bgCol, QColor textCol);
    void calculerEtAfficherStats();

    int selectedEtapeId;
    int selectedEtapePlanifId;

    int idEmployeAModifier = -1;

    QString initialNomEmploye;
    QString initialPrenomEmploye;
    QString initialPosteEmploye;
    QString initialEmailEmploye;
    QString initialTelephoneDigitsEmploye;
    QString initialDepartementEmploye;
    QDate initialDateEmbaucheEmploye;
    double initialSalaireEmploye = 0.0;
    QString initialRfidEmploye;

    Qt::SortOrder employeTriAlphaOrdre = Qt::AscendingOrder;
    bool employeTriAlphaActif = false;


    void rafraichirListeEtapes();
    void remplirTableEtapes(QSqlQueryModel *model);
    void construirePageEtapes();
    void verifierFinFabrication(int idPlanification);
    void rafraichirListeProduits(const QString &filtreCollection = QString());
    void remplirCombosProduitClientEmplacement();
    void calculerStatsProduits();
    void showProdSimDialog();
    void showProduitCoutDialog();
    void showHistoriqueModeDialog();
    void showPlanifIaDialog();

    void rafraichirListeEmployes();
    void calculerStatsRH();
    void reponseChatbot();
    void preparerFormulaireEmploye(bool estModif, int idx = -1);
    void showEmpEvalTab();
    void showEmpAncienneteTab();
    void showEmpAssistantTab();

    void goToTabEmployes(int index);
    void goToTabEmployesByText(const QString& title);
    void forceTabEmployes(int index);
    bool chargerEmployePourModification(int id);

    void rafraichirListeClients();
    void remplirTableClients(QSqlQueryModel *model);
    void calculerStatsClients();
    void exporterFactureClient();
    void showClientFideliteTab();
    void showClientIaTab();

    // Smart output (Atelier) : moteur + journalisation (si table Oracle presente)
    void installerChoixMoteurProduitUi();
    int choixMoteurDepuisFormulaireNouveau() const;
    int choixMoteurDepuisFormulaireModif() const;
    void journaliserMoteurSmart(int idProduit, int idCommande, const QString &actionCode, const QString &detail) const;
    int declencherMoteurProduit(int idProduit, int idCommande = -1);


    void rafraichirListeDepots();
    void calculerStatsDepots();
    void preparerFormulaireDepot(bool estModif, int idx = -1);
    void showDepotOptimizeTab();
    void showDepotRavitaillementTab();

    bool validerMatiereAjout();
    void rafraichirListeMatieres();
    void calculerStatsStock();
    void preparerFormulaireStock(bool estModif, int idx = -1);
    void showStockCompareTab();
    void showStockCalculTab();
    void preparerFormulairePlanif(bool estModification);
    void preparerFormulaireProduit(bool estModif, int idx = -1);
    void ouvrirDialogueClient(bool estModif);


    void ouvrirStatsProduits();
    void ouvrirStatsRH();
    void ouvrirStatsStock();
    void ouvrirStatsClients();
    void ouvrirStatsDepot();
    void ouvrirStatsPlanification();
    void preparerFormulaireModif(int idx);
    void ouvrirIAPrediction();
    void construireDashboardAccueil();
    void construirePageAccueil();
    void construirePageLogin();

    void alerteSucces(const QString &titre, const QString &message);
    void alerteErreur(const QString &titre, const QString &message);
    void alerteWarning(const QString &titre, const QString &message);
    void alerteInfo(const QString &titre, const QString &message);

    bool isFashionOracleHealthy(int timeoutMs = 1200) const;
    bool startFashionOracleBackendProcess(QString *errorOut = nullptr);
    bool ensureFashionOracleBackendReady(QString *errorOut = nullptr, int startupTimeoutMs = 30000);
    QString resolveFashionOracleDir() const;
    QString resolveFashionOraclePython() const;

    QProcess *m_fashionOracleBackendProcess = nullptr;
    bool m_fashionOracleBackendOwned = false;

    QNetworkAccessManager m_namCostSim;
    QPointer<QNetworkReply> m_costSimReply;
    QPointer<QTextEdit> m_costSimHtmlOut;
    QPointer<QNetworkReply> m_histCapsuleReply;

    SmartMotorController m_smartMotor;
    QRadioButton *m_rbChoixNew0 = nullptr;
    QRadioButton *m_rbChoixNew1 = nullptr;
    QRadioButton *m_rbChoixNew2 = nullptr;
    QRadioButton *m_rbChoixMod0 = nullptr;
    QRadioButton *m_rbChoixMod1 = nullptr;
    QRadioButton *m_rbChoixMod2 = nullptr;
};

#endif
