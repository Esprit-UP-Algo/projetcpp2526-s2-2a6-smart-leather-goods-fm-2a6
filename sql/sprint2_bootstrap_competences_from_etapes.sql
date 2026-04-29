-- Sprint 2 - Bootstrap competences depuis historique ETAPES
-- Objectif: eviter la saisie manuelle longue
-- Regle de niveau initial:
--   - 1 a 2 occurrences sur une etape -> niveau 2
--   - >= 3 occurrences sur une etape -> niveau 3
-- Ne modifie pas les competences deja existantes (MERGE en insert-only).

-- Verif rapide prealable (optionnel):
-- SELECT COUNT(*) FROM ETAPES;
-- SELECT COUNT(*) FROM COMPETENCES_EMPLOYES;

MERGE INTO COMPETENCES_EMPLOYES t
USING (
    SELECT
        e.ID_EMPLOYE,
        UPPER(TRIM(e.ETAPE_ACTUELLE)) AS ETAPE,
        COUNT(*) AS NB_OCC,
        CASE
            WHEN COUNT(*) >= 3 THEN 3
            ELSE 2
        END AS NIV_INIT
    FROM ETAPES e
    WHERE e.ID_EMPLOYE IS NOT NULL
      AND e.ETAPE_ACTUELLE IS NOT NULL
      AND TRIM(e.ETAPE_ACTUELLE) IS NOT NULL
      AND UPPER(TRIM(e.ETAPE_ACTUELLE)) IN ('COUPE','ASSEMBLAGE','COUTURE','FINITION')
    GROUP BY e.ID_EMPLOYE, UPPER(TRIM(e.ETAPE_ACTUELLE))
) s
ON (t.ID_EMPLOYE = s.ID_EMPLOYE AND UPPER(TRIM(t.ETAPE)) = s.ETAPE)
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
        s.NIV_INIT,
        1,
        'Bootstrap auto depuis ETAPES (occ=' || s.NB_OCC || ')',
        SYSDATE
    );

COMMIT;

-- Rapport de controle:
SELECT ETAPE, NIVEAU, COUNT(*) AS NB_EMP
FROM COMPETENCES_EMPLOYES
GROUP BY ETAPE, NIVEAU
ORDER BY ETAPE, NIVEAU;

SELECT ID_EMPLOYE, ETAPE, NIVEAU, ACTIF, COMMENTAIRE
FROM COMPETENCES_EMPLOYES
ORDER BY ID_EMPLOYE, ETAPE;
