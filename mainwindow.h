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
#include <QJsonArray>
#include <QDialog>
#include <QPointer>
#include <QTextBrowser>
#include <QEvent>
#include <QTextToSpeech>

class QNetworkAccessManager;
class QNetworkReply;
class QProcess;
class QTimer;
class AssistantVoiceController;

#include "arduino.h"

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
struct DepotInfo { QString id; QString emplacement; QString etagere; double capaciteMax; double quantiteActuelle; double valeurGaz; QString typeStockage; };
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // === Slots CRUD Employés ===
    void on_btn_valider_emp_clicked();
    void on_btn_valider_modif_emp_clicked();
    void on_btn_delete_emp_clicked();
    void on_btn_edit_emp_clicked();
    void on_btn_sort_alpha_emp_clicked();

    // === Slots CRUD Produits ===
    void on_btn_valider_produit_clicked();
    void on_btn_valider_modif_produit_clicked();
    void on_btn_delete_produit_clicked();
    void on_btn_edit_produit_clicked();
    void on_tableProduits_cellClicked(int row, int column);

    // === Stock — matières premières (auto-connexion setupUi) ===
    void on_btn_valider_stock_clicked();

    // === Slots CRUD Clients ===
    void on_btn_valider_client_clicked();
    void on_btn_valider_modif_client_clicked();

    void onAssistantTtsStateChanged(QTextToSpeech::State state);

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

    // Sélection produit (utile même après tri de la table)
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

    // Fonctions Communes
    void exporterPDF(QTableWidget *table, QString titreDocument);
    void exporterCSV(QTableWidget *table, const QString &titreDocument);

    // Module Planif
    void rafraichirListeCommandes();
    void configurerTimelineGantt();
    void dessinerBarre(int ligne, int colDebut, int duree, QString texte, QColor bgCol, QColor textCol);
    void calculerEtAfficherStats();

    int selectedEtapeId;       // ID_SUIVI sélectionné
    int selectedEtapePlanifId; // ID_PLANIFICATION sélectionné

    // Utilisé pour la modification d'un employé (lecture depuis Oracle).
    int idEmployeAModifier = -1;

    // Snapshot des valeurs initiales (pour détecter "aucune modification").
    QString initialNomEmploye;
    QString initialPrenomEmploye;
    QString initialPosteEmploye;
    QString initialEmailEmploye;
    QString initialTelephoneDigitsEmploye;
    QString initialDepartementEmploye;
    QDate initialDateEmbaucheEmploye;
    double initialSalaireEmploye = 0.0;
    QString initialRfidEmploye;

    // Tri (Employes - colonne Nom)
    Qt::SortOrder employeTriAlphaOrdre = Qt::AscendingOrder;
    bool employeTriAlphaActif = false;


    void rafraichirListeEtapes();
    void remplirTableEtapes(QSqlQueryModel *model);
    void construirePageEtapes();
    void verifierFinFabrication(int idPlanification);
    // Module Produits
    void rafraichirListeProduits(const QString &filtreCollection = QString());
    void remplirCombosProduitClientEmplacement();
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

    // Helpers navigation / chargement fiche employé
    void goToTabEmployes(int index);
    void goToTabEmployesByText(const QString& title);
    void forceTabEmployes(int index);
    bool chargerEmployePourModification(int id);

    // CRUD Clients
    void rafraichirListeClients();
    void remplirTableClients(QSqlQueryModel *model);
    void calculerStatsClients();
    void exporterFactureClient();
    void showClientFideliteTab();
    void showClientIaTab();


    // Module Dépôt
    void rafraichirListeDepots();
    void calculerStatsDepots();
    void preparerFormulaireDepot(bool estModif, int idx = -1);
    void showDepotOptimizeTab();
    void setupDepotExpertUI();
    void showDepotRavitaillementTab();
    void showDepotRavitaillementMapTab();
    void showDepotValeurGazTab();

    // Module Stock (NOUVEAU - SPA avec onglets)
    bool validerMatiereAjout();
    void rafraichirListeMatieres();
    void calculerStatsStock();
    void preparerFormulaireStock(bool estModif, int idx = -1);
    void showStockRavitaillementTab();
    void showStockCalculTab();
    void setupStockExpertUI();
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

    QNetworkAccessManager *m_namOpenRouter = nullptr;
    QNetworkReply *m_classificationReply = nullptr;
    QNetworkReply *m_assistantChatReply = nullptr;
    /// Historique chat Assistant RH (messages OpenRouter : system + user + assistant).
    QJsonArray m_assistantChatMessages;
    /// Dernière introspection schéma Oracle (tables/colonnes, sans données).
    QString m_assistantSchemaMetaOracle;

    QString construireResumeSchemaOracleMeta() const;
    QString construirePromptSystemeAssistant(const QString &schemaMeta) const;
    /// Lecture sécurisée (extraits Oracle) pour enrichir la question envoyée au modèle.
    QString contexteDonneesEmployesPourQuestion(const QString &question) const;
    QString contextePlanificationsPourQuestion(const QString &question) const;
    QString contexteClientsPourQuestion(const QString &question) const;
    QString contexteProduitsPourQuestion(const QString &question) const;
    QString contexteMatieresPourQuestion(const QString &question) const;
    QString contexteEtapesPourQuestion(const QString &question) const;
    QString contexteDonneesMetierPourQuestion(const QString &question) const;
    /// Clé OpenRouter (variable d’environnement ou openrouter_key.txt), rechargée au démarrage et à l’onglet IA.
    QString m_openRouterKeyImportee;

    void rafraichirCleOpenRouterDisque();

    /// Bouton flottant (robot) sur la page RH — ouvre le chat.
    QPushButton *m_fabAssistant = nullptr;
    QDialog *m_dialogAssistant = nullptr;
    QPointer<QTextBrowser> m_ptrAssistantChatView;
    QPointer<QLineEdit> m_ptrAssistantInput;
    bool m_assistantUiConstruit = false;
    bool m_assistantVoixActive = false;
    QTextToSpeech *m_ttsAssistant = nullptr;
    QProcess *m_ttsSpeakProcess = nullptr;
    AssistantVoiceController *m_assistantVoice = nullptr;

    void installerBulleAssistantFlottant();
    void positionnerBulleAssistant();
    void ouvrirFenetreAssistantRh();
    void construireInterfaceAssistantSiBesoin();
    void rafraichirAssistantVueChat();
    void parlerTexteAssistant(const QString &texte);
    /// Relance l’écoute après la réponse (après la fin du TTS si activé) — half-duplex.
    void programmerReecouteMicroAssistantSiBesoin();
};

#endif // MAINWINDOW_H
