# 🔧 DÉPANNAGE QT CREATOR - PROBLÈMES DE COMPILATION

## 🎯 PROBLÈME: Qt Creator ne compile pas

### SOLUTION 1: Reconfigurer le projet dans Qt Creator

1. **Fermer Qt Creator complètement**

2. **Supprimer les fichiers de configuration**
   - Supprimer le dossier `build/`
   - Supprimer le dossier `.qtcreator/`
   - Supprimer le fichier `CMakeLists.txt.user` (s'il existe)

3. **Rouvrir Qt Creator**
   - Fichier → Ouvrir un fichier ou projet
   - Sélectionner `CMakeLists.txt`

4. **Configurer le projet**
   - Sélectionner le kit **Desktop Qt 6.7.3 MinGW 64-bit**
   - Cliquer sur "Configure Project"
   - Attendre la fin de la configuration

5. **Compiler**
   - Build → Rebuild All (Ctrl+Shift+B)

---

### SOLUTION 2: Utiliser le script batch

1. **Fermer Qt Creator**

2. **Double-cliquer sur `reconfigurer_et_compiler.bat`**

3. **Ajuster les chemins si nécessaire**
   
   Ouvrir `reconfigurer_et_compiler.bat` et modifier:
   ```batch
   set QT_DIR=C:\Qt\6.7.3\mingw_64
   set CMAKE_DIR=C:\Qt\Tools\CMake_64\bin
   set MINGW_DIR=C:\Qt\Tools\mingw1310_64\bin
   ```

4. **Réexécuter le script**

5. **Rouvrir Qt Creator**
   - Fichier → Ouvrir un fichier ou projet
   - Sélectionner `CMakeLists.txt`
   - Le projet devrait être configuré

---

### SOLUTION 3: Vérifier les fichiers manquants

Vérifier que tous les fichiers existent:

```bash
# Fichiers de matières premières
dir matierepremiere.h
dir matierepremiere.cpp

# Fichier de configuration
dir CMakeLists.txt

# Fichier de connexion
dir connexion.h
dir connexion.cpp
```

Si un fichier manque, le copier depuis le dossier d'intégration:

```bash
copy "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)\matierepremiere.h" .
copy "projetcpp2526-s2-2a6-smart-leather-goods-fm-2a6-Integration (2)\matierepremiere.cpp" .
```

---

### SOLUTION 4: Vérifier CMakeLists.txt

Ouvrir `CMakeLists.txt` et vérifier que la ligne suivante existe:

```cmake
matierepremiere.h matierepremiere.cpp
```

Elle devrait être autour de la ligne 34.

---

### SOLUTION 5: Nettoyer complètement

```bash
# Supprimer tous les fichiers de build
rmdir /s /q build
rmdir /s /q .qtcreator
del CMakeLists.txt.user

# Recréer le dossier build
mkdir build
```

Puis rouvrir Qt Creator et reconfigurer.

---

## 🔍 DIAGNOSTIQUER L'ERREUR EXACTE

### Dans Qt Creator:

1. **Aller dans l'onglet "Compilation"** (en bas)

2. **Chercher les lignes avec "error:"**

3. **Types d'erreurs courantes:**

#### Erreur: "matierepremiere.h: No such file or directory"
**Cause:** Fichier manquant  
**Solution:** Copier le fichier depuis le dossier d'intégration

#### Erreur: "undefined reference to MatierePremiere::..."
**Cause:** matierepremiere.cpp pas compilé  
**Solution:** Vérifier CMakeLists.txt, ligne 34

#### Erreur: "QSqlQuery: No such file or directory"
**Cause:** Module Qt Sql manquant  
**Solution:** Vérifier que Qt Sql est installé

#### Erreur: "cannot find -lQt6Sql"
**Cause:** Bibliothèque Qt Sql manquante  
**Solution:** Réinstaller Qt avec le module Sql

---

## 🛠️ VÉRIFICATIONS SYSTÈME

### 1. Vérifier l'installation Qt

```bash
# Vérifier que Qt est installé
dir C:\Qt\6.7.3\mingw_64\bin\Qt6Core.dll

# Vérifier que MinGW est installé
dir C:\Qt\Tools\mingw1310_64\bin\g++.exe

# Vérifier que CMake est installé
dir C:\Qt\Tools\CMake_64\bin\cmake.exe
```

### 2. Vérifier les variables d'environnement

Dans Qt Creator:
- Outils → Options → Kits
- Vérifier que le kit MinGW 64-bit est configuré
- Vérifier les chemins du compilateur et de Qt

### 3. Vérifier les modules Qt installés

Dans Qt Maintenance Tool:
- Lancer `C:\Qt\MaintenanceTool.exe`
- Vérifier que ces modules sont installés:
  - ✅ Qt 6.7.3
  - ✅ MinGW 13.1.0 64-bit
  - ✅ Qt Creator
  - ✅ CMake
  - ✅ Ninja

---

## 📋 CHECKLIST DE DÉPANNAGE

- [ ] Fermer Qt Creator
- [ ] Supprimer le dossier `build/`
- [ ] Supprimer le dossier `.qtcreator/`
- [ ] Vérifier que `matierepremiere.h` existe
- [ ] Vérifier que `matierepremiere.cpp` existe
- [ ] Vérifier `CMakeLists.txt` ligne 34
- [ ] Rouvrir Qt Creator
- [ ] Ouvrir `CMakeLists.txt`
- [ ] Configurer avec kit MinGW 64-bit
- [ ] Build → Rebuild All
- [ ] Vérifier l'onglet "Compilation" pour les erreurs

---

## 🚨 SI RIEN NE FONCTIONNE

### Option 1: Compilation manuelle

```bash
# Ouvrir PowerShell dans le dossier du projet
cd C:\Users\jkglz\Desktop\projetCPP

# Ajouter Qt au PATH
$env:PATH = "C:\Qt\6.7.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;$env:PATH"

# Créer et configurer
mkdir build -Force
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\mingw_64 ..

# Compiler
cmake --build .

# Lancer
.\ProjetCpp.exe
```

### Option 2: Utiliser un autre IDE

- **Visual Studio Code** avec l'extension CMake
- **CLion** de JetBrains
- **Visual Studio 2022** avec Qt VS Tools

---

## 📞 OBTENIR DE L'AIDE

Si le problème persiste, fournir ces informations:

1. **Message d'erreur exact** (copier depuis l'onglet Compilation)
2. **Version de Qt** (Aide → À propos de Qt Creator)
3. **Système d'exploitation** (Windows 10/11)
4. **Contenu de CMakeLists.txt** (lignes 30-40)
5. **Résultat de:** `dir matierepremiere.*`

---

## ✅ VÉRIFICATION FINALE

Une fois la compilation réussie:

```bash
# Vérifier que l'exécutable existe
dir build\ProjetCpp.exe

# Lancer l'application
cd build
ProjetCpp.exe
```

L'application devrait démarrer sans erreur.

---

**Bon courage! 💪**
