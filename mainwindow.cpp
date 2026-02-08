#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDate>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <algorithm>
#include <QPageSize> // Indispensable pour l'impression Qt6

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- 1. SETUP VISUEL ---
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

    // --- 2. DONNÉES DE DÉPART ---
    // Commandes (Planification)
    mesCommandes.append({"OF-101", "Sac Voyage Cuir", 50, "Cuir Vachette", QDate::currentDate(), "10/02/2026", "En cours", "", 0});
    mesCommandes.append({"OF-102", "Portefeuille Luxe", 120, "Cuir Agneau", QDate::currentDate().addDays(1), "12/02/2026", "Planifié", "", 0});
    mesCommandes.append({"OF-103", "Ceinture Homme", 200, "Cuir Vachette", QDate::currentDate().addDays(2), "15/02/2026", "Planifié", "", 0});

    // Produits (Catalogue)
    mesProduits.append({"REF-001", "Sac Voyage Cuir", 120.50, "Hiver 2026", "Vachette Pleine Fleur", 5});
    mesProduits.append({"REF-002", "Portefeuille Luxe", 45.00, "Intemporel", "Agneau Plongé", 2});
    mesProduits.append({"REF-003", "Ceinture Homme", 25.00, "Été 2026", "Vachette Pleine Fleur", 1});

    // Employés (RH)
    mesEmployes.append({"EMP-001", "Dupont", "Jean", "Chef Atelier", "Production", QDate(2020, 5, 10), 2800.0, "RF-123"});
    mesEmployes.append({"EMP-002", "Martin", "Sophie", "Artisan Maroquinier", "Production", QDate(2022, 1, 15), 2200.0, "RF-456"});
    mesEmployes.append({"EMP-003", "Lefevre", "Paul", "Logistique", "Logistique", QDate(2023, 8, 20), 1900.0, "RF-789"});

    // Initialisation des tableaux
    rafraichirListeCommandes();
    configurerTimelineGantt();
    rafraichirListeProduits();
    rafraichirListeEmployes();

    // Démarrage sur l'accueil
    ui->stackedWidget->setCurrentWidget(ui->page_home);

    // --- 3. NAVIGATION (MENU GAUCHE) ---
    connect(ui->btn_nav_dashboard, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_home); });
    connect(ui->btn_nav_planif, &QPushButton::clicked, [=](){ rafraichirListeCommandes(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_nav_fab, &QPushButton::clicked, [=](){ configurerTimelineGantt(); ui->stackedWidget->setCurrentWidget(ui->page_fab_list); });
    connect(ui->btn_nav_produit, &QPushButton::clicked, [=](){ rafraichirListeProduits(); ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_nav_rh, &QPushButton::clicked, [=](){ rafraichirListeEmployes(); ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });

    // Bouton Accueil
    connect(ui->btn_start_app, &QPushButton::clicked, [=](){ rafraichirListeCommandes(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });

    // BOUTONS RETOUR (Navigation interne)
    connect(ui->btn_back_planif, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_back_fab, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_fab_list); });
    connect(ui->btn_back_stats, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });

    connect(ui->btn_back_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_back_stats_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });

    connect(ui->btn_back_emp, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });
    connect(ui->btn_back_stats_emp, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_employe_list); });


    // ==========================================
    // === MODULE 1 : PLANIFICATION & STATS ===
    // ==========================================

    // Correction Bouton Stats Planification
    connect(ui->btn_stat_plan, &QPushButton::clicked, [=](){
        calculerEtAfficherStats();
        ui->stackedWidget->setCurrentWidget(ui->page_stats);
    });

    // PDF Planification
    connect(ui->btn_pdf, &QPushButton::clicked, [=](){
        exporterPDF(ui->tablePlanif, "Planning de Production - FIL D'OR");
    });

    // Formulaire Nouveau / Modif
    auto preparerNouveauPlanif = [=](){
        modeModification = false; indexModification = -1;
        ui->gb_form->setTitle("Nouvelle Commande");
        ui->btn_valider_planif->setText("CONFIRMER CRÉATION");
        ui->sb_qte->setValue(0); ui->le_fin_prevue->clear(); ui->le_duree_totale->clear();
        ui->dt_lancement->setDate(QDate::currentDate());
        ui->stackedWidget->setCurrentWidget(ui->page_planif_form);
    };
    connect(ui->btn_to_add_planif, &QPushButton::clicked, preparerNouveauPlanif);

    connect(ui->btn_modifier_planif, &QPushButton::clicked, [=](){
        int row = ui->tablePlanif->currentRow();
        if(row < 0 || row >= mesCommandes.size()) { QMessageBox::warning(this, "Erreur", "Sélectionnez une ligne."); return; }
        modeModification = true; indexModification = row;
        CommandeInfo cmd = mesCommandes[row];
        ui->cb_produit->setCurrentText(cmd.produit); ui->sb_qte->setValue(cmd.quantite);
        ui->cb_matiere->setCurrentText(cmd.matiere); ui->dt_lancement->setDate(cmd.dateDebut);
        ui->le_fin_prevue->setText(cmd.dateFinEstimee);
        ui->gb_form->setTitle("Modifier l'OF : " + cmd.id);
        ui->btn_valider_planif->setText("ENREGISTRER");
        ui->stackedWidget->setCurrentWidget(ui->page_planif_form);
    });

    connect(ui->btn_valider_planif, &QPushButton::clicked, [=](){
        QString prod = ui->cb_produit->currentText(); int qte = ui->sb_qte->value(); QString mat = ui->cb_matiere->currentText();
        QDate dateD = ui->dt_lancement->date(); QString dateF = ui->le_fin_prevue->text();
        if(modeModification) {
            if(indexModification >= 0) {
                mesCommandes[indexModification].produit = prod; mesCommandes[indexModification].quantite = qte;
                mesCommandes[indexModification].matiere = mat; mesCommandes[indexModification].dateDebut = dateD;
                mesCommandes[indexModification].dateFinEstimee = dateF;
                QMessageBox::information(this, "Succès", "Modifié !");
            }
        } else {
            CommandeInfo c; c.id = "OF-" + QString::number(100 + mesCommandes.size() + 1);
            c.produit = prod; c.quantite = qte; c.matiere = mat; c.dateDebut = dateD; c.dateFinEstimee = dateF; c.statut = "Planifié"; c.etatEtape = 0;
            mesCommandes.append(c); QMessageBox::information(this, "Succès", "Créé !");
        }
        ui->stackedWidget->setCurrentWidget(ui->page_planif_list); rafraichirListeCommandes(); configurerTimelineGantt();
    });

    connect(ui->btn_calculer_ia, &QPushButton::clicked, [=](){
        int qte = ui->sb_qte->value();
        if(qte > 0) { int jours = (qte * 2) / 8; if(jours < 1) jours = 1; ui->le_fin_prevue->setText(ui->dt_lancement->date().addDays(jours).toString("dd/MM/yyyy")); }
    });

    connect(ui->btn_supprimer_planif, &QPushButton::clicked, [=](){
        int r = ui->tablePlanif->currentRow();
        if(r >= 0) { mesCommandes.removeAt(r); rafraichirListeCommandes(); configurerTimelineGantt(); }
    });

    // ==========================================
    // === MODULE 2 : PRODUITS ===
    // ==========================================
    connect(ui->btn_stats_prod, &QPushButton::clicked, [=](){ calculerStatsProduits(); ui->stackedWidget->setCurrentWidget(ui->page_stats_prod); });

    // PDF Catalogue
    connect(ui->btn_pdf_catalogue, &QPushButton::clicked, [=](){
        exporterPDF(ui->tableProduits, "Catalogue Officiel 2026 - FIL D'OR");
    });

    connect(ui->btn_add_produit, &QPushButton::clicked, [=](){
        modeModifProd = false; indexModifProd = -1; ui->gb_prod->setTitle("Nouveau Produit"); ui->btn_valider_produit->setText("AJOUTER");
        ui->le_ref_prod->clear(); ui->stackedWidget->setCurrentWidget(ui->page_produit_form);
    });
    connect(ui->btn_edit_produit, &QPushButton::clicked, [=](){
        int r = ui->tableProduits->currentRow();
        if(r < 0) return; modeModifProd = true; indexModifProd = r; ProduitInfo p = mesProduits[r];
        ui->le_ref_prod->setText(p.ref); ui->le_nom_prod->setText(p.nom);
        ui->stackedWidget->setCurrentWidget(ui->page_produit_form);
    });
    connect(ui->btn_valider_produit, &QPushButton::clicked, [=](){
        QString ref = ui->le_ref_prod->text(); if(ref.isEmpty()) return;
        if(modeModifProd) { mesProduits[indexModifProd].ref = ref; mesProduits[indexModifProd].nom = ui->le_nom_prod->text(); mesProduits[indexModifProd].coutMatiere = ui->sb_cout_prod->value(); mesProduits[indexModifProd].collection = ui->cb_coll_prod->currentText(); mesProduits[indexModifProd].cuir = ui->cb_cuir_prod->currentText(); mesProduits[indexModifProd].temps = ui->sb_temps_prod->value(); }
        else { ProduitInfo p; p.ref=ref; p.nom=ui->le_nom_prod->text(); p.coutMatiere=ui->sb_cout_prod->value(); p.collection=ui->cb_coll_prod->currentText(); p.cuir=ui->cb_cuir_prod->currentText(); p.temps=ui->sb_temps_prod->value(); mesProduits.append(p); }
        ui->stackedWidget->setCurrentWidget(ui->page_produit_list); rafraichirListeProduits();
    });
    connect(ui->btn_delete_produit, &QPushButton::clicked, [=](){
        int r = ui->tableProduits->currentRow(); if(r >= 0) { mesProduits.removeAt(r); rafraichirListeProduits(); }
    });

    // Innovation Calcul Coût
    auto updateCout = [=](){
        double matiere = ui->sb_cout_prod->value(); int heures = ui->sb_temps_prod->value();
        double total = matiere + (heures * 25.0);
        ui->lbl_cout_total->setText(QString::number(total, 'f', 2) + " €");
    };
    connect(ui->sb_cout_prod, &QDoubleSpinBox::textChanged, updateCout);
    connect(ui->sb_temps_prod, &QSpinBox::textChanged, updateCout);

    connect(ui->btn_innovation_mode, &QPushButton::clicked, [=](){ QMessageBox::information(this, "Tendances", "🔮 TENDANCES :\n2026 : Cuirs exotiques."); });
    connect(ui->btn_sort_price, &QPushButton::clicked, [=](){
        std::sort(mesProduits.begin(), mesProduits.end(), [](const ProduitInfo &a, const ProduitInfo &b){ return a.coutMatiere < b.coutMatiere; }); rafraichirListeProduits();
    });
    connect(ui->btn_search_col, &QPushButton::clicked, [=](){
        QString f = ui->le_search_coll->text().toLower();
        for(int i=0; i<ui->tableProduits->rowCount(); i++) ui->tableProduits->setRowHidden(i, !ui->tableProduits->item(i, 3)->text().toLower().contains(f));
    });


    // ==========================================
    // === MODULE 3 : RESSOURCES HUMAINES ===
    // ==========================================

    connect(ui->btn_add_emp, &QPushButton::clicked, [=](){
        modeModifEmp = false; indexModifEmp = -1; ui->gb_emp->setTitle("Recrutement"); ui->btn_valider_emp->setText("CONFIRMER RECRUTEMENT");
        ui->le_id_emp->clear(); ui->le_nom_emp->clear(); ui->le_prenom_emp->clear(); ui->sb_salaire_emp->setValue(0); ui->le_rfid_emp->clear(); ui->lbl_score_result->setText("Score : -");
        ui->stackedWidget->setCurrentWidget(ui->page_employe_form);
    });

    connect(ui->btn_edit_emp, &QPushButton::clicked, [=](){
        int r = ui->tableEmployes->currentRow();
        if(r < 0) { QMessageBox::warning(this, "Erreur", "Sélectionnez un employé."); return; }
        modeModifEmp = true; indexModifEmp = r;
        EmployeInfo e = mesEmployes[r];
        ui->le_id_emp->setText(e.id); ui->le_nom_emp->setText(e.nom); ui->le_prenom_emp->setText(e.prenom);
        ui->cb_poste_emp->setCurrentText(e.poste); ui->cb_dept_emp->setCurrentText(e.departement);
        ui->dt_embauche->setDate(e.dateEmbauche); ui->sb_salaire_emp->setValue(e.salaire);
        ui->le_rfid_emp->setText(e.rfid);
        ui->gb_emp->setTitle("Modifier : " + e.nom); ui->btn_valider_emp->setText("ENREGISTRER");
        ui->stackedWidget->setCurrentWidget(ui->page_employe_form);
    });

    connect(ui->btn_valider_emp, &QPushButton::clicked, [=](){
        if(ui->le_nom_emp->text().isEmpty()) { QMessageBox::warning(this, "Erreur", "Nom requis"); return; }
        EmployeInfo e;
        e.id = ui->le_id_emp->text(); e.nom = ui->le_nom_emp->text(); e.prenom = ui->le_prenom_emp->text();
        e.poste = ui->cb_poste_emp->currentText(); e.departement = ui->cb_dept_emp->currentText();
        e.dateEmbauche = ui->dt_embauche->date(); e.salaire = ui->sb_salaire_emp->value(); e.rfid = ui->le_rfid_emp->text();

        if(modeModifEmp) { mesEmployes[indexModifEmp] = e; QMessageBox::information(this,"Succès","Modifié !"); }
        else { mesEmployes.append(e); QMessageBox::information(this,"Succès","Recruté !"); }
        ui->stackedWidget->setCurrentWidget(ui->page_employe_list); rafraichirListeEmployes();
    });

    connect(ui->btn_delete_emp, &QPushButton::clicked, [=](){
        int r = ui->tableEmployes->currentRow(); if(r >= 0 && QMessageBox::question(this,"Supprimer","Confirmer ?") == QMessageBox::Yes) { mesEmployes.removeAt(r); rafraichirListeEmployes(); }
    });

    connect(ui->btn_calcul_score, &QPushButton::clicked, [=](){
        double score = (ui->sb_salaire_emp->value() / 1000.0) * 1.5;
        int annees = ui->dt_embauche->date().daysTo(QDate::currentDate()) / 365;
        score += annees;
        QString appreciation = (score > 8) ? "EXCELLENT" : (score > 5) ? "BON" : "JUNIOR";
        ui->lbl_score_result->setText(QString::number(score, 'f', 1) + " / 10 -> " + appreciation);
    });

    // Tri Ancienneté AVEC EMOJIS
    connect(ui->btn_sort_anciennete, &QPushButton::clicked, [=](){
        static bool triDecroissant = true;
        if(triDecroissant) {
            std::sort(mesEmployes.begin(), mesEmployes.end(), [](const EmployeInfo &a, const EmployeInfo &b){
                return a.dateEmbauche > b.dateEmbauche;
            });
            ui->btn_sort_anciennete->setText("📅 Ancienneté ▼"); // Flèche Bas
        } else {
            std::sort(mesEmployes.begin(), mesEmployes.end(), [](const EmployeInfo &a, const EmployeInfo &b){
                return a.dateEmbauche < b.dateEmbauche;
            });
            ui->btn_sort_anciennete->setText("📅 Ancienneté ▲"); // Flèche Haut
        }
        triDecroissant = !triDecroissant;
        rafraichirListeEmployes();
    });

    // PDF RH
    connect(ui->btn_pdf_emp, &QPushButton::clicked, [=](){
        exporterPDF(ui->tableEmployes, "Registre du Personnel - FIL D'OR");
    });

    // Chatbot RH
    connect(ui->btn_stats_emp, &QPushButton::clicked, [=](){
        ui->val_tot_emp->setText(QString::number(mesEmployes.size()));
        double masse = 0; for(auto e:mesEmployes) masse+=e.salaire;
        ui->val_masse_sal->setText(QString::number(masse) + " €");
        ui->stackedWidget->setCurrentWidget(ui->page_stats_emp);
    });
    connect(ui->btn_send_chat, &QPushButton::clicked, [=](){ reponseChatbot(); });
    connect(ui->le_chat_input, &QLineEdit::returnPressed, [=](){ reponseChatbot(); });


    // ==========================================
    // === MODULE 4 : FABRICATION (Supervision) ===
    // ==========================================
    connect(ui->tableTimeline, &QTableWidget::cellClicked, [=](int row, int col){
        if(row < 0) return; indexCommandeSelectionnee = row;
        ui->lbl_sel_cmd->setText(mesCommandes[row].id); ui->lbl_sel_prod->setText(mesCommandes[row].produit);
        ui->lbl_resultat_delta->setText("..."); ui->lbl_resultat_delta->setStyleSheet("color: gray;"); ui->sb_temps_reel_input->setValue(0);
    });
    connect(ui->cb_etape_suivi, &QComboBox::currentTextChanged, [=](const QString &text){
        double temps = 2.0; if(text == "Assemblage") temps=5.0; else if(text=="Couture") temps=4.0; else if(text=="Finition") temps=1.0;
        ui->lbl_temps_prevu->setText(QString::number(temps)+" h");
    });
    connect(ui->btn_valider_etape, &QPushButton::clicked, [=](){
        if(indexCommandeSelectionnee == -1) return;
        double reel = ui->sb_temps_reel_input->value(); double prevu = ui->lbl_temps_prevu->text().split(" ")[0].toDouble();
        mesCommandes[indexCommandeSelectionnee].etapeAuditee = ui->cb_etape_suivi->currentText();
        if(reel > prevu) { ui->lbl_resultat_delta->setText("RETARD"); ui->lbl_resultat_delta->setStyleSheet("background-color: #bf360c; color: white; padding:5px;"); mesCommandes[indexCommandeSelectionnee].etatEtape = 2; }
        else { ui->lbl_resultat_delta->setText("OK"); ui->lbl_resultat_delta->setStyleSheet("background-color: #2e7d32; color: white; padding:5px;"); mesCommandes[indexCommandeSelectionnee].etatEtape = 1; }
        configurerTimelineGantt();
    });
    connect(ui->btn_refresh_timeline, &QPushButton::clicked, [=](){ configurerTimelineGantt(); });
}

