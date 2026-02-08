#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QVector>
#include <QDate>
#include <QStyledItemDelegate>
#include <QPainter>

// --- BIBLIOTHÈQUES POUR LE PDF ---
#include <QPrinter>
#include <QFileDialog>
// ---------------------------------

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// --- DELEGATE POUR LES COULEURS (Gantt) ---
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

// --- STRUCTURES DE DONNÉES ---

// 1. Module Produits
struct ProduitInfo {
    QString ref;
    QString nom;
    double coutMatiere;
    QString collection;
    QString cuir;
    int temps;
};

// 2. Module Planification
struct CommandeInfo {
    QString id;
    QString produit;
    int quantite;
    QString matiere;
    QDate dateDebut;
    QString dateFinEstimee;
    QString statut;
    QString etapeAuditee;
    int etatEtape; // 0=Neutre, 1=OK, 2=Retard
};

// 3. Module Employés (RH)
struct EmployeInfo {
    QString id;
    QString nom;
    QString prenom;
    QString poste;
    QString departement;
    QDate dateEmbauche;
    double salaire;
    QString rfid;
};

// --- CLASSE PRINCIPALE ---

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // --- VECTEURS DE STOCKAGE ---
    QVector<CommandeInfo> mesCommandes;
    QVector<ProduitInfo> mesProduits;
    QVector<EmployeInfo> mesEmployes;

    // --- OUTILS ---
    ColorDelegate *myColorDelegate;

    // --- VARIABLES D'ÉTATS (Sélection & Modification) ---

    // Pour Planification
    int indexCommandeSelectionnee = -1;
    bool modeModification = false;
    int indexModification = -1;

    // Pour Produits
    bool modeModifProd = false;
    int indexModifProd = -1;

    // Pour Employés
    bool modeModifEmp = false;
    int indexModifEmp = -1;

    // --- FONCTIONS MÉTIERS ---

    // 1. Fonctions Planification & Fabrication
    void rafraichirListeCommandes();
    void configurerTimelineGantt();
    void dessinerBarre(int ligne, int colDebut, int duree, QString texte, QColor bgCol, QColor textCol);
    void calculerEtAfficherStats();

    // 2. Fonctions Produits
    void rafraichirListeProduits();
    void calculerStatsProduits();

    // 3. Fonctions Employés (RH)
    void rafraichirListeEmployes();
    void reponseChatbot();

    // 4. Fonction Générique PDF
    void exporterPDF(QTableWidget *table, QString titreDocument);
};

#endif // MAINWINDOW_H
