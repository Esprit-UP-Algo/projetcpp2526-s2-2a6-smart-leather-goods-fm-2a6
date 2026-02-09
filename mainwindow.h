#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QVector<CommandeInfo> mesCommandes;
    QVector<ProduitInfo> mesProduits;
    QVector<EmployeInfo> mesEmployes;
    QVector<MatiereInfo> mesMatieres;

    ColorDelegate *myColorDelegate;

    int indexCommandeSelectionnee = -1;
    bool modeModification = false; int indexModification = -1;
    bool modeModifProd = false; int indexModifProd = -1;
    bool modeModifEmp = false; int indexModifEmp = -1;
    bool modeModifStock = false; int indexModifStock = -1;

    // Fonctions Communes
    void exporterPDF(QTableWidget *table, QString titreDocument);

    // Module Planif
    void rafraichirListeCommandes();
    void configurerTimelineGantt();
    void dessinerBarre(int ligne, int colDebut, int duree, QString texte, QColor bgCol, QColor textCol);
    void calculerEtAfficherStats();

    // Module Produits
    void rafraichirListeProduits();
    void calculerStatsProduits();
    void showProdSimDialog(); // [NOUVEAU]
    void showPlanifIaDialog(); // [NOUVEAU] Pop-up IA Planification

    // Module RH
    void rafraichirListeEmployes();
    void calculerStatsRH();
    void reponseChatbot();
    void showEmpEvalDialog(); // [NOUVEAU]

    // Module Stock (EXISTANT)
    void rafraichirListeMatieres();
    void calculerStatsStock();
    void showCompareDialog();
    void showBesoinDialog();
};

#endif // MAINWINDOW_H
