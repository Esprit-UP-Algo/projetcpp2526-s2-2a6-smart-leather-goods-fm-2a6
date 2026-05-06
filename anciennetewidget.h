#ifndef ANCIENNETEWIDGET_H
#define ANCIENNETEWIDGET_H

#include <QDate>
#include <QWidget>

namespace Ui {
class AncienneteWidget;
}

/** Données affichées (évite une dépendance à MainWindow::EmployeInfo). */
struct AncienneteEmployeData {
    QString nom;
    QString prenom;
    QDate dateEmbauche;
    bool empty = true;
};

/**
 * Carte « Ancienneté & primes » (max 600×560, sans défilement vertical).
 */
class AncienneteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AncienneteWidget(QWidget *parent = nullptr);
    ~AncienneteWidget() override;

    void setEmployeData(const AncienneteEmployeData &data);
    void setEmptyState();

private:
    void rebuildContent();

    Ui::AncienneteWidget *ui;
    AncienneteEmployeData m_data;
};

#endif
