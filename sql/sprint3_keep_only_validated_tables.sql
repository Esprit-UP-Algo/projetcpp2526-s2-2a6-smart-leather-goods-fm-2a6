-- Nettoyage schema Oracle: conserver uniquement les tables validees E/A
-- IMPORTANT: script destructif (DROP TABLE ... CASCADE CONSTRAINTS PURGE)
-- Tables conservees:
--   PLANIFICATION, ETAPES, PRODUITS, EMPLOYES, CLIENTS, DEPOTS, MATIERES_PREMIERES
--
-- NOTE:
-- - Ce script supprime explicitement les tables "hors perimetre" vues dans ton schema.
-- - Execute ce script seulement apres sauvegarde.

-- Optionnel: afficher ce qui sera conserve
SELECT TABLE_NAME
FROM USER_TABLES
WHERE TABLE_NAME IN (
  'PLANIFICATION','ETAPES','PRODUITS','EMPLOYES','CLIENTS','DEPOTS','MATIERES_PREMIERES'
)
ORDER BY TABLE_NAME;

-- Supprimer les tables hors perimetre valide
DROP TABLE ABSENCES_EMPLOYES CASCADE CONSTRAINTS PURGE;
DROP TABLE CALENDRIER_PROD CASCADE CONSTRAINTS PURGE;
DROP TABLE CAPACITE_POSTE_JOUR CASCADE CONSTRAINTS PURGE;
DROP TABLE DECOUPE_PLACEMENT CASCADE CONSTRAINTS PURGE;
DROP TABLE DECOUPE_PLAN CASCADE CONSTRAINTS PURGE;
DROP TABLE PRODUIT_PIECES CASCADE CONSTRAINTS PURGE;

-- Tu as deux variantes matiere dans le schema.
-- Si ton modele final valide est MATIERES_PREMIERES (pluriel),
-- alors on supprime MATIERE_PREMIERE (singulier):
DROP TABLE MATIERE_PREMIERE CASCADE CONSTRAINTS PURGE;

COMMIT;

-- Verification finale: tables restantes
SELECT TABLE_NAME
FROM USER_TABLES
ORDER BY TABLE_NAME;
