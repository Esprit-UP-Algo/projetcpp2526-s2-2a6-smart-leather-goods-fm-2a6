# 🔧 Correction de l'Affichage du Formulaire "Ajouter Matière"

## ❌ Problème

Le formulaire "Ajouter Matière" n'affiche pas les champs de saisie. On ne voit que :
- Le titre "NOUVELLE MATIÈRE PREMIÈRE"
- Un grand espace blanc
- Le panneau "Guide de Création" à droite

Les champs de saisie (Code MP, Catégorie, Lot, etc.) ne sont pas visibles.

## 🔍 Cause Probable

Les widgets du formulaire sont définis dans le fichier `.ui` mais ne sont pas visibles à l'écran. Causes possibles :
1. Widgets masqués par défaut
2. Problème de layout ou de taille
3. Style CSS qui rend les widgets invisibles
4. Problème de compilation du fichier `.ui`

## ✅ Solution Appliquée

### Modification 1 : Forcer la Visibilité des Widgets

Ajout de code dans le constructeur `MainWindow::MainWindow()` pour forcer l'affichage de tous les widgets du formulaire :

```cpp
// S'assurer que tous les widgets du formulaire sont visibles
if (ui->le_stock_code) ui->le_stock_code->setVisible(true);
if (ui->cb_stock_cat) ui->cb_stock_cat->setVisible(true);
if (ui->le_stock_lot) ui->le_stock_lot->setVisible(true);
if (ui->cb_stock_etat) ui->cb_stock_etat->setVisible(true);
if (ui->le_stock_coul) ui->le_stock_coul->setVisible(true);
if (ui->sb_stock_qte) ui->sb_stock_qte->setVisible(true);
if (ui->cb_stock_type) ui->cb_stock_type->setVisible(true);
if (ui->cb_stock_qual) ui->cb_stock_qual->setVisible(true);
if (ui->btn_valider_stock) ui->btn_valider_stock->setVisible(true);

// S'assurer que les labels sont visibles
if (ui->lbl_sa_code) ui->lbl_sa_code->setVisible(true);
if (ui->lbl_sa_cat) ui->lbl_sa_cat->setVisible(true);
if (ui->lbl_sa_lot) ui->lbl_sa_lot->setVisible(true);
if (ui->lbl_sa_etat) ui->lbl_sa_etat->setVisible(true);
if (ui->lbl_sa_coul) ui->lbl_sa_coul->setVisible(true);
if (ui->lbl_sa_qte) ui->lbl_sa_qte->setVisible(true);
if (ui->lbl_sa_type) ui->lbl_sa_type->setVisible(true);
if (ui->lbl_sa_qual) ui->lbl_sa_qual->setVisible(true);
```

### Modification 2 : Valeur Par Défaut pour la Quantité

```cpp
ui->sb_stock_qte->setValue(1.0); // Valeur par défaut
```

## 🚀 Étapes de Correction

### 1. Recompiler le Projet

**Important** : Après modification du code, il faut recompiler complètement :

```bash
# Dans Qt Creator
1. Build > Clean All
2. Build > Rebuild All
```

Ou en ligne de commande :
```bash
cd build
cmake --build . --clean-first
```

### 2. Vérifier le Fichier UI

Si le problème persiste, vérifiez que le fichier `mainwindow.ui` est correctement compilé :

1. Ouvrez `mainwindow.ui` dans Qt Designer
2. Vérifiez que l'onglet "Ajouter Matière" (`tab_stock_ajouter`) contient bien :
   - Un `QVBoxLayout` nommé `vl_stock_ajouter`
   - Un `QLabel` pour le titre (`lbl_titre_ajout_stock`)
   - Un `QFormLayout` nommé `fl_stock_ajouter` avec tous les champs
   - Un `QPushButton` nommé `btn_valider_stock`

3. Sauvegardez le fichier `.ui`
4. Recompilez le projet

### 3. Vérifier les Noms des Widgets

Les noms des widgets dans le fichier `.ui` doivent correspondre exactement à ceux utilisés dans le code C++ :

| Widget | Nom dans .ui | Type |
|--------|--------------|------|
| Code MP | `le_stock_code` | QLineEdit |
| Catégorie | `cb_stock_cat` | QComboBox |
| Numéro de Lot | `le_stock_lot` | QLineEdit |
| État | `cb_stock_etat` | QComboBox |
| Couleur | `le_stock_coul` | QLineEdit |
| Quantité | `sb_stock_qte` | QDoubleSpinBox |
| Type Stockage | `cb_stock_type` | QComboBox |
| Qualité | `cb_stock_qual` | QComboBox |
| Bouton Valider | `btn_valider_stock` | QPushButton |

