#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "client.h"
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QDebug>
#include <QInputDialog>
#include <QTableWidgetItem>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QPainter>
#include <QFileDialog>
#include <QtCharts>
#include <QChartView>
#include <QPieSeries>
#include <QChart>
#include <QPdfWriter>
#include <QPainter>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Configuration de la table
    ui->tableClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableClients->setSelectionMode(QAbstractItemView::SingleSelection);

    // Afficher les clients au démarrage
    refreshTableView();

    // Désactiver les champs d'édition au départ
    enableInputFields(false);

    // Rendre l'ID client en lecture seule (auto-incrémenté)
    ui->le_id_client->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ============= ACTIVER/DÉSACTIVER LES CHAMPS =============
void MainWindow::enableInputFields(bool enable)
{
    ui->le_cin_client->setEnabled(enable);
    ui->le_nom_client->setEnabled(enable);
    ui->le_tel_client->setEnabled(enable);
    ui->le_email_client->setEnabled(enable);
    ui->le_adresse_client->setEnabled(enable);
}

// ============= DÉFINIR LE MODE (AJOUT OU MODIFICATION) =============
void MainWindow::setFormMode(bool isNewMode)
{
    if (isNewMode) {
        // Mode AJOUT : champs vidés et activés
        clearInputFields();
        enableInputFields(true);
        ui->le_id_client->clear();
        ui->le_id_client->setText("(Nouveau - Auto)");
        ui->le_cin_client->setFocus();
    } else {
        // Mode MODIFICATION : champs activés
        enableInputFields(true);
    }
}

// ============= RAFFRAÎCHIR L'AFFICHAGE =============
void MainWindow::refreshTableView()
{
    QSqlQueryModel* model = clientTmp.afficher();

    if (!model) return;

    // Vider la table actuelle
    ui->tableClients->setRowCount(0);

    // Remplir la table avec les données
    int rows = model->rowCount();
    int cols = model->columnCount();  // Maintenant cols = 7

    ui->tableClients->setRowCount(rows);
    ui->tableClients->setColumnCount(cols);

    // Définir les en-têtes (7 colonnes maintenant)
    QStringList headers;
    headers << "ID" << "CIN / MF" << "Nom Client" << "Téléphone" << "Email" << "Adresse" << "Points";
    ui->tableClients->setHorizontalHeaderLabels(headers);

    // Remplir les données
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            QModelIndex index = model->index(i, j);
            QString data = model->data(index).toString();
            ui->tableClients->setItem(i, j, new QTableWidgetItem(data));
        }
    }

    // Ajuster la largeur des colonnes
    ui->tableClients->setColumnWidth(0, 60);
    ui->tableClients->setColumnWidth(1, 120);
    ui->tableClients->setColumnWidth(2, 150);
    ui->tableClients->setColumnWidth(3, 100);
    ui->tableClients->setColumnWidth(4, 180);
    ui->tableClients->setColumnWidth(5, 200);
    ui->tableClients->setColumnWidth(6, 80);  // Largeur pour la colonne Points
}

// ============= EFFACER LES CHAMPS DE SAISIE =============
void MainWindow::clearInputFields()
{
    ui->le_cin_client->clear();
    ui->le_nom_client->clear();
    ui->le_tel_client->clear();
    ui->le_email_client->clear();
    ui->le_adresse_client->clear();
}

// ============= VALIDER LES CHAMPS SAISIS =============
bool MainWindow::validateInputs()
{
    // Validation du CIN
    if (ui->le_cin_client->text().isEmpty()) {
        QMessageBox::warning(this, "Champ vide", "Le CIN / Matricule fiscal est obligatoire !");
        ui->le_cin_client->setFocus();
        return false;
    }

    // Validation du nom
    if (ui->le_nom_client->text().isEmpty()) {
        QMessageBox::warning(this, "Champ vide", "Le nom est obligatoire !");
        ui->le_nom_client->setFocus();
        return false;
    }

    // Validation de l'email
    QString email = ui->le_email_client->text();
    if (!email.isEmpty() && (!email.contains('@') || !email.contains('.'))) {
        QMessageBox::warning(this, "Email invalide",
                             "Veuillez saisir un email valide (ex: nom@domaine.com)");
        ui->le_email_client->setFocus();
        return false;
    }

    // Validation du téléphone (optionnel)
    QString tel = ui->le_tel_client->text();
    if (!tel.isEmpty()) {
        bool validTel = true;
        for (QChar c : tel) {
            if (!c.isDigit() && c != '+' && c != ' ' && c != '-' && c != '.') {
                validTel = false;
                break;
            }
        }
        if (!validTel) {
            QMessageBox::warning(this, "Téléphone invalide",
                                 "Le numéro de téléphone ne doit contenir que des chiffres, +, -, . ou espaces");
            ui->le_tel_client->setFocus();
            return false;
        }
    }

    return true;
}

