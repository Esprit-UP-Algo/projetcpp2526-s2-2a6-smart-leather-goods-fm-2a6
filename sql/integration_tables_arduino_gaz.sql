-- ============================================
-- Script d'intégration des tables Arduino et Gaz
-- Projet: FIL D'OR - Intégration complète
-- Date: 2026-05-05
-- Description: Création des tables ARDUINO_CONFIG, MOTEUR_LOGS, GAZ_ALERTS
-- ============================================

SET SERVEROUTPUT ON;

-- =============================================
-- 1. TABLE ARDUINO_CONFIG
-- =============================================
PROMPT ========================================
PROMPT Création de la table ARDUINO_CONFIG
PROMPT ========================================

-- Suppression de la table si elle existe
BEGIN
   EXECUTE IMMEDIATE 'DROP TABLE ARDUINO_CONFIG CASCADE CONSTRAINTS';
   DBMS_OUTPUT.PUT_LINE('Table ARDUINO_CONFIG supprimée.');
EXCEPTION
   WHEN OTHERS THEN
      IF SQLCODE != -942 THEN
         RAISE;
      ELSE
         DBMS_OUTPUT.PUT_LINE('Table ARDUINO_CONFIG n''existe pas encore.');
      END IF;
END;
/

-- Création de la table ARDUINO_CONFIG
CREATE TABLE ARDUINO_CONFIG (
    CLE VARCHAR2(50) PRIMARY KEY,
    VALEUR VARCHAR2(255) NOT NULL,
    DESCRIPTION VARCHAR2(500),
    DATE_CREATION DATE DEFAULT SYSDATE,
    DATE_MODIFICATION DATE DEFAULT SYSDATE
);

COMMENT ON TABLE ARDUINO_CONFIG IS 'Configuration des paramètres Arduino (ports COM, baudrate, etc.)';
COMMENT ON COLUMN ARDUINO_CONFIG.CLE IS 'Clé de configuration (ex: PORT_COM, BAUDRATE)';
COMMENT ON COLUMN ARDUINO_CONFIG.VALEUR IS 'Valeur de la configuration';

-- Insertion des configurations par défaut
MERGE INTO ARDUINO_CONFIG c
USING (SELECT 'PORT_COM' AS CLE, 'COM5' AS VALEUR, 'Port série de communication Arduino' AS DESCRIPTION FROM dual) s
ON (c.CLE = s.CLE)
WHEN MATCHED THEN 
    UPDATE SET c.VALEUR = s.VALEUR, c.DESCRIPTION = s.DESCRIPTION, c.DATE_MODIFICATION = SYSDATE
WHEN NOT MATCHED THEN 
    INSERT (CLE, VALEUR, DESCRIPTION) VALUES (s.CLE, s.VALEUR, s.DESCRIPTION);

MERGE INTO ARDUINO_CONFIG c
USING (SELECT 'BAUDRATE' AS CLE, '9600' AS VALEUR, 'Vitesse de communication série' AS DESCRIPTION FROM dual) s
ON (c.CLE = s.CLE)
WHEN MATCHED THEN 
    UPDATE SET c.VALEUR = s.VALEUR, c.DESCRIPTION = s.DESCRIPTION, c.DATE_MODIFICATION = SYSDATE
WHEN NOT MATCHED THEN 
    INSERT (CLE, VALEUR, DESCRIPTION) VALUES (s.CLE, s.VALEUR, s.DESCRIPTION);

MERGE INTO ARDUINO_CONFIG c
USING (SELECT 'TIMEOUT_MS' AS CLE, '5000' AS VALEUR, 'Timeout de connexion en millisecondes' AS DESCRIPTION FROM dual) s
ON (c.CLE = s.CLE)
WHEN MATCHED THEN 
    UPDATE SET c.VALEUR = s.VALEUR, c.DESCRIPTION = s.DESCRIPTION, c.DATE_MODIFICATION = SYSDATE
WHEN NOT MATCHED THEN 
    INSERT (CLE, VALEUR, DESCRIPTION) VALUES (s.CLE, s.VALEUR, s.DESCRIPTION);

COMMIT;

PROMPT Configuration Arduino insérée avec succès.
SELECT * FROM ARDUINO_CONFIG ORDER BY CLE;

-- =============================================
-- 2. TABLE MOTEUR_LOGS
-- =============================================
PROMPT ========================================
PROMPT Création de la table MOTEUR_LOGS
PROMPT ========================================

