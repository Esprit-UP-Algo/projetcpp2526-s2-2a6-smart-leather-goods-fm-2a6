-- ============================================
-- Script de vérification de l'installation
-- Projet: FIL D'OR
-- ============================================

SET SERVEROUTPUT ON;
SET LINESIZE 200;
SET PAGESIZE 100;

PROMPT ========================================
PROMPT VÉRIFICATION DE L'INSTALLATION
PROMPT ========================================
PROMPT;

-- =============================================
-- 1. VÉRIFICATION DES TABLES
-- =============================================
PROMPT [1/6] Vérification des tables...
PROMPT;

SELECT 
    CASE 
        WHEN COUNT(*) = 7 THEN '✓ SUCCÈS'
        ELSE '✗ ÉCHEC'
    END AS STATUT,
    COUNT(*) AS NB_TABLES_TROUVEES,
    '7 attendues' AS NB_TABLES_ATTENDUES
FROM user_tables 
WHERE table_name IN (
    'ARDUINO_CONFIG', 
    'MOTEUR_LOGS', 
    'GAZ_ALERTS',
    'MATIERES_PREMIERES', 
    'FOURNISSEURS', 
    'STOCK_MP', 
    'RAVITAILLEMENT_LOG'
);

PROMPT;
PROMPT Détail des tables:
SELECT table_name AS "TABLE", num_rows AS "NB_LIGNES"
FROM user_tables 
WHERE table_name IN (
    'ARDUINO_CONFIG', 
    'MOTEUR_LOGS', 
    'GAZ_ALERTS',
    'MATIERES_PREMIERES', 
    'FOURNISSEURS', 
    'STOCK_MP', 
    'RAVITAILLEMENT_LOG'
)
ORDER BY table_name;

-- =============================================
-- 2. VÉRIFICATION DES SÉQUENCES
-- =============================================
PROMPT;
PROMPT [2/6] Vérification des séquences...
PROMPT;

SELECT 
    CASE 
        WHEN COUNT(*) >= 5 THEN '✓ SUCCÈS'
        ELSE '✗ ÉCHEC'
    END AS STATUT,
    COUNT(*) AS NB_SEQUENCES_TROUVEES,
    '5+ attendues' AS NB_SEQUENCES_ATTENDUES
FROM user_sequences 
WHERE sequence_name IN (
    'SEQ_MAT', 
    'SEQ_FOURNISSEUR', 
    'SEQ_STOCK_MP',
    'SEQ_MOTEUR_LOGS',
    'SEQ_GAZ_ALERTS'
);

PROMPT;
PROMPT Détail des séquences:
SELECT sequence_name AS "SÉQUENCE", last_number AS "VALEUR_ACTUELLE"
FROM user_sequences 
WHERE sequence_name IN (
    'SEQ_MAT', 
    'SEQ_FOURNISSEUR', 
    'SEQ_STOCK_MP',
    'SEQ_MOTEUR_LOGS',
    'SEQ_GAZ_ALERTS'
)
ORDER BY sequence_name;

-- =============================================
-- 3. VÉRIFICATION DES VUES
-- =============================================
PROMPT;
PROMPT [3/6] Vérification des vues...
PROMPT;

SELECT 
    CASE 
        WHEN COUNT(*) >= 5 THEN '✓ SUCCÈS'
        ELSE '✗ ÉCHEC'
    END AS STATUT,
    COUNT(*) AS NB_VUES_TROUVEES,
    '5+ attendues' AS NB_VUES_ATTENDUES
FROM user_views 
WHERE view_name IN (
    'V_STOCK_AGREGE', 
    'V_FOURNISSEURS_ACTIFS', 
    'V_ANALYSE_RAVITAILLEMENT',
    'V_ALERTES_GAZ_ACTIVES',
    'V_MONITORING_ARDUINO'
);

PROMPT;
PROMPT Détail des vues:
SELECT view_name AS "VUE"
FROM user_views 
WHERE view_name IN (
    'V_STOCK_AGREGE', 
    'V_FOURNISSEURS_ACTIFS', 
    'V_ANALYSE_RAVITAILLEMENT',
    'V_ALERTES_GAZ_ACTIVES',
    'V_MONITORING_ARDUINO'
)
ORDER BY view_name;

