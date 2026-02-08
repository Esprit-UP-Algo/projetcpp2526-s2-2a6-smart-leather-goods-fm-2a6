#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDate>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <algorithm> // Pour std::sort

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- LOGOS ---
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

    // --- DONNÉES TEST PLANIF ---
    mesCommandes.append({"OF-101", "Sac Voyage Cuir", 50, "Cuir Vachette", QDate::currentDate(), "10/02/2026", "En cours", "", 0});
    mesCommandes.append({"OF-102", "Portefeuille Luxe", 120, "Cuir Agneau", QDate::currentDate().addDays(1), "12/02/2026", "Planifié", "", 0});

    // --- DONNÉES TEST PRODUITS ---
    mesProduits.append({"REF-001", "Sac Voyage Cuir", 120.50, "Hiver 2026", "Vachette Pleine Fleur", 5});
    mesProduits.append({"REF-002", "Portefeuille Luxe", 45.00, "Intemporel", "Agneau Plongé", 2});
    mesProduits.append({"REF-003", "Ceinture Homme", 25.00, "Été 2026", "Vachette Pleine Fleur", 1});

    rafraichirListeCommandes();
    configurerTimelineGantt();
    rafraichirListeProduits();

    // --- NAVIGATION ---
    ui->stackedWidget->setCurrentWidget(ui->page_home);

    connect(ui->btn_nav_dashboard, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_home); });
    connect(ui->btn_nav_planif, &QPushButton::clicked, [=](){
        rafraichirListeCommandes(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list);
    });
    connect(ui->btn_nav_fab, &QPushButton::clicked, [=](){
        configurerTimelineGantt(); ui->stackedWidget->setCurrentWidget(ui->page_fab_list);
    });
    // Nav Produit
    connect(ui->btn_nav_produit, &QPushButton::clicked, [=](){
        rafraichirListeProduits(); ui->stackedWidget->setCurrentWidget(ui->page_produit_list);
    });

    connect(ui->btn_start_app, &QPushButton::clicked, [=](){
        rafraichirListeCommandes(); ui->stackedWidget->setCurrentWidget(ui->page_planif_list);
    });

    // Boutons Retour
    connect(ui->btn_back_planif, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_back_prod, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_produit_list); });
    connect(ui->btn_back_fab, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_fab_list); });
    connect(ui->btn_back_stats, &QPushButton::clicked, [=](){ ui->stackedWidget->setCurrentWidget(ui->page_planif_list); });
    connect(ui->btn_stat_plan, &QPushButton::clicked, [=](){ calculerEtAfficherStats(); ui->stackedWidget->setCurrentWidget(ui->page_stats); });

    // ==========================================
    // === MODULE PRODUITS (LOGIQUE) ===
    // ==========================================

    // 1. Ouvrir Formulaire Ajout
    connect(ui->btn_add_produit, &QPushButton::clicked, [=](){
        modeModifProd = false;
        indexModifProd = -1;
        ui->gb_prod->setTitle("Nouveau Produit");
        ui->btn_valider_produit->setText("AJOUTER AU CATALOGUE");
        // Reset
        ui->le_ref_prod->clear();
        ui->le_nom_prod->clear();
        ui->sb_cout_prod->setValue(0);
        ui->sb_temps_prod->setValue(0);
        ui->lbl_cout_total->setText("0.00 €");
        ui->stackedWidget->setCurrentWidget(ui->page_produit_form);
    });

    // 2. Ouvrir Formulaire Modif
    connect(ui->btn_edit_produit, &QPushButton::clicked, [=](){
        int row = ui->tableProduits->currentRow();
        if(row < 0) { QMessageBox::warning(this, "Erreur", "Sélectionnez un produit."); return; }

        modeModifProd = true;
        indexModifProd = row;
        ProduitInfo p = mesProduits[row];

        ui->le_ref_prod->setText(p.ref);
        ui->le_nom_prod->setText(p.nom);
        ui->sb_cout_prod->setValue(p.coutMatiere);
        ui->cb_coll_prod->setCurrentText(p.collection);
        ui->cb_cuir_prod->setCurrentText(p.cuir);
        ui->sb_temps_prod->setValue(p.temps);

        ui->gb_prod->setTitle("Modifier : " + p.nom);
        ui->btn_valider_produit->setText("ENREGISTRER MODIFICATIONS");
        ui->stackedWidget->setCurrentWidget(ui->page_produit_form);
    });

    // 3. Valider (Ajout ou Modif)
    connect(ui->btn_valider_produit, &QPushButton::clicked, [=](){
        QString ref = ui->le_ref_prod->text();
        QString nom = ui->le_nom_prod->text();
        if(ref.isEmpty() || nom.isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Référence et Nom obligatoires."); return;
        }

        if(modeModifProd) {
            mesProduits[indexModifProd].ref = ref;
            mesProduits[indexModifProd].nom = nom;
            mesProduits[indexModifProd].coutMatiere = ui->sb_cout_prod->value();
            mesProduits[indexModifProd].collection = ui->cb_coll_prod->currentText();
            mesProduits[indexModifProd].cuir = ui->cb_cuir_prod->currentText();
            mesProduits[indexModifProd].temps = ui->sb_temps_prod->value();
            QMessageBox::information(this, "Succès", "Produit modifié !");
        } else {
            ProduitInfo p;
            p.ref = ref; p.nom = nom;
            p.coutMatiere = ui->sb_cout_prod->value();
            p.collection = ui->cb_coll_prod->currentText();
            p.cuir = ui->cb_cuir_prod->currentText();
            p.temps = ui->sb_temps_prod->value();
            mesProduits.append(p);
            QMessageBox::information(this, "Succès", "Produit ajouté !");
        }
        ui->stackedWidget->setCurrentWidget(ui->page_produit_list);
        rafraichirListeProduits();
    });

    // 4. Supprimer
    connect(ui->btn_delete_produit, &QPushButton::clicked, [=](){
        int r = ui->tableProduits->currentRow();
        if(r >= 0 && QMessageBox::question(this,"Supprimer","Confirmer ?") == QMessageBox::Yes) {
            mesProduits.removeAt(r);
            rafraichirListeProduits();
        }
    });

    // 5. INNOVATION 1 : Calcul Coût Automatique
    auto updateCout = [=](){
        double matiere = ui->sb_cout_prod->value();
        int heures = ui->sb_temps_prod->value();
        double tauxHoraire = 25.0; // 25€ de l'heure
        double total = matiere + (heures * tauxHoraire);
        ui->lbl_cout_total->setText(QString::number(total, 'f', 2) + " €");
    };
    connect(ui->sb_cout_prod, &QDoubleSpinBox::textChanged, updateCout);
    connect(ui->sb_temps_prod, &QSpinBox::textChanged, updateCout);

    // 6. INNOVATION 2 : Historique Mode
    connect(ui->btn_innovation_mode, &QPushButton::clicked, [=](){
        QMessageBox::information(this, "Tendances & Historique",
                                 "🔮 TENDANCES FIL D'OR :\n\n"
                                 "• 2024 : Retour du cuir grainé et couleurs terre.\n"
                                 "• 2025 : Minimalisme, boucles dorées fines.\n"
                                 "• 2026 (Prévu) : Cuirs exotiques et formats mini.\n\n"
                                 "💡 Conseil IA : Augmentez la production de 'Sac Voyage' pour l'hiver.");
    });

    // 7. Tri par Prix
    connect(ui->btn_sort_price, &QPushButton::clicked, [=](){
        std::sort(mesProduits.begin(), mesProduits.end(), [](const ProduitInfo &a, const ProduitInfo &b){
            return a.coutMatiere < b.coutMatiere;
        });
        rafraichirListeProduits();
    });

    // 8. Recherche Collection
    connect(ui->btn_search_col, &QPushButton::clicked, [=](){
        QString filter = ui->le_search_coll->text().toLower();
        for(int i=0; i<ui->tableProduits->rowCount(); i++) {
            bool match = ui->tableProduits->item(i, 3)->text().toLower().contains(filter);
            ui->tableProduits->setRowHidden(i, !match);
        }
    });

    // 9. PDF (Simulation)
    connect(ui->btn_pdf_catalogue, &QPushButton::clicked, [=](){
        QMessageBox::information(this, "Export PDF", "Catalogue exporté : 'Catalogue_FilDor_2026.pdf'");
    });


    // ==========================================
    // === MODULE PLANIFICATION (EXISTANT) ===
    // ==========================================

    auto preparerNouveauPlanif = [=](){
        modeModification = false;
        indexModification = -1;
        ui->gb_form->setTitle("Nouvelle Commande");
        ui->btn_valider_planif->setText("CONFIRMER CRÉATION");
        ui->sb_qte->setValue(0);
        ui->le_fin_prevue->clear();
        ui->le_duree_totale->clear();
        ui->dt_lancement->setDate(QDate::currentDate());
        ui->stackedWidget->setCurrentWidget(ui->page_planif_form);
    };

    connect(ui->btn_to_add_planif, &QPushButton::clicked, preparerNouveauPlanif);

    connect(ui->btn_modifier_planif, &QPushButton::clicked, [=](){
        int row = ui->tablePlanif->currentRow();
        if(row < 0 || row >= mesCommandes.size()) {
            QMessageBox::warning(this, "Erreur", "Sélectionnez une ligne."); return;
        }
        modeModification = true;
        indexModification = row;
        CommandeInfo cmd = mesCommandes[row];
        ui->cb_produit->setCurrentText(cmd.produit);
        ui->sb_qte->setValue(cmd.quantite);
        ui->cb_matiere->setCurrentText(cmd.matiere);
        ui->dt_lancement->setDate(cmd.dateDebut);
        ui->le_fin_prevue->setText(cmd.dateFinEstimee);
        ui->gb_form->setTitle("Modifier l'OF : " + cmd.id);
        ui->btn_valider_planif->setText("ENREGISTRER MODIFICATIONS");
        ui->stackedWidget->setCurrentWidget(ui->page_planif_form);
    });

    connect(ui->btn_valider_planif, &QPushButton::clicked, [=](){
        QString prod = ui->cb_produit->currentText();
        int qte = ui->sb_qte->value();
        QString mat = ui->cb_matiere->currentText();
        QDate dateD = ui->dt_lancement->date();
        QString dateF = ui->le_fin_prevue->text();

        if(modeModification) {
            if(indexModification >= 0 && indexModification < mesCommandes.size()) {
                mesCommandes[indexModification].produit = prod;
                mesCommandes[indexModification].quantite = qte;
                mesCommandes[indexModification].matiere = mat;
                mesCommandes[indexModification].dateDebut = dateD;
                mesCommandes[indexModification].dateFinEstimee = dateF;
                QMessageBox::information(this, "Succès", "Modifié !");
            }
        } else {
            CommandeInfo c;
            c.id = "OF-" + QString::number(100 + mesCommandes.size() + 1);
            c.produit = prod;
            c.quantite = qte;
            c.matiere = mat;
            c.dateDebut = dateD;
            c.dateFinEstimee = dateF;
            c.statut = "Planifié";
            c.etatEtape = 0;
            mesCommandes.append(c);
            QMessageBox::information(this, "Succès", "Créé !");
        }
        ui->stackedWidget->setCurrentWidget(ui->page_planif_list);
        rafraichirListeCommandes();
        configurerTimelineGantt();
    });

    connect(ui->btn_calculer_ia, &QPushButton::clicked, [=](){
        int qte = ui->sb_qte->value();
        if(qte > 0) {
            int jours = (qte * 2) / 8; if(jours < 1) jours = 1;
            ui->le_fin_prevue->setText(ui->dt_lancement->date().addDays(jours).toString("dd/MM/yyyy"));
            ui->le_duree_totale->setText(QString::number(jours) + " Jours");
        }
    });

    connect(ui->btn_supprimer_planif, &QPushButton::clicked, [=](){
        int r = ui->tablePlanif->currentRow();
        if(r >= 0) {
            if(QMessageBox::question(this,"Supprimer","Confirmer ?") == QMessageBox::Yes) {
                mesCommandes.removeAt(r);
                rafraichirListeCommandes();
                configurerTimelineGantt();
            }
        }
    });

    // --- SUPERVISION ---
    connect(ui->tableTimeline, &QTableWidget::cellClicked, [=](int row, int col){
        if(row < 0 || row >= mesCommandes.size()) return;
        indexCommandeSelectionnee = row;
        ui->lbl_sel_cmd->setText(mesCommandes[row].id);
        ui->lbl_sel_prod->setText(mesCommandes[row].produit);
        ui->lbl_resultat_delta->setText("...");
        ui->lbl_resultat_delta->setStyleSheet("color: gray;");
        ui->sb_temps_reel_input->setValue(0);
    });

    connect(ui->cb_etape_suivi, &QComboBox::currentTextChanged, [=](const QString &text){
        double temps = 2.0;
        if(text == "Coupe") temps = 2.0;
        else if(text == "Assemblage") temps = 5.0;
        else if(text == "Couture") temps = 4.0;
        else if(text == "Finition") temps = 1.0;
        ui->lbl_temps_prevu->setText(QString::number(temps) + " h");
    });

    connect(ui->btn_valider_etape, &QPushButton::clicked, [=](){
        if(indexCommandeSelectionnee == -1) return;
        double reel = ui->sb_temps_reel_input->value();
        double prevu = ui->lbl_temps_prevu->text().split(" ")[0].toDouble();
        mesCommandes[indexCommandeSelectionnee].etapeAuditee = ui->cb_etape_suivi->currentText();

        if(reel > prevu) {
            ui->lbl_resultat_delta->setText("RETARD");
            ui->lbl_resultat_delta->setStyleSheet("background-color: #bf360c; color: white; padding: 5px;");
            mesCommandes[indexCommandeSelectionnee].etatEtape = 2;
        } else {
            ui->lbl_resultat_delta->setText("OK");
            ui->lbl_resultat_delta->setStyleSheet("background-color: #2e7d32; color: white; padding: 5px;");
            mesCommandes[indexCommandeSelectionnee].etatEtape = 1;
        }
        configurerTimelineGantt();
    });

    connect(ui->btn_refresh_timeline, &QPushButton::clicked, [=](){ configurerTimelineGantt(); });
}