-- Suppression de la séquence si elle existe
BEGIN
   EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_MOTEUR_LOGS';
   DBMS_OUTPUT.PUT_LINE('Séquence SEQ_MOTEUR_LOGS supprimée.');
EXCEPTION
   WHEN OTHERS THEN
      IF SQLCODE != -2289 THEN
         RAISE;
      ELSE
         DBMS_OUTPUT.PUT_LINE('Séquence SEQ_MOTEUR_LOGS n''existe pas encore.');
      END IF;
END;
/

-- Suppression de la table si elle existe
BEGIN
   EXECUTE IMMEDIATE 'DROP TABLE MOTEUR_LOGS CASCADE CONSTRAINTS';
   DBMS_OUTPUT.PUT_LINE('Table MOTEUR_LOGS supprimée.');
EXCEPTION
   WHEN OTHERS THEN
      IF SQLCODE != -942 THEN
         RAISE;
      ELSE
         DBMS_OUTPUT.PUT_LINE('Table MOTEUR_LOGS n''existe pas encore.');
      END IF;
END;
/

-- Création de la table MOTEUR_LOGS
CREATE TABLE MOTEUR_LOGS (
    LOG_ID NUMBER PRIMARY KEY,
    PRODUCT_ID NUMBER NOT NULL,
    TIMESTAMP TIMESTAMP DEFAULT SYSTIMESTAMP,
    ACTION VARCHAR2(100) NOT NULL,
    STATUT VARCHAR2(50) DEFAULT 'EN_COURS',
    TEMPERATURE NUMBER(5,2),
    HUMIDITE NUMBER(5,2),
    VALEUR_GAZ NUMBER(10,2),
    MESSAGE VARCHAR2(500),
    UTILISATEUR VARCHAR2(100),
    CONSTRAINT FK_MOTEUR_PRODUCT FOREIGN KEY (PRODUCT_ID) REFERENCES PRODUITS(ID_PRODUIT) ON DELETE CASCADE,
    CONSTRAINT CHK_STATUT_MOTEUR CHECK (STATUT IN ('EN_COURS', 'TERMINE', 'ERREUR', 'ANNULE'))
);

-- Création de la séquence
CREATE SEQUENCE SEQ_MOTEUR_LOGS
    START WITH 1
    INCREMENT BY 1
    NOCACHE
    NOCYCLE;

-- Création des index pour optimiser les performances
CREATE INDEX IDX_MOTEUR_PRODUCT ON MOTEUR_LOGS(PRODUCT_ID);
CREATE INDEX IDX_MOTEUR_TIMESTAMP ON MOTEUR_LOGS(TIMESTAMP DESC);
CREATE INDEX IDX_MOTEUR_STATUT ON MOTEUR_LOGS(STATUT);

COMMENT ON TABLE MOTEUR_LOGS IS 'Logs des opérations moteur et capteurs Arduino';
COMMENT ON COLUMN MOTEUR_LOGS.PRODUCT_ID IS 'Référence au produit concerné';
COMMENT ON COLUMN MOTEUR_LOGS.ACTION IS 'Type d''action effectuée';
COMMENT ON COLUMN MOTEUR_LOGS.STATUT IS 'Statut de l''opération';

-- Insertion de données de test
INSERT INTO MOTEUR_LOGS (LOG_ID, PRODUCT_ID, ACTION, STATUT, TEMPERATURE, HUMIDITE, VALEUR_GAZ, MESSAGE, UTILISATEUR)
SELECT SEQ_MOTEUR_LOGS.NEXTVAL, 
       ID_PRODUIT, 
       'CONTROLE_QUALITE', 
       'TERMINE', 
       22.5, 
       45.0, 
       150.0, 
       'Contrôle qualité automatique effectué', 
       'SYSTEM'
FROM PRODUITS 
WHERE ROWNUM <= 5;

COMMIT;

PROMPT Table MOTEUR_LOGS créée avec succès.

-- Requête de vérification avec jointure
PROMPT ========================================
PROMPT Vérification des logs moteur
PROMPT ========================================
SELECT ML.LOG_ID, 
       ML.PRODUCT_ID, 
       P.DESIGNATION, 
       ML.ACTION, 
       ML.STATUT, 
       TO_CHAR(ML.TIMESTAMP, 'DD/MM/YYYY HH24:MI:SS') AS DATE_LOG,
       ML.TEMPERATURE,
       ML.HUMIDITE