## 🔧 Solutions Alternatives

### Solution 1 : Vérifier le Style CSS

Le style appliqué pourrait rendre les widgets invisibles. Vérifiez dans `mainwindow.cpp` :

```cpp
ui->tab_stock_ajouter->setStyleSheet(formStyle);
```

Essayez temporairement de commenter cette ligne pour voir si le formulaire s'affiche :

```cpp
// ui->tab_stock_ajouter->setStyleSheet(formStyle);
```

### Solution 2 : Forcer la Mise à Jour du Layout

Ajoutez ce code après l'initialisation :

```cpp
if (ui->tab_stock_ajouter) {
    ui->tab_stock_ajouter->updateGeometry();
    ui->tab_stock_ajouter->update();
}
```

### Solution 3 : Vérifier la Taille Minimale

Assurez-vous que les widgets ont une taille minimale :

```cpp
if (ui->le_stock_code) {
    ui->le_stock_code->setMinimumHeight(40);
    ui->le_stock_code->setMinimumWidth(200);
}
```

## 📊 Structure Attendue du Formulaire

Le formulaire devrait afficher :

```
┌─────────────────────────────────────────────┐
│  ➕ NOUVELLE MATIÈRE PREMIÈRE               │
├─────────────────────────────────────────────┤
│                                             │
│  Code MP :         [CUIR-2024-001]         │
│  Catégorie :       [Cuir ▼]                │
│  Num. Lot :        [LOT-2024-A]            │
│  État :            [BRUT ▼]                │
│  Couleur :         [Marron]                │
│  Quantité :        [1.00]                  │
│  Type Stockage :   [Sec ▼]                 │
│  Qualité :         [A ▼]                   │
│                                             │
│         [✅ Valider l'ajout]               │
│                                             │
└─────────────────────────────────────────────┘
```

## 🎯 Vérification Après Correction

Après recompilation, vérifiez que :

1. ✅ Le titre "NOUVELLE MATIÈRE PREMIÈRE" est visible
2. ✅ Les 8 champs de saisie sont visibles :
   - Code MP (champ texte)
   - Catégorie (liste déroulante)
   - Numéro de Lot (champ texte)
   - État (liste déroulante)
   - Couleur (champ texte)
   - Quantité (spinbox)
   - Type Stockage (liste déroulante)
   - Qualité (liste déroulante)
3. ✅ Le bouton "✅ Valider l'ajout" est visible
4. ✅ Les labels à gauche de chaque champ sont visibles

## 🐛 Diagnostic Avancé

Si le problème persiste après recompilation, ajoutez ce code de debug dans le constructeur :

```cpp
qDebug() << "=== DEBUG FORMULAIRE STOCK ===";
qDebug() << "tab_stock_ajouter visible:" << (ui->tab_stock_ajouter ? ui->tab_stock_ajouter->isVisible() : false);
qDebug() << "le_stock_code visible:" << (ui->le_stock_code ? ui->le_stock_code->isVisible() : false);
qDebug() << "cb_stock_cat visible:" << (ui->cb_stock_cat ? ui->cb_stock_cat->isVisible() : false);
qDebug() << "btn_valider_stock visible:" << (ui->btn_valider_stock ? ui->btn_valider_stock->isVisible() : false);
qDebug() << "==============================";
```

Lancez l'application et vérifiez la sortie dans la console Qt Creator.

## 📝 Notes Importantes

1. **Recompilation Complète** : Après modification du code ou du fichier `.ui`, faites toujours une recompilation complète (Clean + Rebuild)
2. **Cache Qt** : Parfois, Qt Creator garde un cache. Fermez Qt Creator, supprimez le dossier `build`, puis rouvrez et recompilez
3. **Fichier .ui** : Ne modifiez jamais le fichier `.ui` manuellement en XML, utilisez toujours Qt Designer

## 🎉 Résultat Attendu

Après correction et recompilation, le formulaire "Ajouter Matière" devrait afficher tous les champs de saisie et vous pourrez :
- Saisir les informations de la matière première
- Valider l'ajout
- Voir la matière ajoutée dans la liste du stock
