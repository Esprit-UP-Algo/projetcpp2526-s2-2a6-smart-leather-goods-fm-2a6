-- Sprint 2 - Upgrade competences avec mode securise DRY RUN / APPLY
-- Usage SQL*Plus (robuste, non interactif):
--   1) Assurer que COMPETENCES_EMPLOYES existe (script sprint2_competences_etapes.sql)
--   2) Optionnel: surcharger les variables avant execution:
--      DEFINE MODE = 'DRY'
--      DEFINE SEUIL_N3 = 3
--      DEFINE SEUIL_N4 = 6
--      DEFINE SEUIL_N5 = 20
--   3) Lancer:
--      @sql/sprint2_upgrade_competences_levels.sql
--
-- Regle de niveau cible (depuis historique ETAPES):
--   1..(SEUIL_N3-1) occurrences => niveau 2
--   SEUIL_N3..(SEUIL_N4-1)       => niveau 3
--   SEUIL_N4..(SEUIL_N5-1)       => niveau 4
--   >= SEUIL_N5                  => niveau 5
--
-- Le script:
--   - met a jour UNIQUEMENT si niveau existant < niveau cible
--   - insere les competences manquantes
--   - ne baisse jamais un niveau existant

SET SERVEROUTPUT ON;
SET VERIFY OFF;

-- Parametres par defaut (evite les prompts ACCEPT fragiles en copier/coller SQL*Plus)
DEFINE MODE = 'DRY'
DEFINE SEUIL_N3 = 3
DEFINE SEUIL_N4 = 8
DEFINE SEUIL_N5 = 20

PROMPT Mode=&MODE | Seuils: N3=&SEUIL_N3 N4=&SEUIL_N4 N5=&SEUIL_N5

DECLARE
    v_cnt NUMBER;
BEGIN
    SELECT COUNT(*) INTO v_cnt FROM USER_TABLES WHERE TABLE_NAME = 'COMPETENCES_EMPLOYES';
    IF v_cnt = 0 THEN
        RAISE_APPLICATION_ERROR(-20010,
            'Table COMPETENCES_EMPLOYES absente. Lancez d''abord: @sql/sprint2_competences_etapes.sql');
    END IF;

    SELECT COUNT(*) INTO v_cnt FROM USER_TABLES WHERE TABLE_NAME = 'ETAPES';
    IF v_cnt = 0 THEN
        RAISE_APPLICATION_ERROR(-20011, 'Table ETAPES absente dans ce schema.');
    END IF;
END;
/

PROMPT =========================================
PROMPT [1/4] PREVIEW - Resume des actions
PROMPT =========================================

WITH hist AS (
    SELECT
        e.ID_EMPLOYE,
        UPPER(TRIM(e.ETAPE_ACTUELLE)) AS ETAPE,
        COUNT(*) AS NB_OCC,
        CASE
            WHEN COUNT(*) >= &&SEUIL_N5 THEN 5
            WHEN COUNT(*) >= &&SEUIL_N4 THEN 4
            WHEN COUNT(*) >= &&SEUIL_N3 THEN 3
            ELSE 2
        END AS NIV_CIBLE
    FROM ETAPES e
    WHERE e.ID_EMPLOYE IS NOT NULL
      AND e.ETAPE_ACTUELLE IS NOT NULL
      AND TRIM(e.ETAPE_ACTUELLE) IS NOT NULL
      AND UPPER(TRIM(e.ETAPE_ACTUELLE)) IN ('COUPE','ASSEMBLAGE','COUTURE','FINITION')
    GROUP BY e.ID_EMPLOYE, UPPER(TRIM(e.ETAPE_ACTUELLE))
),
joined_data AS (
    SELECT
        h.ID_EMPLOYE,
        h.ETAPE,
        h.NB_OCC,
        h.NIV_CIBLE,
        c.ID_COMPETENCE,
        c.NIVEAU AS NIV_ACTUEL
    FROM hist h
    LEFT JOIN COMPETENCES_EMPLOYES c
      ON c.ID_EMPLOYE = h.ID_EMPLOYE
     AND UPPER(TRIM(c.ETAPE)) = h.ETAPE
)
SELECT
    CASE
        WHEN ID_COMPETENCE IS NULL THEN 'INSERT'
        WHEN NVL(NIV_ACTUEL, 0) < NIV_CIBLE THEN 'UPGRADE'
        ELSE 'UNCHANGED'
    END AS ACTION,
    COUNT(*) AS NB
FROM joined_data
GROUP BY
    CASE
        WHEN ID_COMPETENCE IS NULL THEN 'INSERT'
        WHEN NVL(NIV_ACTUEL, 0) < NIV_CIBLE THEN 'UPGRADE'
        ELSE 'UNCHANGED'
    END
ORDER BY ACTION;

PROMPT =========================================
PROMPT [2/4] PREVIEW - Details upgrades proposes
PROMPT =========================================

