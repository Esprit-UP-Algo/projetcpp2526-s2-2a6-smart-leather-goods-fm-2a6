#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ordrefabrication.h"
#include "matierepremiere.h"
#include "etape.h"
#include "depot.h"
#include "produit.h"
#include "client.h"
#include "arduino.h"
#include "serialmanager.h"
#include "classificationiawidget.h"
#include "assistant_voice.h"
#include "rfidreader/rfidreader.h"
#include "chatbotwidget.h"
#include "arduinowidget.h"
#include <QSqlQueryModel>
#include <QMainWindow>
#include <QTableWidget>
#include <QVector>
#include <QDate>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPrinter>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextToSpeech>
#include <QProcess>
#include <QTimer>
#include <QResizeEvent>

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
#include <QRadioButton>
#include <QGroupBox>
#include <QTextEdit>
#include <QHeaderView>
#include <QMap>
#include <QColor>
#include <QGraphicsView>
#include <functional>
#include <QtCharts/QChartView>
#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QPointer>
#include <QTextBrowser>
#include <QEvent>
#include <QTextToSpeech>

/// Résultat du dialog d'affectation multi-employés (partagé entre .h et .cpp)
struct AffectationEquipeResult {
    bool useManuel = false;
    bool enforceCompetences = false;
    QVector<int> equipeOrdre;
    QMap<QString, QVector<int>> manuelParEtape;
};

class QNetworkReply;
class QProcess;
class QTimer;
class AssistantVoiceController;

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

    struct FashionOracleConcept {
        QString productTypeEn;
        QString styleEn;
        QString materialEn;
        QString colorEn;
        QString targetAudienceEn;
        QString occasionEn;
        int targetYear = 2025;
        QString style;
        QString palette;
        QString material;
        int conceptIndex = 0;
    };

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

    void on_btn_test_buzzer_clicked();
    void onAssistantTtsStateChanged(QTextToSpeech::State state);

