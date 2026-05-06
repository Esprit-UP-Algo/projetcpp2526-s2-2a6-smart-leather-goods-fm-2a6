-- ============================================================================
-- Script SQL : Création des tables pour le module Ravitaillement Intelligent
-- Projet: FIL D'OR - Gestion des Matières Premières
-- Date: 05/05/2026
-- ============================================================================

-- ============================================================================
-- 1. TABLE RAVITAILLEMENT_LOG (Historique des décisions)
-- ============================================================================

-- Supprimer la table si elle existe (pour réinitialisation)
BEGIN
   EXECUTE IMMEDIATE 'DROP TABLE RAVITAILLEMENT_LOG CASCADE CONSTRAINTS';
EXCEPTION
   WHEN OTHERS THEN
      IF SQLCODE != -942 THEN
         RAISE;
      END IF;
END;
/

-- Créer la table
CREATE TABLE RAVITAILLEMENT_LOG (
    LOG_ID VARCHAR2(40) PRIMARY KEY,
    DATE_LOG TIMESTAMP DEFAULT SYSTIMESTAMP,
    MATIERE VARCHAR2(80) NOT NULL,
    FOURNISSEUR VARCHAR2(120) NOT NULL,
    SCORE_GLOBAL NUMBER(6,2),
    QUANTITE_COMMANDE NUMBER(12,2),
    BUDGET_ESTIME NUMBER(12,2),
    RISQUE VARCHAR2(20),
    COUVERTURE_JOURS NUMBER(10,2),
    STRATEGIE CLOB
);

-- Index pour améliorer les performances
CREATE INDEX IDX_RAVIT_DATE ON RAVITAILLEMENT_LOG(DATE_LOG);
CREATE INDEX IDX_RAVIT_MATIERE ON RAVITAILLEMENT_LOG(MATIERE);

-- Commentaires
COMMENT ON TABLE RAVITAILLEMENT_LOG IS 'Historique des décisions de ravitaillement';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.LOG_ID IS 'Identifiant unique (timestamp)';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.DATE_LOG IS 'Date et heure de la décision';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.MATIERE IS 'Type de cuir (Vachette/Agneau/Veau)';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.FOURNISSEUR IS 'Nom du fournisseur sélectionné';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.SCORE_GLOBAL IS 'Score pondéré du fournisseur (0-100)';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.QUANTITE_COMMANDE IS 'Quantité commandée en M²';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.BUDGET_ESTIME IS 'Budget estimé en DT';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.RISQUE IS 'Niveau de risque (Faible/Modéré/Élevé)';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.COUVERTURE_JOURS IS 'Couverture stock en jours';
COMMENT ON COLUMN RAVITAILLEMENT_LOG.STRATEGIE IS 'Détails de la stratégie EOQ/Wilson';

PROMPT '✓ Table RAVITAILLEMENT_LOG créée avec succès';