WITH hist AS (
    SELECT
        e.ID_EMPLOYE,
        UPPER(TRIM(e.ETAPE_ACTUELLE)) AS ETAPE,
        COUNT(*) AS NB_OCC,
        CASE
            WHEN COUNT(*) >= &&SEUIL_N5 THEN 5
            WHEN COUNT(*) >= &&SEUIL_N4 THEN 4
            WHEN COUNT(*) >= &&SEUIL_N3 THEN 3
            ELSE 2
        END AS NIV_CIBLE
    FROM ETAPES e
    WHERE e.ID_EMPLOYE IS NOT NULL
      AND e.ETAPE_ACTUELLE IS NOT NULL
      AND TRIM(e.ETAPE_ACTUELLE) IS NOT NULL
      AND UPPER(TRIM(e.ETAPE_ACTUELLE)) IN ('COUPE','ASSEMBLAGE','COUTURE','FINITION')
    GROUP BY e.ID_EMPLOYE, UPPER(TRIM(e.ETAPE_ACTUELLE))
)
SELECT
    h.ID_EMPLOYE,
    h.ETAPE,
    h.NB_OCC,
    NVL(c.NIVEAU, 0) AS NIV_ACTUEL,
    h.NIV_CIBLE
FROM hist h
LEFT JOIN COMPETENCES_EMPLOYES c
  ON c.ID_EMPLOYE = h.ID_EMPLOYE
 AND UPPER(TRIM(c.ETAPE)) = h.ETAPE
WHERE c.ID_COMPETENCE IS NULL
   OR NVL(c.NIVEAU, 0) < h.NIV_CIBLE
ORDER BY h.ID_EMPLOYE, h.ETAPE;

PROMPT =========================================
PROMPT [3/4] EXECUTION conditionnelle
PROMPT =========================================

DECLARE
    v_mode VARCHAR2(10) := UPPER(TRIM('&&MODE'));
    v_n3 NUMBER := &&SEUIL_N3;
    v_n4 NUMBER := &&SEUIL_N4;
    v_n5 NUMBER := &&SEUIL_N5;
BEGIN
    IF v_mode NOT IN ('DRY', 'APPLY') THEN
        RAISE_APPLICATION_ERROR(-20001, 'Mode invalide. Utiliser DRY ou APPLY.');
    END IF;
    IF v_n3 < 2 OR v_n4 <= v_n3 OR v_n5 <= v_n4 THEN
        RAISE_APPLICATION_ERROR(-20002, 'Seuils invalides: il faut 2 <= N3 < N4 < N5.');
    END IF;

    IF v_mode = 'DRY' THEN
        DBMS_OUTPUT.PUT_LINE('DRY RUN: aucune ecriture effectuee. Seuils N3=' || v_n3 || ', N4=' || v_n4 || ', N5=' || v_n5);
    ELSE
        EXECUTE IMMEDIATE q'[
            MERGE INTO COMPETENCES_EMPLOYES t
            USING (
                SELECT
                    e.ID_EMPLOYE,
                    UPPER(TRIM(e.ETAPE_ACTUELLE)) AS ETAPE,
                    COUNT(*) AS NB_OCC,
                    CASE
                        WHEN COUNT(*) >= ]' || v_n5 || q'[ THEN 5
                        WHEN COUNT(*) >= ]' || v_n4 || q'[ THEN 4
                        WHEN COUNT(*) >= ]' || v_n3 || q'[ THEN 3
                        ELSE 2
                    END AS NIV_CIBLE
                FROM ETAPES e
                WHERE e.ID_EMPLOYE IS NOT NULL
                  AND e.ETAPE_ACTUELLE IS NOT NULL
                  AND TRIM(e.ETAPE_ACTUELLE) IS NOT NULL
                  AND UPPER(TRIM(e.ETAPE_ACTUELLE)) IN ('COUPE','ASSEMBLAGE','COUTURE','FINITION')
                GROUP BY e.ID_EMPLOYE, UPPER(TRIM(e.ETAPE_ACTUELLE))
            ) s
            ON (t.ID_EMPLOYE = s.ID_EMPLOYE AND UPPER(TRIM(t.ETAPE)) = s.ETAPE)
            WHEN MATCHED THEN
                UPDATE SET
                    t.NIVEAU = CASE WHEN NVL(t.NIVEAU, 0) < s.NIV_CIBLE THEN s.NIV_CIBLE ELSE t.NIVEAU END,
                    t.ACTIF = 1,
                    t.COMMENTAIRE = CASE
                        WHEN NVL(t.NIVEAU, 0) < s.NIV_CIBLE
                            THEN 'Upgrade auto depuis ETAPES (occ=' || s.NB_OCC || ')'
                        ELSE t.COMMENTAIRE
                    END,
                    t.UPDATED_AT = SYSDATE
                WHERE NVL(t.NIVEAU, 0) < s.NIV_CIBLE
            WHEN NOT MATCHED THEN
                INSERT (
                    ID_COMPETENCE,
                    ID_EMPLOYE,
                    ETAPE,
                    NIVEAU,
                    ACTIF,
                    COMMENTAIRE,
                    UPDATED_AT
                )
                VALUES (
                    NULL,
                    s.ID_EMPLOYE,
                    s.ETAPE,
                    s.NIV_CIBLE,
                    1,
                    'Bootstrap/upgrade auto depuis ETAPES (occ=' || s.NB_OCC || ')',
                    SYSDATE
                )
        ]';

        COMMIT;
        DBMS_OUTPUT.PUT_LINE('APPLY: merge execute et commit effectue.');
    END IF;
END;
/

PROMPT =========================================
PROMPT [4/4] Rapport final
PROMPT =========================================

SELECT ETAPE, NIVEAU, COUNT(*) AS NB_EMP
FROM COMPETENCES_EMPLOYES
GROUP BY ETAPE, NIVEAU
ORDER BY ETAPE, NIVEAU;

PROMPT Terminee.