FROM MOTEUR_LOGS ML
JOIN PRODUITS P ON P.ID_PRODUIT = ML.PRODUCT_ID
ORDER BY ML.TIMESTAMP DESC
FETCH FIRST 10 ROWS ONLY;

-- =============================================
-- 3. TABLE GAZ_ALERTS
-- =============================================
PROMPT ========================================
PROMPT Création de la table GAZ_ALERTS
PROMPT ========================================

-- Suppression de la séquence si elle existe
BEGIN
   EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_GAZ_ALERTS';
   DBMS_OUTPUT.PUT_LINE('Séquence SEQ_GAZ_ALERTS supprimée.');
EXCEPTION
   WHEN OTHERS THEN
      IF SQLCODE != -2289 THEN
         RAISE;
      ELSE
         DBMS_OUTPUT.PUT_LINE('Séquence SEQ_GAZ_ALERTS n''existe pas encore.');
      END IF;
END;
/

-- Suppression de la table si elle existe
BEGIN
   EXECUTE IMMEDIATE 'DROP TABLE GAZ_ALERTS CASCADE CONSTRAINTS';
   DBMS_OUTPUT.PUT_LINE('Table GAZ_ALERTS supprimée.');
EXCEPTION
   WHEN OTHERS THEN
      IF SQLCODE != -942 THEN
         RAISE;
      ELSE
         DBMS_OUTPUT.PUT_LINE('Table GAZ_ALERTS n''existe pas encore.');
      END IF;
END;
/

-- Création de la table GAZ_ALERTS
CREATE TABLE GAZ_ALERTS (
    ID NUMBER PRIMARY KEY,
    EMPLACEMENT_ID NUMBER,
    VALEUR_GAZ NUMBER(10,2) NOT NULL,
    MESSAGE VARCHAR2(255 BYTE),
    DATE_ALERT DATE DEFAULT SYSDATE,
    NIVEAU_ALERTE VARCHAR2(20) DEFAULT 'MOYEN',
    STATUT_TRAITEMENT VARCHAR2(50) DEFAULT 'NON_TRAITE',
    TRAITE_PAR VARCHAR2(100),
    DATE_TRAITEMENT DATE,
    COMMENTAIRE VARCHAR2(500),
    CONSTRAINT CHK_NIVEAU_ALERTE CHECK (NIVEAU_ALERTE IN ('BAS', 'MOYEN', 'ELEVE', 'CRITIQUE')),
    CONSTRAINT CHK_STATUT_TRAITEMENT CHECK (STATUT_TRAITEMENT IN ('NON_TRAITE', 'EN_COURS', 'TRAITE', 'IGNORE'))
);

-- Création de la séquence
CREATE SEQUENCE SEQ_GAZ_ALERTS
    START WITH 1
    INCREMENT BY 1
    NOCACHE
    NOCYCLE;

-- Création des index
CREATE INDEX IDX_GAZ_EMPLACEMENT ON GAZ_ALERTS(EMPLACEMENT_ID);
CREATE INDEX IDX_GAZ_DATE ON GAZ_ALERTS(DATE_ALERT DESC);
CREATE INDEX IDX_GAZ_NIVEAU ON GAZ_ALERTS(NIVEAU_ALERTE);
CREATE INDEX IDX_GAZ_STATUT ON GAZ_ALERTS(STATUT_TRAITEMENT);

COMMENT ON TABLE GAZ_ALERTS IS 'Alertes de détection de gaz dans les emplacements de stockage';
COMMENT ON COLUMN GAZ_ALERTS.EMPLACEMENT_ID IS 'Référence à l''emplacement de stockage';
COMMENT ON COLUMN GAZ_ALERTS.VALEUR_GAZ IS 'Valeur mesurée du gaz (ppm ou autre unité)';
COMMENT ON COLUMN GAZ_ALERTS.NIVEAU_ALERTE IS 'Niveau de criticité de l''alerte';

-- Insertion de données de test
INSERT INTO GAZ_ALERTS (ID, EMPLACEMENT_ID, VALEUR_GAZ, MESSAGE, NIVEAU_ALERTE, STATUT_TRAITEMENT)
VALUES (SEQ_GAZ_ALERTS.NEXTVAL, 1, 250.50, 'Détection de gaz au-dessus du seuil normal', 'MOYEN', 'NON_TRAITE');

