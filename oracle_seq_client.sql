-- Utilisateur typique de l'app (connexion.cpp) : Projet_cpp
-- Connecte-toi dans SQL Developer avec CE MÊME compte, puis exécute UNE fois.

-- Si aucune séquence du même nom n'existe : crée-la après le dernier ID_CLIENT
DECLARE
  v_next INTEGER;
BEGIN
  SELECT NVL(MAX(ID_CLIENT), 0) + 1 INTO v_next FROM CLIENTS;
  EXECUTE IMMEDIATE
    'CREATE SEQUENCE SEQ_CLIENT START WITH ' || v_next ||
    ' INCREMENT BY 1 NOCACHE NOCYCLE';
END;
/

-- Si erreur ORA-00955 (nom déjà utilisé), la séquence existe déjà :
--   SELECT SEQ_CLIENT.NEXTVAL FROM DUAL;
-- ou
--   DROP SEQUENCE SEQ_CLIENT;
-- puis relance le bloc ci-dessus.