// ============= CHARGER UN CLIENT DANS LE FORMULAIRE =============
void MainWindow::loadClientToForm(int row)
{
    // Vérifier que row est valide et que les items existent
    if (row >= 0 && row < ui->tableClients->rowCount()) {
        QTableWidgetItem* itemId = ui->tableClients->item(row, 0);
        QTableWidgetItem* itemCin = ui->tableClients->item(row, 1);
        QTableWidgetItem* itemNom = ui->tableClients->item(row, 2);
        QTableWidgetItem* itemTel = ui->tableClients->item(row, 3);
        QTableWidgetItem* itemEmail = ui->tableClients->item(row, 4);
        QTableWidgetItem* itemAdresse = ui->tableClients->item(row, 5);

        if (itemId) ui->le_id_client->setText(itemId->text());
        if (itemCin) ui->le_cin_client->setText(itemCin->text());
        if (itemNom) ui->le_nom_client->setText(itemNom->text());
        if (itemTel) ui->le_tel_client->setText(itemTel->text());
        if (itemEmail) ui->le_email_client->setText(itemEmail->text());
        if (itemAdresse) ui->le_adresse_client->setText(itemAdresse->text());

        // Passer en mode modification
        enableInputFields(true);

        // Mettre à jour le statut et la prévision IA
        updateClientStatus();
        updateIAPrediction();
    }
}

// ============= METTRE À JOUR LE STATUT DE FIDÉLITÉ =============
void MainWindow::updateClientStatus()
{
    // Exemple de logique de statut basée sur le nombre d'achats (à adapter)
    QString status = "STATUT : SILVER 🥈";
    if (ui->lbl_statut_client) {
        ui->lbl_statut_client->setText(status);
    }
}

// ============= METTRE À JOUR LA PRÉVISION IA =============
void MainWindow::updateIAPrediction()
{
    // Exemple de prévision (à adapter avec vos données réelles)
    QString prediction = "Potentiel : ÉLEVÉ (85%)";
    if (ui->lbl_ia_prevision) {
        ui->lbl_ia_prevision->setText(prediction);
    }
}

// ============= AJOUTER UN CLIENT =============

// ============= MODIFIER UN CLIENT =============
void MainWindow::on_btn_modifier_clicked()
{
    if (ui->le_id_client->text().isEmpty() || ui->le_id_client->text() == "(Nouveau - Auto)") {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un client à modifier !");
        return;
    }

    if (!validateInputs()) return;

    int id = ui->le_id_client->text().toInt();
    QString cin = ui->le_cin_client->text().trimmed();
    QString nom = ui->le_nom_client->text().trimmed();
    QString tel = ui->le_tel_client->text().trimmed();
    QString email = ui->le_email_client->text().trimmed();
    QString adresse = ui->le_adresse_client->text().trimmed();

  Client client(id, cin, nom, tel, email, adresse, 0);

    if (client.modifier(id)) {
        QMessageBox::information(this, "Succès", "Client modifié avec succès !");
        refreshTableView();
        clearInputFields();
        enableInputFields(false);
        ui->le_id_client->clear();
        ui->le_id_client->setText("(Nouveau - Auto)");

        // Mettre à jour le statut
        updateClientStatus();
        updateIAPrediction();
    } else {
        QMessageBox::critical(this, "Erreur",
                              "Échec de la modification.\nVérifiez que l'ID existe.");
    }
}