INSERT INTO GAZ_ALERTS (ID, EMPLACEMENT_ID, VALEUR_GAZ, MESSAGE, NIVEAU_ALERTE, STATUT_TRAITEMENT)
VALUES (SEQ_GAZ_ALERTS.NEXTVAL, 2, 450.75, 'Alerte gaz élevée - Vérification requise', 'ELEVE', 'NON_TRAITE');

INSERT INTO GAZ_ALERTS (ID, EMPLACEMENT_ID, VALEUR_GAZ, MESSAGE, NIVEAU_ALERTE, STATUT_TRAITEMENT)
VALUES (SEQ_GAZ_ALERTS.NEXTVAL, 1, 800.00, 'ALERTE CRITIQUE - Évacuation recommandée', 'CRITIQUE', 'EN_COURS');

COMMIT;

PROMPT Table GAZ_ALERTS créée avec succès.
SELECT * FROM GAZ_ALERTS ORDER BY DATE_ALERT DESC;

-- =============================================
-- 4. VUES UTILES
-- =============================================
PROMPT ========================================
PROMPT Création des vues utiles
PROMPT ========================================

-- Vue pour les alertes non traitées
CREATE OR REPLACE VIEW V_ALERTES_GAZ_ACTIVES AS
SELECT 
    ID,
    EMPLACEMENT_ID,
    VALEUR_GAZ,
    MESSAGE,
    NIVEAU_ALERTE,
    STATUT_TRAITEMENT,
    TO_CHAR(DATE_ALERT, 'DD/MM/YYYY HH24:MI:SS') AS DATE_ALERTE_FORMAT,
    ROUND((SYSDATE - DATE_ALERT) * 24, 2) AS HEURES_DEPUIS_ALERTE
FROM GAZ_ALERTS
WHERE STATUT_TRAITEMENT IN ('NON_TRAITE', 'EN_COURS')
ORDER BY 
    CASE NIVEAU_ALERTE 
        WHEN 'CRITIQUE' THEN 1
        WHEN 'ELEVE' THEN 2
        WHEN 'MOYEN' THEN 3
        WHEN 'BAS' THEN 4
    END,
    DATE_ALERT DESC;

-- Vue pour le monitoring Arduino
CREATE OR REPLACE VIEW V_MONITORING_ARDUINO AS
SELECT 
    ML.LOG_ID,
    ML.PRODUCT_ID,
    P.DESIGNATION AS PRODUIT,
    ML.ACTION,
    ML.STATUT,
    ML.TEMPERATURE,
    ML.HUMIDITE,
    ML.VALEUR_GAZ,
    TO_CHAR(ML.TIMESTAMP, 'DD/MM/YYYY HH24:MI:SS') AS DATE_LOG,
    ML.MESSAGE
FROM MOTEUR_LOGS ML
LEFT JOIN PRODUITS P ON P.ID_PRODUIT = ML.PRODUCT_ID
ORDER BY ML.TIMESTAMP DESC;

PROMPT Vues créées avec succès.

-- =============================================
-- 5. PROCÉDURES STOCKÉES
-- =============================================
PROMPT ========================================
PROMPT Création des procédures stockées
PROMPT ========================================

-- Procédure pour enregistrer un log moteur
CREATE OR REPLACE PROCEDURE SP_LOG_MOTEUR(
    p_product_id IN NUMBER,
    p_action IN VARCHAR2,
    p_statut IN VARCHAR2 DEFAULT 'EN_COURS',
    p_temperature IN NUMBER DEFAULT NULL,
    p_humidite IN NUMBER DEFAULT NULL,
    p_valeur_gaz IN NUMBER DEFAULT NULL,
    p_message IN VARCHAR2 DEFAULT NULL,
    p_utilisateur IN VARCHAR2 DEFAULT 'SYSTEM'
) AS
BEGIN
    INSERT INTO MOTEUR_LOGS (
        LOG_ID, PRODUCT_ID, ACTION, STATUT, 
        TEMPERATURE, HUMIDITE, VALEUR_GAZ, MESSAGE, UTILISATEUR
    ) VALUES (
        SEQ_MOTEUR_LOGS.NEXTVAL, p_product_id, p_action, p_statut,
        p_temperature, p_humidite, p_valeur_gaz, p_message, p_utilisateur
    );
    COMMIT;
END;
/