MainWindow::~MainWindow() { delete ui; }

// --- FONCTIONS ---

void MainWindow::rafraichirListeProduits() {
    ui->tableProduits->setRowCount(0);
    ui->tableProduits->setColumnCount(6);
    ui->tableProduits->setHorizontalHeaderLabels({"Référence", "Désignation", "Coût Mat.", "Collection", "Type Cuir", "Temps (h)"});

    for(int i=0; i<mesProduits.size(); i++) {
        ui->tableProduits->insertRow(i);
        ui->tableProduits->setItem(i, 0, new QTableWidgetItem(mesProduits[i].ref));
        ui->tableProduits->setItem(i, 1, new QTableWidgetItem(mesProduits[i].nom));
        ui->tableProduits->setItem(i, 2, new QTableWidgetItem(QString::number(mesProduits[i].coutMatiere) + " €"));
        ui->tableProduits->setItem(i, 3, new QTableWidgetItem(mesProduits[i].collection));
        ui->tableProduits->setItem(i, 4, new QTableWidgetItem(mesProduits[i].cuir));
        ui->tableProduits->setItem(i, 5, new QTableWidgetItem(QString::number(mesProduits[i].temps)));
    }
}

void MainWindow::calculerEtAfficherStats() {
    int total = mesCommandes.size();
    ui->lbl_stat_total_cmd->setText(QString::number(total));
    int qty=0; int ret=0; int s=0, p=0, c=0;
    for(auto m : mesCommandes) {
        qty += m.quantite;
        if(m.etatEtape == 2) ret++;
        if(m.produit.contains("Sac")) s++;
        else if(m.produit.contains("Portefeuille")) p++;
        else c++;
    }
    ui->lbl_stat_total_qty->setText(QString::number(qty));
    ui->lbl_stat_retard->setText(QString::number(total>0?(ret*100.0/total):0, 'f', 1) + " %");
    ui->pb_sac->setMaximum(total); ui->pb_sac->setValue(s);
    ui->pb_portefeuille->setMaximum(total); ui->pb_portefeuille->setValue(p);
    ui->pb_ceinture->setMaximum(total); ui->pb_ceinture->setValue(c);
}

