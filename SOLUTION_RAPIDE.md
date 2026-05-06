# ⚡ SOLUTION RAPIDE - PROBLÈME DE COMPILATION

## 🎯 PROBLÈME
Qt Creator ne compile pas le projet.

## ✅ SOLUTION EN 5 ÉTAPES

### ÉTAPE 1: Fermer Qt Creator
- Fermer complètement Qt Creator
- Vérifier qu'aucun processus Qt Creator ne tourne (Gestionnaire des tâches)

### ÉTAPE 2: Nettoyer le projet
Ouvrir PowerShell dans le dossier du projet et exécuter:

```powershell
# Supprimer le dossier build
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

# Supprimer les fichiers de configuration Qt Creator
Remove-Item -Recurse -Force .qtcreator -ErrorAction SilentlyContinue
Remove-Item CMakeLists.txt.user -ErrorAction SilentlyContinue

# Créer un nouveau dossier build vide
New-Item -ItemType Directory -Path build
```

### ÉTAPE 3: Rouvrir Qt Creator
1. Lancer Qt Creator
2. Fichier → Ouvrir un fichier ou projet
3. Sélectionner `CMakeLists.txt`
4. **IMPORTANT:** Sélectionner le kit **Desktop Qt 6.7.3 MinGW 64-bit**
5. Cliquer sur "Configure Project"
6. Attendre que la configuration se termine (barre de progression en bas)

### ÉTAPE 4: Compiler
1. Dans Qt Creator, aller dans le menu **Build**
2. Cliquer sur **Rebuild All** (ou Ctrl+Shift+B)
3. Attendre la fin de la compilation
4. Vérifier l'onglet "Compilation" en bas pour les erreurs

### ÉTAPE 5: Lancer
1. Cliquer sur le bouton ▶️ (Play) en bas à gauche
2. Ou appuyer sur Ctrl+R
3. L'application devrait démarrer

---

## 🔧 SI ÇA NE FONCTIONNE TOUJOURS PAS

### Vérifier les fichiers
Tous ces fichiers doivent exister:
```
✓ matierepremiere.h
✓ matierepremiere.cpp
✓ connexion.h
✓ connexion.cpp
✓ mainwindow.h
✓ mainwindow.cpp
✓ CMakeLists.txt
```

### Vérifier CMakeLists.txt
Ouvrir `CMakeLists.txt` et chercher la ligne 34:
```cmake
matierepremiere.h matierepremiere.cpp
```

Si elle n'existe pas, l'ajouter après la ligne:
```cmake
etape.h etape.cpp
```

### Vérifier l'onglet "Compilation"
Dans Qt Creator, en bas, cliquer sur l'onglet "Compilation" et chercher:
- Les lignes avec `error:`
- Les lignes avec `undefined reference`
- Les lignes avec `No such file`

**Copier le message d'erreur exact** et chercher la solution correspondante.

---

## 📋 ERREURS COURANTES

### Erreur: "matierepremiere.h: No such file or directory"
**Solution:**
```powershell
# Copier depuis le dossier d'intégration
Copy-Item "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)\matierepremiere.h" .
Copy-Item "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)\matierepremiere.cpp" .
```

### Erreur: "undefined reference to MatierePremiere::..."
**Solution:** Vérifier que `matierepremiere.cpp` est dans CMakeLists.txt ligne 34

### Erreur: "cannot find -lQt6Sql"
**Solution:** Réinstaller Qt avec le module Sql:
1. Lancer `C:\Qt\MaintenanceTool.exe`
2. Ajouter/Supprimer des composants
3. Cocher "Qt 6.7.3 → MinGW 64-bit"
4. Installer

---

## 🚀 ALTERNATIVE: COMPILATION MANUELLE

Si Qt Creator ne fonctionne pas, compiler manuellement:

### Méthode 1: Script Batch
Double-cliquer sur `reconfigurer_et_compiler.bat`

### Méthode 2: PowerShell
```powershell
# Ajouter Qt au PATH
$env:PATH = "C:\Qt\6.7.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;$env:PATH"

# Nettoyer
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path build

# Configurer
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\mingw_64 ..

# Compiler
cmake --build .

# Lancer
.\ProjetCpp.exe
```

---

## ✅ VÉRIFICATION FINALE

Une fois compilé avec succès:

1. **Vérifier l'exécutable:**
   ```powershell
   Test-Path build\ProjetCpp.exe
   ```
   Devrait retourner `True`

2. **Lancer l'application:**
   ```powershell
   cd build
   .\ProjetCpp.exe
   ```

3. **Tester le module Matières Premières:**
   - Aller dans l'onglet "Stock"
   - Cliquer sur "Matières Premières"
   - Vérifier que 5 matières s'affichent

---

## 📞 BESOIN D'AIDE?

Si le problème persiste:

1. **Copier le message d'erreur exact** depuis l'onglet "Compilation"
2. **Vérifier les fichiers:**
   ```powershell
   Get-ChildItem matierepremiere.*
   ```
3. **Consulter:** `DEPANNAGE_QT_CREATOR.md`

---

**Bon courage! 💪**
