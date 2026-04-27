# 🔧 Correction du Crash au Clic sur "Ravitaillement"

## ❌ Problème

L'application se fermait (crash) lorsqu'on cliquait sur l'onglet "Ravitaillement" dans le module Stock.

## 🔍 Cause du Crash

Le crash était causé par un appel à `updateKPIsAuto()` avant que la fonction ne soit initialisée.

### Séquence du Problème :

1. **Déclaration** (ligne ~6356) :
   ```cpp
   std::function<void()> updateKPIsAuto;  // Fonction vide (nullptr)
   ```

2. **Utilisation dans remplirTable** (ligne ~6478) :
   ```cpp
   updateKPIsAuto();  // ❌ CRASH ! La fonction n'est pas encore assignée
   ```

3. **Assignation** (ligne ~6883) :
   ```cpp
   updateKPIsAuto = [=]() {  // Assignation du vrai code
       // ... calculs ...
   };
   ```

4. **Appel de remplirTable** (ligne ~6633) :
   ```cpp
   remplirTable();  // Appelé AVANT l'assignation de updateKPIsAuto
   ```

### Ordre d'Exécution Problématique :

```
1. Déclaration de updateKPIsAuto (vide)
2. Déclaration de remplirTable (qui appelle updateKPIsAuto)
3. Appel de remplirTable() ← CRASH ICI
4. Assignation de updateKPIsAuto (trop tard)
```

## ✅ Solution Appliquée

Ajout d'une **vérification de nullité** avant chaque appel à `updateKPIsAuto()`.

### Modification 1 : Dans `remplirTable`

**Avant** :
```cpp
// Mise à jour automatique des KPIs après remplissage du tableau
updateKPIsAuto();
```

**Après** :
```cpp
// Mise à jour automatique des KPIs après remplissage du tableau
if (updateKPIsAuto) updateKPIsAuto();
```

### Modification 2 : Dans `cellClicked`

**Avant** :
```cpp
// Mise à jour automatique des KPIs après sélection manuelle
updateKPIsAuto();
```

**Après** :
```cpp
// Mise à jour automatique des KPIs après sélection manuelle
if (updateKPIsAuto) updateKPIsAuto();
```

### Modification 3 : Dans `onStockChanged`

**Avant** :
```cpp
auto onStockChanged = [=]() {
    updateJauge(sbStockActuel->value(), sbSeuilSecurite->value(), sbConsoPrevue->value());
    updateKPIsAuto(); // Mise à jour automatique des KPIs
};
```

**Après** :
```cpp
auto onStockChanged = [=]() {
    updateJauge(sbStockActuel->value(), sbSeuilSecurite->value(), sbConsoPrevue->value());
    if (updateKPIsAuto) updateKPIsAuto(); // Mise à jour automatique des KPIs
};
```

## 🎯 Comportement Après Correction

### Premier Appel (avant assignation)
```cpp
remplirTable();
  └─> if (updateKPIsAuto) updateKPIsAuto();
       └─> updateKPIsAuto est nullptr
       └─> Condition false, pas d'appel
       └─> ✅ Pas de crash
```

### Appels Suivants (après assignation)
```cpp
remplirTable();
  └─> if (updateKPIsAuto) updateKPIsAuto();
       └─> updateKPIsAuto est assigné
       └─> Condition true, appel de la fonction
       └─> ✅ KPIs mis à jour
```

## 📊 Ordre d'Exécution Corrigé

```
1. Déclaration de updateKPIsAuto (vide)
2. Déclaration de remplirTable (qui vérifie updateKPIsAuto)
3. Appel de remplirTable() ← ✅ Pas de crash (vérification)
4. Assignation de updateKPIsAuto
5. Appels suivants de remplirTable() ← ✅ KPIs mis à jour
```

## 🔒 Sécurité

La vérification `if (updateKPIsAuto)` garantit que :
- ✅ Pas de crash si la fonction n'est pas encore assignée
- ✅ Appel normal une fois la fonction assignée
- ✅ Comportement sûr dans tous les cas

## 🎉 Résultat

L'application ne crash plus au clic sur "Ravitaillement" ! Les KPIs se mettent à jour automatiquement une fois que `updateKPIsAuto` est assigné.

## 📝 Note Technique

En C++, un `std::function` non initialisé est équivalent à `nullptr`. La vérification `if (updateKPIsAuto)` teste si la fonction a été assignée avant de l'appeler, évitant ainsi un appel à une fonction nulle qui causerait un crash.