private:
    Arduino *arduino;
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
    // Affectation d'équipe configurée depuis le formulaire (avant sauvegarde)
    AffectationEquipeResult m_pendingAffectAdd;
    bool m_affectAddConfigured = false;
    AffectationEquipeResult m_pendingAffectModif;
    bool m_affectModifConfigured = false;
    bool modeModifProd = false; int indexModifProd = -1;
    bool modeModifEmp = false; int indexModifEmp = -1;
    bool modeModifStock = false; int indexModifStock = -1;
    bool modeModifClient = false; int indexModifClient = -1;
    bool modeModifDepot = false; int indexModifDepot = -1;

    /// Évite la réentrance (currentChanged + setCurrentIndex) pendant la reconstruction de l'onglet stats.
    bool m_statsProduitsEnCours = false;

    /// Dernière ligne sélectionnée dans la timeline fabrication (OF + produit + qté pour prévu IA).
    int m_fabTimelineIdPlanif = -1;
    QString m_fabTimelineProduit;
    int m_fabTimelineQte = 1;
    /// Rafraîchit le tableau stylé des étapes (onglet fabrication) après premier `construirePageEtapes()`.
    std::function<void()> m_refreshFabEtapesTable;
    // --- IA Estimation (régression linéaire) ---
    QVector<double> mlW; // w0=biais, w1..wn
    QChartView *m_iaChartView   = nullptr;
    QChartView *m_mlCurveView   = nullptr;
    QMap<QString, double> m_baseHUnit;
    /// Empêche de ré-entraîner le ML et de relire l'historique à chaque ouverture de l'onglet IA.
    qint64 m_mlTrainDataCount = -1;
    qint64 m_mlTrainMaxSuivi = -1;
    double m_mlTrainSumTempsReel = -1.0;
    /// Incrémenter lors d'un changement de features ML pour invalider le cache disque logique.
    int m_mlTrainFeatureRev = 0;
    // Modèles ML par étape (COUPE / ASSEMBLAGE / COUTURE / FINITION)
    QMap<QString, QVector<double>> m_mlWeightsPerEtape;
    QMap<QString, QVector<double>> m_mlNormMinPerEtape;
    QMap<QString, QVector<double>> m_mlNormMaxPerEtape;
    QMap<QString, double>          m_mlMaePerEtape;
    QMap<QString, double>          m_mlR2PerEtape;
    QMap<QString, double>          m_mlStdPerEtape;
    qint64 m_baseHCalibRowCount = -1;
    qint64 m_baseHCalibMaxSuivi = -1;
    double m_baseHCalibSumTempsRatio = -1.0;
    /// Contexte courant pour les boutons « what-if »
    struct IaSessionContext {
        int idCmd = -1;
        QString produitNom;
        double qte = 1.0;
        double comp = 1.0;
        double nbEmp = 1.0;
        double cadence = 0.85;
        QDate dateDebut;
        bool valid() const { return idCmd > 0; }
    } m_iaSession;
    struct StepPlan {
        QString etape;
        double hUnitaire = 0.0;
        double chargeHeures = 0.0;
        double joursOuvres = 0.0;
        QDate debut;
        QDate fin;
    };
    struct CapacityAdvice {
        double totalHeures = 0.0;
        double totalJours = 0.0;
        int employesMin = 1;
        double heuresJourMin = 8.0;
        QDate dateFinReco;
    };

    struct CutPiece {
        QString name;
        double wMm = 0.0;
        double hMm = 0.0;
        int qty = 1;
        bool canRotate = true;
        QColor color = QColor(80, 140, 220);
        QString preferredZone;
        int grainDir = 0;
        int qualityLevel = 1;
    };

    struct PlacedPiece {
        QString name;
        int sheetIndex = 0;
        double xMm = 0.0;
        double yMm = 0.0;
        double wMm = 0.0;
        double hMm = 0.0;
        bool rotated = false;
        QColor color;
        QString preferredZone;
        int grainDir = 0;
        int qualityLevel = 1;
    };

    struct CutPlanResult {
        QVector<PlacedPiece> placed;
        int sheetsUsed = 0;
        double sheetWMm = 0.0;
        double sheetHMm = 0.0;
        double spacingMm = 0.0;
        double usedAreaMm2 = 0.0;
        double totalAreaMm2 = 0.0;
        double wasteAreaMm2 = 0.0;
        double wastePct = 0.0;
        double wastePctHeuristic[3] = { -1.0, -1.0, -1.0 };
        int bestHeuristicIndex = 0;
        bool ok = false;
        QString message;
    };

    QTableWidget *m_tableEtapesFab = nullptr;
    QTableWidget *m_tableCutPlacements = nullptr;
    QGraphicsView *m_graphicsCutPreview = nullptr;
    QGraphicsScene *m_sceneCutPreview = nullptr;
    QLabel *m_lblCutSummary = nullptr;
    QDoubleSpinBox *m_spinCutSpacing = nullptr;
    QPushButton *m_btnCutSheetPrev = nullptr;
    QPushButton *m_btnCutSheetNext = nullptr;
    QLabel *m_lblCutSheetNav = nullptr;
    QLabel *m_lblCutApiStatus = nullptr;
    QPushButton *m_btnCutExportPdf = nullptr;
    int m_cutCurrentSheet = 1;
    CutPlanResult m_lastCutPlan;
    QString m_cutExportDir;
    QString m_cutExportPrefix;
    QNetworkAccessManager *m_cutApiManager = nullptr;

    // Fonctions Communes
    void exporterPDF(QTableWidget *table, QString titreDocument);
    void exporterCSV(QTableWidget *table, const QString &titreDocument);

    // Module Planif
    void rafraichirListeCommandes();
    void configurerTimelineGantt();
    void dessinerBarre(int ligne, int colDebut, int duree, QString texte, QColor bgCol, QColor textCol);
    void calculerEtAfficherStats();

    int selectedEtapeId;
    int selectedEtapePlanifId;

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
    /// Remplit un QTableWidget (9 col.) : prévu IA + réel + delta affiché vs prévu IA.
    void peuplerTableEtapesStandard(QTableWidget *tbl, QSqlQueryModel *model);
    void construirePageEtapes();
    void verifierFinFabrication(int idPlanification);

    // Module Produits
    void rafraichirListeProduits(const QString &filtreCollection = QString());
    void remplirCombosProduitClientEmplacement();
    void calculerStatsProduits();
    void showProdSimDialog();
    void showProduitCoutDialog();
    void showHistoriqueModeDialog();
    void showPlanifIaDialog();

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
    void showDepotRavitaillementTab();
    void showDepotRavitaillementMapTab();
    void showDepotValeurGazTab();
    void setupDepotExpertUI();

    // Module Stock
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

    // Stats Pop-up
    void ouvrirStatsProduits();
    void ouvrirStatsRH();
    void ouvrirStatsStock();
    void ouvrirStatsClients();
    void ouvrirStatsDepot();
    void ouvrirStatsPlanification();
    void preparerFormulaireModif(int idx);
    void ouvrirIAPrediction();

    bool trainLinearModelFromDB(bool forceRetrain = false);
    double predictStepHours(int etapeCode,
                            double qte,
                            double complexite,
                            double nbEmployes,
                            double cadence) const;
    void renderIaEstimationForSelectedOrder();
    void iaWhatIfPlusEmp();
    void iaWhatIfOvertime();
    void iaWhatIfReplan();
    void iaWhatIfAdminCal();
    int mapEtapeToCode(const QString& etape) const;
    double baseHoursPerUnit(const QString& etape) const;
    bool isWorkingDay(const QDate& d) const;
    QDate addWorkingDays(const QDate& start, int days) const;
    bool calibrateBaseHoursFromHistory();
    double hUnit(const QString& etape) const;
    double complexityFactor(double c) const;
    double cadenceFactor(double cadence) const;
    double qualityReworkFactor(double txRework) const;
    QVector<StepPlan> planRealisteToutesEtapes(double qte,
                                               double complexite,
                                               double nbEmp,
                                               double heuresJour,
                                               double rendement,
                                               double cadence,
                                               double txRework,
                                               const QDate& dateDebutCmd,
                                               int idCommande,
                                               const QVector<int>& equipeIds = {},
                                               const QHash<QString, int>& overrideEff = {}) const;
    CapacityAdvice computeCapacityAdvice(const QVector<StepPlan>& plans,
                                         const QDate& dateDebut,
                                         int nbEmpActuels,
                                         double heuresJourActuelles,
                                         double rendement,
                                         int objectifJoursOuvres) const;
    QString buildRecommendationHtml(const CapacityAdvice& adv,
                                    int nbEmpActuels,
                                    double heuresJourActuelles,
                                    int objectifJoursOuvres) const;
    int getObjectifJoursFromUi() const;
    QString buildIaHtmlCompact(const QString& cmd,
                               const QString& produit,
                               const QString& tableRows,
                               const QString& totalDebut,
                               const QString& totalFin,
                               double totalJours,
                               double totalHeures,
                               const QString& recommendationHtml,
                               int objectifJours,
                               double ecartJours) const;
    void renderStepPlanHtmlAndChart(const QVector<StepPlan>& plans,
                                    const QString& commandeLabel,
                                    const QString& produitLabel);
    void drawMlRegressionChart();
    void drawIaChart(const QVector<StepPlan>& plans,
                     double totalJours);
    void scheduleIaChartRedraw(const QVector<StepPlan> &plans, double totalJours);

    // --- IA Découpe / nesting matière ---
    CutPlanResult planCutPieces(const QVector<CutPiece>& pieces,
                                double sheetWMm,
                                double sheetHMm,
                                double spacingMm);
    bool exportCutPlanPng(const CutPlanResult& plan,
                          const QString& outputDir,
                          const QString& prefix,
                          int imgW = 3200,
                          int imgH = 2200);
    bool loadCutInputsFromDb(int idProduit,
                             int idMatiere,
                             QVector<CutPiece>& pieces,
                             double& supportWmm,
                             double& supportHmm,
                             const QString& etapeActuelle,
                             QString& err,
                             QString* demoHintOut = nullptr);
    void filtrerPiecesParEtape(QVector<CutPiece>& pieces,
                               const QString& etape,
                               const QString& produitDesignation,
                               QString* hintOut = nullptr) const;
    bool saveCutPlanToDb(int idCmd,
                         int idProduit,
                         int idMatiere,
                         const CutPlanResult& res,
                         int& outPlanId,
                         QString& err);
    void fillCutPlacementTable(const CutPlanResult& res, int sheetOneBasedFilter = 0);
    void showCutPreviewImage(const QString& imagePath);
    void refreshCutSheetNavigation();
    void fillDefaultCutPiecesDemo(QVector<CutPiece>& pieces) const;
    void fillSmartCutPieces(QVector<CutPiece>& pieces, const QString& designation) const;
    static QString detectProductTypeLabel(const QString& designation);
    static QColor colorForPieceType(const QString& pieceName);
    CutPlanResult guillotinePlaceUnits(QVector<CutPiece> unitPieces,
                                      double sheetWMm,
                                      double sheetHMm,
                                      double spacingMm) const;
    bool resolveIdsFromCommande(int idCommande, int& idProduit, int& idMatiere, QString& err);
    void runCutPlanForCurrentSelection();
    void renderCutPlanToScene(int sheetOneBased = 1);
    void checkCutApiHealth();
    static double tryLoadMatiereCoutM2(int idStockMp);
    bool tryRunCutPlanViaApi(int idCmd,
                             int idProduit,
                             int idMatiere,
                             const QVector<CutPiece>& pieces,
                             double supportW,
                             double supportH,
                             double spacing,
                             CutPlanResult& outRes,
                             QString& outInfo,
                             QString& outErr);

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

    // --- Assistant RH IA (OpenRouter) ---
    QNetworkAccessManager *m_namOpenRouter = nullptr;
    QNetworkReply *m_classificationReply = nullptr;
    QNetworkReply *m_assistantChatReply = nullptr;
    QJsonArray m_assistantChatMessages;
    QString m_assistantSchemaMetaOracle;

    QString construireResumeSchemaOracleMeta() const;
    QString construirePromptSystemeAssistant(const QString &schemaMeta) const;
    QString contexteDonneesEmployesPourQuestion(const QString &question) const;
    QString contextePlanificationsPourQuestion(const QString &question) const;
    QString contexteClientsPourQuestion(const QString &question) const;
    QString contexteProduitsPourQuestion(const QString &question) const;
    QString contexteMatieresPourQuestion(const QString &question) const;
    QString contexteEtapesPourQuestion(const QString &question) const;
    QString contexteDonneesMetierPourQuestion(const QString &question) const;

    QString m_openRouterKeyImportee;
    void rafraichirCleOpenRouterDisque();

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
    void programmerReecouteMicroAssistantSiBesoin();

    // --- Arduino / Serial Manager ---
    SerialManager m_serialManager;
    int m_pendingMoteurProductId = -1;
    int m_pendingMoteurCommande = -1;
    QString m_pendingMoteurCmd;
    QString m_pendingMoteurPort;
    int m_pendingMoteurDbChoix = -1;
    qint64 m_pendingMoteurStartedMs = 0;
    QTimer *m_arduinoKeepalive = nullptr;
    qint64 m_gazLastInsertMs = 0;
    QRadioButton *m_rbChoixMod0 = nullptr;
    QRadioButton *m_rbChoixMod1 = nullptr;
    QRadioButton *m_rbChoixMod2 = nullptr;
    QRadioButton *m_rbChoixNew0 = nullptr;
    QRadioButton *m_rbChoixNew1 = nullptr;
    QRadioButton *m_rbChoixNew2 = nullptr;
    ArduinoWidget *m_arduinoWidget = nullptr;

    void installerChoixMoteurProduitUi();
    int choixMoteurDepuisFormulaireNouveau() const;
    int choixMoteurDepuisFormulaireModif() const;
    void journaliserMoteurSmart(int idProduit, int idCommande, const QString &actionCode, const QString &detail) const;
    int declencherMoteurProduit(int idProduit, int idCommande);
    QString getArduinoPort() const;
    void traiterReponseArduino(const QString &reponse, int idProduit, int dbChoix, int idCommande, const QString &cmd, const QString &port, int dureeMs);
    void autoDetectArduino();
    void showArduinoSmartTab();

    // --- RFID ---
    RFIDReader *m_rfidReader = nullptr;
    QString m_currentRFIDPort;
    bool m_rfidAutoFillEnabled = false;

    void initialiserRFID();
    void initialiserPageRFID();
    void ouvrirPointageRFIDTab();
    void actualiserStatutLecteurRFID();
    void afficherResultatScanSurPageRFID(const QString &uid, bool employeConnu, const EmployeInfo &info);
    void onBadgeScanned(const QString &uid);
    void onRFIDConnectionChanged(bool connected);
    void appendRfidJournal(const QString &message);

    // --- Chatbot / Fashion Oracle ---
    QNetworkAccessManager m_namCostSim;
    QNetworkReply *m_costSimReply = nullptr;
    QString m_costSimHtmlOut;
    QNetworkReply *m_histCapsuleReply = nullptr;
    QPushButton *m_chatBubbleButton = nullptr;
    ClassificationIAWidget *m_classificationIaWidget = nullptr;
    ChatbotWidget *m_chatbotWidget = nullptr;
    QProcess *m_fashionOracleBackendProcess = nullptr;
    bool m_fashionOracleBackendOwned = false;

    QString resolveFashionOracleDir() const;
    QString resolveFashionOraclePython() const;
    bool isFashionOracleHealthy(int timeoutMs = 3000) const;
    bool startFashionOracleBackendProcess(QString *errorOut = nullptr);
    bool ensureFashionOracleBackendReady(QString *errorOut = nullptr, int startupTimeoutMs = 15000);
    QJsonObject buildGenerateVisualsPostJson(const FashionOracleConcept &concept);
    QByteArray jsonPayloadForFashionOracleGenerateVisuals(const FashionOracleConcept &concept);
    QNetworkReply* sendFashionOracleGenerateVisualRequest(QNetworkAccessManager *nam, const FashionOracleConcept &concept, int timeoutMs = 60000, QByteArray *payloadOut = nullptr);
    QString buildPromptForConcept(const FashionOracleConcept &concept);
    void ouvrirChatbotFilDor();
    void positionnerBulleChat();

    // --- Stock Compare Tab ---
    void showStockCompareTab();

    // --- Resize Event ---
    void resizeEvent(QResizeEvent *event) override;
};

#endif // MAINWINDOW_H