// ============= SUPPRIMER UN CLIENT =============
void MainWindow::on_btn_supprimer_clicked()
{
    // Récupérer l'ID du client sélectionné dans la table
    int currentRow = ui->tableClients->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un client à supprimer !");
        return;
    }

    QTableWidgetItem* itemId = ui->tableClients->item(currentRow, 0);
    QTableWidgetItem* itemNom = ui->tableClients->item(currentRow, 2);

    if (!itemId || !itemNom) {
        QMessageBox::warning(this, "Erreur", "Impossible de récupérer les informations du client !");
        return;
    }

    int id = itemId->text().toInt();
    QString nom = itemNom->text();

    // Demander confirmation
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
                                                              "Voulez-vous vraiment supprimer le client \"" + nom + "\" (ID: " +
                                                                  QString::number(id) + ") ?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (clientTmp.supprimer(id)) {
            QMessageBox::information(this, "Succès", "Client supprimé avec succès !");
            refreshTableView();
            clearInputFields();
            enableInputFields(false);
            ui->le_id_client->clear();
            ui->le_id_client->setText("(Nouveau - Auto)");
        } else {
            QMessageBox::critical(this, "Erreur",
                                  "Client non trouvé ou échec de la suppression !");
        }
    }
}

// ============= RECHERCHER UN CLIENT =============
void MainWindow::on_btn_rechercher_clicked()
{
    // Créer un dialog de recherche simple
    bool ok;
    QString critere = QInputDialog::getText(this, "Rechercher un client",
                                            "Entrez le nom, CIN ou téléphone à rechercher :",
                                            QLineEdit::Normal, "", &ok);

    if (ok && !critere.isEmpty()) {
        QSqlQueryModel* model = clientTmp.rechercher(critere);

        if (!model) return;

        int rows = model->rowCount();
        ui->tableClients->setRowCount(rows);
        ui->tableClients->setColumnCount(6);

        QStringList headers;
        headers << "ID" << "CIN / MF" << "Nom Client" << "Téléphone" << "Email" << "Adresse";
        ui->tableClients->setHorizontalHeaderLabels(headers);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < 6; j++) {
                QModelIndex index = model->index(i, j);
                ui->tableClients->setItem(i, j, new QTableWidgetItem(model->data(index).toString()));
            }
        }

        if (rows == 0) {
            QMessageBox::information(this, "Recherche", "Aucun client trouvé.");
        }
    } else if (ok && critere.isEmpty()) {
        refreshTableView(); // Afficher tous les clients
    }
}

