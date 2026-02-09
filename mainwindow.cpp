#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDate>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <algorithm>
#include <QPageSize>
#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- VISUEL ---
    QPixmap logo("logo.png");
    if(!logo.isNull()) {
        ui->l_logo_img->setPixmap(logo.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->lbl_big_logo->setPixmap(logo.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->lbl_big_logo->setText("");
    } else {
        ui->lbl_big_logo->setText("FIL D'OR");
        ui->l_logo_img->setText("IMG");
    }

    myColorDelegate = new ColorDelegate(this);
    ui->tableTimeline->setItemDelegate(myColorDelegate);

    // --- DONNEES ---
    mesCommandes.append({"OF-101", "Sac Voyage Cuir", 50, "Cuir Vachette", QDate::currentDate(), "10/02/2026", "En cours", "", 0});
    mesCommandes.append({"OF-102", "Portefeuille Luxe", 120, "Cuir Agneau", QDate::currentDate().addDays(1), "12/02/2026", "Planifié", "", 0});
    mesProduits.append({"REF-001", "Sac Voyage Cuir", 120.50, "Hiver 2026", "Vachette Pleine Fleur", 5});
    mesEmployes.append({"EMP-001", "Dupont", "Jean", "Chef Atelier", "Production", QDate(2020, 5, 10), 2800.0, "RF-123"});
    mesMatieres.append({"CUIR-V-001", "Cuir", "BRUT", "Marron", "A (Premium)", 100.0, "M2", "Zone A", "Allée 1", "Standard", QDate::currentDate()});

    rafraichirListeCommandes(); configurerTimelineGantt();
    rafraichirListeProduits(); rafraichirListeEmployes(); rafraichirListeMatieres();

    ui->stackedWidget->setCurrentWidget(ui->page_home);

    // --- NAVIGATION ---
    connect(ui->btn_nav_dashboard, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_home); });
    connect(ui->btn_nav_planif, &QPushButton::clicked, [=](){ rafraichirListeCommandes(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_nav_fab, &QPushButton::clicked, [=](){ configurerTimelineGantt(); ui->stackedWidget->setCurrentWidget(ui->page_fab_list); });
    connect(ui->btn_nav_produit, &QPushButton::clicked, [=](){ rafraichirListeProduits(); ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_nav_rh, &QPushButton::clicked, [=](){ rafraichirListeEmployes(); ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_nav_stock, &QPushButton::clicked, [=](){ rafraichirListeMatieres(); ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_start_app, &QPushButton::clicked, [=](){ rafraichirListeCommandes(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    // PLANIF : IA ESTIMATION TEMPS (NOUVEAU BOUTON)
    connect(ui->btn_open_planif_ia, &QPushButton::clicked, this, &MainWindow::showPlanifIaDialog);

    // BOUTONS RETOUR
    connect(ui->btn_back_planif, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_back_fab, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_fab_list); });
    connect(ui->btn_back_stats, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_back_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_back_stats_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_back_emp, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_back_stats_emp, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_back_stock, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_back_inn_stock, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });
    connect(ui->btn_back_stats_stock, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_stock_list); });

    // --- MODULES ---
    connect(ui->btn_stat_plan, &QPushButton::clicked, [=](){ calculerEtAfficherStats(); ui->stackedWidget->setCurrentWidget(ui->page_stats); });
    connect(ui->btn_pdf, &QPushButton::clicked, [=](){ exporterPDF(ui->tablePlanif, "Planning Production"); });

    // PRODUITS
    connect(ui->btn_open_simul, &QPushButton::clicked, this, &MainWindow::showProdSimDialog); // [NOUVEAU]
    connect(ui->btn_stats_prod, &QPushButton::clicked, [=](){ calculerStatsProduits(); ui->stackedWidget->setCurrentWidget(ui->page_stats_prod); });
    connect(ui->btn_pdf_catalogue, &QPushButton::clicked, [=](){ exporterPDF(ui->tableProduits, "Catalogue 2026"); });

    // RH
    connect(ui->btn_open_eval, &QPushButton::clicked, this, &MainWindow::showEmpEvalDialog); // [NOUVEAU]
    connect(ui->btn_stats_emp, &QPushButton::clicked, [=](){ calculerStatsRH(); ui->stackedWidget->setCurrentWidget(ui->page_stats_emp); });
    connect(ui->btn_pdf_emp, &QPushButton::clicked, [=](){ exporterPDF(ui->tableEmployes, "Registre Personnel"); });

    // STOCK
    connect(ui->btn_open_compare, &QPushButton::clicked, this, &MainWindow::showCompareDialog); // [RESTITUÉ]
    connect(ui->btn_open_calcul, &QPushButton::clicked, this, &MainWindow::showBesoinDialog);   // [RESTITUÉ]
    connect(ui->btn_stats_stock, &QPushButton::clicked, [=](){ calculerStatsStock(); ui->stackedWidget->setCurrentWidget(ui->page_stock_stats); });
    connect(ui->btn_pdf_stock, &QPushButton::clicked, [=](){ exporterPDF(ui->tableStock, "Inventaire Stock"); });

    // --- CRUD ---
    // Planif CRUD
    auto prepNewPlanif = [=](){ modeModification=false; ui->le_fin_prevue->clear(); ui->stackedWidget->setCurrentWidget(ui->page_planif_form); };
    connect(ui->btn_to_add_planif, &QPushButton::clicked, prepNewPlanif);
    connect(ui->btn_valider_planif, &QPushButton::clicked, [=](){
        QString p=ui->cb_produit->currentText(), m=ui->cb_matiere->currentText();
        if(modeModification) { mesCommandes[indexModification].produit=p; }
        else { mesCommandes.append({"OF-"+QString::number(100+mesCommandes.size()), p, ui->sb_qte->value(), m, ui->dt_lancement->date(), ui->le_fin_prevue->text(), "Planifié", "", 0}); }
        ui->stackedWidget->setCurrentWidget(ui->page_planif_list); rafraichirListeCommandes(); configurerTimelineGantt();
    });
    connect(ui->btn_calculer_ia, &QPushButton::clicked, [=](){
        if(ui->sb_qte->value() > 0) {
            // Logique simple : 2 jours de base + 1 jour tous les 100 articles
            int joursEstimes = 2 + (ui->sb_qte->value() / 100);

            // Calcul et affichage de la date de fin
            QDate fin = ui->dt_lancement->date().addDays(joursEstimes);
            ui->le_fin_prevue->setText(fin.toString("dd/MM/yyyy"));

            // Affichage de la durée dans le champ texte qui était vide
            ui->le_duree_totale->setText(QString::number(joursEstimes) + " Jours");
        }
    });
    connect(ui->btn_modifier_planif, &QPushButton::clicked, [=](){
        int r = ui->tablePlanif->currentRow(); if(r < 0) return;
        modeModification = true; indexModification = r; CommandeInfo c = mesCommandes[r];
        ui->cb_produit->setCurrentText(c.produit); ui->sb_qte->setValue(c.quantite); ui->cb_matiere->setCurrentText(c.matiere);
        ui->dt_lancement->setDate(c.dateDebut); ui->le_fin_prevue->setText(c.dateFinEstimee);
        ui->gb_form->setTitle("Modifier l'OF : " + c.id); ui->btn_valider_planif->setText("ENREGISTRER");
        ui->stackedWidget->setCurrentWidget(ui->page_planif_form);
    });
    connect(ui->btn_supprimer_planif, &QPushButton::clicked, [=](){ int r=ui->tablePlanif->currentRow(); if(r>=0) { mesCommandes.removeAt(r); rafraichirListeCommandes(); } });

    // Prod CRUD
    connect(ui->btn_add_produit, &QPushButton::clicked, [=](){ modeModifProd=false; ui->stackedWidget->setCurrentWidget(ui->page_produit_form); });
    connect(ui->btn_valider_produit, &QPushButton::clicked, [=](){
        if(modeModifProd) mesProduits[indexModifProd].nom=ui->le_nom_prod->text();
        else mesProduits.append({ui->le_ref_prod->text(), ui->le_nom_prod->text(), ui->sb_cout_prod->value(), ui->cb_coll_prod->currentText(), ui->cb_cuir_prod->currentText(), ui->sb_temps_prod->value()});
        ui->stackedWidget->setCurrentWidget(ui->page_produit_list); rafraichirListeProduits();
    });
    connect(ui->btn_edit_produit, &QPushButton::clicked, [=](){ int r=ui->tableProduits->currentRow(); if(r>=0) { modeModifProd=true; indexModifProd=r; ui->le_ref_prod->setText(mesProduits[r].ref); ui->stackedWidget->setCurrentWidget(ui->page_produit_form); } });
    connect(ui->btn_delete_produit, &QPushButton::clicked, [=](){ int r=ui->tableProduits->currentRow(); if(r>=0) { mesProduits.removeAt(r); rafraichirListeProduits(); } });

    // RH CRUD
    connect(ui->btn_add_emp, &QPushButton::clicked, [=](){ modeModifEmp=false; ui->stackedWidget->setCurrentWidget(ui->page_employe_form); });
    connect(ui->btn_valider_emp, &QPushButton::clicked, [=](){
        if(modeModifEmp) mesEmployes[indexModifEmp].nom=ui->le_nom_emp->text();
        else mesEmployes.append({ui->le_id_emp->text(), ui->le_nom_emp->text(), ui->le_prenom_emp->text(), ui->cb_poste_emp->currentText(), ui->cb_dept_emp->currentText(), ui->dt_embauche->date(), ui->sb_salaire_emp->value(), ui->le_rfid_emp->text()});
        ui->stackedWidget->setCurrentWidget(ui->page_employe_list); rafraichirListeEmployes();
    });
    connect(ui->btn_edit_emp, &QPushButton::clicked, [=](){ int r=ui->tableEmployes->currentRow(); if(r>=0) { modeModifEmp=true; indexModifEmp=r; ui->le_nom_emp->setText(mesEmployes[r].nom); ui->stackedWidget->setCurrentWidget(ui->page_employe_form); } });
    connect(ui->btn_delete_emp, &QPushButton::clicked, [=](){ int r=ui->tableEmployes->currentRow(); if(r>=0) { mesEmployes.removeAt(r); rafraichirListeEmployes(); } });
    connect(ui->btn_send_chat, &QPushButton::clicked, [=](){ reponseChatbot(); });

    // Stock CRUD
    connect(ui->btn_add_stock, &QPushButton::clicked, [=](){ modeModifStock=false; ui->stackedWidget->setCurrentWidget(ui->page_stock_form); });
    connect(ui->btn_valider_stock, &QPushButton::clicked, [=](){
        MatiereInfo m; m.code=ui->le_code_mp->text(); m.categorie=ui->cb_cat_mp->currentText(); m.quantite=ui->sb_qte_mp->value();
        if(modeModifStock) mesMatieres[indexModifStock]=m; else mesMatieres.append(m);
        ui->stackedWidget->setCurrentWidget(ui->page_stock_list); rafraichirListeMatieres();
    });
    connect(ui->btn_edit_stock, &QPushButton::clicked, [=](){
        int r=ui->tableStock->currentRow(); if(r<0) return;
        modeModifStock=true; indexModifStock=r; MatiereInfo m = mesMatieres[r];
        ui->le_code_mp->setText(m.code); ui->cb_cat_mp->setCurrentText(m.categorie);
        ui->cb_etat_mp->setCurrentText(m.etat); ui->le_coul_mp->setText(m.couleur);
        ui->cb_qual_mp->setCurrentText(m.qualite); ui->sb_qte_mp->setValue(m.quantite);
        ui->cb_unite_mp->setCurrentText(m.unite); ui->cb_zone_mp->setCurrentText(m.zone);
        ui->le_allee_mp->setText(m.allee); ui->cb_type_stock->setCurrentText(m.typeStock);
        ui->dt_rec_mp->setDate(m.dateRec);
        ui->stackedWidget->setCurrentWidget(ui->page_stock_form);
    });
    connect(ui->btn_delete_stock, &QPushButton::clicked, [=](){
        int r=ui->tableStock->currentRow(); if(r>=0) { mesMatieres.removeAt(r); rafraichirListeMatieres(); }
    });

    // Fabrication
    connect(ui->tableTimeline, &QTableWidget::cellClicked, [=](int r){
        indexCommandeSelectionnee=r;
        ui->lbl_sel_cmd->setText("OF: " + mesCommandes[r].id + " - " + mesCommandes[r].produit);
        ui->lbl_resultat_delta->setText("..."); ui->lbl_resultat_delta->setStyleSheet("color:gray; border:none;");
    });
    connect(ui->cb_etape_suivi, &QComboBox::currentTextChanged, [=](const QString &t){
        double v=2.0; if(t=="Assemblage")v=5.0; else if(t=="Couture")v=4.0; else if(t=="Finition")v=1.0;
        ui->lbl_temps_prevu->setText(QString::number(v)+" h");
    });
    connect(ui->btn_valider_etape, &QPushButton::clicked, [=](){
        if(indexCommandeSelectionnee<0) return;
        double r=ui->sb_temps_reel_input->value(), p=ui->lbl_temps_prevu->text().split(" ")[0].toDouble();
        if(r>p) { ui->lbl_resultat_delta->setText("RETARD"); ui->lbl_resultat_delta->setStyleSheet("background-color:#bf360c;color:white;padding:5px;border-radius:4px;"); mesCommandes[indexCommandeSelectionnee].etatEtape=2; }
        else { ui->lbl_resultat_delta->setText("OK"); ui->lbl_resultat_delta->setStyleSheet("background-color:#2e7d32;color:white;padding:5px;border-radius:4px;"); mesCommandes[indexCommandeSelectionnee].etatEtape=1; }
        configurerTimelineGantt();
    });
    connect(ui->btn_refresh_timeline, &QPushButton::clicked, [=](){ configurerTimelineGantt(); });
    connect(ui->btn_saisie_detail, &QPushButton::clicked, [=](){ QMessageBox::information(this, "Info", "Module Détail"); });
}

MainWindow::~MainWindow() { delete ui; }

// =================================================================
// === POP-UPS (STOCK) - PRESERVES DU CODE FOURNI ===
// =================================================================

void MainWindow::showCompareDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Comparateur de Fournisseurs");
    dialog.setMinimumSize(860, 620);
    dialog.setStyleSheet("QDialog { background-color: #f4f1ea; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(14);

    QLabel *title = new QLabel("Comparateur Intelligent de Fournisseurs");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #3e2723; padding: 10px; background-color: #e1f5fe; border-radius: 10px;");
    mainLayout->addWidget(title);

    QHBoxLayout *selectLayout = new QHBoxLayout();
    QLabel *lblSelect = new QLabel("Matière recherchée :");
    lblSelect->setStyleSheet("font-weight: bold; font-size: 14px;");

    QComboBox *cbMatiere = new QComboBox();
    cbMatiere->addItems({"Cuir Vachette", "Cuir Agneau", "Teinture Noire", "Produit de Finition"});
    cbMatiere->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 14px;");

    QPushButton *btnCompare = new QPushButton("Comparer");
    btnCompare->setStyleSheet("background-color: #00838f; color: white; padding: 8px 20px; border-radius: 15px; font-weight: bold;");

    selectLayout->addWidget(lblSelect);
    selectLayout->addWidget(cbMatiere);
    selectLayout->addWidget(btnCompare);
    mainLayout->addLayout(selectLayout);

    QLabel *lblResults = new QLabel("Résultats de la comparaison :");
    lblResults->setStyleSheet("font-weight: bold; font-size: 14px; padding: 6px;");
    mainLayout->addWidget(lblResults);

    QTableWidget *tableCompare = new QTableWidget(3, 6);
    tableCompare->setHorizontalHeaderLabels({"Fournisseur", "Prix/M²", "Qualité", "Délai", "Stock Dispo", "Note"});
    tableCompare->horizontalHeader()->setStretchLastSection(true);
    tableCompare->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableCompare->setStyleSheet("QHeaderView::section { background-color: #00838f; color: white; padding: 10px; font-weight: bold; }");

    tableCompare->setItem(0, 0, new QTableWidgetItem("TanLeather SA"));
    tableCompare->setItem(0, 1, new QTableWidgetItem("45 DT"));
    tableCompare->setItem(0, 2, new QTableWidgetItem("A (Premium)"));
    tableCompare->setItem(0, 3, new QTableWidgetItem("5 jours"));
    tableCompare->setItem(0, 4, new QTableWidgetItem("500 M²"));
    tableCompare->setItem(0, 5, new QTableWidgetItem("★★★★★"));

    tableCompare->setItem(1, 0, new QTableWidgetItem("Cuir Elite"));
    tableCompare->setItem(1, 1, new QTableWidgetItem("38 DT"));
    tableCompare->setItem(1, 2, new QTableWidgetItem("B (Standard)"));
    tableCompare->setItem(1, 3, new QTableWidgetItem("7 jours"));
    tableCompare->setItem(1, 4, new QTableWidgetItem("300 M²"));
    tableCompare->setItem(1, 5, new QTableWidgetItem("★★★★"));

    tableCompare->setItem(2, 0, new QTableWidgetItem("MegaCuir"));
    tableCompare->setItem(2, 1, new QTableWidgetItem("32 DT"));
    tableCompare->setItem(2, 2, new QTableWidgetItem("C (Économique)"));
    tableCompare->setItem(2, 3, new QTableWidgetItem("10 jours"));
    tableCompare->setItem(2, 4, new QTableWidgetItem("1000 M²"));
    tableCompare->setItem(2, 5, new QTableWidgetItem("★★★"));

    for (int i = 0; i < 6; ++i) {
        tableCompare->item(0, i)->setBackground(QColor(200, 255, 200));
        tableCompare->item(1, i)->setBackground(QColor(255, 250, 205));
        tableCompare->item(2, i)->setBackground(QColor(255, 220, 220));
    }

    mainLayout->addWidget(tableCompare);

    QLabel *lblRecommendation = new QLabel("<b>Recommandation :</b> TanLeather SA (meilleur rapport Qualité/Prix/Fiabilité).");
    lblRecommendation->setStyleSheet("background-color: #c8e6c9; padding: 12px; border-radius: 10px; font-size: 14px;");
    lblRecommendation->setWordWrap(true);
    mainLayout->addWidget(lblRecommendation);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnContact = new QPushButton("Contacter Fournisseur");
    btnContact->setStyleSheet("background-color: #2196f3; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet("background-color: #4caf50; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");

    buttonLayout->addWidget(btnContact);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnClose);
    mainLayout->addLayout(buttonLayout);

    connect(btnCompare, &QPushButton::clicked, &dialog, [&]() {
        QMessageBox::information(&dialog, "Comparaison", "Comparaison effectuée pour : " + cbMatiere->currentText());
    });
    connect(btnContact, &QPushButton::clicked, &dialog, [&]() {
        QMessageBox::information(&dialog, "Contact", "Email envoyé (démo) !");
    });
    connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

void MainWindow::showBesoinDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Calculer Besoin de Matières");
    dialog.setMinimumSize(820, 680);
    dialog.setStyleSheet("QDialog { background-color: #f4f1ea; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(14);

    QLabel *title = new QLabel("Calculateur de Besoins Matières");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #3e2723; padding: 12px; background-color: #fff3e0; border-radius: 10px;");
    mainLayout->addWidget(title);

    QGroupBox *gbCalc = new QGroupBox("Paramètres de Production");
    gbCalc->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; padding: 15px; border: 2px solid #f9a825; border-radius: 10px; background-color: white; }");
    QFormLayout *formLayout = new QFormLayout(gbCalc);
    formLayout->setSpacing(12);

    QComboBox *cbProduit = new QComboBox();
    cbProduit->addItems({"Sac Voyage Cuir", "Portefeuille Luxe", "Ceinture Homme", "Sac à Main Femme"});
    cbProduit->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px;");

    QSpinBox *sbQuantite = new QSpinBox();
    sbQuantite->setRange(1, 10000);
    sbQuantite->setValue(50);
    sbQuantite->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px;");

    QComboBox *cbMatiere = new QComboBox();
    cbMatiere->addItems({"Cuir Vachette", "Cuir Agneau", "Cuir Premium", "Cuir Synthétique"});
    cbMatiere->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px;");

    QDoubleSpinBox *sbPerte = new QDoubleSpinBox();
    sbPerte->setRange(0, 50);
    sbPerte->setValue(15);
    sbPerte->setDecimals(1);
    sbPerte->setSuffix(" %");
    sbPerte->setStyleSheet("padding: 8px; border: 2px solid #bcaaa4; border-radius: 10px; font-size: 13px;");

    formLayout->addRow("Type de Produit :", cbProduit);
    formLayout->addRow("Quantité à produire :", sbQuantite);
    formLayout->addRow("Type de Matière :", cbMatiere);
    formLayout->addRow("Marge de Perte :", sbPerte);

    mainLayout->addWidget(gbCalc);

    QPushButton *btnCalculer = new QPushButton("Calculer les Besoins");
    btnCalculer->setStyleSheet("background-color: #f9a825; color: white; padding: 12px; border-radius: 15px; font-weight: bold; font-size: 15px;");
    mainLayout->addWidget(btnCalculer);

    QGroupBox *gbResults = new QGroupBox("Résultats du Calcul");
    gbResults->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; padding: 15px; border: 2px solid #4caf50; border-radius: 10px; background-color: #e8f5e9; }");
    QVBoxLayout *resultsLayout = new QVBoxLayout(gbResults);
    resultsLayout->setSpacing(8);

    QLabel *lblResult1 = new QLabel("Surface nécessaire par unité : <b>0.8 M²</b>");
    QLabel *lblResult2 = new QLabel("Surface totale (sans perte) : <b>40 M²</b>");
    QLabel *lblResult3 = new QLabel("Surface avec marge de perte (15%) : <b>46 M²</b>");
    QLabel *lblResult4 = new QLabel("Stock disponible : <b>100 M²</b>");
    QLabel *lblResult5 = new QLabel("<b>Statut : SUFFISANT</b>");
    lblResult5->setAlignment(Qt::AlignCenter);
    lblResult5->setWordWrap(true);

    resultsLayout->addWidget(lblResult1);
    resultsLayout->addWidget(lblResult2);
    resultsLayout->addWidget(lblResult3);
    resultsLayout->addWidget(lblResult4);
    resultsLayout->addWidget(lblResult5);

    mainLayout->addWidget(gbResults);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    QPushButton *btnReserver = new QPushButton("Réserver Matière");
    btnReserver->setStyleSheet("background-color: #1e88e5; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");
    QPushButton *btnCommander = new QPushButton("Commander Plus");
    btnCommander->setStyleSheet("background-color: #d84315; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet("background-color: #43a047; color: white; padding: 10px 20px; border-radius: 15px; font-weight: bold;");

    actionLayout->addWidget(btnReserver);
    actionLayout->addWidget(btnCommander);
    actionLayout->addStretch();
    actionLayout->addWidget(btnClose);
    mainLayout->addLayout(actionLayout);

    connect(btnCalculer, &QPushButton::clicked, &dialog, [&]() {
        const int qte = sbQuantite->value();
        const double pertePercent = sbPerte->value();
        const double surfaceParUnite = 0.8;

        const double surfaceTotal = qte * surfaceParUnite;
        const double surfaceAvecPerte = surfaceTotal * (1.0 + pertePercent / 100.0);

        const double stockDispo = 100.0; // Démo
        const bool ok = (stockDispo >= surfaceAvecPerte);

        lblResult2->setText(QString("Surface totale (sans perte) : <b>%1 M²</b>").arg(surfaceTotal, 0, 'f', 1));
        lblResult3->setText(QString("Surface avec marge de perte (%1%) : <b>%2 M²</b>")
                                .arg(pertePercent, 0, 'f', 1)
                                .arg(surfaceAvecPerte, 0, 'f', 1));
        lblResult4->setText(QString("Stock disponible : <b>%1 M²</b>").arg(stockDispo, 0, 'f', 1));

        if (ok) {
            const double reste = stockDispo - surfaceAvecPerte;
            lblResult5->setText(QString("<b>Statut : SUFFISANT</b> (reste %1 M²)").arg(reste, 0, 'f', 1));
            lblResult5->setStyleSheet("padding: 10px; background-color: #c8e6c9; border-radius: 10px; color: #1b5e20;");
        } else {
            const double manque = surfaceAvecPerte - stockDispo;
            lblResult5->setText(QString("<b>Statut : INSUFFISANT</b> (manque %1 M²)").arg(manque, 0, 'f', 1));
            lblResult5->setStyleSheet("padding: 10px; background-color: #ffcdd2; border-radius: 10px; color: #b71c1c;");
        }
    });

    connect(btnReserver, &QPushButton::clicked, &dialog, [&]() {
        QMessageBox::information(&dialog, "Réservation", "Réservation effectuée (démo).");
    });
    connect(btnCommander, &QPushButton::clicked, &dialog, [&]() {
        QMessageBox::information(&dialog, "Commande", "Redirection vers le module Commande (démo).");
    });
    connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

// =================================================================
// === POP-UPS (NOUVEAUX: PRODUITS & RH) ===
// =================================================================

// 3. SIMULATEUR RENTABILITÉ (Produits)
void MainWindow::showProdSimDialog() {
    QDialog d(this); d.setWindowTitle("Simulateur Rentabilité"); d.setMinimumSize(500,400);
    d.setStyleSheet("background-color: #e0f2f1;");
    QVBoxLayout *l = new QVBoxLayout(&d);
    QLabel *ti = new QLabel("💲 Simulateur de Marge");
    ti->setStyleSheet("font-size:18px; font-weight:bold; color:#00695c;"); l->addWidget(ti);
    QFormLayout *f = new QFormLayout();
    QDoubleSpinBox *sbCout = new QDoubleSpinBox(); sbCout->setRange(0,10000); sbCout->setValue(50);
    QDoubleSpinBox *sbPrix = new QDoubleSpinBox(); sbPrix->setRange(0,10000); sbPrix->setValue(120);
    f->addRow("Coût de Production (€):", sbCout);
    f->addRow("Prix de Vente (€):", sbPrix);
    l->addLayout(f);
    QLabel *lblRes = new QLabel("Marge Nette : <b>-</b>");
    QPushButton *btnCalc = new QPushButton("Calculer");
    connect(btnCalc, &QPushButton::clicked, [=](){
        double m = sbPrix->value() - sbCout->value();
        lblRes->setText("Marge : <b>" + QString::number(m) + " €</b>");
    });
    l->addWidget(btnCalc); l->addWidget(lblRes);
    d.exec();
}

// 4. ÉVALUATION COMPÉTENCES (RH)
void MainWindow::showEmpEvalDialog() {
    QDialog d(this); d.setWindowTitle("Évaluation Compétences"); d.setMinimumSize(500,400);
    d.setStyleSheet("background-color: #fff3e0;");
    QVBoxLayout *l = new QVBoxLayout(&d);
    QLabel *ti = new QLabel("📝 Évaluation 360° (IA)");
    ti->setStyleSheet("font-size:18px; font-weight:bold; color:#ef6c00;"); l->addWidget(ti);
    QFormLayout *f = new QFormLayout();
    QComboBox *cb = new QComboBox(); for(const auto &e : mesEmployes) cb->addItem(e.nom);
    QSpinBox *sb = new QSpinBox(); sb->setRange(0,20); sb->setValue(15);
    f->addRow("Employé :", cb); f->addRow("Note Globale (/20) :", sb);
    l->addLayout(f);
    l->addWidget(new QLabel("Appréciation : <b>Excellent</b>"));
    d.exec();
}

// --- FONCTIONS EXISTANTES ---
void MainWindow::exporterPDF(QTableWidget *table, QString titre) {
    QString f = QFileDialog::getSaveFileName(this, "Export", QString(), "PDF (*.pdf)");
    if(f.isEmpty()) return; if(QFileInfo(f).suffix().isEmpty()) f+=".pdf";
    QPrinter p(QPrinter::PrinterResolution); p.setOutputFormat(QPrinter::PdfFormat); p.setPageSize(QPageSize(QPageSize::A4)); p.setOutputFileName(f);
    QPainter painter; if(!painter.begin(&p)) return;
    QRectF r = p.pageLayout().paintRectPixels(p.resolution()); double w=r.width(), h=r.height(); int y=100;
    painter.setFont(QFont("Arial", 20, QFont::Bold)); painter.drawText(0, y, w, 40, Qt::AlignCenter, titre); y+=100;
    int rows=table->rowCount(), cols=table->columnCount(); double cw=(w-100)/cols; int rh=30;
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    for(int c=0;c<cols;c++) painter.drawText(50+(c*cw), y, cw, rh, Qt::AlignCenter, table->horizontalHeaderItem(c)->text());
    y+=rh; painter.setFont(QFont("Arial", 9));
    for(int i=0; i<rows; i++) {
        if(y>h-50) { p.newPage(); y=50; }
        for(int c=0; c<cols; c++) if(table->item(i,c)) painter.drawText(50+(c*cw), y, cw, rh, Qt::AlignCenter, table->item(i,c)->text());
        y+=rh;
    }
    painter.end(); QMessageBox::information(this, "Succès", "PDF généré !");
}

void MainWindow::calculerStatsStock() {
    int total = mesMatieres.size(); ui->val_tot_mat->setText(QString::number(total));
    double volume = 0; int cuir=0, teint=0, chim=0, qa=0, qb=0;
    for(const auto &m : mesMatieres) {
        volume += m.quantite;
        if(m.categorie.contains("Cuir")) cuir++; else if(m.categorie.contains("Teinture")) teint++; else chim++;
        if(m.qualite.contains("A")) qa++; else qb++;
    }
    ui->val_tot_vol->setText(QString::number(volume, 'f', 1));
    ui->pb_cuir->setMaximum(total); ui->pb_cuir->setValue(cuir); ui->pb_teint->setMaximum(total); ui->pb_teint->setValue(teint); ui->pb_chim->setMaximum(total); ui->pb_chim->setValue(chim);
    ui->pb_qa->setMaximum(total); ui->pb_qa->setValue(qa); ui->pb_qb->setMaximum(total); ui->pb_qb->setValue(qb);
}
void MainWindow::rafraichirListeMatieres() {
    ui->tableStock->setRowCount(0); ui->tableStock->setColumnCount(7); ui->tableStock->setHorizontalHeaderLabels({"Code","Cat","Etat","Qual","Qté","Stock","Zone"});
    for(int i=0; i<mesMatieres.size(); i++) {
        ui->tableStock->insertRow(i);
        ui->tableStock->setItem(i,0,new QTableWidgetItem(mesMatieres[i].code));
        ui->tableStock->setItem(i,1,new QTableWidgetItem(mesMatieres[i].categorie));
        ui->tableStock->setItem(i,2,new QTableWidgetItem(mesMatieres[i].etat));
        ui->tableStock->setItem(i,3,new QTableWidgetItem(mesMatieres[i].qualite));
        ui->tableStock->setItem(i,4,new QTableWidgetItem(QString::number(mesMatieres[i].quantite)));
        ui->tableStock->setItem(i,5,new QTableWidgetItem(mesMatieres[i].typeStock));
        ui->tableStock->setItem(i,6,new QTableWidgetItem(mesMatieres[i].zone));
    }
}
void MainWindow::calculerStatsRH() {
    int t=mesEmployes.size(); ui->val_tot_emp->setText(QString::number(t)); double m=0; int p=0,l=0,b=0;
    for(const auto &e:mesEmployes) { m+=e.salaire; if(e.departement.contains("Prod")) p++; else if(e.departement.contains("Log")) l++; else b++; }
    ui->val_masse_sal->setText(QString::number(m)); ui->pb_prod_rh->setMaximum(t); ui->pb_prod_rh->setValue(p); ui->pb_log_rh->setMaximum(t); ui->pb_log_rh->setValue(l); ui->pb_be_rh->setMaximum(t); ui->pb_be_rh->setValue(b);
}
void MainWindow::reponseChatbot() { ui->txt_chat_history->append("Bot: Bonjour !"); }
void MainWindow::rafraichirListeEmployes() {
    ui->tableEmployes->setRowCount(0); ui->tableEmployes->setColumnCount(7); ui->tableEmployes->setHorizontalHeaderLabels({"ID","Nom","Pre","Pos","Dep","Sal","Date"});
    for(int i=0; i<mesEmployes.size(); i++) {
        ui->tableEmployes->insertRow(i);
        ui->tableEmployes->setItem(i,0,new QTableWidgetItem(mesEmployes[i].id));
        ui->tableEmployes->setItem(i,1,new QTableWidgetItem(mesEmployes[i].nom));
        ui->tableEmployes->setItem(i,2,new QTableWidgetItem(mesEmployes[i].prenom));
        ui->tableEmployes->setItem(i,3,new QTableWidgetItem(mesEmployes[i].poste));
        ui->tableEmployes->setItem(i,4,new QTableWidgetItem(mesEmployes[i].departement));
        ui->tableEmployes->setItem(i,5,new QTableWidgetItem(QString::number(mesEmployes[i].salaire)));
        ui->tableEmployes->setItem(i,6,new QTableWidgetItem(mesEmployes[i].dateEmbauche.toString("dd/MM")));
    }
}
void MainWindow::rafraichirListeProduits() {
    ui->tableProduits->setRowCount(0); ui->tableProduits->setColumnCount(6); ui->tableProduits->setHorizontalHeaderLabels({"Ref","Nom","Cout","Col","Cuir","Temps"});
    for(int i=0; i<mesProduits.size(); i++) {
        ui->tableProduits->insertRow(i);
        ui->tableProduits->setItem(i,0,new QTableWidgetItem(mesProduits[i].ref));
        ui->tableProduits->setItem(i,1,new QTableWidgetItem(mesProduits[i].nom));
        ui->tableProduits->setItem(i,2,new QTableWidgetItem(QString::number(mesProduits[i].coutMatiere)));
        ui->tableProduits->setItem(i,3,new QTableWidgetItem(mesProduits[i].collection));
        ui->tableProduits->setItem(i,4,new QTableWidgetItem(mesProduits[i].cuir));
        ui->tableProduits->setItem(i,5,new QTableWidgetItem(QString::number(mesProduits[i].temps)));
    }
}
void MainWindow::calculerStatsProduits() {
    int t=mesProduits.size(); ui->val_tot_ref->setText(QString::number(t)); double c=0; int h=0,e=0,i=0;
    for(const auto &p:mesProduits) { c+=p.coutMatiere; if(p.collection.contains("Hiv")) h++; else if(p.collection.contains("Et")) e++; else i++; }
    ui->val_avg_cost->setText(QString::number(c/t)); ui->pb_hiver->setMaximum(t); ui->pb_hiver->setValue(h); ui->pb_ete->setMaximum(t); ui->pb_ete->setValue(e); ui->pb_intemp->setMaximum(t); ui->pb_intemp->setValue(i);
}
void MainWindow::rafraichirListeCommandes() {
    ui->tablePlanif->setRowCount(0); ui->tablePlanif->setColumnCount(7); ui->tablePlanif->setHorizontalHeaderLabels({"ID","Prod","Qté","Mat","Deb","Fin","Stat"});
    for(int i=0; i<mesCommandes.size(); i++) {
        ui->tablePlanif->insertRow(i);
        ui->tablePlanif->setItem(i,0,new QTableWidgetItem(mesCommandes[i].id));
        ui->tablePlanif->setItem(i,1,new QTableWidgetItem(mesCommandes[i].produit));
        ui->tablePlanif->setItem(i,2,new QTableWidgetItem(QString::number(mesCommandes[i].quantite)));
        ui->tablePlanif->setItem(i,3,new QTableWidgetItem(mesCommandes[i].matiere));
        ui->tablePlanif->setItem(i,4,new QTableWidgetItem(mesCommandes[i].dateDebut.toString("dd/MM")));
        ui->tablePlanif->setItem(i,5,new QTableWidgetItem(mesCommandes[i].dateFinEstimee));
        ui->tablePlanif->setItem(i,6,new QTableWidgetItem(mesCommandes[i].statut));
    }
}
void MainWindow::calculerEtAfficherStats() {
    int t=mesCommandes.size(); ui->lbl_stat_total_cmd->setText(QString::number(t)); int q=0,r=0;
    for(const auto &m:mesCommandes) { q+=m.quantite; if(m.etatEtape==2) r++; }
    ui->lbl_stat_total_qty->setText(QString::number(q)); ui->lbl_stat_retard->setText(QString::number(r));
}
// --------------------------- IA PLANIFICATION (POP-UP) ---------------------------
void MainWindow::showPlanifIaDialog() {
    QDialog d(this);
    d.setWindowTitle("IA Estimation Temps");
    d.setMinimumSize(500, 400);
    d.setStyleSheet("QDialog { background-color: #f3e5f5; }"); // Fond violet clair

    QVBoxLayout *l = new QVBoxLayout(&d);

    // Titre
    QLabel *ti = new QLabel("🧠 IA Estimation & Optimisation");
    ti->setStyleSheet("font-size: 18px; font-weight: bold; color: #7b1fa2; padding: 10px; background: white; border-radius: 5px;");
    ti->setAlignment(Qt::AlignCenter);
    l->addWidget(ti);

    // Formulaire
    QFormLayout *f = new QFormLayout();

    QComboBox *cbProd = new QComboBox();
    cbProd->addItems({"Sac Voyage Cuir", "Portefeuille Luxe", "Ceinture Homme"});
    cbProd->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; background: white;");

    QSpinBox *sbQte = new QSpinBox();
    sbQte->setRange(0, 10000);
    sbQte->setValue(100);
    sbQte->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; background: white;");

    f->addRow("Produit :", cbProd);
    f->addRow("Quantité à produire :", sbQte);

    l->addSpacing(10);
    l->addLayout(f);

    // Résultat
    QLabel *lblRes = new QLabel("Durée Estimée : -");
    lblRes->setStyleSheet("font-size: 16px; font-weight: bold; color: #4a148c; padding: 15px; background: white; border: 2px solid #7b1fa2; border-radius: 8px; margin-top: 10px;");
    lblRes->setAlignment(Qt::AlignCenter);

    // Bouton Calculer
    QPushButton *btnCalc = new QPushButton("Lancer l'Analyse IA");
    btnCalc->setStyleSheet("background-color: #7b1fa2; color: white; font-weight: bold; padding: 12px; border-radius: 6px; font-size: 14px;");

    // Logique du calcul (identique à celle que vous aviez, mais dans la pop-up)
    connect(btnCalc, &QPushButton::clicked, [=](){
        // Exemple de logique : 2 jours de base + 1 jour tous les 100 articles
        // + une petite variation selon le produit (ex: Sac = plus long)
        int base = 2;
        if(cbProd->currentText().contains("Sac")) base = 4;

        int j = base + (sbQte->value() / 50);

        // Date de fin estimée (par rapport à aujourd'hui)
        QDate fin = QDate::currentDate().addDays(j);

        lblRes->setText("Durée Estimée : " + QString::number(j) + " Jours\n"
                                                                  "Fin Prévue : " + fin.toString("dd/MM/yyyy"));
    });

    l->addSpacing(10);
    l->addWidget(btnCalc);
    l->addWidget(lblRes);

    // Bouton Fermer
    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet("background-color: #8e24aa; color: white; padding: 8px; border-radius: 4px;");
    connect(btnClose, &QPushButton::clicked, &d, &QDialog::accept);
    l->addWidget(btnClose);

    d.exec();
}
// --------------------------- FABRICATION (TIMELINE GANTT) ---------------------------
void MainWindow::configurerTimelineGantt() {
    QTableWidget *t = ui->tableTimeline;
    t->clear();
    t->setRowCount(0);

    int jours = 30; // Nombre de jours à afficher
    t->setColumnCount(1 + jours); // 1 colonne Produit + 30 colonnes jours

    // Génération des en-têtes avec la date d'aujourd'hui
    QStringList headers;
    headers << "PRODUIT";

    QDate today = QDate::currentDate(); // Date du jour dynamique

    for(int i = 0; i < jours; i++) {
        // Affiche le jour (ex: "09" pour le 9 février)
        headers << today.addDays(i).toString("dd");
    }

    t->setHorizontalHeaderLabels(headers);

    // Ajustement des largeurs
    t->setColumnWidth(0, 200); // Colonne Produit large
    for(int i = 1; i <= jours; i++) {
        t->setColumnWidth(i, 35); // Colonnes jours étroites
    }

    // Remplissage des données (Commandes)
    t->setRowCount(mesCommandes.size());
    for(int i = 0; i < mesCommandes.size(); i++) {
        // Colonne 0 : Nom de la commande
        t->setItem(i, 0, new QTableWidgetItem(mesCommandes[i].id));
        t->setRowHeight(i, 50);

        // Calcul du décalage (offset) par rapport à aujourd'hui
        // Si la commande commence aujourd'hui, offset = 0.
        // Si elle commence demain, offset = 1.
        int offset = today.daysTo(mesCommandes[i].dateDebut);

        // Si la commande est dans le passé, on ne l'affiche pas ou on la coupe
        if(offset < 0) offset = 0;

        // Définition des couleurs (Identique à ta demande précédente)
        QColor c1(41,128,185); // Bleu (Coupe)
        QColor c2(211,84,0);   // Orange (Assemblage)
        QColor c3(39,174,96);  // Vert (Couture)
        QColor c4(241,196,15); // Jaune (Finition)

        // Gestion du statut (Retard / OK)
        if(mesCommandes[i].etatEtape == 2) { // Retard
            QColor r(192,57,43); // Rouge
            if(mesCommandes[i].etapeAuditee == "Coupe") c1 = r;
            else if(mesCommandes[i].etapeAuditee == "Assemblage") c2 = r;
            else if(mesCommandes[i].etapeAuditee == "Couture") c3 = r;
            else c4 = r;
        }
        else if(mesCommandes[i].etatEtape == 1) { // OK
            QColor o(46,204,113); // Vert clair
            if(mesCommandes[i].etapeAuditee == "Coupe") c1 = o;
            else if(mesCommandes[i].etapeAuditee == "Assemblage") c2 = o;
            else if(mesCommandes[i].etapeAuditee == "Couture") c3 = o;
            else c4 = o;
        }

        // Dessin des barres (offset + 1 car la colonne 0 est le nom du produit)
        // Vérification pour ne pas dessiner hors du tableau
        if(offset < jours) dessinerBarre(i, offset + 1, 2, "Cp", c1, Qt::white);
        if(offset + 2 < jours) dessinerBarre(i, offset + 3, 3, "As", c2, Qt::white);
        if(offset + 5 < jours) dessinerBarre(i, offset + 6, 3, "Co", c3, Qt::white);
        if(offset + 8 < jours) dessinerBarre(i, offset + 9, 2, "Fi", c4, Qt::black);
    }

}
void MainWindow::dessinerBarre(int r, int c, int d, QString t, QColor b, QColor f) {
    if(c+d>30) return; QTableWidgetItem *it=new QTableWidgetItem(t); it->setData(Qt::BackgroundRole,b); it->setForeground(f);
    ui->tableTimeline->setItem(r,c,it); ui->tableTimeline->setSpan(r,c,1,d);
}
