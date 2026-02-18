/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QFrame *frame_sidebar;
    QVBoxLayout *vl_nav;
    QHBoxLayout *logo_png;
    QLabel *l_logo_img;
    QLabel *l_app_title;
    QPushButton *btn_nav_dashboard;
    QPushButton *btn_nav_produit;
    QPushButton *btn_nav_stock;
    QPushButton *btn_nav_clients;
    QPushButton *btn_nav_depot;
    QPushButton *btn_nav_planif;
    QPushButton *btn_nav_fab;
    QPushButton *btn_nav_rh;
    QSpacerItem *vs;
    QStackedWidget *stackedWidget;
    QWidget *page_home;
    QVBoxLayout *vl_h;
    QLabel *lbl_big_logo;
    QSpacerItem *vs_h1;
    QLabel *lbl_welcome_title;
    QLabel *lbl_welcome_sub;
    QSpacerItem *vs_h2;
    QPushButton *btn_start_app;
    QSpacerItem *vs_h3;
    QWidget *page_login;
    QVBoxLayout *vl_login;
    QVBoxLayout *vl_login_center;
    QFrame *login_card;
    QVBoxLayout *vl_login_card;
    QLabel *lbl_login_title;
    QLabel *lbl_login_sub;
    QSpacerItem *vs_login_1;
    QLineEdit *le_login_nom;
    QLineEdit *le_login_prenom;
    QLineEdit *le_login_mdp;
    QHBoxLayout *hl_login_actions;
    QPushButton *btn_login_back;
    QSpacerItem *hs_login;
    QPushButton *btn_login;
    QSpacerItem *vs_login_2;
    QWidget *page_planif_list;
    QVBoxLayout *vl_p;
    QLabel *lbl_p1;
    QHBoxLayout *hl_p_tools;
    QLineEdit *le_recherche;
    QPushButton *btn_rechercher;
    QPushButton *btn_sort_alpha_planif;
    QPushButton *btn_open_planif_ia;
    QPushButton *btn_stat_plan;
    QPushButton *btn_pdf;
    QPushButton *btn_to_add_planif;
    QTableWidget *tablePlanif;
    QHBoxLayout *hl_bot_p;
    QPushButton *btn_tri;
    QSpacerItem *hs_bot;
    QPushButton *btn_modifier_planif;
    QPushButton *btn_supprimer_planif;
    QWidget *page_fab_list;
    QVBoxLayout *vl_f;
    QHBoxLayout *hl_f_top;
    QVBoxLayout *vl_titre_f;
    QLabel *l_tf;
    QLabel *l_sub_f;
    QSpacerItem *hs_f1;
    QLabel *l_legende_colors;
    QHBoxLayout *hl_tools_timeline;
    QPushButton *btn_refresh_timeline;
    QSpacerItem *hs_f2;
    QPushButton *btn_saisie_detail;
    QTableWidget *tableTimeline;
    QFrame *line_sep_fab;
    QFrame *frame_supervision;
    QHBoxLayout *hl_sup;
    QVBoxLayout *vl_sup_info;
    QLabel *lbl_sel_cmd_title;
    QLabel *lbl_sel_cmd;
    QFrame *line_sup;
    QFormLayout *fl_sup_saisie;
    QLabel *l_et;
    QComboBox *cb_etape_suivi;
    QLabel *l_tp;
    QLabel *lbl_temps_prevu;
    QLabel *l_tr;
    QDoubleSpinBox *sb_temps_reel_input;
    QPushButton *btn_valider_etape;
    QLabel *lbl_resultat_delta;
    QWidget *page_planif_form;
    QVBoxLayout *vl_pform;
    QPushButton *btn_back_planif;
    QGroupBox *gb_form;
    QFormLayout *fl_form;
    QLabel *l1;
    QComboBox *cb_produit;
    QLabel *l2;
    QSpinBox *sb_qte;
    QLabel *l3;
    QComboBox *cb_matiere;
    QLabel *l4;
    QDateTimeEdit *dt_lancement;
    QLabel *l5;
    QHBoxLayout *hl_ia;
    QLineEdit *le_fin_prevue;
    QPushButton *btn_calculer_ia;
    QLineEdit *le_duree_totale;
    QPushButton *btn_valider_planif;
    QSpacerItem *vs_f;
    QWidget *page_stats;
    QVBoxLayout *vl_stats;
    QPushButton *btn_back_stats;
    QLabel *lbl_title_s;
    QHBoxLayout *hl_stats_kpi;
    QFrame *st1;
    QVBoxLayout *v_s1;
    QLabel *ico_plan_1;
    QLabel *lbl_stat_total_cmd;
    QLabel *lbl_t1;
    QFrame *st2;
    QVBoxLayout *v_s2;
    QLabel *ico_plan_2;
    QLabel *lbl_stat_total_qty;
    QLabel *lbl_t2;
    QFrame *st3;
    QVBoxLayout *v_s3;
    QLabel *ico_plan_3;
    QLabel *lbl_stat_retard;
    QLabel *lbl_t3;
    QHBoxLayout *hl_charts_planif;
    QGroupBox *gb_charts;
    QFormLayout *fl_charts;
    QLabel *l_c1;
    QProgressBar *pb_sac;
    QLabel *l_c2;
    QProgressBar *pb_portefeuille;
    QLabel *l_c3;
    QProgressBar *pb_ceinture;
    QGroupBox *gb_charts_statut;
    QFormLayout *fl_statut;
    QLabel *l_cmd_plan;
    QProgressBar *pb_cmd_plan;
    QLabel *l_cmd_cours;
    QProgressBar *pb_cmd_cours;
    QLabel *l_cmd_ret;
    QProgressBar *pb_cmd_ret;
    QHBoxLayout *hl_actions_planif;
    QPushButton *btn_export_excel_planif;
    QPushButton *btn_print_planif;
    QSpacerItem *hs_actions_planif;
    QPushButton *btn_close_stats_planif;
    QSpacerItem *vs_stats;
    QWidget *page_produit_list;
    QVBoxLayout *vl_prod_list;
    QLabel *lbl_t_prod;
    QHBoxLayout *hl_tools_prod;
    QLineEdit *le_search_coll;
    QPushButton *btn_search_col;
    QPushButton *btn_sort_alpha_prod;
    QPushButton *btn_cout_produit;
    QPushButton *btn_hist_mode;
    QPushButton *btn_stats_prod;
    QPushButton *btn_pdf_catalogue;
    QPushButton *btn_add_produit;
    QTableWidget *tableProduits;
    QHBoxLayout *hl_bot_prod;
    QSpacerItem *hs_bp;
    QPushButton *btn_edit_produit;
    QPushButton *btn_delete_produit;
    QWidget *page_produit_form;
    QVBoxLayout *vl_prod_form;
    QPushButton *btn_back_prod;
    QGroupBox *gb_prod;
    QFormLayout *fl_prod;
    QLabel *lp1;
    QLineEdit *le_ref_prod;
    QLabel *lp2;
    QLineEdit *le_nom_prod;
    QLabel *lp3;
    QComboBox *cb_coll_prod;
    QLabel *lp4;
    QComboBox *cb_cuir_prod;
    QLabel *lp5;
    QDoubleSpinBox *sb_cout_prod;
    QLabel *lp6;
    QSpinBox *sb_temps_prod;
    QLabel *lp7;
    QLabel *lbl_cout_total;
    QPushButton *btn_valider_produit;
    QSpacerItem *vs_pf;
    QWidget *page_stats_prod;
    QVBoxLayout *vl_stats_p;
    QPushButton *btn_back_stats_prod;
    QLabel *lbl_title_sp;
    QHBoxLayout *hl_kpi_prod;
    QFrame *stp1;
    QVBoxLayout *vp1;
    QLabel *ico_prod_1;
    QLabel *val_tot_ref;
    QLabel *lbl_tot_ref;
    QFrame *stp2;
    QVBoxLayout *vp2;
    QLabel *ico_prod_2;
    QLabel *val_avg_cost;
    QLabel *lbl_avg_cost;
    QFrame *stp3;
    QVBoxLayout *vp3;
    QLabel *ico_prod_3;
    QLabel *val_tot_val_prod;
    QLabel *lbl_tot_val_prod;
    QHBoxLayout *hl_charts_prod;
    QGroupBox *gb_chart_coll;
    QFormLayout *fl_coll;
    QLabel *l_hiver;
    QProgressBar *pb_hiver;
    QLabel *l_ete;
    QProgressBar *pb_ete;
    QLabel *l_intemp;
    QProgressBar *pb_intemp;
    QGroupBox *gb_chart_cuir;
    QFormLayout *fl_cuir;
    QLabel *l_cuir_v;
    QProgressBar *pb_cuir_vachette;
    QLabel *l_cuir_a;
    QProgressBar *pb_cuir_agneau;
    QLabel *l_cuir_c;
    QProgressBar *pb_cuir_croco;
    QHBoxLayout *hl_actions_prod;
    QPushButton *btn_export_excel_prod;
    QPushButton *btn_print_prod;
    QSpacerItem *hs_actions_prod;
    QPushButton *btn_close_stats_prod;
    QSpacerItem *vs_sp;
    QWidget *page_employe_list;
    QVBoxLayout *vl_emp;
    QLabel *lbl_emp;
    QHBoxLayout *hl_tools_emp;
    QLineEdit *le_search_emp;
    QPushButton *btn_search_emp;
    QPushButton *btn_sort_alpha_emp;
    QPushButton *btn_sort_anciennete;
    QPushButton *btn_open_eval;
    QPushButton *btn_stats_emp;
    QPushButton *btn_stats_emp_stats;
    QPushButton *btn_pdf_emp;
    QPushButton *btn_add_emp;
    QTableWidget *tableEmployes;
    QHBoxLayout *hl_bot_emp;
    QSpacerItem *hs_be;
    QPushButton *btn_edit_emp;
    QPushButton *btn_delete_emp;
    QWidget *page_employe_form;
    QVBoxLayout *vl_emp_form;
    QPushButton *btn_back_emp;
    QGroupBox *gb_emp;
    QFormLayout *fl_emp;
    QLabel *le1;
    QLineEdit *le_id_emp;
    QLabel *le2;
    QLineEdit *le_nom_emp;
    QLabel *le3;
    QLineEdit *le_prenom_emp;
    QLabel *le4;
    QComboBox *cb_poste_emp;
    QLabel *le5;
    QComboBox *cb_dept_emp;
    QLabel *le6;
    QDateEdit *dt_embauche;
    QLabel *le7;
    QDoubleSpinBox *sb_salaire_emp;
    QLabel *le8;
    QLineEdit *le_rfid_emp;
    QPushButton *btn_calcul_score;
    QLabel *lbl_score_result;
    QPushButton *btn_valider_emp;
    QSpacerItem *vs_ef;
    QWidget *page_stats_emp;
    QVBoxLayout *vl_stats_e;
    QHBoxLayout *hl_head_se;
    QPushButton *btn_back_stats_emp;
    QLabel *lbl_title_se;
    QPushButton *btn_dummy3;
    QHBoxLayout *hl_chat_area;
    QVBoxLayout *vl_kpi_rh;
    QFrame *st_rh1;
    QVBoxLayout *v_rh1;
    QLabel *val_tot_emp;
    QLabel *lbl_tot_emp;
    QFrame *st_rh2;
    QVBoxLayout *v_rh2;
    QLabel *val_masse_sal;
    QLabel *lbl_masse_sal;
    QGroupBox *gb_chart_dept;
    QFormLayout *fl_dept;
    QLabel *l_prod;
    QProgressBar *pb_prod_rh;
    QLabel *l_log;
    QProgressBar *pb_log_rh;
    QLabel *l_be;
    QProgressBar *pb_be_rh;
    QSpacerItem *vs_rh_left;
    QGroupBox *gb_chat;
    QVBoxLayout *vl_chat;
    QTextBrowser *txt_chat_history;
    QHBoxLayout *hl_chat_input;
    QLineEdit *le_chat_input;
    QPushButton *btn_send_chat;
    QSpacerItem *vs_stats_e;
    QWidget *page_client_list;
    QVBoxLayout *vl_client_list;
    QLabel *lbl_cli;
    QHBoxLayout *hl_tools_cli;
    QLineEdit *le_search_client;
    QPushButton *btn_search_client;
    QPushButton *btn_sort_alpha_client;
    QPushButton *btn_sort_fidelite;
    QPushButton *btn_open_client_ia;
    QPushButton *btn_open_fidelite;
    QPushButton *btn_stats_client;
    QPushButton *btn_pdf_facture;
    QPushButton *btn_add_client;
    QTableWidget *tableClients;
    QHBoxLayout *hl_bot_cli;
    QSpacerItem *hs_cli;
    QPushButton *btn_edit_client;
    QPushButton *btn_delete_client;
    QWidget *page_client_form;
    QVBoxLayout *vl_client_form;
    QPushButton *btn_back_client;
    QGroupBox *gb_client;
    QFormLayout *fl_client;
    QLabel *lc1;
    QLineEdit *le_id_client;
    QLabel *lc2;
    QLineEdit *le_nom_client;
    QLabel *lc3;
    QLineEdit *le_tel_client;
    QLabel *lc4;
    QLineEdit *le_adresse_client;
    QLabel *lc5;
    QLineEdit *le_email_client;
    QLabel *lc6;
    QSpinBox *sb_points_client;
    QPushButton *btn_valider_client;
    QSpacerItem *vs_cf;
    QWidget *page_stats_client;
    QVBoxLayout *vl_stats_client;
    QHBoxLayout *hl_head_sc;
    QPushButton *btn_back_stats_client;
    QLabel *lbl_title_sc;
    QPushButton *btn_dummy_cli;
    QHBoxLayout *hl_kpi_cli;
    QFrame *st_cli1;
    QVBoxLayout *v_cli1;
    QLabel *ico_cli_1;
    QLabel *val_tot_cli;
    QLabel *lbl_tot_cli;
    QFrame *st_cli2;
    QVBoxLayout *v_cli2;
    QLabel *ico_cli_2;
    QLabel *val_vip_cli;
    QLabel *lbl_vip_cli;
    QFrame *st_cli3;
    QVBoxLayout *v_cli3;
    QLabel *ico_cli_3;
    QLabel *val_avg_pts_cli;
    QLabel *lbl_avg_pts_cli;
    QHBoxLayout *hl_charts_cli;
    QGroupBox *gb_geo_cli;
    QFormLayout *fl_geo_cli;
    QLabel *l_cli_tunis;
    QProgressBar *pb_cli_tunis;
    QLabel *l_cli_sfax;
    QProgressBar *pb_cli_sfax;
    QLabel *l_cli_autres;
    QProgressBar *pb_cli_autres;
    QGroupBox *gb_fidel_cli;
    QFormLayout *fl_fidel_cli;
    QLabel *l_cli_std;
    QProgressBar *pb_cli_std;
    QLabel *l_cli_silver;
    QProgressBar *pb_cli_silver;
    QLabel *l_cli_vip;
    QProgressBar *pb_cli_vip;
    QHBoxLayout *hl_actions_cli;
    QPushButton *btn_export_excel_client;
    QPushButton *btn_print_client;
    QSpacerItem *hs_actions_cli;
    QPushButton *btn_close_stats_client_dash;
    QSpacerItem *vs_sc;
    QWidget *page_depot_list;
    QVBoxLayout *vl_depot_list;
    QLabel *lbl_dep;
    QHBoxLayout *hl_tools_dep;
    QLineEdit *le_search_depot;
    QPushButton *btn_search_depot;
    QPushButton *btn_sort_alpha_depot;
    QPushButton *btn_sort_remplissage;
    QPushButton *btn_open_optimize;
    QPushButton *btn_open_ravit;
    QPushButton *btn_stats_depot;
    QPushButton *btn_pdf_depot;
    QPushButton *btn_add_depot;
    QTableWidget *tableDepot;
    QHBoxLayout *hl_bot_dep;
    QSpacerItem *hs_dep;
    QPushButton *btn_edit_depot;
    QPushButton *btn_delete_depot;
    QWidget *page_depot_form;
    QVBoxLayout *vl_depot_form;
    QPushButton *btn_back_depot;
    QGroupBox *gb_depot;
    QFormLayout *fl_depot;
    QLabel *ld1;
    QLineEdit *le_id_depot;
    QLabel *ld2;
    QLineEdit *le_emplacement_depot;
    QLabel *ld3;
    QLineEdit *le_etagere_depot;
    QLabel *ld4;
    QDoubleSpinBox *sb_capacite_depot;
    QLabel *ld5;
    QDoubleSpinBox *sb_qte_depot;
    QLabel *ld6;
    QComboBox *cb_type_depot;
    QPushButton *btn_valider_depot;
    QSpacerItem *vs_df;
    QWidget *page_stats_depot;
    QVBoxLayout *vl_stats_depot;
    QHBoxLayout *hl_head_sd;
    QPushButton *btn_back_stats_depot;
    QLabel *lbl_title_sd;
    QPushButton *btn_dummy_dep;
    QHBoxLayout *hl_kpi_dep;
    QFrame *st_dep1;
    QVBoxLayout *v_dep1;
    QLabel *ico_dep_1;
    QLabel *val_tot_dep;
    QLabel *lbl_tot_dep;
    QFrame *st_dep2;
    QVBoxLayout *v_dep2;
    QLabel *ico_dep_2;
    QLabel *val_avg_fill;
    QLabel *lbl_avg_fill;
    QFrame *st_dep3;
    QVBoxLayout *v_dep3;
    QLabel *ico_dep_3;
    QLabel *val_max_fill_dep;
    QLabel *lbl_max_fill_dep;
    QHBoxLayout *hl_charts_dep;
    QGroupBox *gb_type_dep;
    QFormLayout *fl_type_dep;
    QLabel *l_dep_froid;
    QProgressBar *pb_dep_froid;
    QLabel *l_dep_sec;
    QProgressBar *pb_dep_sec;
    QGroupBox *gb_fill_dep;
    QFormLayout *fl_fill_dep;
    QLabel *l_dep_low;
    QProgressBar *pb_dep_low;
    QLabel *l_dep_mid;
    QProgressBar *pb_dep_mid;
    QLabel *l_dep_high;
    QProgressBar *pb_dep_high;
    QHBoxLayout *hl_actions_depot;
    QPushButton *btn_export_excel_depot;
    QPushButton *btn_print_depot;
    QSpacerItem *hs_actions_depot;
    QPushButton *btn_close_stats_depot_dash;
    QSpacerItem *vs_sd;
    QWidget *page_stock_list;
    QVBoxLayout *vl_stock;
    QLabel *lbl_stk;
    QHBoxLayout *hl_tools_stk;
    QLineEdit *le_search_stock;
    QPushButton *btn_search_stock;
    QPushButton *btn_sort_alpha_stock;
    QPushButton *btn_open_compare;
    QPushButton *btn_open_calcul;
    QPushButton *btn_stats_stock;
    QPushButton *btn_pdf_stock;
    QPushButton *btn_add_stock;
    QTableWidget *tableStock;
    QHBoxLayout *hl_bot_stk;
    QSpacerItem *hs_bs;
    QPushButton *btn_edit_stock;
    QPushButton *btn_delete_stock;
    QWidget *page_stock_form;
    QVBoxLayout *vl_stk_form;
    QPushButton *btn_back_stock;
    QHBoxLayout *hl_forms_stk;
    QGroupBox *gb_caract;
    QFormLayout *fl_caract;
    QLabel *ls1;
    QLineEdit *le_code_mp;
    QLabel *ls2;
    QComboBox *cb_cat_mp;
    QLabel *ls3;
    QComboBox *cb_etat_mp;
    QLabel *ls4;
    QLineEdit *le_coul_mp;
    QLabel *ls5;
    QComboBox *cb_qual_mp;
    QGroupBox *gb_log;
    QFormLayout *fl_log;
    QLabel *ls6;
    QHBoxLayout *hl_qte;
    QDoubleSpinBox *sb_qte_mp;
    QComboBox *cb_unite_mp;
    QLabel *ls7;
    QHBoxLayout *hl_zn;
    QComboBox *cb_zone_mp;
    QLineEdit *le_allee_mp;
    QLabel *ls8;
    QComboBox *cb_type_stock;
    QLabel *ls9;
    QDateEdit *dt_rec_mp;
    QPushButton *btn_valider_stock;
    QSpacerItem *vs_sf;
    QWidget *page_stock_inn;
    QVBoxLayout *vl_stk_inn;
    QPushButton *btn_back_inn_stock;
    QWidget *page_stock_stats;
    QVBoxLayout *vl_stats_stk;
    QHBoxLayout *hl_head_sst;
    QPushButton *btn_back_stats_stock;
    QLabel *lbl_title_sst;
    QPushButton *btn_dummy4;
    QHBoxLayout *hl_kpi_stk;
    QFrame *st_stk1;
    QVBoxLayout *v_stk1;
    QLabel *ico_stk1;
    QLabel *val_tot_mat;
    QLabel *lbl_tot_mat;
    QFrame *st_stk2;
    QVBoxLayout *v_stk2;
    QLabel *ico_stk2;
    QLabel *val_tot_vol;
    QLabel *lbl_tot_vol;
    QFrame *st_stk3;
    QVBoxLayout *v_stk3;
    QLabel *ico_stk3;
    QLabel *val_tot_val_stock;
    QLabel *lbl_tot_val_stock;
    QHBoxLayout *hl_charts_stk;
    QGroupBox *gb_chart_cat;
    QFormLayout *fl_cat;
    QLabel *l_cuir;
    QProgressBar *pb_cuir;
    QLabel *l_teint;
    QProgressBar *pb_teint;
    QLabel *l_chim;
    QProgressBar *pb_chim;
    QGroupBox *gb_chart_qual;
    QFormLayout *fl_qual;
    QLabel *l_qa;
    QProgressBar *pb_qa;
    QLabel *l_qb;
    QProgressBar *pb_qb;
    QLabel *l_qc;
    QProgressBar *pb_qc;
    QHBoxLayout *hl_extra_charts_stk;
    QGroupBox *gb_pie_stk;
    QVBoxLayout *vl_pie_stk;
    QWidget *w_pie_stk;
    QGroupBox *gb_bar_stk;
    QVBoxLayout *vl_bar_stk;
    QWidget *w_bar_stk;
    QHBoxLayout *hl_actions_sst;
    QPushButton *btn_export_excel_stock;
    QPushButton *btn_print_stock;
    QSpacerItem *hs_actions_sst;
    QPushButton *btn_close_stats_stock;
    QSpacerItem *vs_stk_end;
    QWidget *page_fab_detail;
    QVBoxLayout *vl_4;
    QPushButton *btn_back_fab;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1400, 850);
        MainWindow->setStyleSheet(QString::fromUtf8("\n"
"    /* --- GLOBAL --- */\n"
"    QMainWindow, QWidget#centralwidget { background-color: #f3f0eb; font-family: \"Segoe UI\"; color: #3e2723; }\n"
"    QFrame#frame_sidebar { background-color: #1a1210; border-right: 1px solid #3e2723; min-width: 240px; max-width: 240px; }\n"
"    QWidget#widget_logo_container { background-color: #2c1a16; border-bottom: 2px solid #8d5524; }\n"
"    QLabel#l_app_title { color: #e0c097; font-size: 20px; font-weight: 800; letter-spacing: 2px; text-transform: uppercase; }\n"
"    QPushButton.nav_btn { text-align: left; padding: 18px 25px; border: none; background: transparent; color: #aca09d; font-weight: 600; font-size: 14px; }\n"
"    QPushButton.nav_btn:hover { background-color: #3e2723; color: #fff; border-left: 4px solid #d4af37; padding-left: 29px; }\n"
"    QWidget#page_home { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #2c1a16, stop:1 #1a1210); }\n"
"    QWidget#page_login { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0"
                        " #2c1a16, stop:1 #1a1210); }\n"
"    QLabel#lbl_welcome_title { font-size: 56px; font-weight: 300; color: #ffffff; letter-spacing: 2px; }\n"
"    QLabel#lbl_welcome_sub { font-size: 22px; color: #d4af37; font-style: italic; margin-bottom: 50px; }\n"
"    QPushButton#btn_start_app { background-color: transparent; color: #d4af37; font-size: 18px; font-weight: bold; padding: 15px 50px; border: 2px solid #d4af37; border-radius: 4px; text-transform: uppercase; }\n"
"    QPushButton#btn_start_app:hover { background-color: #d4af37; color: #1a1210; }\n"
"\n"
"    /* --- LOGIN PAGE --- */\n"
"    QFrame#login_card { background: rgba(255,255,255,0.06); border: 1px solid rgba(212,175,55,0.35); border-radius: 16px; }\n"
"    QLabel#lbl_login_title { font-size: 34px; font-weight: 700; color: #ffffff; letter-spacing: 1px; }\n"
"    QLabel#lbl_login_sub { font-size: 16px; color: #d4af37; font-style: italic; }\n"
"    QLineEdit#le_login_nom, QLineEdit#le_login_prenom, QLineEdit#le_login_mdp {\n"
"        background: rgba(255,"
                        "255,255,0.92);\n"
"        border: 1px solid rgba(212,175,55,0.55);\n"
"        padding: 12px;\n"
"        border-radius: 10px;\n"
"        font-size: 14px;\n"
"        color: #2c1a16;\n"
"    }\n"
"    QPushButton#btn_login {\n"
"        background-color: #d4af37;\n"
"        color: #1a1210;\n"
"        padding: 12px 26px;\n"
"        border-radius: 12px;\n"
"        font-weight: 900;\n"
"        text-transform: uppercase;\n"
"    }\n"
"    QPushButton#btn_login:hover { background-color: #e0c097; }\n"
"    QPushButton#btn_login_back {\n"
"        background-color: transparent;\n"
"        color: #d4af37;\n"
"        border: 1px solid #d4af37;\n"
"        padding: 12px 22px;\n"
"        border-radius: 12px;\n"
"        font-weight: 800;\n"
"    }\n"
"    QPushButton#btn_login_back:hover { background-color: rgba(212,175,55,0.12); }\n"
"    QTableWidget { background-color: #ffffff; gridline-color: #ecf0f1; border: 1px solid #dcdcdc; border-radius: 4px; }\n"
"    QHeaderView::section { background-color: #2c1a16; "
                        "color: #e0c097; padding: 12px; border: none; font-weight: bold; text-transform: uppercase; }\n"
"    QTableWidget::item { border-bottom: 1px solid #f0f0f0; padding: 5px; }\n"
"    QTableWidget#tableTimeline::item { background-color: transparent; border: 0px; }\n"
"    QPushButton { border-radius: 4px; padding: 8px 16px; font-weight: bold; border: none; }\n"
"\n"
"    /* Boutons Or */\n"
"    QPushButton#btn_to_add_planif, QPushButton#btn_valider_planif, QPushButton#btn_calculer_ia, QPushButton#btn_valider_etape,\n"
"    QPushButton#btn_add_produit, QPushButton#btn_valider_produit, QPushButton#btn_add_emp, QPushButton#btn_valider_emp, QPushButton#btn_send_chat,\n"
"    QPushButton#btn_add_stock, QPushButton#btn_valider_stock, QPushButton#btn_calculer_besoin, QPushButton#btn_compare_stock,\n"
"    QPushButton#btn_add_client, QPushButton#btn_valider_client,\n"
"    QPushButton#btn_add_depot, QPushButton#btn_valider_depot {\n"
"        background-color: #8d5524; color: white;\n"
"    }\n"
"    QPushButton#btn_to_a"
                        "dd_planif:hover, QPushButton#btn_add_produit:hover { background-color: #a0673b; }\n"
"\n"
"    /* Boutons Marron */\n"
"    QPushButton#btn_rechercher, QPushButton#btn_tri, QPushButton#btn_sort_alpha_planif, QPushButton#btn_stat_plan, QPushButton#btn_modifier_planif, QPushButton#btn_pdf,\n"
"    QPushButton#btn_back_planif, QPushButton#btn_back_fab, QPushButton#btn_back_stats, QPushButton#btn_refresh_timeline,\n"
"    QPushButton#btn_edit_produit, QPushButton#btn_search_col, QPushButton#btn_sort_alpha_prod, QPushButton#btn_pdf_catalogue,\n"
"    QPushButton#btn_back_prod, QPushButton#btn_stats_prod, QPushButton#btn_back_stats_prod,\n"
"    QPushButton#btn_edit_emp, QPushButton#btn_search_emp, QPushButton#btn_sort_alpha_emp, QPushButton#btn_sort_anciennete, QPushButton#btn_pdf_emp, QPushButton#btn_stats_emp, QPushButton#btn_stats_emp_stats, QPushButton#btn_back_emp, QPushButton#btn_calcul_score, QPushButton#btn_back_stats_emp,\n"
"    QPushButton#btn_edit_stock, QPushButton#btn_search_stock, QPushButton#btn_sor"
                        "t_alpha_stock, QPushButton#btn_pdf_stock, QPushButton#btn_saisie_detail, QPushButton#btn_back_stock, QPushButton#btn_back_inn_stock,\n"
"    QPushButton#btn_stats_stock, QPushButton#btn_back_stats_stock,\n"
"    QPushButton#btn_search_client, QPushButton#btn_sort_alpha_client, QPushButton#btn_sort_fidelite, QPushButton#btn_stats_client, QPushButton#btn_pdf_facture, QPushButton#btn_back_client, QPushButton#btn_back_stats_client, QPushButton#btn_edit_client,\n"
"    QPushButton#btn_search_depot, QPushButton#btn_sort_alpha_depot, QPushButton#btn_sort_remplissage, QPushButton#btn_stats_depot, QPushButton#btn_pdf_depot, QPushButton#btn_back_depot, QPushButton#btn_back_stats_depot, QPushButton#btn_edit_depot {\n"
"        background-color: #795548; color: white;\n"
"    }\n"
"    QPushButton#btn_check_delta, QPushButton#btn_supprimer_planif, QPushButton#btn_delete_produit, QPushButton#btn_delete_emp, QPushButton#btn_delete_stock, QPushButton#btn_delete_client, QPushButton#btn_delete_depot { background-color: #b71c1c"
                        "; color: white; }\n"
"\n"
"    /* Boutons Sp\303\251ciaux Innovations */\n"
"    QPushButton#btn_open_compare { background-color: #00838f; color: white; }\n"
"    QPushButton#btn_open_calcul { background-color: #f9a825; color: white; }\n"
"    QPushButton#btn_cout_produit { background-color: #00897b; color: white; }\n"
"    QPushButton#btn_hist_mode { background-color: #7b1fa2; color: white; }\n"
"    QPushButton#btn_open_eval { background-color: #ef6c00; color: white; }\n"
"\n"
"    QPushButton#btn_open_optimize { background-color: #00838f; color: white; }\n"
"    QPushButton#btn_open_ravit { background-color: #f9a825; color: white; }\n"
"    QPushButton#btn_open_fidelite { background-color: #00897b; color: white; }\n"
"\n"
"    /* NOUVEAU BOUTON PLANIF IA (Violet) */\n"
"    QPushButton#btn_open_planif_ia { background-color: #7b1fa2; color: white; }\n"
"\n"
"    QPushButton#btn_open_client_ia { background-color: #7b1fa2; color: white; }\n"
"\n"
"    QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox {"
                        " background: #ffffff; border: 1px solid #ccc; padding: 8px; border-radius: 4px; color: #333; }\n"
"    QLabel#lbl_iot_temp, QLabel.lbl_kpi_inn { color: #d84315; font-weight:bold; border: 2px solid #d84315; padding:8px; border-radius:6px; background:white; font-size:14px; }\n"
"    QLabel#lbl_iot_hum { color: #0277bd; font-weight:bold; border: 2px solid #0277bd; padding:8px; border-radius:6px; background:white; }\n"
"    QTextBrowser { background-color: #fff; border: 1px solid #dcdcdc; border-radius: 8px; padding: 10px; font-size: 14px; }\n"
"    /* --- DASHBOARD STATS (style like screenshot) --- */\n"
"    QLabel#lbl_title_s, QLabel#lbl_title_sp, QLabel#lbl_title_sst, QLabel#lbl_title_sc, QLabel#lbl_title_sd, QLabel#lbl_title_se {\n"
"        background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #2c1a16, stop:1 #795548);\n"
"        color: #ffffff;\n"
"        border-radius: 14px;\n"
"        padding: 18px;\n"
"        font-size: 18px;\n"
"        font-weight: 800;\n"
"        letter-spacing: "
                        "1px;\n"
"    }\n"
"\n"
"    QFrame.stat_card {\n"
"        border-radius: 14px;\n"
"        border: none;\n"
"        padding: 12px;\n"
"        background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #2c1a16, stop:1 #1a1210);\n"
"    }\n"
"    QFrame.stat_card QLabel { color: #ffffff; font-weight: 700; }\n"
"    QLabel.stat_val { font-size: 34px; font-weight: 900; color: #ffffff; }\n"
"\n"
"    /* KPI gradients (reuse existing palette colors already used in the app) */\n"
"    QFrame#st1, QFrame#stp1, QFrame#st_rh1, QFrame#st_cli1, QFrame#st_dep1, QFrame#st_stk1 {\n"
"        background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #27ae60, stop:1 #00897b);\n"
"    }\n"
"    QFrame#st2, QFrame#stp2, QFrame#st_rh2, QFrame#st_cli2, QFrame#st_dep2, QFrame#st_stk2 {\n"
"        background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #0277bd, stop:1 #00838f);\n"
"    }\n"
"    QFrame#st3, QFrame#stp3, QFrame#st_rh3, QFrame#st_cli3, QFrame#st_dep3, QFrame#st_stk3 {\n"
"  "
                        "      background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #ef6c00, stop:1 #f9a825);\n"
"    }\n"
"\n"
"    /* Stats groupboxes only (keep forms unchanged) */\n"
"    QGroupBox#gb_charts, QGroupBox#gb_charts_statut, QGroupBox#gb_chart_coll, QGroupBox#gb_chart_cuir,\n"
"    QGroupBox#gb_chart_cat, QGroupBox#gb_chart_qual, QGroupBox#gb_geo_cli, QGroupBox#gb_fidel_cli,\n"
"    QGroupBox#gb_type_dep, QGroupBox#gb_fill_dep, QGroupBox#gb_chart_dept, QGroupBox#gb_chart_poste {\n"
"        background: #ffffff;\n"
"        border: 2px solid #d7ccc8;\n"
"        border-radius: 12px;\n"
"        margin-top: 10px;\n"
"        padding: 12px;\n"
"        font-weight: 700;\n"
"    }\n"
"    QGroupBox#gb_charts::title, QGroupBox#gb_charts_statut::title, QGroupBox#gb_chart_coll::title, QGroupBox#gb_chart_cuir::title,\n"
"    QGroupBox#gb_chart_cat::title, QGroupBox#gb_chart_qual::title, QGroupBox#gb_geo_cli::title, QGroupBox#gb_fidel_cli::title,\n"
"    QGroupBox#gb_type_dep::title, QGroupBox#gb_fill_dep::tit"
                        "le, QGroupBox#gb_chart_dept::title, QGroupBox#gb_chart_poste::title {\n"
"        subcontrol-origin: margin;\n"
"        left: 12px;\n"
"        padding: 0 6px;\n"
"        color: #8d5524;\n"
"    }\n"
"\n"
"    QProgressBar { border: 1px solid #d7ccc8; border-radius: 9px; text-align: center; background: #f3f0eb; color:#3e2723; height: 18px; font-weight: 700; }\n"
"    QProgressBar::chunk { border-radius: 9px; background-color: #27ae60; }\n"
"\n"
"    /* Per-bar colors (dashboard look) */\n"
"    QProgressBar#pb_cuir::chunk { background-color: #0277bd; }\n"
"    QProgressBar#pb_teint::chunk { background-color: #ef6c00; }\n"
"    QProgressBar#pb_chim::chunk { background-color: #27ae60; }\n"
"    QProgressBar#pb_qa::chunk { background-color: #f9a825; }\n"
"    QProgressBar#pb_qb::chunk { background-color: #90a4ae; }\n"
"    QProgressBar#pb_qc::chunk { background-color: #8d5524; }\n"
"\n"
"    /* Dashboard actions */\n"
"    QPushButton.btn_excel { background-color: #27ae60; color: white; border-radius: 18px; pad"
                        "ding: 10px 18px; }\n"
"    QPushButton.btn_excel:hover { background-color: #00897b; }\n"
"    QPushButton.btn_print { background-color: #0277bd; color: white; border-radius: 18px; padding: 10px 18px; }\n"
"    QPushButton.btn_print:hover { background-color: #00838f; }\n"
"    QPushButton.btn_close { background-color: #3e2723; color: white; border-radius: 18px; padding: 10px 18px; }\n"
"    QPushButton.btn_close:hover { background-color: #1a1210; }\n"
"   "));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        frame_sidebar = new QFrame(centralwidget);
        frame_sidebar->setObjectName("frame_sidebar");
        vl_nav = new QVBoxLayout(frame_sidebar);
        vl_nav->setSpacing(0);
        vl_nav->setObjectName("vl_nav");
        vl_nav->setContentsMargins(0, 0, 0, -1);
        logo_png = new QHBoxLayout();
        logo_png->setSpacing(15);
        logo_png->setObjectName("logo_png");
        logo_png->setContentsMargins(20, 20, 10, 20);
        l_logo_img = new QLabel(frame_sidebar);
        l_logo_img->setObjectName("l_logo_img");
        l_logo_img->setMinimumSize(QSize(40, 40));
        l_logo_img->setMaximumSize(QSize(40, 40));
        l_logo_img->setPixmap(QPixmap(QString::fromUtf8(":/logo.png")));
        l_logo_img->setScaledContents(true);

        logo_png->addWidget(l_logo_img);

        l_app_title = new QLabel(frame_sidebar);
        l_app_title->setObjectName("l_app_title");

        logo_png->addWidget(l_app_title);


        vl_nav->addLayout(logo_png);

        btn_nav_dashboard = new QPushButton(frame_sidebar);
        btn_nav_dashboard->setObjectName("btn_nav_dashboard");

        vl_nav->addWidget(btn_nav_dashboard);

        btn_nav_produit = new QPushButton(frame_sidebar);
        btn_nav_produit->setObjectName("btn_nav_produit");

        vl_nav->addWidget(btn_nav_produit);

        btn_nav_stock = new QPushButton(frame_sidebar);
        btn_nav_stock->setObjectName("btn_nav_stock");

        vl_nav->addWidget(btn_nav_stock);

        btn_nav_clients = new QPushButton(frame_sidebar);
        btn_nav_clients->setObjectName("btn_nav_clients");

        vl_nav->addWidget(btn_nav_clients);

        btn_nav_depot = new QPushButton(frame_sidebar);
        btn_nav_depot->setObjectName("btn_nav_depot");

        vl_nav->addWidget(btn_nav_depot);

        btn_nav_planif = new QPushButton(frame_sidebar);
        btn_nav_planif->setObjectName("btn_nav_planif");

        vl_nav->addWidget(btn_nav_planif);

        btn_nav_fab = new QPushButton(frame_sidebar);
        btn_nav_fab->setObjectName("btn_nav_fab");

        vl_nav->addWidget(btn_nav_fab);

        btn_nav_rh = new QPushButton(frame_sidebar);
        btn_nav_rh->setObjectName("btn_nav_rh");

        vl_nav->addWidget(btn_nav_rh);

        vs = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_nav->addItem(vs);


        horizontalLayout->addWidget(frame_sidebar);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        page_home = new QWidget();
        page_home->setObjectName("page_home");
        vl_h = new QVBoxLayout(page_home);
        vl_h->setObjectName("vl_h");
        lbl_big_logo = new QLabel(page_home);
        lbl_big_logo->setObjectName("lbl_big_logo");
        lbl_big_logo->setMinimumSize(QSize(150, 150));
        lbl_big_logo->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vl_h->addWidget(lbl_big_logo);

        vs_h1 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_h->addItem(vs_h1);

        lbl_welcome_title = new QLabel(page_home);
        lbl_welcome_title->setObjectName("lbl_welcome_title");
        lbl_welcome_title->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vl_h->addWidget(lbl_welcome_title);

        lbl_welcome_sub = new QLabel(page_home);
        lbl_welcome_sub->setObjectName("lbl_welcome_sub");
        lbl_welcome_sub->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vl_h->addWidget(lbl_welcome_sub);

        vs_h2 = new QSpacerItem(20, 60, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_h->addItem(vs_h2);

        btn_start_app = new QPushButton(page_home);
        btn_start_app->setObjectName("btn_start_app");
        btn_start_app->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        vl_h->addWidget(btn_start_app);

        vs_h3 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_h->addItem(vs_h3);

        stackedWidget->addWidget(page_home);
        page_login = new QWidget();
        page_login->setObjectName("page_login");
        vl_login = new QVBoxLayout(page_login);
        vl_login->setObjectName("vl_login");
        vl_login->setContentsMargins(0, 0, 0, 0);
        vl_login_center = new QVBoxLayout();
        vl_login_center->setObjectName("vl_login_center");
        login_card = new QFrame(page_login);
        login_card->setObjectName("login_card");
        login_card->setMinimumSize(QSize(520, 420));
        vl_login_card = new QVBoxLayout(login_card);
        vl_login_card->setSpacing(16);
        vl_login_card->setObjectName("vl_login_card");
        vl_login_card->setContentsMargins(26, 26, 26, 26);
        lbl_login_title = new QLabel(login_card);
        lbl_login_title->setObjectName("lbl_login_title");
        lbl_login_title->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vl_login_card->addWidget(lbl_login_title);

        lbl_login_sub = new QLabel(login_card);
        lbl_login_sub->setObjectName("lbl_login_sub");
        lbl_login_sub->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vl_login_card->addWidget(lbl_login_sub);

        vs_login_1 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_login_card->addItem(vs_login_1);

        le_login_nom = new QLineEdit(login_card);
        le_login_nom->setObjectName("le_login_nom");

        vl_login_card->addWidget(le_login_nom);

        le_login_prenom = new QLineEdit(login_card);
        le_login_prenom->setObjectName("le_login_prenom");

        vl_login_card->addWidget(le_login_prenom);

        le_login_mdp = new QLineEdit(login_card);
        le_login_mdp->setObjectName("le_login_mdp");
        le_login_mdp->setEchoMode(QLineEdit::EchoMode::Password);

        vl_login_card->addWidget(le_login_mdp);

        hl_login_actions = new QHBoxLayout();
        hl_login_actions->setObjectName("hl_login_actions");
        btn_login_back = new QPushButton(login_card);
        btn_login_back->setObjectName("btn_login_back");
        btn_login_back->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        hl_login_actions->addWidget(btn_login_back);

        hs_login = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_login_actions->addItem(hs_login);

        btn_login = new QPushButton(login_card);
        btn_login->setObjectName("btn_login");
        btn_login->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

        hl_login_actions->addWidget(btn_login);


        vl_login_card->addLayout(hl_login_actions);

        vs_login_2 = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_login_card->addItem(vs_login_2);


        vl_login_center->addWidget(login_card);


        vl_login->addLayout(vl_login_center);

        stackedWidget->addWidget(page_login);
        page_planif_list = new QWidget();
        page_planif_list->setObjectName("page_planif_list");
        vl_p = new QVBoxLayout(page_planif_list);
        vl_p->setContentsMargins(40, 40, 40, 40);
        vl_p->setObjectName("vl_p");
        lbl_p1 = new QLabel(page_planif_list);
        lbl_p1->setObjectName("lbl_p1");

        vl_p->addWidget(lbl_p1);

        hl_p_tools = new QHBoxLayout();
        hl_p_tools->setObjectName("hl_p_tools");
        le_recherche = new QLineEdit(page_planif_list);
        le_recherche->setObjectName("le_recherche");

        hl_p_tools->addWidget(le_recherche);

        btn_rechercher = new QPushButton(page_planif_list);
        btn_rechercher->setObjectName("btn_rechercher");

        hl_p_tools->addWidget(btn_rechercher);

        btn_sort_alpha_planif = new QPushButton(page_planif_list);
        btn_sort_alpha_planif->setObjectName("btn_sort_alpha_planif");

        hl_p_tools->addWidget(btn_sort_alpha_planif);

        btn_open_planif_ia = new QPushButton(page_planif_list);
        btn_open_planif_ia->setObjectName("btn_open_planif_ia");

        hl_p_tools->addWidget(btn_open_planif_ia);

        btn_stat_plan = new QPushButton(page_planif_list);
        btn_stat_plan->setObjectName("btn_stat_plan");

        hl_p_tools->addWidget(btn_stat_plan);

        btn_pdf = new QPushButton(page_planif_list);
        btn_pdf->setObjectName("btn_pdf");

        hl_p_tools->addWidget(btn_pdf);

        btn_to_add_planif = new QPushButton(page_planif_list);
        btn_to_add_planif->setObjectName("btn_to_add_planif");

        hl_p_tools->addWidget(btn_to_add_planif);


        vl_p->addLayout(hl_p_tools);

        tablePlanif = new QTableWidget(page_planif_list);
        if (tablePlanif->columnCount() < 7)
            tablePlanif->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tablePlanif->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tablePlanif->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tablePlanif->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tablePlanif->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tablePlanif->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tablePlanif->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tablePlanif->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        tablePlanif->setObjectName("tablePlanif");
        tablePlanif->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        vl_p->addWidget(tablePlanif);

        hl_bot_p = new QHBoxLayout();
        hl_bot_p->setObjectName("hl_bot_p");
        btn_tri = new QPushButton(page_planif_list);
        btn_tri->setObjectName("btn_tri");

        hl_bot_p->addWidget(btn_tri);

        hs_bot = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_bot_p->addItem(hs_bot);

        btn_modifier_planif = new QPushButton(page_planif_list);
        btn_modifier_planif->setObjectName("btn_modifier_planif");

        hl_bot_p->addWidget(btn_modifier_planif);

        btn_supprimer_planif = new QPushButton(page_planif_list);
        btn_supprimer_planif->setObjectName("btn_supprimer_planif");

        hl_bot_p->addWidget(btn_supprimer_planif);


        vl_p->addLayout(hl_bot_p);

        stackedWidget->addWidget(page_planif_list);
        page_fab_list = new QWidget();
        page_fab_list->setObjectName("page_fab_list");
        vl_f = new QVBoxLayout(page_fab_list);
        vl_f->setSpacing(20);
        vl_f->setObjectName("vl_f");
        vl_f->setContentsMargins(40, 40, 40, 40);
        hl_f_top = new QHBoxLayout();
        hl_f_top->setObjectName("hl_f_top");
        vl_titre_f = new QVBoxLayout();
        vl_titre_f->setObjectName("vl_titre_f");
        l_tf = new QLabel(page_fab_list);
        l_tf->setObjectName("l_tf");

        vl_titre_f->addWidget(l_tf);

        l_sub_f = new QLabel(page_fab_list);
        l_sub_f->setObjectName("l_sub_f");

        vl_titre_f->addWidget(l_sub_f);


        hl_f_top->addLayout(vl_titre_f);

        hs_f1 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_f_top->addItem(hs_f1);

        l_legende_colors = new QLabel(page_fab_list);
        l_legende_colors->setObjectName("l_legende_colors");

        hl_f_top->addWidget(l_legende_colors);


        vl_f->addLayout(hl_f_top);

        hl_tools_timeline = new QHBoxLayout();
        hl_tools_timeline->setObjectName("hl_tools_timeline");
        btn_refresh_timeline = new QPushButton(page_fab_list);
        btn_refresh_timeline->setObjectName("btn_refresh_timeline");

        hl_tools_timeline->addWidget(btn_refresh_timeline);

        hs_f2 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_tools_timeline->addItem(hs_f2);

        btn_saisie_detail = new QPushButton(page_fab_list);
        btn_saisie_detail->setObjectName("btn_saisie_detail");

        hl_tools_timeline->addWidget(btn_saisie_detail);


        vl_f->addLayout(hl_tools_timeline);

        tableTimeline = new QTableWidget(page_fab_list);
        if (tableTimeline->columnCount() < 1)
            tableTimeline->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableTimeline->setHorizontalHeaderItem(0, __qtablewidgetitem7);
        tableTimeline->setObjectName("tableTimeline");
        tableTimeline->verticalHeader()->setVisible(false);

        vl_f->addWidget(tableTimeline);

        line_sep_fab = new QFrame(page_fab_list);
        line_sep_fab->setObjectName("line_sep_fab");
        line_sep_fab->setFrameShape(QFrame::Shape::HLine);
        line_sep_fab->setFrameShadow(QFrame::Shadow::Sunken);

        vl_f->addWidget(line_sep_fab);

        frame_supervision = new QFrame(page_fab_list);
        frame_supervision->setObjectName("frame_supervision");
        hl_sup = new QHBoxLayout(frame_supervision);
        hl_sup->setContentsMargins(15, 15, 15, 15);
        hl_sup->setObjectName("hl_sup");
        vl_sup_info = new QVBoxLayout();
        vl_sup_info->setObjectName("vl_sup_info");
        lbl_sel_cmd_title = new QLabel(frame_supervision);
        lbl_sel_cmd_title->setObjectName("lbl_sel_cmd_title");

        vl_sup_info->addWidget(lbl_sel_cmd_title);

        lbl_sel_cmd = new QLabel(frame_supervision);
        lbl_sel_cmd->setObjectName("lbl_sel_cmd");

        vl_sup_info->addWidget(lbl_sel_cmd);


        hl_sup->addLayout(vl_sup_info);

        line_sup = new QFrame(frame_supervision);
        line_sup->setObjectName("line_sup");
        line_sup->setFrameShape(QFrame::Shape::VLine);
        line_sup->setFrameShadow(QFrame::Shadow::Sunken);

        hl_sup->addWidget(line_sup);

        fl_sup_saisie = new QFormLayout();
        fl_sup_saisie->setObjectName("fl_sup_saisie");
        l_et = new QLabel(frame_supervision);
        l_et->setObjectName("l_et");

        fl_sup_saisie->setWidget(0, QFormLayout::LabelRole, l_et);

        cb_etape_suivi = new QComboBox(frame_supervision);
        cb_etape_suivi->addItem(QString());
        cb_etape_suivi->addItem(QString());
        cb_etape_suivi->addItem(QString());
        cb_etape_suivi->addItem(QString());
        cb_etape_suivi->setObjectName("cb_etape_suivi");

        fl_sup_saisie->setWidget(0, QFormLayout::FieldRole, cb_etape_suivi);

        l_tp = new QLabel(frame_supervision);
        l_tp->setObjectName("l_tp");

        fl_sup_saisie->setWidget(1, QFormLayout::LabelRole, l_tp);

        lbl_temps_prevu = new QLabel(frame_supervision);
        lbl_temps_prevu->setObjectName("lbl_temps_prevu");

        fl_sup_saisie->setWidget(1, QFormLayout::FieldRole, lbl_temps_prevu);

        l_tr = new QLabel(frame_supervision);
        l_tr->setObjectName("l_tr");

        fl_sup_saisie->setWidget(2, QFormLayout::LabelRole, l_tr);

        sb_temps_reel_input = new QDoubleSpinBox(frame_supervision);
        sb_temps_reel_input->setObjectName("sb_temps_reel_input");

        fl_sup_saisie->setWidget(2, QFormLayout::FieldRole, sb_temps_reel_input);


        hl_sup->addLayout(fl_sup_saisie);

        btn_valider_etape = new QPushButton(frame_supervision);
        btn_valider_etape->setObjectName("btn_valider_etape");

        hl_sup->addWidget(btn_valider_etape);

        lbl_resultat_delta = new QLabel(frame_supervision);
        lbl_resultat_delta->setObjectName("lbl_resultat_delta");
        lbl_resultat_delta->setAlignment(Qt::AlignmentFlag::AlignCenter);

        hl_sup->addWidget(lbl_resultat_delta);


        vl_f->addWidget(frame_supervision);

        stackedWidget->addWidget(page_fab_list);
        page_planif_form = new QWidget();
        page_planif_form->setObjectName("page_planif_form");
        vl_pform = new QVBoxLayout(page_planif_form);
        vl_pform->setObjectName("vl_pform");
        vl_pform->setContentsMargins(40, 40, 40, -1);
        btn_back_planif = new QPushButton(page_planif_form);
        btn_back_planif->setObjectName("btn_back_planif");

        vl_pform->addWidget(btn_back_planif);

        gb_form = new QGroupBox(page_planif_form);
        gb_form->setObjectName("gb_form");
        fl_form = new QFormLayout(gb_form);
        fl_form->setObjectName("fl_form");
        fl_form->setVerticalSpacing(20);
        l1 = new QLabel(gb_form);
        l1->setObjectName("l1");

        fl_form->setWidget(0, QFormLayout::LabelRole, l1);

        cb_produit = new QComboBox(gb_form);
        cb_produit->addItem(QString());
        cb_produit->addItem(QString());
        cb_produit->setObjectName("cb_produit");

        fl_form->setWidget(0, QFormLayout::FieldRole, cb_produit);

        l2 = new QLabel(gb_form);
        l2->setObjectName("l2");

        fl_form->setWidget(1, QFormLayout::LabelRole, l2);

        sb_qte = new QSpinBox(gb_form);
        sb_qte->setObjectName("sb_qte");
        sb_qte->setMaximum(10000);

        fl_form->setWidget(1, QFormLayout::FieldRole, sb_qte);

        l3 = new QLabel(gb_form);
        l3->setObjectName("l3");

        fl_form->setWidget(2, QFormLayout::LabelRole, l3);

        cb_matiere = new QComboBox(gb_form);
        cb_matiere->addItem(QString());
        cb_matiere->addItem(QString());
        cb_matiere->setObjectName("cb_matiere");

        fl_form->setWidget(2, QFormLayout::FieldRole, cb_matiere);

        l4 = new QLabel(gb_form);
        l4->setObjectName("l4");

        fl_form->setWidget(3, QFormLayout::LabelRole, l4);

        dt_lancement = new QDateTimeEdit(gb_form);
        dt_lancement->setObjectName("dt_lancement");
        dt_lancement->setCalendarPopup(true);

        fl_form->setWidget(3, QFormLayout::FieldRole, dt_lancement);

        l5 = new QLabel(gb_form);
        l5->setObjectName("l5");

        fl_form->setWidget(4, QFormLayout::LabelRole, l5);

        hl_ia = new QHBoxLayout();
        hl_ia->setObjectName("hl_ia");
        le_fin_prevue = new QLineEdit(gb_form);
        le_fin_prevue->setObjectName("le_fin_prevue");

        hl_ia->addWidget(le_fin_prevue);

        btn_calculer_ia = new QPushButton(gb_form);
        btn_calculer_ia->setObjectName("btn_calculer_ia");

        hl_ia->addWidget(btn_calculer_ia);


        fl_form->setLayout(4, QFormLayout::FieldRole, hl_ia);

        le_duree_totale = new QLineEdit(gb_form);
        le_duree_totale->setObjectName("le_duree_totale");

        fl_form->setWidget(5, QFormLayout::FieldRole, le_duree_totale);


        vl_pform->addWidget(gb_form);

        btn_valider_planif = new QPushButton(page_planif_form);
        btn_valider_planif->setObjectName("btn_valider_planif");

        vl_pform->addWidget(btn_valider_planif);

        vs_f = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_pform->addItem(vs_f);

        stackedWidget->addWidget(page_planif_form);
        page_stats = new QWidget();
        page_stats->setObjectName("page_stats");
        vl_stats = new QVBoxLayout(page_stats);
        vl_stats->setContentsMargins(40, 40, 40, 40);
        vl_stats->setObjectName("vl_stats");
        btn_back_stats = new QPushButton(page_stats);
        btn_back_stats->setObjectName("btn_back_stats");

        vl_stats->addWidget(btn_back_stats);

        lbl_title_s = new QLabel(page_stats);
        lbl_title_s->setObjectName("lbl_title_s");
        lbl_title_s->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vl_stats->addWidget(lbl_title_s);

        hl_stats_kpi = new QHBoxLayout();
        hl_stats_kpi->setObjectName("hl_stats_kpi");
        st1 = new QFrame(page_stats);
        st1->setObjectName("st1");
        v_s1 = new QVBoxLayout(st1);
        v_s1->setObjectName("v_s1");
        ico_plan_1 = new QLabel(st1);
        ico_plan_1->setObjectName("ico_plan_1");
        ico_plan_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s1->addWidget(ico_plan_1);

        lbl_stat_total_cmd = new QLabel(st1);
        lbl_stat_total_cmd->setObjectName("lbl_stat_total_cmd");
        lbl_stat_total_cmd->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s1->addWidget(lbl_stat_total_cmd);

        lbl_t1 = new QLabel(st1);
        lbl_t1->setObjectName("lbl_t1");
        lbl_t1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s1->addWidget(lbl_t1);


        hl_stats_kpi->addWidget(st1);

        st2 = new QFrame(page_stats);
        st2->setObjectName("st2");
        v_s2 = new QVBoxLayout(st2);
        v_s2->setObjectName("v_s2");
        ico_plan_2 = new QLabel(st2);
        ico_plan_2->setObjectName("ico_plan_2");
        ico_plan_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s2->addWidget(ico_plan_2);

        lbl_stat_total_qty = new QLabel(st2);
        lbl_stat_total_qty->setObjectName("lbl_stat_total_qty");
        lbl_stat_total_qty->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s2->addWidget(lbl_stat_total_qty);

        lbl_t2 = new QLabel(st2);
        lbl_t2->setObjectName("lbl_t2");
        lbl_t2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s2->addWidget(lbl_t2);


        hl_stats_kpi->addWidget(st2);

        st3 = new QFrame(page_stats);
        st3->setObjectName("st3");
        v_s3 = new QVBoxLayout(st3);
        v_s3->setObjectName("v_s3");
        ico_plan_3 = new QLabel(st3);
        ico_plan_3->setObjectName("ico_plan_3");
        ico_plan_3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s3->addWidget(ico_plan_3);

        lbl_stat_retard = new QLabel(st3);
        lbl_stat_retard->setObjectName("lbl_stat_retard");
        lbl_stat_retard->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s3->addWidget(lbl_stat_retard);

        lbl_t3 = new QLabel(st3);
        lbl_t3->setObjectName("lbl_t3");
        lbl_t3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_s3->addWidget(lbl_t3);


        hl_stats_kpi->addWidget(st3);


        vl_stats->addLayout(hl_stats_kpi);

        hl_charts_planif = new QHBoxLayout();
        hl_charts_planif->setObjectName("hl_charts_planif");
        gb_charts = new QGroupBox(page_stats);
        gb_charts->setObjectName("gb_charts");
        fl_charts = new QFormLayout(gb_charts);
        fl_charts->setObjectName("fl_charts");
        l_c1 = new QLabel(gb_charts);
        l_c1->setObjectName("l_c1");

        fl_charts->setWidget(0, QFormLayout::LabelRole, l_c1);

        pb_sac = new QProgressBar(gb_charts);
        pb_sac->setObjectName("pb_sac");
        pb_sac->setValue(0);

        fl_charts->setWidget(0, QFormLayout::FieldRole, pb_sac);

        l_c2 = new QLabel(gb_charts);
        l_c2->setObjectName("l_c2");

        fl_charts->setWidget(1, QFormLayout::LabelRole, l_c2);

        pb_portefeuille = new QProgressBar(gb_charts);
        pb_portefeuille->setObjectName("pb_portefeuille");
        pb_portefeuille->setValue(0);

        fl_charts->setWidget(1, QFormLayout::FieldRole, pb_portefeuille);

        l_c3 = new QLabel(gb_charts);
        l_c3->setObjectName("l_c3");

        fl_charts->setWidget(2, QFormLayout::LabelRole, l_c3);

        pb_ceinture = new QProgressBar(gb_charts);
        pb_ceinture->setObjectName("pb_ceinture");
        pb_ceinture->setValue(0);

        fl_charts->setWidget(2, QFormLayout::FieldRole, pb_ceinture);


        hl_charts_planif->addWidget(gb_charts);

        gb_charts_statut = new QGroupBox(page_stats);
        gb_charts_statut->setObjectName("gb_charts_statut");
        fl_statut = new QFormLayout(gb_charts_statut);
        fl_statut->setObjectName("fl_statut");
        l_cmd_plan = new QLabel(gb_charts_statut);
        l_cmd_plan->setObjectName("l_cmd_plan");

        fl_statut->setWidget(0, QFormLayout::LabelRole, l_cmd_plan);

        pb_cmd_plan = new QProgressBar(gb_charts_statut);
        pb_cmd_plan->setObjectName("pb_cmd_plan");
        pb_cmd_plan->setValue(0);

        fl_statut->setWidget(0, QFormLayout::FieldRole, pb_cmd_plan);

        l_cmd_cours = new QLabel(gb_charts_statut);
        l_cmd_cours->setObjectName("l_cmd_cours");

        fl_statut->setWidget(1, QFormLayout::LabelRole, l_cmd_cours);

        pb_cmd_cours = new QProgressBar(gb_charts_statut);
        pb_cmd_cours->setObjectName("pb_cmd_cours");
        pb_cmd_cours->setValue(0);

        fl_statut->setWidget(1, QFormLayout::FieldRole, pb_cmd_cours);

        l_cmd_ret = new QLabel(gb_charts_statut);
        l_cmd_ret->setObjectName("l_cmd_ret");

        fl_statut->setWidget(2, QFormLayout::LabelRole, l_cmd_ret);

        pb_cmd_ret = new QProgressBar(gb_charts_statut);
        pb_cmd_ret->setObjectName("pb_cmd_ret");
        pb_cmd_ret->setValue(0);

        fl_statut->setWidget(2, QFormLayout::FieldRole, pb_cmd_ret);


        hl_charts_planif->addWidget(gb_charts_statut);


        vl_stats->addLayout(hl_charts_planif);

        hl_actions_planif = new QHBoxLayout();
        hl_actions_planif->setObjectName("hl_actions_planif");
        btn_export_excel_planif = new QPushButton(page_stats);
        btn_export_excel_planif->setObjectName("btn_export_excel_planif");

        hl_actions_planif->addWidget(btn_export_excel_planif);

        btn_print_planif = new QPushButton(page_stats);
        btn_print_planif->setObjectName("btn_print_planif");

        hl_actions_planif->addWidget(btn_print_planif);

        hs_actions_planif = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_actions_planif->addItem(hs_actions_planif);

        btn_close_stats_planif = new QPushButton(page_stats);
        btn_close_stats_planif->setObjectName("btn_close_stats_planif");

        hl_actions_planif->addWidget(btn_close_stats_planif);


        vl_stats->addLayout(hl_actions_planif);

        vs_stats = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_stats->addItem(vs_stats);

        stackedWidget->addWidget(page_stats);
        page_produit_list = new QWidget();
        page_produit_list->setObjectName("page_produit_list");
        vl_prod_list = new QVBoxLayout(page_produit_list);
        vl_prod_list->setContentsMargins(40, 40, 40, 40);
        vl_prod_list->setObjectName("vl_prod_list");
        lbl_t_prod = new QLabel(page_produit_list);
        lbl_t_prod->setObjectName("lbl_t_prod");

        vl_prod_list->addWidget(lbl_t_prod);

        hl_tools_prod = new QHBoxLayout();
        hl_tools_prod->setObjectName("hl_tools_prod");
        le_search_coll = new QLineEdit(page_produit_list);
        le_search_coll->setObjectName("le_search_coll");

        hl_tools_prod->addWidget(le_search_coll);

        btn_search_col = new QPushButton(page_produit_list);
        btn_search_col->setObjectName("btn_search_col");

        hl_tools_prod->addWidget(btn_search_col);

        btn_sort_alpha_prod = new QPushButton(page_produit_list);
        btn_sort_alpha_prod->setObjectName("btn_sort_alpha_prod");

        hl_tools_prod->addWidget(btn_sort_alpha_prod);

        btn_cout_produit = new QPushButton(page_produit_list);
        btn_cout_produit->setObjectName("btn_cout_produit");

        hl_tools_prod->addWidget(btn_cout_produit);

        btn_hist_mode = new QPushButton(page_produit_list);
        btn_hist_mode->setObjectName("btn_hist_mode");

        hl_tools_prod->addWidget(btn_hist_mode);

        btn_stats_prod = new QPushButton(page_produit_list);
        btn_stats_prod->setObjectName("btn_stats_prod");

        hl_tools_prod->addWidget(btn_stats_prod);

        btn_pdf_catalogue = new QPushButton(page_produit_list);
        btn_pdf_catalogue->setObjectName("btn_pdf_catalogue");

        hl_tools_prod->addWidget(btn_pdf_catalogue);

        btn_add_produit = new QPushButton(page_produit_list);
        btn_add_produit->setObjectName("btn_add_produit");

        hl_tools_prod->addWidget(btn_add_produit);


        vl_prod_list->addLayout(hl_tools_prod);

        tableProduits = new QTableWidget(page_produit_list);
        if (tableProduits->columnCount() < 6)
            tableProduits->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableProduits->setHorizontalHeaderItem(0, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableProduits->setHorizontalHeaderItem(1, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableProduits->setHorizontalHeaderItem(2, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        tableProduits->setHorizontalHeaderItem(3, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tableProduits->setHorizontalHeaderItem(4, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        tableProduits->setHorizontalHeaderItem(5, __qtablewidgetitem13);
        tableProduits->setObjectName("tableProduits");
        tableProduits->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        vl_prod_list->addWidget(tableProduits);

        hl_bot_prod = new QHBoxLayout();
        hl_bot_prod->setObjectName("hl_bot_prod");
        hs_bp = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_bot_prod->addItem(hs_bp);

        btn_edit_produit = new QPushButton(page_produit_list);
        btn_edit_produit->setObjectName("btn_edit_produit");

        hl_bot_prod->addWidget(btn_edit_produit);

        btn_delete_produit = new QPushButton(page_produit_list);
        btn_delete_produit->setObjectName("btn_delete_produit");

        hl_bot_prod->addWidget(btn_delete_produit);


        vl_prod_list->addLayout(hl_bot_prod);

        stackedWidget->addWidget(page_produit_list);
        page_produit_form = new QWidget();
        page_produit_form->setObjectName("page_produit_form");
        vl_prod_form = new QVBoxLayout(page_produit_form);
        vl_prod_form->setContentsMargins(40, 40, 40, 40);
        vl_prod_form->setObjectName("vl_prod_form");
        btn_back_prod = new QPushButton(page_produit_form);
        btn_back_prod->setObjectName("btn_back_prod");

        vl_prod_form->addWidget(btn_back_prod);

        gb_prod = new QGroupBox(page_produit_form);
        gb_prod->setObjectName("gb_prod");
        fl_prod = new QFormLayout(gb_prod);
        fl_prod->setObjectName("fl_prod");
        fl_prod->setVerticalSpacing(20);
        lp1 = new QLabel(gb_prod);
        lp1->setObjectName("lp1");

        fl_prod->setWidget(0, QFormLayout::LabelRole, lp1);

        le_ref_prod = new QLineEdit(gb_prod);
        le_ref_prod->setObjectName("le_ref_prod");

        fl_prod->setWidget(0, QFormLayout::FieldRole, le_ref_prod);

        lp2 = new QLabel(gb_prod);
        lp2->setObjectName("lp2");

        fl_prod->setWidget(1, QFormLayout::LabelRole, lp2);

        le_nom_prod = new QLineEdit(gb_prod);
        le_nom_prod->setObjectName("le_nom_prod");

        fl_prod->setWidget(1, QFormLayout::FieldRole, le_nom_prod);

        lp3 = new QLabel(gb_prod);
        lp3->setObjectName("lp3");

        fl_prod->setWidget(2, QFormLayout::LabelRole, lp3);

        cb_coll_prod = new QComboBox(gb_prod);
        cb_coll_prod->addItem(QString());
        cb_coll_prod->addItem(QString());
        cb_coll_prod->addItem(QString());
        cb_coll_prod->setObjectName("cb_coll_prod");

        fl_prod->setWidget(2, QFormLayout::FieldRole, cb_coll_prod);

        lp4 = new QLabel(gb_prod);
        lp4->setObjectName("lp4");

        fl_prod->setWidget(3, QFormLayout::LabelRole, lp4);

        cb_cuir_prod = new QComboBox(gb_prod);
        cb_cuir_prod->addItem(QString());
        cb_cuir_prod->addItem(QString());
        cb_cuir_prod->addItem(QString());
        cb_cuir_prod->setObjectName("cb_cuir_prod");

        fl_prod->setWidget(3, QFormLayout::FieldRole, cb_cuir_prod);

        lp5 = new QLabel(gb_prod);
        lp5->setObjectName("lp5");

        fl_prod->setWidget(4, QFormLayout::LabelRole, lp5);

        sb_cout_prod = new QDoubleSpinBox(gb_prod);
        sb_cout_prod->setObjectName("sb_cout_prod");
        sb_cout_prod->setMaximum(10000.000000000000000);

        fl_prod->setWidget(4, QFormLayout::FieldRole, sb_cout_prod);

        lp6 = new QLabel(gb_prod);
        lp6->setObjectName("lp6");

        fl_prod->setWidget(5, QFormLayout::LabelRole, lp6);

        sb_temps_prod = new QSpinBox(gb_prod);
        sb_temps_prod->setObjectName("sb_temps_prod");

        fl_prod->setWidget(5, QFormLayout::FieldRole, sb_temps_prod);

        lp7 = new QLabel(gb_prod);
        lp7->setObjectName("lp7");

        fl_prod->setWidget(6, QFormLayout::LabelRole, lp7);

        lbl_cout_total = new QLabel(gb_prod);
        lbl_cout_total->setObjectName("lbl_cout_total");

        fl_prod->setWidget(6, QFormLayout::FieldRole, lbl_cout_total);


        vl_prod_form->addWidget(gb_prod);

        btn_valider_produit = new QPushButton(page_produit_form);
        btn_valider_produit->setObjectName("btn_valider_produit");

        vl_prod_form->addWidget(btn_valider_produit);

        vs_pf = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_prod_form->addItem(vs_pf);

        stackedWidget->addWidget(page_produit_form);
        page_stats_prod = new QWidget();
        page_stats_prod->setObjectName("page_stats_prod");
        vl_stats_p = new QVBoxLayout(page_stats_prod);
        vl_stats_p->setContentsMargins(40, 40, 40, 40);
        vl_stats_p->setObjectName("vl_stats_p");
        btn_back_stats_prod = new QPushButton(page_stats_prod);
        btn_back_stats_prod->setObjectName("btn_back_stats_prod");

        vl_stats_p->addWidget(btn_back_stats_prod);

        lbl_title_sp = new QLabel(page_stats_prod);
        lbl_title_sp->setObjectName("lbl_title_sp");
        lbl_title_sp->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vl_stats_p->addWidget(lbl_title_sp);

        hl_kpi_prod = new QHBoxLayout();
        hl_kpi_prod->setObjectName("hl_kpi_prod");
        stp1 = new QFrame(page_stats_prod);
        stp1->setObjectName("stp1");
        vp1 = new QVBoxLayout(stp1);
        vp1->setObjectName("vp1");
        ico_prod_1 = new QLabel(stp1);
        ico_prod_1->setObjectName("ico_prod_1");
        ico_prod_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp1->addWidget(ico_prod_1);

        val_tot_ref = new QLabel(stp1);
        val_tot_ref->setObjectName("val_tot_ref");
        val_tot_ref->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp1->addWidget(val_tot_ref);

        lbl_tot_ref = new QLabel(stp1);
        lbl_tot_ref->setObjectName("lbl_tot_ref");
        lbl_tot_ref->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp1->addWidget(lbl_tot_ref);


        hl_kpi_prod->addWidget(stp1);

        stp2 = new QFrame(page_stats_prod);
        stp2->setObjectName("stp2");
        vp2 = new QVBoxLayout(stp2);
        vp2->setObjectName("vp2");
        ico_prod_2 = new QLabel(stp2);
        ico_prod_2->setObjectName("ico_prod_2");
        ico_prod_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp2->addWidget(ico_prod_2);

        val_avg_cost = new QLabel(stp2);
        val_avg_cost->setObjectName("val_avg_cost");
        val_avg_cost->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp2->addWidget(val_avg_cost);

        lbl_avg_cost = new QLabel(stp2);
        lbl_avg_cost->setObjectName("lbl_avg_cost");
        lbl_avg_cost->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp2->addWidget(lbl_avg_cost);


        hl_kpi_prod->addWidget(stp2);

        stp3 = new QFrame(page_stats_prod);
        stp3->setObjectName("stp3");
        vp3 = new QVBoxLayout(stp3);
        vp3->setObjectName("vp3");
        ico_prod_3 = new QLabel(stp3);
        ico_prod_3->setObjectName("ico_prod_3");
        ico_prod_3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp3->addWidget(ico_prod_3);

        val_tot_val_prod = new QLabel(stp3);
        val_tot_val_prod->setObjectName("val_tot_val_prod");
        val_tot_val_prod->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp3->addWidget(val_tot_val_prod);

        lbl_tot_val_prod = new QLabel(stp3);
        lbl_tot_val_prod->setObjectName("lbl_tot_val_prod");
        lbl_tot_val_prod->setAlignment(Qt::AlignmentFlag::AlignCenter);

        vp3->addWidget(lbl_tot_val_prod);


        hl_kpi_prod->addWidget(stp3);


        vl_stats_p->addLayout(hl_kpi_prod);

        hl_charts_prod = new QHBoxLayout();
        hl_charts_prod->setObjectName("hl_charts_prod");
        gb_chart_coll = new QGroupBox(page_stats_prod);
        gb_chart_coll->setObjectName("gb_chart_coll");
        fl_coll = new QFormLayout(gb_chart_coll);
        fl_coll->setObjectName("fl_coll");
        l_hiver = new QLabel(gb_chart_coll);
        l_hiver->setObjectName("l_hiver");

        fl_coll->setWidget(0, QFormLayout::LabelRole, l_hiver);

        pb_hiver = new QProgressBar(gb_chart_coll);
        pb_hiver->setObjectName("pb_hiver");
        pb_hiver->setValue(0);

        fl_coll->setWidget(0, QFormLayout::FieldRole, pb_hiver);

        l_ete = new QLabel(gb_chart_coll);
        l_ete->setObjectName("l_ete");

        fl_coll->setWidget(1, QFormLayout::LabelRole, l_ete);

        pb_ete = new QProgressBar(gb_chart_coll);
        pb_ete->setObjectName("pb_ete");
        pb_ete->setValue(0);

        fl_coll->setWidget(1, QFormLayout::FieldRole, pb_ete);

        l_intemp = new QLabel(gb_chart_coll);
        l_intemp->setObjectName("l_intemp");

        fl_coll->setWidget(2, QFormLayout::LabelRole, l_intemp);

        pb_intemp = new QProgressBar(gb_chart_coll);
        pb_intemp->setObjectName("pb_intemp");
        pb_intemp->setValue(0);

        fl_coll->setWidget(2, QFormLayout::FieldRole, pb_intemp);


        hl_charts_prod->addWidget(gb_chart_coll);

        gb_chart_cuir = new QGroupBox(page_stats_prod);
        gb_chart_cuir->setObjectName("gb_chart_cuir");
        fl_cuir = new QFormLayout(gb_chart_cuir);
        fl_cuir->setObjectName("fl_cuir");
        l_cuir_v = new QLabel(gb_chart_cuir);
        l_cuir_v->setObjectName("l_cuir_v");

        fl_cuir->setWidget(0, QFormLayout::LabelRole, l_cuir_v);

        pb_cuir_vachette = new QProgressBar(gb_chart_cuir);
        pb_cuir_vachette->setObjectName("pb_cuir_vachette");
        pb_cuir_vachette->setValue(0);

        fl_cuir->setWidget(0, QFormLayout::FieldRole, pb_cuir_vachette);

        l_cuir_a = new QLabel(gb_chart_cuir);
        l_cuir_a->setObjectName("l_cuir_a");

        fl_cuir->setWidget(1, QFormLayout::LabelRole, l_cuir_a);

        pb_cuir_agneau = new QProgressBar(gb_chart_cuir);
        pb_cuir_agneau->setObjectName("pb_cuir_agneau");
        pb_cuir_agneau->setValue(0);

        fl_cuir->setWidget(1, QFormLayout::FieldRole, pb_cuir_agneau);

        l_cuir_c = new QLabel(gb_chart_cuir);
        l_cuir_c->setObjectName("l_cuir_c");

        fl_cuir->setWidget(2, QFormLayout::LabelRole, l_cuir_c);

        pb_cuir_croco = new QProgressBar(gb_chart_cuir);
        pb_cuir_croco->setObjectName("pb_cuir_croco");
        pb_cuir_croco->setValue(0);

        fl_cuir->setWidget(2, QFormLayout::FieldRole, pb_cuir_croco);


        hl_charts_prod->addWidget(gb_chart_cuir);


        vl_stats_p->addLayout(hl_charts_prod);

        hl_actions_prod = new QHBoxLayout();
        hl_actions_prod->setObjectName("hl_actions_prod");
        btn_export_excel_prod = new QPushButton(page_stats_prod);
        btn_export_excel_prod->setObjectName("btn_export_excel_prod");

        hl_actions_prod->addWidget(btn_export_excel_prod);

        btn_print_prod = new QPushButton(page_stats_prod);
        btn_print_prod->setObjectName("btn_print_prod");

        hl_actions_prod->addWidget(btn_print_prod);

        hs_actions_prod = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_actions_prod->addItem(hs_actions_prod);

        btn_close_stats_prod = new QPushButton(page_stats_prod);
        btn_close_stats_prod->setObjectName("btn_close_stats_prod");

        hl_actions_prod->addWidget(btn_close_stats_prod);


        vl_stats_p->addLayout(hl_actions_prod);

        vs_sp = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_stats_p->addItem(vs_sp);

        stackedWidget->addWidget(page_stats_prod);
        page_employe_list = new QWidget();
        page_employe_list->setObjectName("page_employe_list");
        vl_emp = new QVBoxLayout(page_employe_list);
        vl_emp->setContentsMargins(40, 40, 40, 40);
        vl_emp->setObjectName("vl_emp");
        lbl_emp = new QLabel(page_employe_list);
        lbl_emp->setObjectName("lbl_emp");

        vl_emp->addWidget(lbl_emp);

        hl_tools_emp = new QHBoxLayout();
        hl_tools_emp->setObjectName("hl_tools_emp");
        le_search_emp = new QLineEdit(page_employe_list);
        le_search_emp->setObjectName("le_search_emp");

        hl_tools_emp->addWidget(le_search_emp);

        btn_search_emp = new QPushButton(page_employe_list);
        btn_search_emp->setObjectName("btn_search_emp");

        hl_tools_emp->addWidget(btn_search_emp);

        btn_sort_alpha_emp = new QPushButton(page_employe_list);
        btn_sort_alpha_emp->setObjectName("btn_sort_alpha_emp");

        hl_tools_emp->addWidget(btn_sort_alpha_emp);

        btn_sort_anciennete = new QPushButton(page_employe_list);
        btn_sort_anciennete->setObjectName("btn_sort_anciennete");

        hl_tools_emp->addWidget(btn_sort_anciennete);

        btn_open_eval = new QPushButton(page_employe_list);
        btn_open_eval->setObjectName("btn_open_eval");

        hl_tools_emp->addWidget(btn_open_eval);

        btn_stats_emp = new QPushButton(page_employe_list);
        btn_stats_emp->setObjectName("btn_stats_emp");

        hl_tools_emp->addWidget(btn_stats_emp);

        btn_stats_emp_stats = new QPushButton(page_employe_list);
        btn_stats_emp_stats->setObjectName("btn_stats_emp_stats");

        hl_tools_emp->addWidget(btn_stats_emp_stats);

        btn_pdf_emp = new QPushButton(page_employe_list);
        btn_pdf_emp->setObjectName("btn_pdf_emp");

        hl_tools_emp->addWidget(btn_pdf_emp);

        btn_add_emp = new QPushButton(page_employe_list);
        btn_add_emp->setObjectName("btn_add_emp");

        hl_tools_emp->addWidget(btn_add_emp);


        vl_emp->addLayout(hl_tools_emp);

        tableEmployes = new QTableWidget(page_employe_list);
        if (tableEmployes->columnCount() < 7)
            tableEmployes->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        tableEmployes->setHorizontalHeaderItem(0, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        tableEmployes->setHorizontalHeaderItem(1, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        tableEmployes->setHorizontalHeaderItem(2, __qtablewidgetitem16);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        tableEmployes->setHorizontalHeaderItem(3, __qtablewidgetitem17);
        QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
        tableEmployes->setHorizontalHeaderItem(4, __qtablewidgetitem18);
        QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
        tableEmployes->setHorizontalHeaderItem(5, __qtablewidgetitem19);
        QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
        tableEmployes->setHorizontalHeaderItem(6, __qtablewidgetitem20);
        tableEmployes->setObjectName("tableEmployes");
        tableEmployes->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        vl_emp->addWidget(tableEmployes);

        hl_bot_emp = new QHBoxLayout();
        hl_bot_emp->setObjectName("hl_bot_emp");
        hs_be = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_bot_emp->addItem(hs_be);

        btn_edit_emp = new QPushButton(page_employe_list);
        btn_edit_emp->setObjectName("btn_edit_emp");

        hl_bot_emp->addWidget(btn_edit_emp);

        btn_delete_emp = new QPushButton(page_employe_list);
        btn_delete_emp->setObjectName("btn_delete_emp");

        hl_bot_emp->addWidget(btn_delete_emp);


        vl_emp->addLayout(hl_bot_emp);

        stackedWidget->addWidget(page_employe_list);
        page_employe_form = new QWidget();
        page_employe_form->setObjectName("page_employe_form");
        vl_emp_form = new QVBoxLayout(page_employe_form);
        vl_emp_form->setContentsMargins(40, 40, 40, 40);
        vl_emp_form->setObjectName("vl_emp_form");
        btn_back_emp = new QPushButton(page_employe_form);
        btn_back_emp->setObjectName("btn_back_emp");

        vl_emp_form->addWidget(btn_back_emp);

        gb_emp = new QGroupBox(page_employe_form);
        gb_emp->setObjectName("gb_emp");
        fl_emp = new QFormLayout(gb_emp);
        fl_emp->setObjectName("fl_emp");
        fl_emp->setVerticalSpacing(15);
        le1 = new QLabel(gb_emp);
        le1->setObjectName("le1");

        fl_emp->setWidget(0, QFormLayout::LabelRole, le1);

        le_id_emp = new QLineEdit(gb_emp);
        le_id_emp->setObjectName("le_id_emp");

        fl_emp->setWidget(0, QFormLayout::FieldRole, le_id_emp);

        le2 = new QLabel(gb_emp);
        le2->setObjectName("le2");

        fl_emp->setWidget(1, QFormLayout::LabelRole, le2);

        le_nom_emp = new QLineEdit(gb_emp);
        le_nom_emp->setObjectName("le_nom_emp");

        fl_emp->setWidget(1, QFormLayout::FieldRole, le_nom_emp);

        le3 = new QLabel(gb_emp);
        le3->setObjectName("le3");

        fl_emp->setWidget(2, QFormLayout::LabelRole, le3);

        le_prenom_emp = new QLineEdit(gb_emp);
        le_prenom_emp->setObjectName("le_prenom_emp");

        fl_emp->setWidget(2, QFormLayout::FieldRole, le_prenom_emp);

        le4 = new QLabel(gb_emp);
        le4->setObjectName("le4");

        fl_emp->setWidget(3, QFormLayout::LabelRole, le4);

        cb_poste_emp = new QComboBox(gb_emp);
        cb_poste_emp->addItem(QString());
        cb_poste_emp->addItem(QString());
        cb_poste_emp->addItem(QString());
        cb_poste_emp->addItem(QString());
        cb_poste_emp->setObjectName("cb_poste_emp");

        fl_emp->setWidget(3, QFormLayout::FieldRole, cb_poste_emp);

        le5 = new QLabel(gb_emp);
        le5->setObjectName("le5");

        fl_emp->setWidget(4, QFormLayout::LabelRole, le5);

        cb_dept_emp = new QComboBox(gb_emp);
        cb_dept_emp->addItem(QString());
        cb_dept_emp->addItem(QString());
        cb_dept_emp->addItem(QString());
        cb_dept_emp->setObjectName("cb_dept_emp");

        fl_emp->setWidget(4, QFormLayout::FieldRole, cb_dept_emp);

        le6 = new QLabel(gb_emp);
        le6->setObjectName("le6");

        fl_emp->setWidget(5, QFormLayout::LabelRole, le6);

        dt_embauche = new QDateEdit(gb_emp);
        dt_embauche->setObjectName("dt_embauche");
        dt_embauche->setCalendarPopup(true);

        fl_emp->setWidget(5, QFormLayout::FieldRole, dt_embauche);

        le7 = new QLabel(gb_emp);
        le7->setObjectName("le7");

        fl_emp->setWidget(6, QFormLayout::LabelRole, le7);

        sb_salaire_emp = new QDoubleSpinBox(gb_emp);
        sb_salaire_emp->setObjectName("sb_salaire_emp");
        sb_salaire_emp->setMaximum(10000.000000000000000);

        fl_emp->setWidget(6, QFormLayout::FieldRole, sb_salaire_emp);

        le8 = new QLabel(gb_emp);
        le8->setObjectName("le8");

        fl_emp->setWidget(7, QFormLayout::LabelRole, le8);

        le_rfid_emp = new QLineEdit(gb_emp);
        le_rfid_emp->setObjectName("le_rfid_emp");

        fl_emp->setWidget(7, QFormLayout::FieldRole, le_rfid_emp);

        btn_calcul_score = new QPushButton(gb_emp);
        btn_calcul_score->setObjectName("btn_calcul_score");

        fl_emp->setWidget(8, QFormLayout::LabelRole, btn_calcul_score);

        lbl_score_result = new QLabel(gb_emp);
        lbl_score_result->setObjectName("lbl_score_result");

        fl_emp->setWidget(8, QFormLayout::FieldRole, lbl_score_result);


        vl_emp_form->addWidget(gb_emp);

        btn_valider_emp = new QPushButton(page_employe_form);
        btn_valider_emp->setObjectName("btn_valider_emp");

        vl_emp_form->addWidget(btn_valider_emp);

        vs_ef = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_emp_form->addItem(vs_ef);

        stackedWidget->addWidget(page_employe_form);
        page_stats_emp = new QWidget();
        page_stats_emp->setObjectName("page_stats_emp");
        vl_stats_e = new QVBoxLayout(page_stats_emp);
        vl_stats_e->setContentsMargins(40, 40, 40, 40);
        vl_stats_e->setObjectName("vl_stats_e");
        hl_head_se = new QHBoxLayout();
        hl_head_se->setObjectName("hl_head_se");
        btn_back_stats_emp = new QPushButton(page_stats_emp);
        btn_back_stats_emp->setObjectName("btn_back_stats_emp");

        hl_head_se->addWidget(btn_back_stats_emp);

        lbl_title_se = new QLabel(page_stats_emp);
        lbl_title_se->setObjectName("lbl_title_se");
        lbl_title_se->setAlignment(Qt::AlignmentFlag::AlignCenter);

        hl_head_se->addWidget(lbl_title_se);

        btn_dummy3 = new QPushButton(page_stats_emp);
        btn_dummy3->setObjectName("btn_dummy3");
        btn_dummy3->setFlat(true);

        hl_head_se->addWidget(btn_dummy3);


        vl_stats_e->addLayout(hl_head_se);

        hl_chat_area = new QHBoxLayout();
        hl_chat_area->setObjectName("hl_chat_area");
        vl_kpi_rh = new QVBoxLayout();
        vl_kpi_rh->setObjectName("vl_kpi_rh");
        st_rh1 = new QFrame(page_stats_emp);
        st_rh1->setObjectName("st_rh1");
        v_rh1 = new QVBoxLayout(st_rh1);
        v_rh1->setObjectName("v_rh1");
        val_tot_emp = new QLabel(st_rh1);
        val_tot_emp->setObjectName("val_tot_emp");
        val_tot_emp->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_rh1->addWidget(val_tot_emp);

        lbl_tot_emp = new QLabel(st_rh1);
        lbl_tot_emp->setObjectName("lbl_tot_emp");
        lbl_tot_emp->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_rh1->addWidget(lbl_tot_emp);


        vl_kpi_rh->addWidget(st_rh1);

        st_rh2 = new QFrame(page_stats_emp);
        st_rh2->setObjectName("st_rh2");
        v_rh2 = new QVBoxLayout(st_rh2);
        v_rh2->setObjectName("v_rh2");
        val_masse_sal = new QLabel(st_rh2);
        val_masse_sal->setObjectName("val_masse_sal");
        val_masse_sal->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_rh2->addWidget(val_masse_sal);

        lbl_masse_sal = new QLabel(st_rh2);
        lbl_masse_sal->setObjectName("lbl_masse_sal");
        lbl_masse_sal->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_rh2->addWidget(lbl_masse_sal);


        vl_kpi_rh->addWidget(st_rh2);

        gb_chart_dept = new QGroupBox(page_stats_emp);
        gb_chart_dept->setObjectName("gb_chart_dept");
        fl_dept = new QFormLayout(gb_chart_dept);
        fl_dept->setObjectName("fl_dept");
        l_prod = new QLabel(gb_chart_dept);
        l_prod->setObjectName("l_prod");

        fl_dept->setWidget(0, QFormLayout::LabelRole, l_prod);

        pb_prod_rh = new QProgressBar(gb_chart_dept);
        pb_prod_rh->setObjectName("pb_prod_rh");
        pb_prod_rh->setValue(0);

        fl_dept->setWidget(0, QFormLayout::FieldRole, pb_prod_rh);

        l_log = new QLabel(gb_chart_dept);
        l_log->setObjectName("l_log");

        fl_dept->setWidget(1, QFormLayout::LabelRole, l_log);

        pb_log_rh = new QProgressBar(gb_chart_dept);
        pb_log_rh->setObjectName("pb_log_rh");
        pb_log_rh->setValue(0);

        fl_dept->setWidget(1, QFormLayout::FieldRole, pb_log_rh);

        l_be = new QLabel(gb_chart_dept);
        l_be->setObjectName("l_be");

        fl_dept->setWidget(2, QFormLayout::LabelRole, l_be);

        pb_be_rh = new QProgressBar(gb_chart_dept);
        pb_be_rh->setObjectName("pb_be_rh");
        pb_be_rh->setValue(0);

        fl_dept->setWidget(2, QFormLayout::FieldRole, pb_be_rh);


        vl_kpi_rh->addWidget(gb_chart_dept);

        vs_rh_left = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_kpi_rh->addItem(vs_rh_left);


        hl_chat_area->addLayout(vl_kpi_rh);

        gb_chat = new QGroupBox(page_stats_emp);
        gb_chat->setObjectName("gb_chat");
        vl_chat = new QVBoxLayout(gb_chat);
        vl_chat->setObjectName("vl_chat");
        txt_chat_history = new QTextBrowser(gb_chat);
        txt_chat_history->setObjectName("txt_chat_history");

        vl_chat->addWidget(txt_chat_history);

        hl_chat_input = new QHBoxLayout();
        hl_chat_input->setObjectName("hl_chat_input");
        le_chat_input = new QLineEdit(gb_chat);
        le_chat_input->setObjectName("le_chat_input");

        hl_chat_input->addWidget(le_chat_input);

        btn_send_chat = new QPushButton(gb_chat);
        btn_send_chat->setObjectName("btn_send_chat");

        hl_chat_input->addWidget(btn_send_chat);


        vl_chat->addLayout(hl_chat_input);


        hl_chat_area->addWidget(gb_chat);


        vl_stats_e->addLayout(hl_chat_area);

        vs_stats_e = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_stats_e->addItem(vs_stats_e);

        stackedWidget->addWidget(page_stats_emp);
        page_client_list = new QWidget();
        page_client_list->setObjectName("page_client_list");
        vl_client_list = new QVBoxLayout(page_client_list);
        vl_client_list->setContentsMargins(40, 40, 40, 40);
        vl_client_list->setObjectName("vl_client_list");
        lbl_cli = new QLabel(page_client_list);
        lbl_cli->setObjectName("lbl_cli");

        vl_client_list->addWidget(lbl_cli);

        hl_tools_cli = new QHBoxLayout();
        hl_tools_cli->setObjectName("hl_tools_cli");
        le_search_client = new QLineEdit(page_client_list);
        le_search_client->setObjectName("le_search_client");

        hl_tools_cli->addWidget(le_search_client);

        btn_search_client = new QPushButton(page_client_list);
        btn_search_client->setObjectName("btn_search_client");

        hl_tools_cli->addWidget(btn_search_client);

        btn_sort_alpha_client = new QPushButton(page_client_list);
        btn_sort_alpha_client->setObjectName("btn_sort_alpha_client");

        hl_tools_cli->addWidget(btn_sort_alpha_client);

        btn_sort_fidelite = new QPushButton(page_client_list);
        btn_sort_fidelite->setObjectName("btn_sort_fidelite");

        hl_tools_cli->addWidget(btn_sort_fidelite);

        btn_open_client_ia = new QPushButton(page_client_list);
        btn_open_client_ia->setObjectName("btn_open_client_ia");

        hl_tools_cli->addWidget(btn_open_client_ia);

        btn_open_fidelite = new QPushButton(page_client_list);
        btn_open_fidelite->setObjectName("btn_open_fidelite");

        hl_tools_cli->addWidget(btn_open_fidelite);

        btn_stats_client = new QPushButton(page_client_list);
        btn_stats_client->setObjectName("btn_stats_client");

        hl_tools_cli->addWidget(btn_stats_client);

        btn_pdf_facture = new QPushButton(page_client_list);
        btn_pdf_facture->setObjectName("btn_pdf_facture");

        hl_tools_cli->addWidget(btn_pdf_facture);

        btn_add_client = new QPushButton(page_client_list);
        btn_add_client->setObjectName("btn_add_client");

        hl_tools_cli->addWidget(btn_add_client);


        vl_client_list->addLayout(hl_tools_cli);

        tableClients = new QTableWidget(page_client_list);
        if (tableClients->columnCount() < 6)
            tableClients->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem21 = new QTableWidgetItem();
        tableClients->setHorizontalHeaderItem(0, __qtablewidgetitem21);
        QTableWidgetItem *__qtablewidgetitem22 = new QTableWidgetItem();
        tableClients->setHorizontalHeaderItem(1, __qtablewidgetitem22);
        QTableWidgetItem *__qtablewidgetitem23 = new QTableWidgetItem();
        tableClients->setHorizontalHeaderItem(2, __qtablewidgetitem23);
        QTableWidgetItem *__qtablewidgetitem24 = new QTableWidgetItem();
        tableClients->setHorizontalHeaderItem(3, __qtablewidgetitem24);
        QTableWidgetItem *__qtablewidgetitem25 = new QTableWidgetItem();
        tableClients->setHorizontalHeaderItem(4, __qtablewidgetitem25);
        QTableWidgetItem *__qtablewidgetitem26 = new QTableWidgetItem();
        tableClients->setHorizontalHeaderItem(5, __qtablewidgetitem26);
        tableClients->setObjectName("tableClients");
        tableClients->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        vl_client_list->addWidget(tableClients);

        hl_bot_cli = new QHBoxLayout();
        hl_bot_cli->setObjectName("hl_bot_cli");
        hs_cli = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_bot_cli->addItem(hs_cli);

        btn_edit_client = new QPushButton(page_client_list);
        btn_edit_client->setObjectName("btn_edit_client");

        hl_bot_cli->addWidget(btn_edit_client);

        btn_delete_client = new QPushButton(page_client_list);
        btn_delete_client->setObjectName("btn_delete_client");

        hl_bot_cli->addWidget(btn_delete_client);


        vl_client_list->addLayout(hl_bot_cli);

        stackedWidget->addWidget(page_client_list);
        page_client_form = new QWidget();
        page_client_form->setObjectName("page_client_form");
        vl_client_form = new QVBoxLayout(page_client_form);
        vl_client_form->setContentsMargins(40, 40, 40, 40);
        vl_client_form->setObjectName("vl_client_form");
        btn_back_client = new QPushButton(page_client_form);
        btn_back_client->setObjectName("btn_back_client");

        vl_client_form->addWidget(btn_back_client);

        gb_client = new QGroupBox(page_client_form);
        gb_client->setObjectName("gb_client");
        fl_client = new QFormLayout(gb_client);
        fl_client->setObjectName("fl_client");
        fl_client->setVerticalSpacing(15);
        lc1 = new QLabel(gb_client);
        lc1->setObjectName("lc1");

        fl_client->setWidget(0, QFormLayout::LabelRole, lc1);

        le_id_client = new QLineEdit(gb_client);
        le_id_client->setObjectName("le_id_client");

        fl_client->setWidget(0, QFormLayout::FieldRole, le_id_client);

        lc2 = new QLabel(gb_client);
        lc2->setObjectName("lc2");

        fl_client->setWidget(1, QFormLayout::LabelRole, lc2);

        le_nom_client = new QLineEdit(gb_client);
        le_nom_client->setObjectName("le_nom_client");

        fl_client->setWidget(1, QFormLayout::FieldRole, le_nom_client);

        lc3 = new QLabel(gb_client);
        lc3->setObjectName("lc3");

        fl_client->setWidget(2, QFormLayout::LabelRole, lc3);

        le_tel_client = new QLineEdit(gb_client);
        le_tel_client->setObjectName("le_tel_client");

        fl_client->setWidget(2, QFormLayout::FieldRole, le_tel_client);

        lc4 = new QLabel(gb_client);
        lc4->setObjectName("lc4");

        fl_client->setWidget(3, QFormLayout::LabelRole, lc4);

        le_adresse_client = new QLineEdit(gb_client);
        le_adresse_client->setObjectName("le_adresse_client");

        fl_client->setWidget(3, QFormLayout::FieldRole, le_adresse_client);

        lc5 = new QLabel(gb_client);
        lc5->setObjectName("lc5");

        fl_client->setWidget(4, QFormLayout::LabelRole, lc5);

        le_email_client = new QLineEdit(gb_client);
        le_email_client->setObjectName("le_email_client");

        fl_client->setWidget(4, QFormLayout::FieldRole, le_email_client);

        lc6 = new QLabel(gb_client);
        lc6->setObjectName("lc6");

        fl_client->setWidget(5, QFormLayout::LabelRole, lc6);

        sb_points_client = new QSpinBox(gb_client);
        sb_points_client->setObjectName("sb_points_client");
        sb_points_client->setMaximum(100000);

        fl_client->setWidget(5, QFormLayout::FieldRole, sb_points_client);


        vl_client_form->addWidget(gb_client);

        btn_valider_client = new QPushButton(page_client_form);
        btn_valider_client->setObjectName("btn_valider_client");

        vl_client_form->addWidget(btn_valider_client);

        vs_cf = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_client_form->addItem(vs_cf);

        stackedWidget->addWidget(page_client_form);
        page_stats_client = new QWidget();
        page_stats_client->setObjectName("page_stats_client");
        vl_stats_client = new QVBoxLayout(page_stats_client);
        vl_stats_client->setContentsMargins(40, 40, 40, 40);
        vl_stats_client->setObjectName("vl_stats_client");
        hl_head_sc = new QHBoxLayout();
        hl_head_sc->setObjectName("hl_head_sc");
        btn_back_stats_client = new QPushButton(page_stats_client);
        btn_back_stats_client->setObjectName("btn_back_stats_client");

        hl_head_sc->addWidget(btn_back_stats_client);

        lbl_title_sc = new QLabel(page_stats_client);
        lbl_title_sc->setObjectName("lbl_title_sc");
        lbl_title_sc->setAlignment(Qt::AlignmentFlag::AlignCenter);

        hl_head_sc->addWidget(lbl_title_sc);

        btn_dummy_cli = new QPushButton(page_stats_client);
        btn_dummy_cli->setObjectName("btn_dummy_cli");
        btn_dummy_cli->setFlat(true);

        hl_head_sc->addWidget(btn_dummy_cli);


        vl_stats_client->addLayout(hl_head_sc);

        hl_kpi_cli = new QHBoxLayout();
        hl_kpi_cli->setObjectName("hl_kpi_cli");
        st_cli1 = new QFrame(page_stats_client);
        st_cli1->setObjectName("st_cli1");
        v_cli1 = new QVBoxLayout(st_cli1);
        v_cli1->setObjectName("v_cli1");
        ico_cli_1 = new QLabel(st_cli1);
        ico_cli_1->setObjectName("ico_cli_1");
        ico_cli_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli1->addWidget(ico_cli_1);

        val_tot_cli = new QLabel(st_cli1);
        val_tot_cli->setObjectName("val_tot_cli");
        val_tot_cli->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli1->addWidget(val_tot_cli);

        lbl_tot_cli = new QLabel(st_cli1);
        lbl_tot_cli->setObjectName("lbl_tot_cli");
        lbl_tot_cli->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli1->addWidget(lbl_tot_cli);


        hl_kpi_cli->addWidget(st_cli1);

        st_cli2 = new QFrame(page_stats_client);
        st_cli2->setObjectName("st_cli2");
        v_cli2 = new QVBoxLayout(st_cli2);
        v_cli2->setObjectName("v_cli2");
        ico_cli_2 = new QLabel(st_cli2);
        ico_cli_2->setObjectName("ico_cli_2");
        ico_cli_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli2->addWidget(ico_cli_2);

        val_vip_cli = new QLabel(st_cli2);
        val_vip_cli->setObjectName("val_vip_cli");
        val_vip_cli->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli2->addWidget(val_vip_cli);

        lbl_vip_cli = new QLabel(st_cli2);
        lbl_vip_cli->setObjectName("lbl_vip_cli");
        lbl_vip_cli->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli2->addWidget(lbl_vip_cli);


        hl_kpi_cli->addWidget(st_cli2);

        st_cli3 = new QFrame(page_stats_client);
        st_cli3->setObjectName("st_cli3");
        v_cli3 = new QVBoxLayout(st_cli3);
        v_cli3->setObjectName("v_cli3");
        ico_cli_3 = new QLabel(st_cli3);
        ico_cli_3->setObjectName("ico_cli_3");
        ico_cli_3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli3->addWidget(ico_cli_3);

        val_avg_pts_cli = new QLabel(st_cli3);
        val_avg_pts_cli->setObjectName("val_avg_pts_cli");
        val_avg_pts_cli->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli3->addWidget(val_avg_pts_cli);

        lbl_avg_pts_cli = new QLabel(st_cli3);
        lbl_avg_pts_cli->setObjectName("lbl_avg_pts_cli");
        lbl_avg_pts_cli->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_cli3->addWidget(lbl_avg_pts_cli);


        hl_kpi_cli->addWidget(st_cli3);


        vl_stats_client->addLayout(hl_kpi_cli);

        hl_charts_cli = new QHBoxLayout();
        hl_charts_cli->setObjectName("hl_charts_cli");
        gb_geo_cli = new QGroupBox(page_stats_client);
        gb_geo_cli->setObjectName("gb_geo_cli");
        fl_geo_cli = new QFormLayout(gb_geo_cli);
        fl_geo_cli->setObjectName("fl_geo_cli");
        l_cli_tunis = new QLabel(gb_geo_cli);
        l_cli_tunis->setObjectName("l_cli_tunis");

        fl_geo_cli->setWidget(0, QFormLayout::LabelRole, l_cli_tunis);

        pb_cli_tunis = new QProgressBar(gb_geo_cli);
        pb_cli_tunis->setObjectName("pb_cli_tunis");
        pb_cli_tunis->setValue(0);

        fl_geo_cli->setWidget(0, QFormLayout::FieldRole, pb_cli_tunis);

        l_cli_sfax = new QLabel(gb_geo_cli);
        l_cli_sfax->setObjectName("l_cli_sfax");

        fl_geo_cli->setWidget(1, QFormLayout::LabelRole, l_cli_sfax);

        pb_cli_sfax = new QProgressBar(gb_geo_cli);
        pb_cli_sfax->setObjectName("pb_cli_sfax");
        pb_cli_sfax->setValue(0);

        fl_geo_cli->setWidget(1, QFormLayout::FieldRole, pb_cli_sfax);

        l_cli_autres = new QLabel(gb_geo_cli);
        l_cli_autres->setObjectName("l_cli_autres");

        fl_geo_cli->setWidget(2, QFormLayout::LabelRole, l_cli_autres);

        pb_cli_autres = new QProgressBar(gb_geo_cli);
        pb_cli_autres->setObjectName("pb_cli_autres");
        pb_cli_autres->setValue(0);

        fl_geo_cli->setWidget(2, QFormLayout::FieldRole, pb_cli_autres);


        hl_charts_cli->addWidget(gb_geo_cli);

        gb_fidel_cli = new QGroupBox(page_stats_client);
        gb_fidel_cli->setObjectName("gb_fidel_cli");
        fl_fidel_cli = new QFormLayout(gb_fidel_cli);
        fl_fidel_cli->setObjectName("fl_fidel_cli");
        l_cli_std = new QLabel(gb_fidel_cli);
        l_cli_std->setObjectName("l_cli_std");

        fl_fidel_cli->setWidget(0, QFormLayout::LabelRole, l_cli_std);

        pb_cli_std = new QProgressBar(gb_fidel_cli);
        pb_cli_std->setObjectName("pb_cli_std");
        pb_cli_std->setValue(0);

        fl_fidel_cli->setWidget(0, QFormLayout::FieldRole, pb_cli_std);

        l_cli_silver = new QLabel(gb_fidel_cli);
        l_cli_silver->setObjectName("l_cli_silver");

        fl_fidel_cli->setWidget(1, QFormLayout::LabelRole, l_cli_silver);

        pb_cli_silver = new QProgressBar(gb_fidel_cli);
        pb_cli_silver->setObjectName("pb_cli_silver");
        pb_cli_silver->setValue(0);

        fl_fidel_cli->setWidget(1, QFormLayout::FieldRole, pb_cli_silver);

        l_cli_vip = new QLabel(gb_fidel_cli);
        l_cli_vip->setObjectName("l_cli_vip");

        fl_fidel_cli->setWidget(2, QFormLayout::LabelRole, l_cli_vip);

        pb_cli_vip = new QProgressBar(gb_fidel_cli);
        pb_cli_vip->setObjectName("pb_cli_vip");
        pb_cli_vip->setValue(0);

        fl_fidel_cli->setWidget(2, QFormLayout::FieldRole, pb_cli_vip);


        hl_charts_cli->addWidget(gb_fidel_cli);


        vl_stats_client->addLayout(hl_charts_cli);

        hl_actions_cli = new QHBoxLayout();
        hl_actions_cli->setObjectName("hl_actions_cli");
        btn_export_excel_client = new QPushButton(page_stats_client);
        btn_export_excel_client->setObjectName("btn_export_excel_client");

        hl_actions_cli->addWidget(btn_export_excel_client);

        btn_print_client = new QPushButton(page_stats_client);
        btn_print_client->setObjectName("btn_print_client");

        hl_actions_cli->addWidget(btn_print_client);

        hs_actions_cli = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_actions_cli->addItem(hs_actions_cli);

        btn_close_stats_client_dash = new QPushButton(page_stats_client);
        btn_close_stats_client_dash->setObjectName("btn_close_stats_client_dash");

        hl_actions_cli->addWidget(btn_close_stats_client_dash);


        vl_stats_client->addLayout(hl_actions_cli);

        vs_sc = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_stats_client->addItem(vs_sc);

        stackedWidget->addWidget(page_stats_client);
        page_depot_list = new QWidget();
        page_depot_list->setObjectName("page_depot_list");
        vl_depot_list = new QVBoxLayout(page_depot_list);
        vl_depot_list->setContentsMargins(40, 40, 40, 40);
        vl_depot_list->setObjectName("vl_depot_list");
        lbl_dep = new QLabel(page_depot_list);
        lbl_dep->setObjectName("lbl_dep");

        vl_depot_list->addWidget(lbl_dep);

        hl_tools_dep = new QHBoxLayout();
        hl_tools_dep->setObjectName("hl_tools_dep");
        le_search_depot = new QLineEdit(page_depot_list);
        le_search_depot->setObjectName("le_search_depot");

        hl_tools_dep->addWidget(le_search_depot);

        btn_search_depot = new QPushButton(page_depot_list);
        btn_search_depot->setObjectName("btn_search_depot");

        hl_tools_dep->addWidget(btn_search_depot);

        btn_sort_alpha_depot = new QPushButton(page_depot_list);
        btn_sort_alpha_depot->setObjectName("btn_sort_alpha_depot");

        hl_tools_dep->addWidget(btn_sort_alpha_depot);

        btn_sort_remplissage = new QPushButton(page_depot_list);
        btn_sort_remplissage->setObjectName("btn_sort_remplissage");

        hl_tools_dep->addWidget(btn_sort_remplissage);

        btn_open_optimize = new QPushButton(page_depot_list);
        btn_open_optimize->setObjectName("btn_open_optimize");

        hl_tools_dep->addWidget(btn_open_optimize);

        btn_open_ravit = new QPushButton(page_depot_list);
        btn_open_ravit->setObjectName("btn_open_ravit");

        hl_tools_dep->addWidget(btn_open_ravit);

        btn_stats_depot = new QPushButton(page_depot_list);
        btn_stats_depot->setObjectName("btn_stats_depot");

        hl_tools_dep->addWidget(btn_stats_depot);

        btn_pdf_depot = new QPushButton(page_depot_list);
        btn_pdf_depot->setObjectName("btn_pdf_depot");

        hl_tools_dep->addWidget(btn_pdf_depot);

        btn_add_depot = new QPushButton(page_depot_list);
        btn_add_depot->setObjectName("btn_add_depot");

        hl_tools_dep->addWidget(btn_add_depot);


        vl_depot_list->addLayout(hl_tools_dep);

        tableDepot = new QTableWidget(page_depot_list);
        if (tableDepot->columnCount() < 7)
            tableDepot->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem27 = new QTableWidgetItem();
        tableDepot->setHorizontalHeaderItem(0, __qtablewidgetitem27);
        QTableWidgetItem *__qtablewidgetitem28 = new QTableWidgetItem();
        tableDepot->setHorizontalHeaderItem(1, __qtablewidgetitem28);
        QTableWidgetItem *__qtablewidgetitem29 = new QTableWidgetItem();
        tableDepot->setHorizontalHeaderItem(2, __qtablewidgetitem29);
        QTableWidgetItem *__qtablewidgetitem30 = new QTableWidgetItem();
        tableDepot->setHorizontalHeaderItem(3, __qtablewidgetitem30);
        QTableWidgetItem *__qtablewidgetitem31 = new QTableWidgetItem();
        tableDepot->setHorizontalHeaderItem(4, __qtablewidgetitem31);
        QTableWidgetItem *__qtablewidgetitem32 = new QTableWidgetItem();
        tableDepot->setHorizontalHeaderItem(5, __qtablewidgetitem32);
        QTableWidgetItem *__qtablewidgetitem33 = new QTableWidgetItem();
        tableDepot->setHorizontalHeaderItem(6, __qtablewidgetitem33);
        tableDepot->setObjectName("tableDepot");
        tableDepot->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        vl_depot_list->addWidget(tableDepot);

        hl_bot_dep = new QHBoxLayout();
        hl_bot_dep->setObjectName("hl_bot_dep");
        hs_dep = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_bot_dep->addItem(hs_dep);

        btn_edit_depot = new QPushButton(page_depot_list);
        btn_edit_depot->setObjectName("btn_edit_depot");

        hl_bot_dep->addWidget(btn_edit_depot);

        btn_delete_depot = new QPushButton(page_depot_list);
        btn_delete_depot->setObjectName("btn_delete_depot");

        hl_bot_dep->addWidget(btn_delete_depot);


        vl_depot_list->addLayout(hl_bot_dep);

        stackedWidget->addWidget(page_depot_list);
        page_depot_form = new QWidget();
        page_depot_form->setObjectName("page_depot_form");
        vl_depot_form = new QVBoxLayout(page_depot_form);
        vl_depot_form->setContentsMargins(40, 40, 40, 40);
        vl_depot_form->setObjectName("vl_depot_form");
        btn_back_depot = new QPushButton(page_depot_form);
        btn_back_depot->setObjectName("btn_back_depot");

        vl_depot_form->addWidget(btn_back_depot);

        gb_depot = new QGroupBox(page_depot_form);
        gb_depot->setObjectName("gb_depot");
        fl_depot = new QFormLayout(gb_depot);
        fl_depot->setObjectName("fl_depot");
        fl_depot->setVerticalSpacing(15);
        ld1 = new QLabel(gb_depot);
        ld1->setObjectName("ld1");

        fl_depot->setWidget(0, QFormLayout::LabelRole, ld1);

        le_id_depot = new QLineEdit(gb_depot);
        le_id_depot->setObjectName("le_id_depot");

        fl_depot->setWidget(0, QFormLayout::FieldRole, le_id_depot);

        ld2 = new QLabel(gb_depot);
        ld2->setObjectName("ld2");

        fl_depot->setWidget(1, QFormLayout::LabelRole, ld2);

        le_emplacement_depot = new QLineEdit(gb_depot);
        le_emplacement_depot->setObjectName("le_emplacement_depot");

        fl_depot->setWidget(1, QFormLayout::FieldRole, le_emplacement_depot);

        ld3 = new QLabel(gb_depot);
        ld3->setObjectName("ld3");

        fl_depot->setWidget(2, QFormLayout::LabelRole, ld3);

        le_etagere_depot = new QLineEdit(gb_depot);
        le_etagere_depot->setObjectName("le_etagere_depot");

        fl_depot->setWidget(2, QFormLayout::FieldRole, le_etagere_depot);

        ld4 = new QLabel(gb_depot);
        ld4->setObjectName("ld4");

        fl_depot->setWidget(3, QFormLayout::LabelRole, ld4);

        sb_capacite_depot = new QDoubleSpinBox(gb_depot);
        sb_capacite_depot->setObjectName("sb_capacite_depot");
        sb_capacite_depot->setMaximum(100000.000000000000000);

        fl_depot->setWidget(3, QFormLayout::FieldRole, sb_capacite_depot);

        ld5 = new QLabel(gb_depot);
        ld5->setObjectName("ld5");

        fl_depot->setWidget(4, QFormLayout::LabelRole, ld5);

        sb_qte_depot = new QDoubleSpinBox(gb_depot);
        sb_qte_depot->setObjectName("sb_qte_depot");
        sb_qte_depot->setMaximum(100000.000000000000000);

        fl_depot->setWidget(4, QFormLayout::FieldRole, sb_qte_depot);

        ld6 = new QLabel(gb_depot);
        ld6->setObjectName("ld6");

        fl_depot->setWidget(5, QFormLayout::LabelRole, ld6);

        cb_type_depot = new QComboBox(gb_depot);
        cb_type_depot->addItem(QString());
        cb_type_depot->addItem(QString());
        cb_type_depot->setObjectName("cb_type_depot");

        fl_depot->setWidget(5, QFormLayout::FieldRole, cb_type_depot);


        vl_depot_form->addWidget(gb_depot);

        btn_valider_depot = new QPushButton(page_depot_form);
        btn_valider_depot->setObjectName("btn_valider_depot");

        vl_depot_form->addWidget(btn_valider_depot);

        vs_df = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_depot_form->addItem(vs_df);

        stackedWidget->addWidget(page_depot_form);
        page_stats_depot = new QWidget();
        page_stats_depot->setObjectName("page_stats_depot");
        vl_stats_depot = new QVBoxLayout(page_stats_depot);
        vl_stats_depot->setContentsMargins(40, 40, 40, 40);
        vl_stats_depot->setObjectName("vl_stats_depot");
        hl_head_sd = new QHBoxLayout();
        hl_head_sd->setObjectName("hl_head_sd");
        btn_back_stats_depot = new QPushButton(page_stats_depot);
        btn_back_stats_depot->setObjectName("btn_back_stats_depot");

        hl_head_sd->addWidget(btn_back_stats_depot);

        lbl_title_sd = new QLabel(page_stats_depot);
        lbl_title_sd->setObjectName("lbl_title_sd");
        lbl_title_sd->setAlignment(Qt::AlignmentFlag::AlignCenter);

        hl_head_sd->addWidget(lbl_title_sd);

        btn_dummy_dep = new QPushButton(page_stats_depot);
        btn_dummy_dep->setObjectName("btn_dummy_dep");
        btn_dummy_dep->setFlat(true);

        hl_head_sd->addWidget(btn_dummy_dep);


        vl_stats_depot->addLayout(hl_head_sd);

        hl_kpi_dep = new QHBoxLayout();
        hl_kpi_dep->setObjectName("hl_kpi_dep");
        st_dep1 = new QFrame(page_stats_depot);
        st_dep1->setObjectName("st_dep1");
        v_dep1 = new QVBoxLayout(st_dep1);
        v_dep1->setObjectName("v_dep1");
        ico_dep_1 = new QLabel(st_dep1);
        ico_dep_1->setObjectName("ico_dep_1");
        ico_dep_1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep1->addWidget(ico_dep_1);

        val_tot_dep = new QLabel(st_dep1);
        val_tot_dep->setObjectName("val_tot_dep");
        val_tot_dep->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep1->addWidget(val_tot_dep);

        lbl_tot_dep = new QLabel(st_dep1);
        lbl_tot_dep->setObjectName("lbl_tot_dep");
        lbl_tot_dep->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep1->addWidget(lbl_tot_dep);


        hl_kpi_dep->addWidget(st_dep1);

        st_dep2 = new QFrame(page_stats_depot);
        st_dep2->setObjectName("st_dep2");
        v_dep2 = new QVBoxLayout(st_dep2);
        v_dep2->setObjectName("v_dep2");
        ico_dep_2 = new QLabel(st_dep2);
        ico_dep_2->setObjectName("ico_dep_2");
        ico_dep_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep2->addWidget(ico_dep_2);

        val_avg_fill = new QLabel(st_dep2);
        val_avg_fill->setObjectName("val_avg_fill");
        val_avg_fill->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep2->addWidget(val_avg_fill);

        lbl_avg_fill = new QLabel(st_dep2);
        lbl_avg_fill->setObjectName("lbl_avg_fill");
        lbl_avg_fill->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep2->addWidget(lbl_avg_fill);


        hl_kpi_dep->addWidget(st_dep2);

        st_dep3 = new QFrame(page_stats_depot);
        st_dep3->setObjectName("st_dep3");
        v_dep3 = new QVBoxLayout(st_dep3);
        v_dep3->setObjectName("v_dep3");
        ico_dep_3 = new QLabel(st_dep3);
        ico_dep_3->setObjectName("ico_dep_3");
        ico_dep_3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep3->addWidget(ico_dep_3);

        val_max_fill_dep = new QLabel(st_dep3);
        val_max_fill_dep->setObjectName("val_max_fill_dep");
        val_max_fill_dep->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep3->addWidget(val_max_fill_dep);

        lbl_max_fill_dep = new QLabel(st_dep3);
        lbl_max_fill_dep->setObjectName("lbl_max_fill_dep");
        lbl_max_fill_dep->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_dep3->addWidget(lbl_max_fill_dep);


        hl_kpi_dep->addWidget(st_dep3);


        vl_stats_depot->addLayout(hl_kpi_dep);

        hl_charts_dep = new QHBoxLayout();
        hl_charts_dep->setObjectName("hl_charts_dep");
        gb_type_dep = new QGroupBox(page_stats_depot);
        gb_type_dep->setObjectName("gb_type_dep");
        fl_type_dep = new QFormLayout(gb_type_dep);
        fl_type_dep->setObjectName("fl_type_dep");
        l_dep_froid = new QLabel(gb_type_dep);
        l_dep_froid->setObjectName("l_dep_froid");

        fl_type_dep->setWidget(0, QFormLayout::LabelRole, l_dep_froid);

        pb_dep_froid = new QProgressBar(gb_type_dep);
        pb_dep_froid->setObjectName("pb_dep_froid");
        pb_dep_froid->setValue(0);

        fl_type_dep->setWidget(0, QFormLayout::FieldRole, pb_dep_froid);

        l_dep_sec = new QLabel(gb_type_dep);
        l_dep_sec->setObjectName("l_dep_sec");

        fl_type_dep->setWidget(1, QFormLayout::LabelRole, l_dep_sec);

        pb_dep_sec = new QProgressBar(gb_type_dep);
        pb_dep_sec->setObjectName("pb_dep_sec");
        pb_dep_sec->setValue(0);

        fl_type_dep->setWidget(1, QFormLayout::FieldRole, pb_dep_sec);


        hl_charts_dep->addWidget(gb_type_dep);

        gb_fill_dep = new QGroupBox(page_stats_depot);
        gb_fill_dep->setObjectName("gb_fill_dep");
        fl_fill_dep = new QFormLayout(gb_fill_dep);
        fl_fill_dep->setObjectName("fl_fill_dep");
        l_dep_low = new QLabel(gb_fill_dep);
        l_dep_low->setObjectName("l_dep_low");

        fl_fill_dep->setWidget(0, QFormLayout::LabelRole, l_dep_low);

        pb_dep_low = new QProgressBar(gb_fill_dep);
        pb_dep_low->setObjectName("pb_dep_low");
        pb_dep_low->setValue(0);

        fl_fill_dep->setWidget(0, QFormLayout::FieldRole, pb_dep_low);

        l_dep_mid = new QLabel(gb_fill_dep);
        l_dep_mid->setObjectName("l_dep_mid");

        fl_fill_dep->setWidget(1, QFormLayout::LabelRole, l_dep_mid);

        pb_dep_mid = new QProgressBar(gb_fill_dep);
        pb_dep_mid->setObjectName("pb_dep_mid");
        pb_dep_mid->setValue(0);

        fl_fill_dep->setWidget(1, QFormLayout::FieldRole, pb_dep_mid);

        l_dep_high = new QLabel(gb_fill_dep);
        l_dep_high->setObjectName("l_dep_high");

        fl_fill_dep->setWidget(2, QFormLayout::LabelRole, l_dep_high);

        pb_dep_high = new QProgressBar(gb_fill_dep);
        pb_dep_high->setObjectName("pb_dep_high");
        pb_dep_high->setValue(0);

        fl_fill_dep->setWidget(2, QFormLayout::FieldRole, pb_dep_high);


        hl_charts_dep->addWidget(gb_fill_dep);


        vl_stats_depot->addLayout(hl_charts_dep);

        hl_actions_depot = new QHBoxLayout();
        hl_actions_depot->setObjectName("hl_actions_depot");
        btn_export_excel_depot = new QPushButton(page_stats_depot);
        btn_export_excel_depot->setObjectName("btn_export_excel_depot");

        hl_actions_depot->addWidget(btn_export_excel_depot);

        btn_print_depot = new QPushButton(page_stats_depot);
        btn_print_depot->setObjectName("btn_print_depot");

        hl_actions_depot->addWidget(btn_print_depot);

        hs_actions_depot = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_actions_depot->addItem(hs_actions_depot);

        btn_close_stats_depot_dash = new QPushButton(page_stats_depot);
        btn_close_stats_depot_dash->setObjectName("btn_close_stats_depot_dash");

        hl_actions_depot->addWidget(btn_close_stats_depot_dash);


        vl_stats_depot->addLayout(hl_actions_depot);

        vs_sd = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_stats_depot->addItem(vs_sd);

        stackedWidget->addWidget(page_stats_depot);
        page_stock_list = new QWidget();
        page_stock_list->setObjectName("page_stock_list");
        vl_stock = new QVBoxLayout(page_stock_list);
        vl_stock->setContentsMargins(40, 40, 40, 40);
        vl_stock->setObjectName("vl_stock");
        lbl_stk = new QLabel(page_stock_list);
        lbl_stk->setObjectName("lbl_stk");

        vl_stock->addWidget(lbl_stk);

        hl_tools_stk = new QHBoxLayout();
        hl_tools_stk->setObjectName("hl_tools_stk");
        le_search_stock = new QLineEdit(page_stock_list);
        le_search_stock->setObjectName("le_search_stock");

        hl_tools_stk->addWidget(le_search_stock);

        btn_search_stock = new QPushButton(page_stock_list);
        btn_search_stock->setObjectName("btn_search_stock");

        hl_tools_stk->addWidget(btn_search_stock);

        btn_sort_alpha_stock = new QPushButton(page_stock_list);
        btn_sort_alpha_stock->setObjectName("btn_sort_alpha_stock");

        hl_tools_stk->addWidget(btn_sort_alpha_stock);

        btn_open_compare = new QPushButton(page_stock_list);
        btn_open_compare->setObjectName("btn_open_compare");

        hl_tools_stk->addWidget(btn_open_compare);

        btn_open_calcul = new QPushButton(page_stock_list);
        btn_open_calcul->setObjectName("btn_open_calcul");

        hl_tools_stk->addWidget(btn_open_calcul);

        btn_stats_stock = new QPushButton(page_stock_list);
        btn_stats_stock->setObjectName("btn_stats_stock");

        hl_tools_stk->addWidget(btn_stats_stock);

        btn_pdf_stock = new QPushButton(page_stock_list);
        btn_pdf_stock->setObjectName("btn_pdf_stock");

        hl_tools_stk->addWidget(btn_pdf_stock);

        btn_add_stock = new QPushButton(page_stock_list);
        btn_add_stock->setObjectName("btn_add_stock");

        hl_tools_stk->addWidget(btn_add_stock);


        vl_stock->addLayout(hl_tools_stk);

        tableStock = new QTableWidget(page_stock_list);
        if (tableStock->columnCount() < 7)
            tableStock->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem34 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(0, __qtablewidgetitem34);
        QTableWidgetItem *__qtablewidgetitem35 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(1, __qtablewidgetitem35);
        QTableWidgetItem *__qtablewidgetitem36 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(2, __qtablewidgetitem36);
        QTableWidgetItem *__qtablewidgetitem37 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(3, __qtablewidgetitem37);
        QTableWidgetItem *__qtablewidgetitem38 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(4, __qtablewidgetitem38);
        QTableWidgetItem *__qtablewidgetitem39 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(5, __qtablewidgetitem39);
        QTableWidgetItem *__qtablewidgetitem40 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(6, __qtablewidgetitem40);
        tableStock->setObjectName("tableStock");
        tableStock->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        vl_stock->addWidget(tableStock);

        hl_bot_stk = new QHBoxLayout();
        hl_bot_stk->setObjectName("hl_bot_stk");
        hs_bs = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_bot_stk->addItem(hs_bs);

        btn_edit_stock = new QPushButton(page_stock_list);
        btn_edit_stock->setObjectName("btn_edit_stock");

        hl_bot_stk->addWidget(btn_edit_stock);

        btn_delete_stock = new QPushButton(page_stock_list);
        btn_delete_stock->setObjectName("btn_delete_stock");

        hl_bot_stk->addWidget(btn_delete_stock);


        vl_stock->addLayout(hl_bot_stk);

        stackedWidget->addWidget(page_stock_list);
        page_stock_form = new QWidget();
        page_stock_form->setObjectName("page_stock_form");
        vl_stk_form = new QVBoxLayout(page_stock_form);
        vl_stk_form->setContentsMargins(40, 40, 40, 40);
        vl_stk_form->setObjectName("vl_stk_form");
        btn_back_stock = new QPushButton(page_stock_form);
        btn_back_stock->setObjectName("btn_back_stock");

        vl_stk_form->addWidget(btn_back_stock);

        hl_forms_stk = new QHBoxLayout();
        hl_forms_stk->setObjectName("hl_forms_stk");
        gb_caract = new QGroupBox(page_stock_form);
        gb_caract->setObjectName("gb_caract");
        fl_caract = new QFormLayout(gb_caract);
        fl_caract->setObjectName("fl_caract");
        ls1 = new QLabel(gb_caract);
        ls1->setObjectName("ls1");

        fl_caract->setWidget(0, QFormLayout::LabelRole, ls1);

        le_code_mp = new QLineEdit(gb_caract);
        le_code_mp->setObjectName("le_code_mp");

        fl_caract->setWidget(0, QFormLayout::FieldRole, le_code_mp);

        ls2 = new QLabel(gb_caract);
        ls2->setObjectName("ls2");

        fl_caract->setWidget(1, QFormLayout::LabelRole, ls2);

        cb_cat_mp = new QComboBox(gb_caract);
        cb_cat_mp->addItem(QString());
        cb_cat_mp->addItem(QString());
        cb_cat_mp->addItem(QString());
        cb_cat_mp->setObjectName("cb_cat_mp");

        fl_caract->setWidget(1, QFormLayout::FieldRole, cb_cat_mp);

        ls3 = new QLabel(gb_caract);
        ls3->setObjectName("ls3");

        fl_caract->setWidget(2, QFormLayout::LabelRole, ls3);

        cb_etat_mp = new QComboBox(gb_caract);
        cb_etat_mp->addItem(QString());
        cb_etat_mp->addItem(QString());
        cb_etat_mp->setObjectName("cb_etat_mp");

        fl_caract->setWidget(2, QFormLayout::FieldRole, cb_etat_mp);

        ls4 = new QLabel(gb_caract);
        ls4->setObjectName("ls4");

        fl_caract->setWidget(3, QFormLayout::LabelRole, ls4);

        le_coul_mp = new QLineEdit(gb_caract);
        le_coul_mp->setObjectName("le_coul_mp");

        fl_caract->setWidget(3, QFormLayout::FieldRole, le_coul_mp);

        ls5 = new QLabel(gb_caract);
        ls5->setObjectName("ls5");

        fl_caract->setWidget(4, QFormLayout::LabelRole, ls5);

        cb_qual_mp = new QComboBox(gb_caract);
        cb_qual_mp->addItem(QString());
        cb_qual_mp->addItem(QString());
        cb_qual_mp->addItem(QString());
        cb_qual_mp->setObjectName("cb_qual_mp");

        fl_caract->setWidget(4, QFormLayout::FieldRole, cb_qual_mp);


        hl_forms_stk->addWidget(gb_caract);

        gb_log = new QGroupBox(page_stock_form);
        gb_log->setObjectName("gb_log");
        fl_log = new QFormLayout(gb_log);
        fl_log->setObjectName("fl_log");
        ls6 = new QLabel(gb_log);
        ls6->setObjectName("ls6");

        fl_log->setWidget(0, QFormLayout::LabelRole, ls6);

        hl_qte = new QHBoxLayout();
        hl_qte->setObjectName("hl_qte");
        sb_qte_mp = new QDoubleSpinBox(gb_log);
        sb_qte_mp->setObjectName("sb_qte_mp");
        sb_qte_mp->setMaximum(10000.000000000000000);

        hl_qte->addWidget(sb_qte_mp);

        cb_unite_mp = new QComboBox(gb_log);
        cb_unite_mp->addItem(QString());
        cb_unite_mp->addItem(QString());
        cb_unite_mp->setObjectName("cb_unite_mp");

        hl_qte->addWidget(cb_unite_mp);


        fl_log->setLayout(0, QFormLayout::FieldRole, hl_qte);

        ls7 = new QLabel(gb_log);
        ls7->setObjectName("ls7");

        fl_log->setWidget(1, QFormLayout::LabelRole, ls7);

        hl_zn = new QHBoxLayout();
        hl_zn->setObjectName("hl_zn");
        cb_zone_mp = new QComboBox(gb_log);
        cb_zone_mp->addItem(QString());
        cb_zone_mp->addItem(QString());
        cb_zone_mp->setObjectName("cb_zone_mp");

        hl_zn->addWidget(cb_zone_mp);

        le_allee_mp = new QLineEdit(gb_log);
        le_allee_mp->setObjectName("le_allee_mp");

        hl_zn->addWidget(le_allee_mp);


        fl_log->setLayout(1, QFormLayout::FieldRole, hl_zn);

        ls8 = new QLabel(gb_log);
        ls8->setObjectName("ls8");

        fl_log->setWidget(2, QFormLayout::LabelRole, ls8);

        cb_type_stock = new QComboBox(gb_log);
        cb_type_stock->addItem(QString());
        cb_type_stock->addItem(QString());
        cb_type_stock->addItem(QString());
        cb_type_stock->setObjectName("cb_type_stock");

        fl_log->setWidget(2, QFormLayout::FieldRole, cb_type_stock);

        ls9 = new QLabel(gb_log);
        ls9->setObjectName("ls9");

        fl_log->setWidget(3, QFormLayout::LabelRole, ls9);

        dt_rec_mp = new QDateEdit(gb_log);
        dt_rec_mp->setObjectName("dt_rec_mp");
        dt_rec_mp->setCalendarPopup(true);

        fl_log->setWidget(3, QFormLayout::FieldRole, dt_rec_mp);


        hl_forms_stk->addWidget(gb_log);


        vl_stk_form->addLayout(hl_forms_stk);

        btn_valider_stock = new QPushButton(page_stock_form);
        btn_valider_stock->setObjectName("btn_valider_stock");

        vl_stk_form->addWidget(btn_valider_stock);

        vs_sf = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_stk_form->addItem(vs_sf);

        stackedWidget->addWidget(page_stock_form);
        page_stock_inn = new QWidget();
        page_stock_inn->setObjectName("page_stock_inn");
        vl_stk_inn = new QVBoxLayout(page_stock_inn);
        vl_stk_inn->setObjectName("vl_stk_inn");
        btn_back_inn_stock = new QPushButton(page_stock_inn);
        btn_back_inn_stock->setObjectName("btn_back_inn_stock");

        vl_stk_inn->addWidget(btn_back_inn_stock);

        stackedWidget->addWidget(page_stock_inn);
        page_stock_stats = new QWidget();
        page_stock_stats->setObjectName("page_stock_stats");
        vl_stats_stk = new QVBoxLayout(page_stock_stats);
        vl_stats_stk->setContentsMargins(40, 40, 40, 40);
        vl_stats_stk->setObjectName("vl_stats_stk");
        hl_head_sst = new QHBoxLayout();
        hl_head_sst->setObjectName("hl_head_sst");
        btn_back_stats_stock = new QPushButton(page_stock_stats);
        btn_back_stats_stock->setObjectName("btn_back_stats_stock");

        hl_head_sst->addWidget(btn_back_stats_stock);

        lbl_title_sst = new QLabel(page_stock_stats);
        lbl_title_sst->setObjectName("lbl_title_sst");
        lbl_title_sst->setAlignment(Qt::AlignmentFlag::AlignCenter);

        hl_head_sst->addWidget(lbl_title_sst);

        btn_dummy4 = new QPushButton(page_stock_stats);
        btn_dummy4->setObjectName("btn_dummy4");
        btn_dummy4->setFlat(true);

        hl_head_sst->addWidget(btn_dummy4);


        vl_stats_stk->addLayout(hl_head_sst);

        hl_kpi_stk = new QHBoxLayout();
        hl_kpi_stk->setObjectName("hl_kpi_stk");
        st_stk1 = new QFrame(page_stock_stats);
        st_stk1->setObjectName("st_stk1");
        v_stk1 = new QVBoxLayout(st_stk1);
        v_stk1->setObjectName("v_stk1");
        ico_stk1 = new QLabel(st_stk1);
        ico_stk1->setObjectName("ico_stk1");
        ico_stk1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk1->addWidget(ico_stk1);

        val_tot_mat = new QLabel(st_stk1);
        val_tot_mat->setObjectName("val_tot_mat");
        val_tot_mat->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk1->addWidget(val_tot_mat);

        lbl_tot_mat = new QLabel(st_stk1);
        lbl_tot_mat->setObjectName("lbl_tot_mat");
        lbl_tot_mat->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk1->addWidget(lbl_tot_mat);


        hl_kpi_stk->addWidget(st_stk1);

        st_stk2 = new QFrame(page_stock_stats);
        st_stk2->setObjectName("st_stk2");
        v_stk2 = new QVBoxLayout(st_stk2);
        v_stk2->setObjectName("v_stk2");
        ico_stk2 = new QLabel(st_stk2);
        ico_stk2->setObjectName("ico_stk2");
        ico_stk2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk2->addWidget(ico_stk2);

        val_tot_vol = new QLabel(st_stk2);
        val_tot_vol->setObjectName("val_tot_vol");
        val_tot_vol->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk2->addWidget(val_tot_vol);

        lbl_tot_vol = new QLabel(st_stk2);
        lbl_tot_vol->setObjectName("lbl_tot_vol");
        lbl_tot_vol->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk2->addWidget(lbl_tot_vol);


        hl_kpi_stk->addWidget(st_stk2);

        st_stk3 = new QFrame(page_stock_stats);
        st_stk3->setObjectName("st_stk3");
        v_stk3 = new QVBoxLayout(st_stk3);
        v_stk3->setObjectName("v_stk3");
        ico_stk3 = new QLabel(st_stk3);
        ico_stk3->setObjectName("ico_stk3");
        ico_stk3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk3->addWidget(ico_stk3);

        val_tot_val_stock = new QLabel(st_stk3);
        val_tot_val_stock->setObjectName("val_tot_val_stock");
        val_tot_val_stock->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk3->addWidget(val_tot_val_stock);

        lbl_tot_val_stock = new QLabel(st_stk3);
        lbl_tot_val_stock->setObjectName("lbl_tot_val_stock");
        lbl_tot_val_stock->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_stk3->addWidget(lbl_tot_val_stock);


        hl_kpi_stk->addWidget(st_stk3);


        vl_stats_stk->addLayout(hl_kpi_stk);

        hl_charts_stk = new QHBoxLayout();
        hl_charts_stk->setObjectName("hl_charts_stk");
        gb_chart_cat = new QGroupBox(page_stock_stats);
        gb_chart_cat->setObjectName("gb_chart_cat");
        fl_cat = new QFormLayout(gb_chart_cat);
        fl_cat->setObjectName("fl_cat");
        l_cuir = new QLabel(gb_chart_cat);
        l_cuir->setObjectName("l_cuir");

        fl_cat->setWidget(0, QFormLayout::LabelRole, l_cuir);

        pb_cuir = new QProgressBar(gb_chart_cat);
        pb_cuir->setObjectName("pb_cuir");
        pb_cuir->setValue(0);

        fl_cat->setWidget(0, QFormLayout::FieldRole, pb_cuir);

        l_teint = new QLabel(gb_chart_cat);
        l_teint->setObjectName("l_teint");

        fl_cat->setWidget(1, QFormLayout::LabelRole, l_teint);

        pb_teint = new QProgressBar(gb_chart_cat);
        pb_teint->setObjectName("pb_teint");
        pb_teint->setValue(0);

        fl_cat->setWidget(1, QFormLayout::FieldRole, pb_teint);

        l_chim = new QLabel(gb_chart_cat);
        l_chim->setObjectName("l_chim");

        fl_cat->setWidget(2, QFormLayout::LabelRole, l_chim);

        pb_chim = new QProgressBar(gb_chart_cat);
        pb_chim->setObjectName("pb_chim");
        pb_chim->setValue(0);

        fl_cat->setWidget(2, QFormLayout::FieldRole, pb_chim);


        hl_charts_stk->addWidget(gb_chart_cat);

        gb_chart_qual = new QGroupBox(page_stock_stats);
        gb_chart_qual->setObjectName("gb_chart_qual");
        fl_qual = new QFormLayout(gb_chart_qual);
        fl_qual->setObjectName("fl_qual");
        l_qa = new QLabel(gb_chart_qual);
        l_qa->setObjectName("l_qa");

        fl_qual->setWidget(0, QFormLayout::LabelRole, l_qa);

        pb_qa = new QProgressBar(gb_chart_qual);
        pb_qa->setObjectName("pb_qa");
        pb_qa->setValue(0);

        fl_qual->setWidget(0, QFormLayout::FieldRole, pb_qa);

        l_qb = new QLabel(gb_chart_qual);
        l_qb->setObjectName("l_qb");

        fl_qual->setWidget(1, QFormLayout::LabelRole, l_qb);

        pb_qb = new QProgressBar(gb_chart_qual);
        pb_qb->setObjectName("pb_qb");
        pb_qb->setValue(0);

        fl_qual->setWidget(1, QFormLayout::FieldRole, pb_qb);

        l_qc = new QLabel(gb_chart_qual);
        l_qc->setObjectName("l_qc");

        fl_qual->setWidget(2, QFormLayout::LabelRole, l_qc);

        pb_qc = new QProgressBar(gb_chart_qual);
        pb_qc->setObjectName("pb_qc");
        pb_qc->setValue(0);

        fl_qual->setWidget(2, QFormLayout::FieldRole, pb_qc);


        hl_charts_stk->addWidget(gb_chart_qual);


        vl_stats_stk->addLayout(hl_charts_stk);

        hl_extra_charts_stk = new QHBoxLayout();
        hl_extra_charts_stk->setObjectName("hl_extra_charts_stk");
        gb_pie_stk = new QGroupBox(page_stock_stats);
        gb_pie_stk->setObjectName("gb_pie_stk");
        vl_pie_stk = new QVBoxLayout(gb_pie_stk);
        vl_pie_stk->setObjectName("vl_pie_stk");
        vl_pie_stk->setContentsMargins(0, 0, 0, 0);
        w_pie_stk = new QWidget(gb_pie_stk);
        w_pie_stk->setObjectName("w_pie_stk");

        vl_pie_stk->addWidget(w_pie_stk);


        hl_extra_charts_stk->addWidget(gb_pie_stk);

        gb_bar_stk = new QGroupBox(page_stock_stats);
        gb_bar_stk->setObjectName("gb_bar_stk");
        vl_bar_stk = new QVBoxLayout(gb_bar_stk);
        vl_bar_stk->setObjectName("vl_bar_stk");
        vl_bar_stk->setContentsMargins(0, 0, 0, 0);
        w_bar_stk = new QWidget(gb_bar_stk);
        w_bar_stk->setObjectName("w_bar_stk");

        vl_bar_stk->addWidget(w_bar_stk);


        hl_extra_charts_stk->addWidget(gb_bar_stk);


        vl_stats_stk->addLayout(hl_extra_charts_stk);

        hl_actions_sst = new QHBoxLayout();
        hl_actions_sst->setObjectName("hl_actions_sst");
        btn_export_excel_stock = new QPushButton(page_stock_stats);
        btn_export_excel_stock->setObjectName("btn_export_excel_stock");

        hl_actions_sst->addWidget(btn_export_excel_stock);

        btn_print_stock = new QPushButton(page_stock_stats);
        btn_print_stock->setObjectName("btn_print_stock");

        hl_actions_sst->addWidget(btn_print_stock);

        hs_actions_sst = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        hl_actions_sst->addItem(hs_actions_sst);

        btn_close_stats_stock = new QPushButton(page_stock_stats);
        btn_close_stats_stock->setObjectName("btn_close_stats_stock");

        hl_actions_sst->addWidget(btn_close_stats_stock);


        vl_stats_stk->addLayout(hl_actions_sst);

        vs_stk_end = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        vl_stats_stk->addItem(vs_stk_end);

        stackedWidget->addWidget(page_stock_stats);
        page_fab_detail = new QWidget();
        page_fab_detail->setObjectName("page_fab_detail");
        vl_4 = new QVBoxLayout(page_fab_detail);
        vl_4->setObjectName("vl_4");
        btn_back_fab = new QPushButton(page_fab_detail);
        btn_back_fab->setObjectName("btn_back_fab");

        vl_4->addWidget(btn_back_fab);

        stackedWidget->addWidget(page_fab_detail);

        horizontalLayout->addWidget(stackedWidget);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "FIL D'OR - Production Manager", nullptr));
        l_app_title->setText(QCoreApplication::translate("MainWindow", "FIL D'OR", nullptr));
        btn_nav_dashboard->setText(QCoreApplication::translate("MainWindow", " \360\237\217\240  ACCUEIL", nullptr));
        btn_nav_dashboard->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "nav_btn", nullptr)));
        btn_nav_produit->setText(QCoreApplication::translate("MainWindow", " \360\237\221\234  Produits", nullptr));
        btn_nav_produit->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "nav_btn", nullptr)));
        btn_nav_stock->setText(QCoreApplication::translate("MainWindow", " \360\237\223\246  Mati\303\250res & Premi\303\250res", nullptr));
        btn_nav_stock->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "nav_btn", nullptr)));
        btn_nav_clients->setText(QCoreApplication::translate("MainWindow", " \360\237\247\276  CLIENTS", nullptr));
        btn_nav_clients->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "nav_btn", nullptr)));
        btn_nav_depot->setText(QCoreApplication::translate("MainWindow", " \360\237\217\254  D\303\211P\303\224T", nullptr));
        btn_nav_depot->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "nav_btn", nullptr)));
        btn_nav_planif->setText(QCoreApplication::translate("MainWindow", " \360\237\223\205  PLANIFICATION", nullptr));
        btn_nav_planif->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "nav_btn", nullptr)));
        btn_nav_fab->setText(QCoreApplication::translate("MainWindow", " \360\237\217\255   Fabrication & SUIVI", nullptr));
        btn_nav_fab->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "nav_btn", nullptr)));
        btn_nav_rh->setText(QCoreApplication::translate("MainWindow", " \360\237\221\245  Employ\303\251s & ", nullptr));
        btn_nav_rh->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "nav_btn", nullptr)));
        lbl_welcome_title->setText(QCoreApplication::translate("MainWindow", "\342\234\250 FIL D'OR \342\234\250", nullptr));
        lbl_welcome_sub->setText(QCoreApplication::translate("MainWindow", "L'Excellence de la Maroquinerie", nullptr));
        btn_start_app->setText(QCoreApplication::translate("MainWindow", "Entrer dans l'Atelier \342\236\224", nullptr));
        lbl_login_title->setText(QCoreApplication::translate("MainWindow", "Connexion", nullptr));
        lbl_login_sub->setText(QCoreApplication::translate("MainWindow", "Acc\303\250s s\303\251curis\303\251 \303\240 l'atelier FIL D'OR", nullptr));
        le_login_nom->setPlaceholderText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        le_login_prenom->setPlaceholderText(QCoreApplication::translate("MainWindow", "Pr\303\251nom", nullptr));
        le_login_mdp->setPlaceholderText(QCoreApplication::translate("MainWindow", "Mot de passe", nullptr));
        btn_login_back->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour", nullptr));
        btn_login->setText(QCoreApplication::translate("MainWindow", "Se connecter", nullptr));
        lbl_p1->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 28px; font-weight: 300; color:#2c1a16;", nullptr));
        lbl_p1->setText(QCoreApplication::translate("MainWindow", "\360\237\223\205 Planification des Commandes", nullptr));
        le_recherche->setPlaceholderText(QCoreApplication::translate("MainWindow", "\360\237\224\215 Rechercher un OF...", nullptr));
        btn_rechercher->setText(QCoreApplication::translate("MainWindow", "Chercher", nullptr));
        btn_sort_alpha_planif->setText(QCoreApplication::translate("MainWindow", "Tri", nullptr));
        btn_open_planif_ia->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #7b1fa2; color: white;", nullptr));
        btn_open_planif_ia->setText(QCoreApplication::translate("MainWindow", "\360\237\247\240 IA Estimation Temps", nullptr));
        btn_stat_plan->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212 Analyses", nullptr));
        btn_pdf->setText(QCoreApplication::translate("MainWindow", "\360\237\223\204 PDF", nullptr));
        btn_to_add_planif->setText(QCoreApplication::translate("MainWindow", "+ Cr\303\251er Ordre", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tablePlanif->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tablePlanif->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Produit", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tablePlanif->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Qt\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tablePlanif->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Mati\303\250re", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tablePlanif->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "D\303\251but", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tablePlanif->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "Fin Est.", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tablePlanif->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "Statut", nullptr));
        btn_tri->setText(QCoreApplication::translate("MainWindow", "\342\232\241 Tri Urgence", nullptr));
        btn_modifier_planif->setText(QCoreApplication::translate("MainWindow", "\342\234\217\357\270\217 Modifier", nullptr));
        btn_supprimer_planif->setText(QCoreApplication::translate("MainWindow", "\360\237\227\221\357\270\217 Supprimer", nullptr));
        l_tf->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 28px; font-weight: bold; color:#2c1a16;", nullptr));
        l_tf->setText(QCoreApplication::translate("MainWindow", "Planning & Supervision", nullptr));
        l_sub_f->setStyleSheet(QCoreApplication::translate("MainWindow", "color: #8d5524; font-weight: 600;", nullptr));
        l_sub_f->setText(QCoreApplication::translate("MainWindow", "Cliquez sur une commande pour valider les \303\251tapes", nullptr));
        l_legende_colors->setStyleSheet(QCoreApplication::translate("MainWindow", "font-weight: bold; background: white; padding: 10px; border: 1px solid #d7ccc8; border-radius: 4px;", nullptr));
        l_legende_colors->setText(QCoreApplication::translate("MainWindow", "\360\237\237\246 Coupe   \360\237\237\247 Assemblage   \360\237\237\251 Couture   \360\237\237\250 Finition   \360\237\237\245 Retard", nullptr));
        btn_refresh_timeline->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #90a4ae; color: white; padding: 8px 15px;", nullptr));
        btn_refresh_timeline->setText(QCoreApplication::translate("MainWindow", "\360\237\224\204 Actualiser", nullptr));
        btn_saisie_detail->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #795548; color: white; padding: 8px 15px;", nullptr));
        btn_saisie_detail->setText(QCoreApplication::translate("MainWindow", "Saisir Temps / D\303\251tail", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableTimeline->horizontalHeaderItem(0);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "PRODUIT", nullptr));
        line_sep_fab->setStyleSheet(QCoreApplication::translate("MainWindow", "color: #d7ccc8;", nullptr));
        frame_supervision->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #fff; border: 1px solid #d7ccc8; border-radius: 4px;", nullptr));
        lbl_sel_cmd_title->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 16px; font-weight: bold; color: #3e2723; border:none;", nullptr));
        lbl_sel_cmd_title->setText(QCoreApplication::translate("MainWindow", "S\303\251lectionnez une commande...", nullptr));
        lbl_sel_cmd->setStyleSheet(QCoreApplication::translate("MainWindow", "color: #5d4037; border:none;", nullptr));
        lbl_sel_cmd->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        l_et->setStyleSheet(QCoreApplication::translate("MainWindow", "border:none;", nullptr));
        l_et->setText(QCoreApplication::translate("MainWindow", "\303\211tape :", nullptr));
        cb_etape_suivi->setItemText(0, QCoreApplication::translate("MainWindow", "Coupe", nullptr));
        cb_etape_suivi->setItemText(1, QCoreApplication::translate("MainWindow", "Assemblage", nullptr));
        cb_etape_suivi->setItemText(2, QCoreApplication::translate("MainWindow", "Couture", nullptr));
        cb_etape_suivi->setItemText(3, QCoreApplication::translate("MainWindow", "Finition", nullptr));

        l_tp->setStyleSheet(QCoreApplication::translate("MainWindow", "border:none;", nullptr));
        l_tp->setText(QCoreApplication::translate("MainWindow", "Pr\303\251vu (h) :", nullptr));
        lbl_temps_prevu->setStyleSheet(QCoreApplication::translate("MainWindow", "font-weight:bold; color:#e65100; border:none;", nullptr));
        lbl_temps_prevu->setText(QCoreApplication::translate("MainWindow", "2 h", nullptr));
        l_tr->setStyleSheet(QCoreApplication::translate("MainWindow", "border:none;", nullptr));
        l_tr->setText(QCoreApplication::translate("MainWindow", "R\303\251el (h) :", nullptr));
        btn_valider_etape->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #8d5524; color: white; font-weight: bold; font-size: 14px; border-radius: 4px;", nullptr));
        btn_valider_etape->setText(QCoreApplication::translate("MainWindow", "Valider", nullptr));
        lbl_resultat_delta->setStyleSheet(QCoreApplication::translate("MainWindow", "border:none;", nullptr));
        lbl_resultat_delta->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        btn_back_planif->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Annuler", nullptr));
        gb_form->setTitle(QCoreApplication::translate("MainWindow", "D\303\251tails", nullptr));
        l1->setText(QCoreApplication::translate("MainWindow", "Produit :", nullptr));
        cb_produit->setItemText(0, QCoreApplication::translate("MainWindow", "Sac Voyage Cuir", nullptr));
        cb_produit->setItemText(1, QCoreApplication::translate("MainWindow", "Portefeuille Luxe", nullptr));

        l2->setText(QCoreApplication::translate("MainWindow", "Quantit\303\251 :", nullptr));
        l3->setText(QCoreApplication::translate("MainWindow", "Mati\303\250re :", nullptr));
        cb_matiere->setItemText(0, QCoreApplication::translate("MainWindow", "Cuir Vachette", nullptr));
        cb_matiere->setItemText(1, QCoreApplication::translate("MainWindow", "Cuir Agneau", nullptr));

        l4->setText(QCoreApplication::translate("MainWindow", "Lancement :", nullptr));
        l5->setText(QCoreApplication::translate("MainWindow", "IA :", nullptr));
        le_fin_prevue->setPlaceholderText(QCoreApplication::translate("MainWindow", "Calcul...", nullptr));
        btn_calculer_ia->setText(QCoreApplication::translate("MainWindow", "\342\232\241", nullptr));
        le_duree_totale->setPlaceholderText(QCoreApplication::translate("MainWindow", "Dur\303\251e...", nullptr));
        btn_valider_planif->setText(QCoreApplication::translate("MainWindow", "\342\234\205 CONFIRMER", nullptr));
        btn_back_stats->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour", nullptr));
        lbl_title_s->setStyleSheet(QString());
        lbl_title_s->setText(QCoreApplication::translate("MainWindow", "TABLEAU DE BORD - PLANIFICATION", nullptr));
        st1->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_plan_1->setText(QCoreApplication::translate("MainWindow", "\360\237\223\246", nullptr));
        lbl_stat_total_cmd->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lbl_stat_total_cmd->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_t1->setText(QCoreApplication::translate("MainWindow", "Total Ordres", nullptr));
        st2->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_plan_2->setText(QCoreApplication::translate("MainWindow", "\360\237\247\265", nullptr));
        lbl_stat_total_qty->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lbl_stat_total_qty->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_t2->setText(QCoreApplication::translate("MainWindow", "Production", nullptr));
        st3->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_plan_3->setText(QCoreApplication::translate("MainWindow", "\342\217\261\357\270\217", nullptr));
        lbl_stat_retard->setText(QCoreApplication::translate("MainWindow", "0%", nullptr));
        lbl_stat_retard->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_t3->setText(QCoreApplication::translate("MainWindow", "Taux Retard", nullptr));
        gb_charts->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par Mod\303\250les", nullptr));
        l_c1->setText(QCoreApplication::translate("MainWindow", "Sacs", nullptr));
        l_c2->setText(QCoreApplication::translate("MainWindow", "Portefeuilles", nullptr));
        l_c3->setText(QCoreApplication::translate("MainWindow", "Ceintures", nullptr));
        gb_charts_statut->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par Statut", nullptr));
        l_cmd_plan->setText(QCoreApplication::translate("MainWindow", "Planifi\303\251", nullptr));
        l_cmd_cours->setText(QCoreApplication::translate("MainWindow", "En cours", nullptr));
        l_cmd_ret->setText(QCoreApplication::translate("MainWindow", "Retard", nullptr));
        btn_export_excel_planif->setText(QCoreApplication::translate("MainWindow", "Exporter Excel", nullptr));
        btn_export_excel_planif->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_excel", nullptr)));
        btn_print_planif->setText(QCoreApplication::translate("MainWindow", "Imprimer", nullptr));
        btn_print_planif->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_print", nullptr)));
        btn_close_stats_planif->setText(QCoreApplication::translate("MainWindow", "Fermer", nullptr));
        btn_close_stats_planif->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_close", nullptr)));
        lbl_t_prod->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 28px; font-weight: 300; color:#2c1a16;", nullptr));
        lbl_t_prod->setText(QCoreApplication::translate("MainWindow", "\360\237\221\234 Catalogue Produits", nullptr));
        le_search_coll->setPlaceholderText(QCoreApplication::translate("MainWindow", "\360\237\224\215 Collection...", nullptr));
        btn_search_col->setText(QCoreApplication::translate("MainWindow", "Filtrer", nullptr));
        btn_sort_alpha_prod->setText(QCoreApplication::translate("MainWindow", "Tri", nullptr));
        btn_cout_produit->setText(QCoreApplication::translate("MainWindow", "\360\237\247\276 Co\303\273t de Produit", nullptr));
        btn_hist_mode->setText(QCoreApplication::translate("MainWindow", "\360\237\225\260\357\270\217 Historique de Mode", nullptr));
        btn_stats_prod->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212 Stats", nullptr));
        btn_pdf_catalogue->setText(QCoreApplication::translate("MainWindow", "\360\237\223\204 PDF", nullptr));
        btn_add_produit->setText(QCoreApplication::translate("MainWindow", "+ Nouveau", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableProduits->horizontalHeaderItem(0);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "R\303\251f", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = tableProduits->horizontalHeaderItem(1);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "D\303\251signation", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = tableProduits->horizontalHeaderItem(2);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("MainWindow", "Co\303\273t", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = tableProduits->horizontalHeaderItem(3);
        ___qtablewidgetitem11->setText(QCoreApplication::translate("MainWindow", "Collection", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = tableProduits->horizontalHeaderItem(4);
        ___qtablewidgetitem12->setText(QCoreApplication::translate("MainWindow", "Cuir", nullptr));
        QTableWidgetItem *___qtablewidgetitem13 = tableProduits->horizontalHeaderItem(5);
        ___qtablewidgetitem13->setText(QCoreApplication::translate("MainWindow", "Temps", nullptr));
        btn_edit_produit->setText(QCoreApplication::translate("MainWindow", "\342\234\217\357\270\217 Modifier", nullptr));
        btn_delete_produit->setText(QCoreApplication::translate("MainWindow", "\360\237\227\221\357\270\217 Supprimer", nullptr));
        btn_back_prod->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour", nullptr));
        gb_prod->setTitle(QCoreApplication::translate("MainWindow", "Fiche Produit", nullptr));
        lp1->setText(QCoreApplication::translate("MainWindow", "R\303\251f\303\251rence :", nullptr));
        lp2->setText(QCoreApplication::translate("MainWindow", "Nom :", nullptr));
        lp3->setText(QCoreApplication::translate("MainWindow", "Collection :", nullptr));
        cb_coll_prod->setItemText(0, QCoreApplication::translate("MainWindow", "Hiver 2026", nullptr));
        cb_coll_prod->setItemText(1, QCoreApplication::translate("MainWindow", "\303\211t\303\251 2026", nullptr));
        cb_coll_prod->setItemText(2, QCoreApplication::translate("MainWindow", "Intemporel", nullptr));

        lp4->setText(QCoreApplication::translate("MainWindow", "Cuir :", nullptr));
        cb_cuir_prod->setItemText(0, QCoreApplication::translate("MainWindow", "Vachette", nullptr));
        cb_cuir_prod->setItemText(1, QCoreApplication::translate("MainWindow", "Agneau", nullptr));
        cb_cuir_prod->setItemText(2, QCoreApplication::translate("MainWindow", "Croco", nullptr));

        lp5->setText(QCoreApplication::translate("MainWindow", "Co\303\273t (\342\202\254) :", nullptr));
        lp6->setText(QCoreApplication::translate("MainWindow", "Temps (h) :", nullptr));
        lp7->setText(QCoreApplication::translate("MainWindow", "Total Est. :", nullptr));
        lbl_cout_total->setStyleSheet(QCoreApplication::translate("MainWindow", "font-weight:bold; color:#8d5524;", nullptr));
        lbl_cout_total->setText(QCoreApplication::translate("MainWindow", "0 \342\202\254", nullptr));
        btn_valider_produit->setText(QCoreApplication::translate("MainWindow", "\342\234\205 ENREGISTRER", nullptr));
        btn_back_stats_prod->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour", nullptr));
        lbl_title_sp->setStyleSheet(QString());
        lbl_title_sp->setText(QCoreApplication::translate("MainWindow", "TABLEAU DE BORD - CATALOGUE", nullptr));
        stp1->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_prod_1->setText(QCoreApplication::translate("MainWindow", "\360\237\221\234", nullptr));
        val_tot_ref->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        val_tot_ref->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_tot_ref->setText(QCoreApplication::translate("MainWindow", "R\303\251f\303\251rences", nullptr));
        stp2->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_prod_2->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212", nullptr));
        val_avg_cost->setText(QCoreApplication::translate("MainWindow", "0 \342\202\254", nullptr));
        val_avg_cost->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_avg_cost->setText(QCoreApplication::translate("MainWindow", "Co\303\273t Moyen", nullptr));
        stp3->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_prod_3->setText(QCoreApplication::translate("MainWindow", "\360\237\222\260", nullptr));
        val_tot_val_prod->setText(QCoreApplication::translate("MainWindow", "0 \342\202\254", nullptr));
        val_tot_val_prod->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_tot_val_prod->setText(QCoreApplication::translate("MainWindow", "Valeur Totale", nullptr));
        gb_chart_coll->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par Collection", nullptr));
        l_hiver->setText(QCoreApplication::translate("MainWindow", "Hiver", nullptr));
        l_ete->setText(QCoreApplication::translate("MainWindow", "\303\211t\303\251", nullptr));
        l_intemp->setText(QCoreApplication::translate("MainWindow", "Intemporel", nullptr));
        gb_chart_cuir->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par Cuir", nullptr));
        l_cuir_v->setText(QCoreApplication::translate("MainWindow", "Vachette", nullptr));
        l_cuir_a->setText(QCoreApplication::translate("MainWindow", "Agneau", nullptr));
        l_cuir_c->setText(QCoreApplication::translate("MainWindow", "Croco", nullptr));
        btn_export_excel_prod->setText(QCoreApplication::translate("MainWindow", "Exporter Excel", nullptr));
        btn_export_excel_prod->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_excel", nullptr)));
        btn_print_prod->setText(QCoreApplication::translate("MainWindow", "Imprimer", nullptr));
        btn_print_prod->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_print", nullptr)));
        btn_close_stats_prod->setText(QCoreApplication::translate("MainWindow", "Fermer", nullptr));
        btn_close_stats_prod->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_close", nullptr)));
        lbl_emp->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 28px; font-weight: 300; color:#2c1a16;", nullptr));
        lbl_emp->setText(QCoreApplication::translate("MainWindow", "\360\237\221\245 Gestion Ressources Humaines", nullptr));
        le_search_emp->setPlaceholderText(QCoreApplication::translate("MainWindow", "\360\237\224\215 Nom, Poste...", nullptr));
        btn_search_emp->setText(QCoreApplication::translate("MainWindow", "Chercher", nullptr));
        btn_sort_alpha_emp->setText(QCoreApplication::translate("MainWindow", "Tri", nullptr));
        btn_sort_anciennete->setText(QCoreApplication::translate("MainWindow", "\360\237\223\205 Anciennet\303\251", nullptr));
        btn_open_eval->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #ef6c00; color: white;", nullptr));
        btn_open_eval->setText(QCoreApplication::translate("MainWindow", "\360\237\223\235 \303\211valuation Comp\303\251tences", nullptr));
        btn_stats_emp->setText(QCoreApplication::translate("MainWindow", "\360\237\244\226 Assistant RH", nullptr));
        btn_stats_emp_stats->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212 Statistiques", nullptr));
        btn_pdf_emp->setText(QCoreApplication::translate("MainWindow", "\360\237\223\204 PDF", nullptr));
        btn_add_emp->setText(QCoreApplication::translate("MainWindow", "+ Recruter", nullptr));
        QTableWidgetItem *___qtablewidgetitem14 = tableEmployes->horizontalHeaderItem(0);
        ___qtablewidgetitem14->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem15 = tableEmployes->horizontalHeaderItem(1);
        ___qtablewidgetitem15->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem16 = tableEmployes->horizontalHeaderItem(2);
        ___qtablewidgetitem16->setText(QCoreApplication::translate("MainWindow", "Pr\303\251nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem17 = tableEmployes->horizontalHeaderItem(3);
        ___qtablewidgetitem17->setText(QCoreApplication::translate("MainWindow", "Poste", nullptr));
        QTableWidgetItem *___qtablewidgetitem18 = tableEmployes->horizontalHeaderItem(4);
        ___qtablewidgetitem18->setText(QCoreApplication::translate("MainWindow", "D\303\251partement", nullptr));
        QTableWidgetItem *___qtablewidgetitem19 = tableEmployes->horizontalHeaderItem(5);
        ___qtablewidgetitem19->setText(QCoreApplication::translate("MainWindow", "Salaire", nullptr));
        QTableWidgetItem *___qtablewidgetitem20 = tableEmployes->horizontalHeaderItem(6);
        ___qtablewidgetitem20->setText(QCoreApplication::translate("MainWindow", "Embauche", nullptr));
        btn_edit_emp->setText(QCoreApplication::translate("MainWindow", "\342\234\217\357\270\217 Modifier", nullptr));
        btn_delete_emp->setText(QCoreApplication::translate("MainWindow", "\360\237\227\221\357\270\217 Supprimer", nullptr));
        btn_back_emp->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour RH", nullptr));
        gb_emp->setTitle(QCoreApplication::translate("MainWindow", "Fiche Employ\303\251", nullptr));
        le1->setText(QCoreApplication::translate("MainWindow", "ID Unique :", nullptr));
        le2->setText(QCoreApplication::translate("MainWindow", "Nom :", nullptr));
        le3->setText(QCoreApplication::translate("MainWindow", "Pr\303\251nom :", nullptr));
        le4->setText(QCoreApplication::translate("MainWindow", "Poste :", nullptr));
        cb_poste_emp->setItemText(0, QCoreApplication::translate("MainWindow", "Artisan Maroquinier", nullptr));
        cb_poste_emp->setItemText(1, QCoreApplication::translate("MainWindow", "Coupeur", nullptr));
        cb_poste_emp->setItemText(2, QCoreApplication::translate("MainWindow", "Chef Atelier", nullptr));
        cb_poste_emp->setItemText(3, QCoreApplication::translate("MainWindow", "Contr\303\264le Qualit\303\251", nullptr));

        le5->setText(QCoreApplication::translate("MainWindow", "D\303\251partement :", nullptr));
        cb_dept_emp->setItemText(0, QCoreApplication::translate("MainWindow", "Production", nullptr));
        cb_dept_emp->setItemText(1, QCoreApplication::translate("MainWindow", "Logistique", nullptr));
        cb_dept_emp->setItemText(2, QCoreApplication::translate("MainWindow", "Bureau d'\303\211tudes", nullptr));

        le6->setText(QCoreApplication::translate("MainWindow", "Date Embauche :", nullptr));
        le7->setText(QCoreApplication::translate("MainWindow", "Salaire (\342\202\254) :", nullptr));
        le8->setText(QCoreApplication::translate("MainWindow", "RFID Tag :", nullptr));
        btn_calcul_score->setText(QCoreApplication::translate("MainWindow", "\360\237\224\215 Calculer Score", nullptr));
        lbl_score_result->setStyleSheet(QCoreApplication::translate("MainWindow", "font-weight: bold; color: #8d5524;", nullptr));
        lbl_score_result->setText(QCoreApplication::translate("MainWindow", "Score : -", nullptr));
        btn_valider_emp->setText(QCoreApplication::translate("MainWindow", "\342\234\205 ENREGISTRER", nullptr));
        btn_back_stats_emp->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour RH", nullptr));
        lbl_title_se->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 28px; font-weight: 300;", nullptr));
        lbl_title_se->setText(QCoreApplication::translate("MainWindow", "\360\237\244\226 Assistant & Analyses RH", nullptr));
        btn_dummy3->setText(QString());
        st_rh1->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        val_tot_emp->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        val_tot_emp->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_tot_emp->setText(QCoreApplication::translate("MainWindow", "Effectif Total", nullptr));
        st_rh2->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        val_masse_sal->setText(QCoreApplication::translate("MainWindow", "0 \342\202\254", nullptr));
        val_masse_sal->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_masse_sal->setText(QCoreApplication::translate("MainWindow", "Masse Salariale", nullptr));
        gb_chart_dept->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par D\303\251partement", nullptr));
        l_prod->setText(QCoreApplication::translate("MainWindow", "Production", nullptr));
        l_log->setText(QCoreApplication::translate("MainWindow", "Logistique", nullptr));
        l_be->setText(QCoreApplication::translate("MainWindow", "Bureau \303\211tudes", nullptr));
        gb_chat->setTitle(QCoreApplication::translate("MainWindow", "Chatbot RH", nullptr));
        txt_chat_history->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Segoe UI'; font-size:14px; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600; color:#8d5524;\">Bot:</span> Bonjour ! Posez-moi une question sur l'\303\251quipe.</p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-style:italic; color:#888888;\">Ex: &quot;Combien d'employ\303\251s ?&quot;, &quot;Qui gagne le plus ?&quot;</span></p></body></html>", nullptr));
        le_chat_input->setPlaceholderText(QCoreApplication::translate("MainWindow", "\303\211crivez votre question ici...", nullptr));
        btn_send_chat->setText(QCoreApplication::translate("MainWindow", "Envoyer", nullptr));
        lbl_cli->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 28px; font-weight: 300; color:#2c1a16;", nullptr));
        lbl_cli->setText(QCoreApplication::translate("MainWindow", "\360\237\247\276 Gestion des Clients", nullptr));
        le_search_client->setPlaceholderText(QCoreApplication::translate("MainWindow", "\360\237\224\215 ID client, nom, t\303\251l\303\251phone...", nullptr));
        btn_search_client->setText(QCoreApplication::translate("MainWindow", "Chercher", nullptr));
        btn_sort_alpha_client->setText(QCoreApplication::translate("MainWindow", "Tri", nullptr));
        btn_sort_fidelite->setText(QCoreApplication::translate("MainWindow", "\342\255\220 Fid\303\251lit\303\251", nullptr));
        btn_open_client_ia->setText(QCoreApplication::translate("MainWindow", "\360\237\247\240 Pr\303\251vision IA", nullptr));
        btn_open_fidelite->setText(QCoreApplication::translate("MainWindow", "\360\237\216\201 Fid\303\251lit\303\251", nullptr));
        btn_stats_client->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212 Stats", nullptr));
        btn_pdf_facture->setText(QCoreApplication::translate("MainWindow", "\360\237\223\204 Facture", nullptr));
        btn_add_client->setText(QCoreApplication::translate("MainWindow", "+ Nouveau", nullptr));
        QTableWidgetItem *___qtablewidgetitem21 = tableClients->horizontalHeaderItem(0);
        ___qtablewidgetitem21->setText(QCoreApplication::translate("MainWindow", "ID Client", nullptr));
        QTableWidgetItem *___qtablewidgetitem22 = tableClients->horizontalHeaderItem(1);
        ___qtablewidgetitem22->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem23 = tableClients->horizontalHeaderItem(2);
        ___qtablewidgetitem23->setText(QCoreApplication::translate("MainWindow", "T\303\251l\303\251phone", nullptr));
        QTableWidgetItem *___qtablewidgetitem24 = tableClients->horizontalHeaderItem(3);
        ___qtablewidgetitem24->setText(QCoreApplication::translate("MainWindow", "Adresse", nullptr));
        QTableWidgetItem *___qtablewidgetitem25 = tableClients->horizontalHeaderItem(4);
        ___qtablewidgetitem25->setText(QCoreApplication::translate("MainWindow", "Email", nullptr));
        QTableWidgetItem *___qtablewidgetitem26 = tableClients->horizontalHeaderItem(5);
        ___qtablewidgetitem26->setText(QCoreApplication::translate("MainWindow", "Fid\303\251lit\303\251", nullptr));
        btn_edit_client->setText(QCoreApplication::translate("MainWindow", "\342\234\217\357\270\217 Modifier", nullptr));
        btn_delete_client->setText(QCoreApplication::translate("MainWindow", "\360\237\227\221\357\270\217 Supprimer", nullptr));
        btn_back_client->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour Clients", nullptr));
        gb_client->setTitle(QCoreApplication::translate("MainWindow", "Fiche Client", nullptr));
        lc1->setText(QCoreApplication::translate("MainWindow", "ID Client :", nullptr));
        lc2->setText(QCoreApplication::translate("MainWindow", "Nom :", nullptr));
        lc3->setText(QCoreApplication::translate("MainWindow", "T\303\251l\303\251phone :", nullptr));
        lc4->setText(QCoreApplication::translate("MainWindow", "Adresse :", nullptr));
        lc5->setText(QCoreApplication::translate("MainWindow", "Email :", nullptr));
        lc6->setText(QCoreApplication::translate("MainWindow", "Points Fid\303\251lit\303\251 :", nullptr));
        btn_valider_client->setText(QCoreApplication::translate("MainWindow", "\342\234\205 ENREGISTRER", nullptr));
        btn_back_stats_client->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour Clients", nullptr));
        lbl_title_sc->setStyleSheet(QString());
        lbl_title_sc->setText(QCoreApplication::translate("MainWindow", "TABLEAU DE BORD - CLIENTS", nullptr));
        btn_dummy_cli->setText(QString());
        st_cli1->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_cli_1->setText(QCoreApplication::translate("MainWindow", "\360\237\221\244", nullptr));
        val_tot_cli->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        val_tot_cli->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_tot_cli->setText(QCoreApplication::translate("MainWindow", "Total Clients", nullptr));
        st_cli2->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_cli_2->setText(QCoreApplication::translate("MainWindow", "\342\255\220", nullptr));
        val_vip_cli->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        val_vip_cli->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_vip_cli->setText(QCoreApplication::translate("MainWindow", "VIP (\342\211\245100 pts)", nullptr));
        st_cli3->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_cli_3->setText(QCoreApplication::translate("MainWindow", "\360\237\216\201", nullptr));
        val_avg_pts_cli->setText(QCoreApplication::translate("MainWindow", "0 pts", nullptr));
        val_avg_pts_cli->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_avg_pts_cli->setText(QCoreApplication::translate("MainWindow", "Points Moyens", nullptr));
        gb_geo_cli->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par Zone", nullptr));
        l_cli_tunis->setText(QCoreApplication::translate("MainWindow", "Tunis", nullptr));
        l_cli_sfax->setText(QCoreApplication::translate("MainWindow", "Sfax", nullptr));
        l_cli_autres->setText(QCoreApplication::translate("MainWindow", "Autres", nullptr));
        gb_fidel_cli->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition Fid\303\251lit\303\251", nullptr));
        l_cli_std->setText(QCoreApplication::translate("MainWindow", "Standard (<50)", nullptr));
        l_cli_silver->setText(QCoreApplication::translate("MainWindow", "Silver (50-99)", nullptr));
        l_cli_vip->setText(QCoreApplication::translate("MainWindow", "VIP (\342\211\245100)", nullptr));
        btn_export_excel_client->setText(QCoreApplication::translate("MainWindow", "Exporter Excel", nullptr));
        btn_export_excel_client->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_excel", nullptr)));
        btn_print_client->setText(QCoreApplication::translate("MainWindow", "Imprimer", nullptr));
        btn_print_client->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_print", nullptr)));
        btn_close_stats_client_dash->setText(QCoreApplication::translate("MainWindow", "Fermer", nullptr));
        btn_close_stats_client_dash->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_close", nullptr)));
        lbl_dep->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 28px; font-weight: 300; color:#2c1a16;", nullptr));
        lbl_dep->setText(QCoreApplication::translate("MainWindow", "\360\237\217\254 Gestion Stock / D\303\251p\303\264t", nullptr));
        le_search_depot->setPlaceholderText(QCoreApplication::translate("MainWindow", "\360\237\224\215 Emplacement, all\303\251e, type...", nullptr));
        btn_search_depot->setText(QCoreApplication::translate("MainWindow", "Chercher", nullptr));
        btn_sort_alpha_depot->setText(QCoreApplication::translate("MainWindow", "Tri", nullptr));
        btn_sort_remplissage->setText(QCoreApplication::translate("MainWindow", "\360\237\223\210 Remplissage", nullptr));
        btn_open_optimize->setText(QCoreApplication::translate("MainWindow", "\360\237\247\251 Optimisation (First Fit)", nullptr));
        btn_open_ravit->setText(QCoreApplication::translate("MainWindow", "\360\237\232\232 Ravitaillement", nullptr));
        btn_stats_depot->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212 Stats", nullptr));
        btn_pdf_depot->setText(QCoreApplication::translate("MainWindow", "\360\237\223\204 PDF", nullptr));
        btn_add_depot->setText(QCoreApplication::translate("MainWindow", "+ Ajouter", nullptr));
        QTableWidgetItem *___qtablewidgetitem27 = tableDepot->horizontalHeaderItem(0);
        ___qtablewidgetitem27->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem28 = tableDepot->horizontalHeaderItem(1);
        ___qtablewidgetitem28->setText(QCoreApplication::translate("MainWindow", "Emplacement", nullptr));
        QTableWidgetItem *___qtablewidgetitem29 = tableDepot->horizontalHeaderItem(2);
        ___qtablewidgetitem29->setText(QCoreApplication::translate("MainWindow", "\303\211tag\303\250re", nullptr));
        QTableWidgetItem *___qtablewidgetitem30 = tableDepot->horizontalHeaderItem(3);
        ___qtablewidgetitem30->setText(QCoreApplication::translate("MainWindow", "Capacit\303\251 Max", nullptr));
        QTableWidgetItem *___qtablewidgetitem31 = tableDepot->horizontalHeaderItem(4);
        ___qtablewidgetitem31->setText(QCoreApplication::translate("MainWindow", "Quantit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem32 = tableDepot->horizontalHeaderItem(5);
        ___qtablewidgetitem32->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        QTableWidgetItem *___qtablewidgetitem33 = tableDepot->horizontalHeaderItem(6);
        ___qtablewidgetitem33->setText(QCoreApplication::translate("MainWindow", "Remplissage", nullptr));
        btn_edit_depot->setText(QCoreApplication::translate("MainWindow", "\342\234\217\357\270\217 Modifier", nullptr));
        btn_delete_depot->setText(QCoreApplication::translate("MainWindow", "\360\237\227\221\357\270\217 Supprimer", nullptr));
        btn_back_depot->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour D\303\251p\303\264t", nullptr));
        gb_depot->setTitle(QCoreApplication::translate("MainWindow", "Emplacement D\303\251p\303\264t", nullptr));
        ld1->setText(QCoreApplication::translate("MainWindow", "ID :", nullptr));
        ld2->setText(QCoreApplication::translate("MainWindow", "Emplacement :", nullptr));
        ld3->setText(QCoreApplication::translate("MainWindow", "\303\211tag\303\250re :", nullptr));
        ld4->setText(QCoreApplication::translate("MainWindow", "Capacit\303\251 Max :", nullptr));
        ld5->setText(QCoreApplication::translate("MainWindow", "Quantit\303\251 Actuelle :", nullptr));
        ld6->setText(QCoreApplication::translate("MainWindow", "Type Stockage :", nullptr));
        cb_type_depot->setItemText(0, QCoreApplication::translate("MainWindow", "Froid", nullptr));
        cb_type_depot->setItemText(1, QCoreApplication::translate("MainWindow", "Sec", nullptr));

        btn_valider_depot->setText(QCoreApplication::translate("MainWindow", "\342\234\205 ENREGISTRER", nullptr));
        btn_back_stats_depot->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour D\303\251p\303\264t", nullptr));
        lbl_title_sd->setStyleSheet(QString());
        lbl_title_sd->setText(QCoreApplication::translate("MainWindow", "TABLEAU DE BORD - D\303\211P\303\224T", nullptr));
        btn_dummy_dep->setText(QString());
        st_dep1->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_dep_1->setText(QCoreApplication::translate("MainWindow", "\360\237\217\254", nullptr));
        val_tot_dep->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        val_tot_dep->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_tot_dep->setText(QCoreApplication::translate("MainWindow", "Emplacements", nullptr));
        st_dep2->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_dep_2->setText(QCoreApplication::translate("MainWindow", "\360\237\223\210", nullptr));
        val_avg_fill->setText(QCoreApplication::translate("MainWindow", "0%", nullptr));
        val_avg_fill->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_avg_fill->setText(QCoreApplication::translate("MainWindow", "Remplissage Moyen", nullptr));
        st_dep3->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_dep_3->setText(QCoreApplication::translate("MainWindow", "\342\255\220", nullptr));
        val_max_fill_dep->setText(QCoreApplication::translate("MainWindow", "0%", nullptr));
        val_max_fill_dep->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_max_fill_dep->setText(QCoreApplication::translate("MainWindow", "Max Remplissage", nullptr));
        gb_type_dep->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par Type", nullptr));
        l_dep_froid->setText(QCoreApplication::translate("MainWindow", "Froid", nullptr));
        l_dep_sec->setText(QCoreApplication::translate("MainWindow", "Sec", nullptr));
        gb_fill_dep->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition Remplissage", nullptr));
        l_dep_low->setText(QCoreApplication::translate("MainWindow", "Faible (\342\211\24433%)", nullptr));
        l_dep_mid->setText(QCoreApplication::translate("MainWindow", "Moyen (34-66%)", nullptr));
        l_dep_high->setText(QCoreApplication::translate("MainWindow", "\303\211lev\303\251 (>66%)", nullptr));
        btn_export_excel_depot->setText(QCoreApplication::translate("MainWindow", "Exporter Excel", nullptr));
        btn_export_excel_depot->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_excel", nullptr)));
        btn_print_depot->setText(QCoreApplication::translate("MainWindow", "Imprimer", nullptr));
        btn_print_depot->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_print", nullptr)));
        btn_close_stats_depot_dash->setText(QCoreApplication::translate("MainWindow", "Fermer", nullptr));
        btn_close_stats_depot_dash->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_close", nullptr)));
        lbl_stk->setStyleSheet(QCoreApplication::translate("MainWindow", "font-size: 28px; font-weight: 300; color:#2c1a16;", nullptr));
        lbl_stk->setText(QCoreApplication::translate("MainWindow", "\360\237\223\246 Gestion des Mati\303\250res Premi\303\250res", nullptr));
        le_search_stock->setPlaceholderText(QCoreApplication::translate("MainWindow", "\360\237\224\215 Code, Cat\303\251gorie...", nullptr));
        btn_search_stock->setText(QCoreApplication::translate("MainWindow", "Chercher", nullptr));
        btn_sort_alpha_stock->setText(QCoreApplication::translate("MainWindow", "Tri", nullptr));
        btn_open_compare->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #00838f; color: white;", nullptr));
        btn_open_compare->setText(QCoreApplication::translate("MainWindow", "\342\232\226\357\270\217 Comparateur", nullptr));
        btn_open_calcul->setStyleSheet(QCoreApplication::translate("MainWindow", "background-color: #f9a825; color: white;", nullptr));
        btn_open_calcul->setText(QCoreApplication::translate("MainWindow", "\360\237\247\256 Calculateur", nullptr));
        btn_stats_stock->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212 Stats", nullptr));
        btn_pdf_stock->setText(QCoreApplication::translate("MainWindow", "\360\237\223\204 PDF", nullptr));
        btn_add_stock->setText(QCoreApplication::translate("MainWindow", "+ Ajouter Mati\303\250re", nullptr));
        QTableWidgetItem *___qtablewidgetitem34 = tableStock->horizontalHeaderItem(0);
        ___qtablewidgetitem34->setText(QCoreApplication::translate("MainWindow", "Code MP", nullptr));
        QTableWidgetItem *___qtablewidgetitem35 = tableStock->horizontalHeaderItem(1);
        ___qtablewidgetitem35->setText(QCoreApplication::translate("MainWindow", "Cat\303\251gorie", nullptr));
        QTableWidgetItem *___qtablewidgetitem36 = tableStock->horizontalHeaderItem(2);
        ___qtablewidgetitem36->setText(QCoreApplication::translate("MainWindow", "\303\211tat", nullptr));
        QTableWidgetItem *___qtablewidgetitem37 = tableStock->horizontalHeaderItem(3);
        ___qtablewidgetitem37->setText(QCoreApplication::translate("MainWindow", "Qualit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem38 = tableStock->horizontalHeaderItem(4);
        ___qtablewidgetitem38->setText(QCoreApplication::translate("MainWindow", "Qt\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem39 = tableStock->horizontalHeaderItem(5);
        ___qtablewidgetitem39->setText(QCoreApplication::translate("MainWindow", "Stockage", nullptr));
        QTableWidgetItem *___qtablewidgetitem40 = tableStock->horizontalHeaderItem(6);
        ___qtablewidgetitem40->setText(QCoreApplication::translate("MainWindow", "Zone", nullptr));
        btn_edit_stock->setText(QCoreApplication::translate("MainWindow", "\342\234\217\357\270\217 Modifier", nullptr));
        btn_delete_stock->setText(QCoreApplication::translate("MainWindow", "\360\237\227\221\357\270\217 Supprimer", nullptr));
        btn_back_stock->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour Stocks", nullptr));
        gb_caract->setTitle(QCoreApplication::translate("MainWindow", "Caract\303\251ristiques Mati\303\250re", nullptr));
        ls1->setText(QCoreApplication::translate("MainWindow", "Code MP :", nullptr));
        ls2->setText(QCoreApplication::translate("MainWindow", "Cat\303\251gorie :", nullptr));
        cb_cat_mp->setItemText(0, QCoreApplication::translate("MainWindow", "Cuir", nullptr));
        cb_cat_mp->setItemText(1, QCoreApplication::translate("MainWindow", "Teinture", nullptr));
        cb_cat_mp->setItemText(2, QCoreApplication::translate("MainWindow", "Produit Chimique", nullptr));

        ls3->setText(QCoreApplication::translate("MainWindow", "\303\211tat :", nullptr));
        cb_etat_mp->setItemText(0, QCoreApplication::translate("MainWindow", "BRUT", nullptr));
        cb_etat_mp->setItemText(1, QCoreApplication::translate("MainWindow", "TEINT", nullptr));

        ls4->setText(QCoreApplication::translate("MainWindow", "Couleur :", nullptr));
        ls5->setText(QCoreApplication::translate("MainWindow", "Qualit\303\251 :", nullptr));
        cb_qual_mp->setItemText(0, QCoreApplication::translate("MainWindow", "A (Premium)", nullptr));
        cb_qual_mp->setItemText(1, QCoreApplication::translate("MainWindow", "B (Standard)", nullptr));
        cb_qual_mp->setItemText(2, QCoreApplication::translate("MainWindow", "C (\303\211co)", nullptr));

        gb_log->setTitle(QCoreApplication::translate("MainWindow", "Logistique", nullptr));
        ls6->setText(QCoreApplication::translate("MainWindow", "Quantit\303\251 :", nullptr));
        cb_unite_mp->setItemText(0, QCoreApplication::translate("MainWindow", "M2", nullptr));
        cb_unite_mp->setItemText(1, QCoreApplication::translate("MainWindow", "Kg", nullptr));

        ls7->setText(QCoreApplication::translate("MainWindow", "Zone :", nullptr));
        cb_zone_mp->setItemText(0, QCoreApplication::translate("MainWindow", "Zone A", nullptr));
        cb_zone_mp->setItemText(1, QCoreApplication::translate("MainWindow", "Zone B", nullptr));

        le_allee_mp->setPlaceholderText(QCoreApplication::translate("MainWindow", "All\303\251e...", nullptr));
        ls8->setText(QCoreApplication::translate("MainWindow", "Type Stockage :", nullptr));
        cb_type_stock->setItemText(0, QCoreApplication::translate("MainWindow", "Standard", nullptr));
        cb_type_stock->setItemText(1, QCoreApplication::translate("MainWindow", "R\303\251frig\303\251r\303\251", nullptr));
        cb_type_stock->setItemText(2, QCoreApplication::translate("MainWindow", "Temp\303\251r\303\251", nullptr));

        ls9->setText(QCoreApplication::translate("MainWindow", "R\303\251ception :", nullptr));
        btn_valider_stock->setText(QCoreApplication::translate("MainWindow", "\342\234\205 ENREGISTRER MATI\303\210RE", nullptr));
        btn_back_inn_stock->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour", nullptr));
        btn_back_stats_stock->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour Stocks", nullptr));
        lbl_title_sst->setStyleSheet(QString());
        lbl_title_sst->setText(QCoreApplication::translate("MainWindow", "TABLEAU DE BORD - GESTION DES STOCKS", nullptr));
        btn_dummy4->setText(QString());
        st_stk1->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_stk1->setText(QCoreApplication::translate("MainWindow", "\360\237\223\246", nullptr));
        val_tot_mat->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        val_tot_mat->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_tot_mat->setText(QCoreApplication::translate("MainWindow", "Total Mati\303\250res", nullptr));
        st_stk2->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_stk2->setText(QCoreApplication::translate("MainWindow", "\360\237\223\217", nullptr));
        val_tot_vol->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        val_tot_vol->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_tot_vol->setText(QCoreApplication::translate("MainWindow", "Stock Total", nullptr));
        st_stk3->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_card", nullptr)));
        ico_stk3->setText(QCoreApplication::translate("MainWindow", "\360\237\222\260", nullptr));
        val_tot_val_stock->setText(QCoreApplication::translate("MainWindow", "0 DT", nullptr));
        val_tot_val_stock->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "stat_val", nullptr)));
        lbl_tot_val_stock->setText(QCoreApplication::translate("MainWindow", "Valeur Totale", nullptr));
        gb_chart_cat->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par Cat\303\251gorie", nullptr));
        l_cuir->setText(QCoreApplication::translate("MainWindow", "Cuir", nullptr));
        l_teint->setText(QCoreApplication::translate("MainWindow", "Teinture", nullptr));
        l_chim->setText(QCoreApplication::translate("MainWindow", "Chimique", nullptr));
        gb_chart_qual->setTitle(QCoreApplication::translate("MainWindow", "R\303\251partition par Qualit\303\251", nullptr));
        l_qa->setText(QCoreApplication::translate("MainWindow", "Qualit\303\251 A", nullptr));
        l_qb->setText(QCoreApplication::translate("MainWindow", "Qualit\303\251 B", nullptr));
        l_qc->setText(QCoreApplication::translate("MainWindow", "Qualit\303\251 C", nullptr));
        gb_pie_stk->setTitle(QCoreApplication::translate("MainWindow", "Camembert \342\200\224 Cat\303\251gories", nullptr));
        gb_bar_stk->setTitle(QCoreApplication::translate("MainWindow", "Barres \342\200\224 Qualit\303\251", nullptr));
        btn_export_excel_stock->setText(QCoreApplication::translate("MainWindow", "Exporter Excel", nullptr));
        btn_export_excel_stock->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_excel", nullptr)));
        btn_print_stock->setText(QCoreApplication::translate("MainWindow", "Imprimer", nullptr));
        btn_print_stock->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_print", nullptr)));
        btn_close_stats_stock->setText(QCoreApplication::translate("MainWindow", "Fermer", nullptr));
        btn_close_stats_stock->setProperty("class", QVariant(QCoreApplication::translate("MainWindow", "btn_close", nullptr)));
        btn_back_fab->setText(QCoreApplication::translate("MainWindow", "\342\206\220 Retour", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
