# 🔧 INSTRUCTIONS DE COMPILATION

## Problème actuel
Le projet ne compile pas avec l'erreur: `[CMakeFiles\ProjetCpp.dir\build.make:773: ProjetCpp.exe] Error 1`

## Solution recommandée

### Étape 1: Nettoyer le projet
1. Fermer Qt Creator complètement
2. Supprimer le dossier `build/` entièrement
3. Supprimer le dossier `.qtcreator/` (si présent)

### Étape 2: Reconfigurer dans Qt Creator
1. Ouvrir Qt Creator
2. Fichier → Ouvrir un fichier ou projet
3. Sélectionner `CMakeLists.txt`
4. Choisir le kit **Desktop Qt 6.7.3 MinGW 64-bit**
5. Cliquer sur "Configure Project"
6. Attendre la fin de la configuration

### Étape 3: Compiler
1. Build → Rebuild All (Ctrl+Shift+B)
2. Vérifier l'onglet "Compilation" pour voir les messages

## Si l'erreur persiste

### Vérifier les fichiers critiques
Tous ces fichiers doivent exister:
- ✅ `mainwindow.cpp`
- ✅ `mainwindow.h`
- ✅ `mainwindow.ui`
- ✅ `matierepremiere.cpp`
- ✅ `matierepremiere.h`
- ✅ `CMakeLists.txt`

### Vérifier CMakeLists.txt
Le fichier `CMakeLists.txt` doit contenir (ligne ~34):
```cmake
matierepremiere.h matierepremiere.cpp
```

### Compilation manuelle (alternative)
Si Qt Creator ne fonctionne pas, essayer la compilation manuelle:

```powershell
# Ouvrir PowerShell dans le dossier du projet
cd C:\Users\jkglz\Desktop\projetCPP

# Ajouter Qt au PATH
$env:PATH = "C:\Qt\6.7.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;$env:PATH"

# Nettoyer
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

# Créer et configurer
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\mingw_64 ..

# Compiler
cmake --build .
```

## Diagnostic de l'erreur

L'erreur "Error 1" est généralement:
1. **Erreur de linkage** - bibliothèque manquante
2. **Erreur de syntaxe C++** - code invalide
3. **Fichier manquant** - fichier .cpp ou .h absent

### Pour identifier l'erreur exacte:
1. Dans Qt Creator, aller dans l'onglet "Compilation" (en bas)
2. Chercher les lignes contenant "error:"
3. Noter le message d'erreur complet
4. Chercher la première erreur (pas les erreurs en cascade)

## Modifications récentes

Les dernières modifications concernent:
1. **Fonction `showStockRavitaillementTab()`** - lignes 13315-16214
2. **Fonction `majPhotoCuir`** - lambda pour charger les images (lignes 14047-14109)

Ces modifications sont syntaxiquement correctes et ne devraient pas causer d'erreur de compilation.

## Vérification rapide

Pour vérifier que le code est correct:
```bash
# Vérifier la syntaxe C++ (si g++ est disponible)
g++ -fsyntax-only -std=c++17 mainwindow.cpp
```

## Contact

Si le problème persiste après avoir suivi ces étapes, fournir:
1. Le message d'erreur COMPLET depuis l'onglet Compilation
2. La version de Qt Creator (Aide → À propos)
3. Le résultat de: `dir matierepremiere.*`

---

**Note:** Le code actuel est syntaxiquement correct. L'erreur est probablement liée à la configuration du projet ou à un fichier manquant, pas au code lui-même.
