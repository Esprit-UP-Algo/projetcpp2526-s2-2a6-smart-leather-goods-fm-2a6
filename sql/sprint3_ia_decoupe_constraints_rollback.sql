-- Sprint 3 - IA Decoupe
-- Rollback propre et idempotent (Oracle)
-- Execute ce script en tant que proprietaire du schema / table.

DECLARE
  PROCEDURE drop_constraint_if_exists(p_owner VARCHAR2, p_table VARCHAR2, p_constraint VARCHAR2) IS
    v_cnt NUMBER := 0;
  BEGIN
    SELECT COUNT(*)
      INTO v_cnt
      FROM ALL_CONSTRAINTS
     WHERE OWNER = UPPER(p_owner)
       AND TABLE_NAME = UPPER(p_table)
       AND CONSTRAINT_NAME = UPPER(p_constraint);
    IF v_cnt > 0 THEN
      EXECUTE IMMEDIATE 'ALTER TABLE ' || p_owner || '.' || p_table || ' DROP CONSTRAINT ' || p_constraint;
    END IF;
  END;

  PROCEDURE drop_column_if_exists(p_owner VARCHAR2, p_table VARCHAR2, p_col VARCHAR2) IS
    v_cnt NUMBER := 0;
  BEGIN
    SELECT COUNT(*)
      INTO v_cnt
      FROM ALL_TAB_COLUMNS
     WHERE OWNER = UPPER(p_owner)
       AND TABLE_NAME = UPPER(p_table)
       AND COLUMN_NAME = UPPER(p_col);
    IF v_cnt > 0 THEN
      EXECUTE IMMEDIATE 'ALTER TABLE ' || p_owner || '.' || p_table || ' DROP COLUMN ' || p_col;
    END IF;
  END;
BEGIN
  drop_constraint_if_exists('PROJET_CPP', 'PRODUIT_PIECES', 'CK_PP_NIVEAU_QUALITE');
  drop_constraint_if_exists('PROJET_CPP', 'PRODUIT_PIECES', 'CK_PP_SENS_GRAIN');
  drop_constraint_if_exists('PROJET_CPP', 'PRODUIT_PIECES', 'CK_PP_ZONE_PREF');

  drop_column_if_exists('PROJET_CPP', 'PRODUIT_PIECES', 'NIVEAU_QUALITE');
  drop_column_if_exists('PROJET_CPP', 'PRODUIT_PIECES', 'SENS_GRAIN');
  drop_column_if_exists('PROJET_CPP', 'PRODUIT_PIECES', 'ZONE_PREFEREE');
END;
/

COMMIT;
