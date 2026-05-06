# 🎯 ÉTAPES VISUELLES - QT CREATOR

## ✅ J'AI NETTOYÉ LE PROJET POUR VOUS!

Les fichiers de configuration ont été supprimés. Maintenant, suivez ces étapes:

---

## 📋 ÉTAPE PAR ÉTAPE (AVEC IMAGES)

### ÉTAPE 1: Fermer Qt Creator
1. ❌ **Fermer complètement Qt Creator**
2. ✅ Vérifier qu'il est bien fermé (Gestionnaire des tâches)

---

### ÉTAPE 2: Rouvrir Qt Creator
1. 🚀 **Lancer Qt Creator**
2. 📂 **Fichier → Ouvrir un fichier ou projet**
3. 📄 **Naviguer vers:** `C:\Users\jkglz\Desktop\projetCPP`
4. 📄 **Sélectionner:** `CMakeLists.txt`
5. ✅ **Cliquer:** "Ouvrir"

---

### ÉTAPE 3: Panneau de Configuration (IMPORTANT!)

Après avoir ouvert CMakeLists.txt, vous devriez voir:

```
┌─────────────────────────────────────────────────┐
│  Configure Project                               │
│                                                  │
│  Select kits to build and run your project:     │
│                                                  │
│  ☐ Desktop Qt 6.7.3 MinGW 64-bit                │
│  ☐ Desktop Qt 6.7.3 MSVC2019 64-bit             │
│  ☐ Android Qt 6.7.3 ARM64                       │
│                                                  │
│  [Configure Project]                            │
└─────────────────────────────────────────────────┘
```

**ACTIONS:**
1. ✅ **COCHER:** `☑ Desktop Qt 6.7.3 MinGW 64-bit`
2. ❌ **DÉCOCHER:** Tous les autres kits
3. 🔵 **CLIQUER:** Bouton "Configure Project"

---

### ÉTAPE 4: Attendre la Configuration

Vous verrez en bas de Qt Creator:

```
Parsing C++ files... [████████░░░░░░░░] 45%
```

⏳ **ATTENDEZ** que la barre atteigne 100%
⏳ Cela peut prendre **1-3 minutes**

---

### ÉTAPE 5: Vérifier que c'est configuré

Une fois terminé, vous devriez voir:

**En bas à gauche:**
```
▶️ [Desktop Qt 6.7.3 MinGW 64-bit] ▼
```

**Dans la barre latérale gauche:**
- ✅ L'icône "Modifier" (📝) devrait être active
- ✅ Plus de triangle jaune ⚠️

---

### ÉTAPE 6: Compiler le Projet

**Option A: Menu**
1. 🔨 **Menu:** Build
2. 🔨 **Cliquer:** Rebuild All

**Option B: Raccourci clavier**
1. ⌨️ **Appuyer:** `Ctrl + Shift + B`

---

### ÉTAPE 7: Suivre la Compilation

**Onglet "Compilation" en bas:**

```
┌─────────────────────────────────────────────────┐
│ Compilation | Problèmes | Recherche | Sortie   │
├─────────────────────────────────────────────────┤
│ [1/45] Building CXX object...                   │
│ [2/45] Building CXX object...                   │
│ [3/45] Building CXX object...                   │
│ ...                                             │
│ [45/45] Linking CXX executable ProjetCpp.exe    │
│                                                 │
│ ✅ Build successful                             │
└─────────────────────────────────────────────────┘
```

⏳ **ATTENDEZ** la fin de la compilation (2-5 minutes)

---

### ÉTAPE 8: Lancer l'Application

**Si compilation réussie:**

1. ▶️ **Cliquer** sur le triangle vert en bas à gauche
2. **OU** appuyer sur `Ctrl + R`

**L'application devrait démarrer! 🎉**

---

## ⚠️ SI VOUS VOYEZ DES ERREURS

### Erreur Type 1: "No valid kits found"

**Vous voyez:**
```
No valid kits found
Please configure at least one kit
```

**SOLUTION:**
1. 🔧 **Menu:** Outils → Options
2. 🔧 **Onglet:** Kits
3. ✅ **Vérifier** qu'un kit MinGW existe
4. ✅ Si aucun kit, cliquer "Add" et configurer

---

### Erreur Type 2: "matierepremiere.h: No such file"

**Vous voyez dans l'onglet Compilation:**
```
error: matierepremiere.h: No such file or directory
```

**SOLUTION:**
1. ❌ **Fermer Qt Creator**
2. 📂 **Ouvrir PowerShell** dans le dossier projet
3. ⌨️ **Exécuter:**
   ```powershell
   Copy-Item "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)\matierepremiere.h" .
   Copy-Item "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)\matierepremiere.cpp" .
   ```
4. 🔄 **Rouvrir Qt Creator**
5. 🔨 **Recompiler**

---

### Erreur Type 3: "undefined reference to MatierePremiere"

**Vous voyez:**
```
undefined reference to `MatierePremiere::ajouter()'
```

**SOLUTION:**
1. 📄 **Ouvrir:** `CMakeLists.txt`
2. 🔍 **Chercher** la ligne 34
3. ✅ **Vérifier** qu'elle contient:
   ```cmake
   matierepremiere.h matierepremiere.cpp
   ```
4. 💾 **Sauvegarder**
5. 🔨 **Recompiler**

---

## 🎯 CHECKLIST RAPIDE

Avant de compiler, vérifiez:

- [ ] Qt Creator est fermé puis rouvert
- [ ] CMakeLists.txt est ouvert
- [ ] Kit MinGW 64-bit est sélectionné
- [ ] "Configure Project" a été cliqué
- [ ] Configuration terminée (100%)
- [ ] Pas de triangle jaune ⚠️
- [ ] Fichiers matierepremiere.* existent
- [ ] CMakeLists.txt contient matierepremiere

---

## 📞 BESOIN D'AIDE?

**Si ça ne fonctionne toujours pas:**

1. 📸 **Faire une capture d'écran** de:
   - L'onglet "Compilation" avec les erreurs
   - Le panneau "Projets" (icône 🔧)

2. 📖 **Consulter:**
   - `DEPANNAGE_QT_CREATOR.md`
   - `SOLUTION_RAPIDE.md`

3. 🔍 **Vérifier les fichiers:**
   ```powershell
   Get-ChildItem matierepremiere.*
   Get-Content CMakeLists.txt | Select-String "matierepremiere"
   ```

---

## ✅ SUCCÈS!

**Si vous voyez:**
```
✅ Build successful
```

**Et que l'application démarre:**

🎉 **FÉLICITATIONS!**

**Testez maintenant:**
1. Aller dans "Stock" → "Matières Premières"
2. Vérifier que 5 matières s'affichent
3. Tester l'ajout d'une nouvelle matière

---

**Vous êtes prêt! 🚀**
