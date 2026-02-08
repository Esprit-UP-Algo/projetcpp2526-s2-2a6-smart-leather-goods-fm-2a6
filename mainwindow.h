#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QVector>
#include <QDate>
#include <QStyledItemDelegate>
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Delegate
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

struct ProduitInfo {
    QString ref;
    QString nom;
    double coutMatiere;
    QString collection;
    QString cuir;
    int temps;
};

struct CommandeInfo {
    QString id;
    QString produit;
    int quantite;
    QString matiere;
    QDate dateDebut;
    QString dateFinEstimee;
    QString statut;
    QString etapeAuditee;
    int etatEtape;
};

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

    ColorDelegate *myColorDelegate;

    int indexCommandeSelectionnee = -1;
    bool modeModification = false;
    int indexModification = -1;
    bool modeModifProd = false;
    int indexModifProd = -1;

    void rafraichirListeCommandes();
    void configurerTimelineGantt();
    void dessinerBarre(int ligne, int colDebut, int duree, QString texte, QColor bgCol, QColor textCol);
    void calculerEtAfficherStats();

    // NOUVELLES FONCTIONS PRODUITS
    void rafraichirListeProduits();
    void calculerStatsProduits(); // <--- ICI
};

#endif // MAINWINDOW_H