-- =============================================
-- 4. VÉRIFICATION DES PROCÉDURES
-- =============================================
PROMPT;
PROMPT [4/6] Vérification des procédures stockées...
PROMPT;

SELECT 
    CASE 
        WHEN COUNT(*) >= 6 THEN '✓ SUCCÈS'
        ELSE '✗ ÉCHEC'
    END AS STATUT,
    COUNT(*) AS NB_PROCEDURES_TROUVEES,
    '6+ attendues' AS NB_PROCEDURES_ATTENDUES
FROM user_objects 
WHERE object_type = 'PROCEDURE' 
AND object_name IN (
    'SP_AJOUTER_MATIERE', 
    'SP_LOG_RAVITAILLEMENT', 
    'SP_CALCULER_BESOIN',
    'SP_LOG_MOTEUR',
    'SP_CREER_ALERTE_GAZ',
    'SP_TRAITER_ALERTE_GAZ'
);

PROMPT;
PROMPT Détail des procédures:
SELECT object_name AS "PROCÉDURE", status AS "STATUT"
FROM user_objects 
WHERE object_type = 'PROCEDURE' 
AND object_name IN (
    'SP_AJOUTER_MATIERE', 
    'SP_LOG_RAVITAILLEMENT', 
    'SP_CALCULER_BESOIN',
    'SP_LOG_MOTEUR',
    'SP_CREER_ALERTE_GAZ',
    'SP_TRAITER_ALERTE_GAZ'
)
ORDER BY object_name;

-- =============================================
-- 5. VÉRIFICATION DES DONNÉES
-- =============================================
PROMPT;
PROMPT [5/6] Vérification des données...
PROMPT;

SELECT 'MATIERES_PREMIERES' AS "TABLE", COUNT(*) AS "NB_LIGNES" FROM MATIERES_PREMIERES
UNION ALL
SELECT 'FOURNISSEURS', COUNT(*) FROM FOURNISSEURS
UNION ALL
SELECT 'STOCK_MP', COUNT(*) FROM STOCK_MP
UNION ALL
SELECT 'ARDUINO_CONFIG', COUNT(*) FROM ARDUINO_CONFIG
UNION ALL
SELECT 'MOTEUR_LOGS', COUNT(*) FROM MOTEUR_LOGS
UNION ALL
SELECT 'GAZ_ALERTS', COUNT(*) FROM GAZ_ALERTS
UNION ALL
SELECT 'RAVITAILLEMENT_LOG', COUNT(*) FROM RAVITAILLEMENT_LOG;

-- =============================================
-- 6. TESTS FONCTIONNELS
-- =============================================
PROMPT;
PROMPT [6/6] Tests fonctionnels...
PROMPT;

-- Test Vue Stock Agrégé
PROMPT Test 1: Vue V_STOCK_AGREGE
SELECT * FROM V_STOCK_AGREGE;

PROMPT;
PROMPT Test 2: Vue V_FOURNISSEURS_ACTIFS (3 premiers)
SELECT * FROM (
    SELECT NOM_FOURNISSEUR, TYPE_CUIR, PRIX_PAR_M2, QUALITE
    FROM V_FOURNISSEURS_ACTIFS
    ORDER BY PRIX_PAR_M2 DESC
) WHERE ROWNUM <= 3;

PROMPT;
PROMPT Test 3: Alertes Gaz Actives
SELECT 
    ID,
    EMPLACEMENT_ID,
    VALEUR_GAZ,
    NIVEAU_ALERTE,
    STATUT_TRAITEMENT,
    TO_CHAR(DATE_ALERT, 'DD/MM/YYYY HH24:MI') AS DATE_ALERTE
FROM V_ALERTES_GAZ_ACTIVES;

PROMPT;
PROMPT Test 4: Configuration Arduino
SELECT CLE, VALEUR, DESCRIPTION FROM ARDUINO_CONFIG ORDER BY CLE;

PROMPT;
PROMPT Test 5: Derniers logs moteur
SELECT * FROM (
    SELECT 
        LOG_ID,
        PRODUCT_ID,
        PRODUIT,
        ACTION,
        STATUT,
        DATE_LOG
    FROM V_MONITORING_ARDUINO
    ORDER BY DATE_LOG DESC
) WHERE ROWNUM <= 5;