-- ============================================================================
-- 2. TABLE FOURNISSEURS (si elle n'existe pas déjà)
-- ============================================================================

DECLARE
    v_count NUMBER;
BEGIN
    SELECT COUNT(*) INTO v_count 
    FROM user_tables 
    WHERE table_name = 'FOURNISSEURS';
    
    IF v_count = 0 THEN
        EXECUTE IMMEDIATE '
        CREATE TABLE FOURNISSEURS (
            ID_FOURNISSEUR NUMBER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
            NOM_FOURNISSEUR VARCHAR2(120) NOT NULL,
            TYPE_CUIR VARCHAR2(50) NOT NULL,
            PRIX_PAR_M2 NUMBER(10,2) NOT NULL,
            QUALITE VARCHAR2(20) NOT NULL,
            DELAI_JOURS NUMBER(3) NOT NULL,
            ACTIF CHAR(1) DEFAULT ''1'' CHECK (ACTIF IN (''0'', ''1'')),
            DATE_CREATION TIMESTAMP DEFAULT SYSTIMESTAMP,
            CONSTRAINT CHK_PRIX_POSITIF CHECK (PRIX_PAR_M2 > 0),
            CONSTRAINT CHK_DELAI_POSITIF CHECK (DELAI_JOURS > 0)
        )';
        
        EXECUTE IMMEDIATE 'CREATE INDEX IDX_FOURN_TYPE ON FOURNISSEURS(TYPE_CUIR)';
        EXECUTE IMMEDIATE 'CREATE INDEX IDX_FOURN_ACTIF ON FOURNISSEURS(ACTIF)';
        
        DBMS_OUTPUT.PUT_LINE('✓ Table FOURNISSEURS créée');
    ELSE
        DBMS_OUTPUT.PUT_LINE('→ Table FOURNISSEURS existe déjà');
    END IF;
END;
/

-- ============================================================================
-- 3. DONNÉES DE TEST - FOURNISSEURS
-- ============================================================================

-- Supprimer les données existantes
DELETE FROM FOURNISSEURS WHERE NOM_FOURNISSEUR IN (
    'TanLeather SA', 'Cuir Elite', 'MegaCuir',
    'SoftHide Pro', 'Agneau Plus', 'Cuir Sud',
    'Veau Prestige', 'Elite Veau', 'Market Cuir'
);

-- Fournisseurs Cuir Vachette
INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('TanLeather SA', 'Cuir Vachette', 45.00, 'A', 5, '1');

INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('Cuir Elite', 'Cuir Vachette', 38.00, 'B', 7, '1');

INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('MegaCuir', 'Cuir Vachette', 32.00, 'C', 10, '1');

-- Fournisseurs Cuir Agneau
INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('SoftHide Pro', 'Cuir Agneau', 52.00, 'A', 4, '1');

INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('Agneau Plus', 'Cuir Agneau', 44.00, 'B', 6, '1');

INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('Cuir Sud', 'Cuir Agneau', 36.00, 'C', 9, '1');

-- Fournisseurs Cuir Veau
INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('Veau Prestige', 'Cuir Veau', 49.00, 'A', 5, '1');

INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('Elite Veau', 'Cuir Veau', 41.00, 'B', 7, '1');

INSERT INTO FOURNISSEURS (NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE, DELAI_JOURS, ACTIF)
VALUES ('Market Cuir', 'Cuir Veau', 34.00, 'C', 11, '1');

COMMIT;

PROMPT '✓ 9 fournisseurs insérés (3 par type de cuir)';

-- ============================================================================
-- 4. DONNÉES DE TEST - HISTORIQUE RAVITAILLEMENT
-- ============================================================================

-- Insérer quelques décisions de test
INSERT INTO RAVITAILLEMENT_LOG (
    LOG_ID, DATE_LOG, MATIERE, FOURNISSEUR, SCORE_GLOBAL, 
    QUANTITE_COMMANDE, BUDGET_ESTIME, RISQUE, COUVERTURE_JOURS, STRATEGIE
) VALUES (
    '20260505120000001',
    SYSTIMESTAMP - INTERVAL '7' DAY,
    'Cuir Vachette',
    'TanLeather SA',
    92.5,
    150.00,
    6750.00,
    'Faible',
    21.5,
    'Scénario Optimal - EOQ: 145 M² - Stock sécurité: 85 M²'
);

INSERT INTO RAVITAILLEMENT_LOG (
    LOG_ID, DATE_LOG, MATIERE, FOURNISSEUR, SCORE_GLOBAL, 
    QUANTITE_COMMANDE, BUDGET_ESTIME, RISQUE, COUVERTURE_JOURS, STRATEGIE
) VALUES (
    '20260505120000002',
    SYSTIMESTAMP - INTERVAL '14' DAY,
    'Cuir Agneau',
    'SoftHide Pro',
    88.3,
    120.00,
    6240.00,
    'Modéré',
    18.2,
    'Scénario Conservateur - EOQ: 130 M² - Stock sécurité: 75 M²'
);

INSERT INTO RAVITAILLEMENT_LOG (
    LOG_ID, DATE_LOG, MATIERE, FOURNISSEUR, SCORE_GLOBAL, 
    QUANTITE_COMMANDE, BUDGET_ESTIME, RISQUE, COUVERTURE_JOURS, STRATEGIE
) VALUES (
    '20260505120000003',
    SYSTIMESTAMP - INTERVAL '21' DAY,
    'Cuir Veau',
    'Veau Prestige',
    90.1,
    180.00,
    8820.00,
    'Faible',
    25.8,
    'Scénario Anticipatif - EOQ: 175 M² - Stock sécurité: 95 M²'
);

COMMIT;

PROMPT '✓ 3 décisions de test insérées dans l''historique';

-- ============================================================================
-- 5. VÉRIFICATION DES TABLES MATIERES_PREMIERES
-- ============================================================================

DECLARE
    v_count NUMBER;
    v_has_categorie NUMBER := 0;
    v_has_couleur NUMBER := 0;
    v_has_qualite NUMBER := 0;
    v_has_quantite NUMBER := 0;
BEGIN
    -- Vérifier que la table existe
    SELECT COUNT(*) INTO v_count 
    FROM user_tables 
    WHERE table_name = 'MATIERES_PREMIERES';
    
    IF v_count = 0 THEN
        DBMS_OUTPUT.PUT_LINE('⚠ ATTENTION: Table MATIERES_PREMIERES n''existe pas!');
        DBMS_OUTPUT.PUT_LINE('  Créez-la avec les colonnes: CATEGORIE_MP, COULEUR, QUALITE, QUANTITE');
    ELSE
        -- Vérifier les colonnes nécessaires
        SELECT COUNT(*) INTO v_has_categorie FROM user_tab_columns 
        WHERE table_name = 'MATIERES_PREMIERES' AND column_name = 'CATEGORIE_MP';
        
        SELECT COUNT(*) INTO v_has_couleur FROM user_tab_columns 
        WHERE table_name = 'MATIERES_PREMIERES' AND column_name = 'COULEUR';
        
        SELECT COUNT(*) INTO v_has_qualite FROM user_tab_columns 
        WHERE table_name = 'MATIERES_PREMIERES' AND column_name = 'QUALITE';
        
        SELECT COUNT(*) INTO v_has_quantite FROM user_tab_columns 
        WHERE table_name = 'MATIERES_PREMIERES' AND column_name = 'QUANTITE';
        
        IF v_has_categorie = 1 AND v_has_couleur = 1 AND v_has_qualite = 1 AND v_has_quantite = 1 THEN
            DBMS_OUTPUT.PUT_LINE('✓ Table MATIERES_PREMIERES: toutes les colonnes requises sont présentes');
        ELSE
            DBMS_OUTPUT.PUT_LINE('⚠ ATTENTION: Colonnes manquantes dans MATIERES_PREMIERES:');
            IF v_has_categorie = 0 THEN DBMS_OUTPUT.PUT_LINE('  - CATEGORIE_MP'); END IF;
            IF v_has_couleur = 0 THEN DBMS_OUTPUT.PUT_LINE('  - COULEUR'); END IF;
            IF v_has_qualite = 0 THEN DBMS_OUTPUT.PUT_LINE('  - QUALITE'); END IF;
            IF v_has_quantite = 0 THEN DBMS_OUTPUT.PUT_LINE('  - QUANTITE'); END IF;
        END IF;
    END IF;
END;
/

-- ============================================================================
-- 6. RÉSUMÉ FINAL
-- ============================================================================

PROMPT '';
PROMPT '========================================';
PROMPT '✓ INSTALLATION TERMINÉE AVEC SUCCÈS';
PROMPT '========================================';
PROMPT '';
PROMPT 'Tables créées:';
PROMPT '  ✓ RAVITAILLEMENT_LOG (historique)';
PROMPT '  ✓ FOURNISSEURS (9 fournisseurs)';
PROMPT '';
PROMPT 'Données de test:';
PROMPT '  ✓ 9 fournisseurs (3 par type de cuir)';
PROMPT '  ✓ 3 décisions historiques';
PROMPT '';
PROMPT 'Prochaines étapes:';
PROMPT '  1. Lancer l''application: ProjetCpp.exe';
PROMPT '  2. Aller dans Stock → Ravitaillement';
PROMPT '  3. Tester toutes les fonctionnalités';
PROMPT '';
PROMPT 'Fonctionnalités disponibles:';
PROMPT '  ✓ Stepper visuel (①→②→③)';
PROMPT '  ✓ Bandeau d''urgence dynamique';
PROMPT '  ✓ Sélection matière avec photo';
PROMPT '  ✓ Formulaire de commande';
PROMPT '  ✓ KPIs temps réel';
PROMPT '  ✓ Tableau scoring fournisseurs';
PROMPT '  ✓ Graphique projection 14 jours';
PROMPT '  ✓ 3 scénarios d''achat';
PROMPT '  ✓ Paramètres EOQ/Wilson';
PROMPT '  ✓ Carte synthèse décision';
PROMPT '  ✓ Historique avec filtres';
PROMPT '';
PROMPT '========================================';
