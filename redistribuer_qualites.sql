-- =========================================================
-- SCRIPT : Redistribution des qualités A/B/C dans MATIERES_PREMIERES
-- À exécuter UNE SEULE FOIS dans Oracle SQL Developer
-- =========================================================

-- Étape 1 : Voir l'état actuel
SELECT QUALITE, COUNT(*), SUM(QUANTITE) 
FROM MATIERES_PREMIERES 
WHERE CATEGORIE_MP = 'Cuir'
GROUP BY QUALITE;

-- Étape 2 : Redistribuer les lignes existantes en A/B/C
-- On prend les IDs triés et on assigne : 1/3 → A, 1/3 → B, 1/3 → C

-- Mettre en B les lignes du milieu (33% à 66%)
UPDATE MATIERES_PREMIERES
SET QUALITE = 'B',
    NUM_LOT = REPLACE(NUM_LOT, '-A', '-B')
WHERE ID_STOCK_MP IN (
    SELECT ID_STOCK_MP FROM (
        SELECT ID_STOCK_MP,
               NTILE(3) OVER (ORDER BY ID_STOCK_MP) AS grp
        FROM MATIERES_PREMIERES
        WHERE CATEGORIE_MP = 'Cuir'
    ) WHERE grp = 2
);

-- Mettre en C les dernières lignes (66% à 100%)
UPDATE MATIERES_PREMIERES
SET QUALITE = 'C',
    NUM_LOT = REPLACE(NUM_LOT, '-A', '-C')
WHERE ID_STOCK_MP IN (
    SELECT ID_STOCK_MP FROM (
        SELECT ID_STOCK_MP,
               NTILE(3) OVER (ORDER BY ID_STOCK_MP) AS grp
        FROM MATIERES_PREMIERES
        WHERE CATEGORIE_MP = 'Cuir'
    ) WHERE grp = 3
);

-- Les lignes restantes (grp = 1) gardent la qualité A

COMMIT;

-- Étape 3 : Vérifier le résultat
SELECT QUALITE, COUNT(*), SUM(QUANTITE) 
FROM MATIERES_PREMIERES 
WHERE CATEGORIE_MP = 'Cuir'
GROUP BY QUALITE
ORDER BY QUALITE;