-- =============================================
-- 7. RÉSUMÉ FINAL
-- =============================================
PROMPT;
PROMPT ========================================
PROMPT RÉSUMÉ FINAL
PROMPT ========================================
PROMPT;

DECLARE
    v_tables NUMBER;
    v_sequences NUMBER;
    v_vues NUMBER;
    v_procedures NUMBER;
    v_total_ok NUMBER := 0;
    v_total_tests NUMBER := 4;
BEGIN
    -- Compter les objets
    SELECT COUNT(*) INTO v_tables FROM user_tables 
    WHERE table_name IN ('ARDUINO_CONFIG', 'MOTEUR_LOGS', 'GAZ_ALERTS', 'MATIERES_PREMIERES', 'FOURNISSEURS', 'STOCK_MP', 'RAVITAILLEMENT_LOG');
    
    SELECT COUNT(*) INTO v_sequences FROM user_sequences 
    WHERE sequence_name IN ('SEQ_MAT', 'SEQ_FOURNISSEUR', 'SEQ_STOCK_MP', 'SEQ_MOTEUR_LOGS', 'SEQ_GAZ_ALERTS');
    
    SELECT COUNT(*) INTO v_vues FROM user_views 
    WHERE view_name IN ('V_STOCK_AGREGE', 'V_FOURNISSEURS_ACTIFS', 'V_ANALYSE_RAVITAILLEMENT', 'V_ALERTES_GAZ_ACTIVES', 'V_MONITORING_ARDUINO');
    
    SELECT COUNT(*) INTO v_procedures FROM user_objects 
    WHERE object_type = 'PROCEDURE' 
    AND object_name IN ('SP_AJOUTER_MATIERE', 'SP_LOG_RAVITAILLEMENT', 'SP_CALCULER_BESOIN', 'SP_LOG_MOTEUR', 'SP_CREER_ALERTE_GAZ', 'SP_TRAITER_ALERTE_GAZ');
    
    -- Évaluer les résultats
    IF v_tables = 7 THEN v_total_ok := v_total_ok + 1; END IF;
    IF v_sequences >= 5 THEN v_total_ok := v_total_ok + 1; END IF;
    IF v_vues >= 5 THEN v_total_ok := v_total_ok + 1; END IF;
    IF v_procedures >= 6 THEN v_total_ok := v_total_ok + 1; END IF;
    
    -- Afficher le résumé
    DBMS_OUTPUT.PUT_LINE('Tables:      ' || v_tables || '/7 ' || CASE WHEN v_tables = 7 THEN '✓' ELSE '✗' END);
    DBMS_OUTPUT.PUT_LINE('Séquences:   ' || v_sequences || '/5+ ' || CASE WHEN v_sequences >= 5 THEN '✓' ELSE '✗' END);
    DBMS_OUTPUT.PUT_LINE('Vues:        ' || v_vues || '/5+ ' || CASE WHEN v_vues >= 5 THEN '✓' ELSE '✗' END);
    DBMS_OUTPUT.PUT_LINE('Procédures:  ' || v_procedures || '/6+ ' || CASE WHEN v_procedures >= 6 THEN '✓' ELSE '✗' END);
    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('Score: ' || v_total_ok || '/' || v_total_tests);
    DBMS_OUTPUT.PUT_LINE('');
    
    IF v_total_ok = v_total_tests THEN
        DBMS_OUTPUT.PUT_LINE('========================================');
        DBMS_OUTPUT.PUT_LINE('✓✓✓ INSTALLATION COMPLÈTE ET RÉUSSIE! ✓✓✓');
        DBMS_OUTPUT.PUT_LINE('========================================');
    ELSE
        DBMS_OUTPUT.PUT_LINE('========================================');
        DBMS_OUTPUT.PUT_LINE('✗ INSTALLATION INCOMPLÈTE');
        DBMS_OUTPUT.PUT_LINE('Veuillez vérifier les erreurs ci-dessus');
        DBMS_OUTPUT.PUT_LINE('========================================');
    END IF;
END;
/

PROMPT;
PROMPT Pour plus d'informations, consultez GUIDE_INTEGRATION_COMPLETE.md
PROMPT;