MainWindow::~MainWindow() { delete ui; }

// --- FONCTIONS LOGIQUES ---

void MainWindow::reponseChatbot() {
    QString q = ui->le_chat_input->text().toLower();
    QString rep = "Je ne comprends pas. Essayez 'effectif', 'salaire' ou 'doyen'.";
    if(q.contains("bonjour")) rep = "Bonjour ! Je suis l'assistant RH.";
    else if(q.contains("effectif") || q.contains("combien")) rep = "Il y a " + QString::number(mesEmployes.size()) + " employés.";
    else if(q.contains("salaire") || q.contains("cout")) { double t=0; for(auto e:mesEmployes) t+=e.salaire; rep = "Masse salariale : " + QString::number(t) + " €."; }
    else if(q.contains("doyen") || q.contains("ancien")) {
        if(!mesEmployes.isEmpty()) {
            EmployeInfo d = mesEmployes[0];
            for(auto e : mesEmployes) if(e.dateEmbauche < d.dateEmbauche) d = e;
            rep = "Le doyen est " + d.nom + " (Embauché en " + d.dateEmbauche.toString("yyyy") + ").";
        }
    }
    ui->txt_chat_history->append("<b>Vous:</b> " + ui->le_chat_input->text());
    ui->txt_chat_history->append("<b>Bot:</b> <span style='color:#8d5524'>" + rep + "</span><br>");
    ui->le_chat_input->clear();
}