-- Procédure pour créer une alerte gaz
CREATE OR REPLACE PROCEDURE SP_CREER_ALERTE_GAZ(
    p_emplacement_id IN NUMBER,
    p_valeur_gaz IN NUMBER,
    p_message IN VARCHAR2 DEFAULT NULL,
    p_niveau_alerte IN VARCHAR2 DEFAULT 'MOYEN'
) AS
BEGIN
    INSERT INTO GAZ_ALERTS (
        ID, EMPLACEMENT_ID, VALEUR_GAZ, MESSAGE, NIVEAU_ALERTE
    ) VALUES (
        SEQ_GAZ_ALERTS.NEXTVAL, p_emplacement_id, p_valeur_gaz, 
        p_message, p_niveau_alerte
    );
    COMMIT;
END;
/

-- Procédure pour traiter une alerte
CREATE OR REPLACE PROCEDURE SP_TRAITER_ALERTE_GAZ(
    p_alert_id IN NUMBER,
    p_traite_par IN VARCHAR2,
    p_commentaire IN VARCHAR2 DEFAULT NULL
) AS
BEGIN
    UPDATE GAZ_ALERTS
    SET STATUT_TRAITEMENT = 'TRAITE',
        TRAITE_PAR = p_traite_par,
        DATE_TRAITEMENT = SYSDATE,
        COMMENTAIRE = p_commentaire
    WHERE ID = p_alert_id;
    COMMIT;
END;
/

PROMPT Procédures stockées créées avec succès.

-- =============================================
-- 6. TRIGGERS
-- =============================================
PROMPT ========================================
PROMPT Création des triggers
PROMPT ========================================

-- Trigger pour auto-incrémenter GAZ_ALERTS
CREATE OR REPLACE TRIGGER TRG_GAZ_ALERTS_ID
BEFORE INSERT ON GAZ_ALERTS
FOR EACH ROW
BEGIN
    IF :NEW.ID IS NULL THEN
        :NEW.ID := SEQ_GAZ_ALERTS.NEXTVAL;
    END IF;
END;
/

-- Trigger pour auto-incrémenter MOTEUR_LOGS
CREATE OR REPLACE TRIGGER TRG_MOTEUR_LOGS_ID
BEFORE INSERT ON MOTEUR_LOGS
FOR EACH ROW
BEGIN
    IF :NEW.LOG_ID IS NULL THEN
        :NEW.LOG_ID := SEQ_MOTEUR_LOGS.NEXTVAL;
    END IF;
END;
/

-- Trigger pour mettre à jour DATE_MODIFICATION dans ARDUINO_CONFIG
CREATE OR REPLACE TRIGGER TRG_ARDUINO_CONFIG_UPDATE
BEFORE UPDATE ON ARDUINO_CONFIG
FOR EACH ROW
BEGIN
    :NEW.DATE_MODIFICATION := SYSDATE;
END;
/

PROMPT Triggers créés avec succès.

-- =============================================
-- 7. RÉSUMÉ ET VÉRIFICATIONS
-- =============================================
PROMPT ========================================
PROMPT RÉSUMÉ DE L'INSTALLATION
PROMPT ========================================

PROMPT;
PROMPT Tables créées:
SELECT table_name FROM user_tables 
WHERE table_name IN ('ARDUINO_CONFIG', 'MOTEUR_LOGS', 'GAZ_ALERTS')
ORDER BY table_name;

PROMPT;
PROMPT Séquences créées:
SELECT sequence_name FROM user_sequences 
WHERE sequence_name IN ('SEQ_MOTEUR_LOGS', 'SEQ_GAZ_ALERTS')
ORDER BY sequence_name;

PROMPT;
PROMPT Vues créées:
SELECT view_name FROM user_views 
WHERE view_name IN ('V_ALERTES_GAZ_ACTIVES', 'V_MONITORING_ARDUINO')
ORDER BY view_name;

PROMPT;
PROMPT Procédures créées:
SELECT object_name FROM user_objects 
WHERE object_type = 'PROCEDURE' 
AND object_name IN ('SP_LOG_MOTEUR', 'SP_CREER_ALERTE_GAZ', 'SP_TRAITER_ALERTE_GAZ')
ORDER BY object_name;

PROMPT;
PROMPT ========================================
PROMPT INSTALLATION TERMINÉE AVEC SUCCÈS!
PROMPT ========================================
PROMPT;
PROMPT Prochaines étapes:
PROMPT 1. Vérifier les données de test
PROMPT 2. Tester les procédures stockées
PROMPT 3. Intégrer avec l'application C++
PROMPT;
