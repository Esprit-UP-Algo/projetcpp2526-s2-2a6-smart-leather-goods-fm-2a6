-- Sprint 3 - IA Decoupe
-- Seed des contraintes metier par type de piece
-- Ce script est rejouable: il met a jour selon NOM_PIECE

-- Face / Dos / Avant / Arriere -> DOSSET, qualite premium
UPDATE PROJET_CPP.PRODUIT_PIECES
SET ZONE_PREFEREE = 'DOSSET',
    SENS_GRAIN = 0,
    NIVEAU_QUALITE = 3
WHERE LOWER(NOM_PIECE) LIKE '%face%'
   OR LOWER(NOM_PIECE) LIKE '%avant%'
   OR LOWER(NOM_PIECE) LIKE '%dos%'
   OR LOWER(NOM_PIECE) LIKE '%arriere%';

-- Rabat / Poche / Fond -> CROUPON, qualite elevee
UPDATE PROJET_CPP.PRODUIT_PIECES
SET ZONE_PREFEREE = 'CROUPON',
    SENS_GRAIN = 0,
    NIVEAU_QUALITE = 2
WHERE LOWER(NOM_PIECE) LIKE '%rabat%'
   OR LOWER(NOM_PIECE) LIKE '%poche%'
   OR LOWER(NOM_PIECE) LIKE '%fond%';

-- Bandouliere / Anse / Bretelle / Sangle -> FLANC, grain vertical
UPDATE PROJET_CPP.PRODUIT_PIECES
SET ZONE_PREFEREE = 'FLANC',
    SENS_GRAIN = 1,
    NIVEAU_QUALITE = 2
WHERE LOWER(NOM_PIECE) LIKE '%bandouli%'
   OR LOWER(NOM_PIECE) LIKE '%anse%'
   OR LOWER(NOM_PIECE) LIKE '%bretelle%'
   OR LOWER(NOM_PIECE) LIKE '%sangle%';

-- Valeurs de securite pour lignes non couvertes
UPDATE PROJET_CPP.PRODUIT_PIECES
SET ZONE_PREFEREE = NVL(ZONE_PREFEREE, 'COLLET'),
    SENS_GRAIN = NVL(SENS_GRAIN, 0),
    NIVEAU_QUALITE = NVL(NIVEAU_QUALITE, 1);

COMMIT;