void MainWindow::rafraichirListeEmployes() {
    ui->tableEmployes->setRowCount(0); ui->tableEmployes->setColumnCount(7);
    ui->tableEmployes->setHorizontalHeaderLabels({"ID", "Nom", "Prénom", "Poste", "Dépt", "Salaire", "Embauche"});
    for(int i=0; i<mesEmployes.size(); i++) {
        ui->tableEmployes->insertRow(i);
        ui->tableEmployes->setItem(i, 0, new QTableWidgetItem(mesEmployes[i].id));
        ui->tableEmployes->setItem(i, 1, new QTableWidgetItem(mesEmployes[i].nom));
        ui->tableEmployes->setItem(i, 2, new QTableWidgetItem(mesEmployes[i].prenom));
        ui->tableEmployes->setItem(i, 3, new QTableWidgetItem(mesEmployes[i].poste));
        ui->tableEmployes->setItem(i, 4, new QTableWidgetItem(mesEmployes[i].departement));
        ui->tableEmployes->setItem(i, 5, new QTableWidgetItem(QString::number(mesEmployes[i].salaire) + " €"));
        ui->tableEmployes->setItem(i, 6, new QTableWidgetItem(mesEmployes[i].dateEmbauche.toString("dd/MM/yyyy")));
    }
}