// ============= GÉNÉRER FACTURE PDF =============
void MainWindow::on_btn_facture_clicked()
{
    int currentRow = ui->tableClients->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un client !");
        return;
    }

    // ================= Récupération des données =================
    QString id = ui->tableClients->item(currentRow, 0)->text();
    QString cin = ui->tableClients->item(currentRow, 1)->text();
    QString nom = ui->tableClients->item(currentRow, 2)->text();
    QString tel = ui->tableClients->item(currentRow, 3)->text();
    QString email = ui->tableClients->item(currentRow, 4)->text();
    QString adresse = ui->tableClients->item(currentRow, 5)->text();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Enregistrer Facture",
                                                    "Facture_" + nom + ".pdf",
                                                    "*.pdf");

    if (fileName.isEmpty()) return;

    // ================= PDF =================
    QPdfWriter pdf(fileName);
    QPainter painter(&pdf);

    int y = 200;
    int line = 250;

    // ================= HEADER =================
    painter.setFont(QFont("Arial", 18, QFont::Bold));
    painter.drawText(200, y, "SMART LEATHER");
    y += line;

    painter.setFont(QFont("Arial", 12));
    painter.drawText(200, y, "FACTURE CLIENT");
    y += line;

    painter.drawLine(100, y, 800, y);
    y += line;

    // ================= INFOS CLIENT =================
    painter.setFont(QFont("Arial", 11));

    painter.drawText(100, y, "ID Client : " + id);
    y += line;

    painter.drawText(100, y, "CIN / MF : " + cin);
    y += line;

    painter.drawText(100, y, "Nom : " + nom);
    y += line;

    painter.drawText(100, y, "Téléphone : " + tel);
    y += line;

    painter.drawText(100, y, "Email : " + email);
    y += line;

    painter.drawText(100, y, "Adresse : " + adresse);
    y += line;

    painter.drawLine(100, y, 800, y);
    y += line;

    // ================= DATE =================
    painter.drawText(100, y, "Date : " + QDate::currentDate().toString("dd/MM/yyyy"));
    y += line;

    painter.drawLine(100, y, 800, y);
    y += line;

    // ================= FOOTER =================
    painter.setFont(QFont("Arial", 10));
    y += 50;
    painter.drawText(100, y, "Merci pour votre confiance !");

    painter.end();

    QMessageBox::information(this, "Succès", "Facture PDF générée !");
}
// ============= TRIER PAR POINTS FIDÉLITÉ =============
void MainWindow::on_btn_ajouter_clicked()
{
    if (!ui->le_cin_client->isEnabled()) {
        setFormMode(true);
        return;
    }

    if (!validateInputs()) return;

    // Au lieu de 5 paramètres, mets 6 paramètres (avec points = 0 par défaut)
    Client client(
        ui->le_cin_client->text().trimmed(),
        ui->le_nom_client->text().trimmed(),
        ui->le_tel_client->text().trimmed(),
        ui->le_email_client->text().trimmed(),
        ui->le_adresse_client->text().trimmed(),
        0  // ← points (0 par défaut)
        );

    if (client.ajouter()) {
        QMessageBox::information(this, "Succès", "Client ajouté !");
        refreshTableView();
        clearInputFields();
        enableInputFields(false);
        ui->le_id_client->setText("(Nouveau - Auto)");
        updateClientStatus();
        updateIAPrediction();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout !");
    }
}
// ============= TRIER PAR POINTS FIDÉLITÉ =============
void MainWindow::on_btn_tri_points_clicked()
{
    QSqlQueryModel* model = clientTmp.trierParPoints();

    if (!model) return;

    int rows = model->rowCount();
    ui->tableClients->setRowCount(rows);
    ui->tableClients->setColumnCount(7);

    QStringList headers;
    headers << "ID" << "CIN / MF" << "Nom Client" << "Téléphone" << "Email" << "Adresse" << "Points";
    ui->tableClients->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 7; j++) {
            QModelIndex index = model->index(i, j);
            ui->tableClients->setItem(i, j, new QTableWidgetItem(model->data(index).toString()));
        }
    }

    QMessageBox::information(this, "Tri", "Clients triés par points de fidélité !");
}

// ============= MAILING CIBLÉ =============
void MainWindow::on_btn_mailing_clicked()
{
    QMessageBox::information(this, "Mailing ciblé",
                             "Envoi d'emails aux clients inactifs\n(Fonctionnalité à implémenter)");
}

// ============= PDF / FACTURES =============
void MainWindow::on_btn_pdf_clicked()
{
    QMessageBox::information(this, "PDF / Factures",
                             "Gestion des factures PDF\n(Fonctionnalité à implémenter)");
}

// ============= STATISTIQUES =============
// ============= STATISTIQUES =============
void MainWindow::on_btn_stat_clicked()
{
    QSqlQuery query;

    // Total clients
    query.exec("SELECT COUNT(*) FROM client");
    int total = 0;
    if (query.next()) total = query.value(0).toInt();

    // VIP
    query.exec("SELECT COUNT(*) FROM client WHERE points > 500");
    int vip = 0;
    if (query.next()) vip = query.value(0).toInt();

    int normal = total - vip;

    // =========================
    QPieSeries *series = new QPieSeries();

    QPieSlice *sliceVIP = series->append("VIP ⭐", vip);
    QPieSlice *sliceNormal = series->append("Normaux 👤", normal);

    // Labels dynamiques avec %
    sliceVIP->setLabel(QString("VIP (%1)").arg(vip));
    sliceNormal->setLabel(QString("Normaux (%1)").arg(normal));

    sliceVIP->setLabelVisible(true);
    sliceNormal->setLabelVisible(true);

    sliceVIP->setExploded(true);

    // =========================
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Statistiques des Clients (Temps réel)");
    chart->setAnimationOptions(QChart::AllAnimations);

    // =========================
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // =========================
    QWidget *window = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(window);

    layout->addWidget(chartView);

    window->resize(650, 450);
    window->setWindowTitle("Dashboard 📊");
    window->show();
}
// ============= SÉLECTION DANS LA TABLE =============
void MainWindow::on_tableClients_clicked(const QModelIndex &index)
{
    loadClientToForm(index.row());
}