void MainWindow::rafraichirListeCommandes() {
    ui->tablePlanif->setRowCount(0);
    ui->tablePlanif->setColumnCount(7);
    ui->tablePlanif->setHorizontalHeaderLabels({"ID","Produit","Qté","Matière","Début","Fin Est.","Statut"});
    for(int i=0; i<mesCommandes.size(); i++) {
        ui->tablePlanif->insertRow(i);
        ui->tablePlanif->setItem(i, 0, new QTableWidgetItem(mesCommandes[i].id));
        ui->tablePlanif->setItem(i, 1, new QTableWidgetItem(mesCommandes[i].produit));
        ui->tablePlanif->setItem(i, 2, new QTableWidgetItem(QString::number(mesCommandes[i].quantite)));
        ui->tablePlanif->setItem(i, 3, new QTableWidgetItem(mesCommandes[i].matiere));
        ui->tablePlanif->setItem(i, 4, new QTableWidgetItem(mesCommandes[i].dateDebut.toString("dd/MM/yyyy")));
        ui->tablePlanif->setItem(i, 5, new QTableWidgetItem(mesCommandes[i].dateFinEstimee));
        ui->tablePlanif->setItem(i, 6, new QTableWidgetItem(mesCommandes[i].statut));
    }
}

void MainWindow::configurerTimelineGantt() {
    QTableWidget *t = ui->tableTimeline;
    t->clear(); t->setRowCount(0);
    int jours = 30; t->setColumnCount(1+jours);
    QStringList h; h << "PRODUIT";
    for(int i=0; i<jours; i++) h << QDate::currentDate().addDays(i).toString("dd/MM");
    t->setHorizontalHeaderLabels(h);
    t->setColumnWidth(0, 200); for(int i=1; i<=jours; i++) t->setColumnWidth(i, 40);

    t->setRowCount(mesCommandes.size());
    for(int i=0; i<mesCommandes.size(); i++) {
        t->setItem(i, 0, new QTableWidgetItem(mesCommandes[i].id + "\n" + mesCommandes[i].produit));
        t->setRowHeight(i, 55);
        int off = QDate::currentDate().daysTo(mesCommandes[i].dateDebut);
        if(off < 0) off = 0;

        QColor c1(41,128,185), c2(211,84,0), c3(39,174,96), c4(241,196,15);
        if(mesCommandes[i].etatEtape == 2) {
            QColor r(192,57,43);
            if(mesCommandes[i].etapeAuditee == "Coupe") c1=r;
            else if(mesCommandes[i].etapeAuditee == "Assemblage") c2=r;
            else if(mesCommandes[i].etapeAuditee == "Couture") c3=r;
            else c4=r;
        } else if(mesCommandes[i].etatEtape == 1) {
            QColor ok(46, 204, 113);
            if(mesCommandes[i].etapeAuditee == "Coupe") c1=ok;
            else if(mesCommandes[i].etapeAuditee == "Assemblage") c2=ok;
            else if(mesCommandes[i].etapeAuditee == "Couture") c3=ok;
            else c4=ok;
        }

        if(off < jours) dessinerBarre(i, off+1, 2, "Coupe", c1, Qt::white);
        if(off+2 < jours) dessinerBarre(i, off+3, 3, "Assemb.", c2, Qt::white);
        if(off+5 < jours) dessinerBarre(i, off+6, 3, "Couture", c3, Qt::white);
        if(off+8 < jours) dessinerBarre(i, off+9, 2, "Finit.", c4, Qt::black);
    }
}

void MainWindow::dessinerBarre(int r, int c, int d, QString txt, QColor bg, QColor fg) {
    if(c+d > ui->tableTimeline->columnCount()) return;
    QTableWidgetItem *it = new QTableWidgetItem(txt);
    it->setData(Qt::BackgroundRole, bg);
    it->setForeground(fg);
    it->setTextAlignment(Qt::AlignCenter);
    ui->tableTimeline->setItem(r, c, it);
    if(d > 1) ui->tableTimeline->setSpan(r, c, 1, d);
}