void MainWindow::rafraichirListeProduits() {
    ui->tableProduits->setRowCount(0); ui->tableProduits->setColumnCount(6);
    ui->tableProduits->setHorizontalHeaderLabels({"Réf", "Nom", "Coût", "Coll", "Cuir", "Temps"});
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
    int t=mesProduits.size(); ui->val_tot_ref->setText(QString::number(t));
    double c=0; int h=0,e=0,in=0;
    for(auto p:mesProduits) { c+=p.coutMatiere; if(p.collection.contains("Hiver")) h++; else if(p.collection.contains("Et")) e++; else in++; }
    ui->val_avg_cost->setText(QString::number(t>0?c/t:0,'f',2)+" €");
    ui->pb_hiver->setMaximum(t); ui->pb_hiver->setValue(h);
    ui->pb_ete->setMaximum(t); ui->pb_ete->setValue(e);
    ui->pb_intemp->setMaximum(t); ui->pb_intemp->setValue(in);
}

void MainWindow::calculerEtAfficherStats() {
    int total = mesCommandes.size();
    ui->lbl_stat_total_cmd->setText(QString::number(total));
    int qty=0; int ret=0; int s=0, p=0, c=0;
    for(auto m : mesCommandes) {
        qty += m.quantite;
        if(m.etatEtape == 2) ret++;
        if(m.produit.contains("Sac")) s++; else if(m.produit.contains("Portefeuille")) p++; else c++;
    }
    ui->lbl_stat_total_qty->setText(QString::number(qty));
    ui->lbl_stat_retard->setText(QString::number(total>0?(ret*100.0/total):0, 'f', 1) + " %");
    ui->pb_sac->setMaximum(total); ui->pb_sac->setValue(s);
    ui->pb_portefeuille->setMaximum(total); ui->pb_portefeuille->setValue(p);
    ui->pb_ceinture->setMaximum(total); ui->pb_ceinture->setValue(c);
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

void MainWindow::configurerTimelineGantt() {
    QTableWidget *t = ui->tableTimeline; t->clear(); t->setRowCount(0);
    int jours = 30; t->setColumnCount(1+jours); QStringList h; h << "PROD";
    for(int i=0; i<jours; i++) h << QDate::currentDate().addDays(i).toString("dd");
    t->setHorizontalHeaderLabels(h); t->setColumnWidth(0, 200); for(int i=1;i<=jours;i++) t->setColumnWidth(i,35);
    t->setRowCount(mesCommandes.size());
    for(int i=0; i<mesCommandes.size(); i++) {
        t->setItem(i,0,new QTableWidgetItem(mesCommandes[i].id)); t->setRowHeight(i,50);
        int off = QDate::currentDate().daysTo(mesCommandes[i].dateDebut); if(off<0) off=0;
        QColor c1(41,128,185), c2(211,84,0), c3(39,174,96), c4(241,196,15);
        if(mesCommandes[i].etatEtape==2) { QColor r(192,57,43); if(mesCommandes[i].etapeAuditee=="Coupe")c1=r; else if(mesCommandes[i].etapeAuditee=="Assemblage")c2=r; else if(mesCommandes[i].etapeAuditee=="Couture")c3=r; else c4=r; }
        else if(mesCommandes[i].etatEtape==1) { QColor o(46,204,113); if(mesCommandes[i].etapeAuditee=="Coupe")c1=o; else if(mesCommandes[i].etapeAuditee=="Assemblage")c2=o; else if(mesCommandes[i].etapeAuditee=="Couture")c3=o; else c4=o; }
        if(off<jours) dessinerBarre(i,off+1,2,"Cp",c1,Qt::white);
        if(off+2<jours) dessinerBarre(i,off+3,3,"As",c2,Qt::white);
        if(off+5<jours) dessinerBarre(i,off+6,3,"Co",c3,Qt::white);
        if(off+8<jours) dessinerBarre(i,off+9,2,"Fi",c4,Qt::black);
    }
}

void MainWindow::dessinerBarre(int r, int c, int d, QString txt, QColor bg, QColor fg) {
    if(c+d > ui->tableTimeline->columnCount()) return;
    QTableWidgetItem *it = new QTableWidgetItem(txt);
    it->setData(Qt::BackgroundRole, bg); it->setForeground(fg); it->setTextAlignment(Qt::AlignCenter);
    ui->tableTimeline->setItem(r, c, it); if(d>1) ui->tableTimeline->setSpan(r,c,1,d);
}

// FONCTION PDF GENERIQUE (Compatible Qt6)
void MainWindow::exporterPDF(QTableWidget *table, QString titreDocument)
{
    // 1. Choisir où sauvegarder
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", QString(), "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty()) fileName.append(".pdf");

    // 2. Configurer l'imprimante PDF (SYNTAXE QT 6)
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);

    // CORRECTION QT6 : Utilisation de QPageSize
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(fileName);

    // 3. Dessiner (Peindre) le PDF
    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::warning(this, "Erreur", "Impossible d'écrire le fichier PDF.");
        return;
    }

    // --- DESIGN DU PDF (ADAPTÉ QT 6) ---
    // CORRECTION QT6 : Récupération des dimensions via pageLayout
    QRectF pageRect = printer.pageLayout().paintRectPixels(printer.resolution());
    double pageWidth = pageRect.width();
    double pageHeight = pageRect.height();

    int y = 100; // Position verticale de départ

    // Titre
    painter.setPen(QColor(62, 39, 35)); // Marron foncé
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    painter.drawText(0, y, pageWidth, 40, Qt::AlignCenter, titreDocument);
    y += 100;

    // Date
    painter.setFont(QFont("Arial", 10));
    painter.drawText(50, y, "Généré le : " + QDate::currentDate().toString("dd/MM/yyyy"));
    y += 50;

    // Configuration Tableau
    int rows = table->rowCount();
    int cols = table->columnCount();

    // Largeur dynamique des colonnes
    double colWidth = (pageWidth - 100) / (cols > 0 ? cols : 1);
    int rowHeight = 40;
    int xStart = 50;

    // Dessin En-têtes
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.setBrush(QColor(141, 85, 36)); // Fond Doré/Marron
    painter.setPen(Qt::white);

    for(int c=0; c<cols; c++) {
        QRectF rect(xStart + (c*colWidth), y, colWidth, rowHeight);
        painter.drawRect(rect); // Fond
        if(table->horizontalHeaderItem(c))
            painter.drawText(rect, Qt::AlignCenter, table->horizontalHeaderItem(c)->text());
    }
    y += rowHeight;

    // Dessin Données
    painter.setFont(QFont("Arial", 9));
    painter.setBrush(Qt::NoBrush); // Plus de fond
    painter.setPen(Qt::black);

    for(int r=0; r<rows; r++) {
        // Vérifier si on dépasse la page (Saut de page basique)
        if (y > pageHeight - 100) {
            printer.newPage();
            y = 50; // On reprend en haut
        }

        for(int c=0; c<cols; c++) {
            QRectF rect(xStart + (c*colWidth), y, colWidth, rowHeight);
            painter.drawRect(rect); // Bordure case
            if(table->item(r, c))
                painter.drawText(rect, Qt::AlignCenter, table->item(r, c)->text());
        }
        y += rowHeight;
    }

    painter.end();
    QMessageBox::information(this, "Succès", "Fichier PDF généré avec succès !\n" + fileName);
}
