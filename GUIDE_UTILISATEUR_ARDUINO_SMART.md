# 📘 GUIDE UTILISATEUR - MODULE ARDUINO SMART

## 🎯 FIL D'OR - Surveillance des Conditions de Stockage

---

## 📋 Table des Matières

1. [Introduction](#introduction)
2. [Démarrage Rapide](#démarrage-rapide)
3. [Interface Utilisateur](#interface-utilisateur)
4. [Fonctionnalités Détaillées](#fonctionnalités-détaillées)
5. [Interprétation des Données](#interprétation-des-données)
6. [Gestion des Alertes](#gestion-des-alertes)
7. [Configuration Avancée](#configuration-avancée)
8. [Export et Rapports](#export-et-rapports)
9. [Dépannage](#dépannage)
10. [Bonnes Pratiques](#bonnes-pratiques)

---

## 🌟 Introduction

### Qu'est-ce que le Module Arduino Smart ?

Le **Module Arduino Smart** est un système de surveillance en temps réel des conditions de stockage pour les matières premières en cuir. Il utilise un capteur DHT11 connecté à un Arduino Uno pour mesurer :

- 💧 **Humidité** : Taux d'humidité relative (0-100%)
- 🌡️ **Température** : Température ambiante (°C)

### Pourquoi c'est important ?

Le cuir est une matière sensible qui nécessite des conditions de stockage optimales :

| Condition | Impact sur le Cuir |
|-----------|-------------------|
| **Trop sec** (< 30%) | Dessèchement, craquelures, perte de souplesse |
| **Trop humide** (> 70%) | Moisissures, décoloration, détérioration |
| **Trop chaud** (> 35°C) | Déformation, perte de qualité, odeurs |

✅ **Conditions optimales** : Humidité 30-70%, Température < 35°C

---

## 🚀 Démarrage Rapide

### Étape 1 : Accéder au Module

1. Lancez l'application **FIL D'OR**
2. Connectez-vous (admin/admin/1234)
3. Cliquez sur **"Matières Premières"** dans le menu gauche
4. Cliquez sur l'onglet **"🔌 Arduino Smart"**

### Étape 2 : Connexion Arduino

1. **Vérifiez** que l'Arduino est branché sur le port USB
2. **Sélectionnez** le port COM (généralement COM4 ou COM5)
3. **Cliquez** sur le bouton vert **"Connecter"**
4. **Attendez** 2 secondes : les données apparaissent !

> 💡 **Astuce** : Le système tente automatiquement de se connecter à COM5 au démarrage.

### Étape 3 : Surveillance

Une fois connecté, vous verrez :
- ✅ Statut : **🟢 Connecté**
- ✅ Humidité et température en temps réel
- ✅ Graphique qui se remplit progressivement
- ✅ Alertes si conditions anormales

---

## 🖥️ Interface Utilisateur

### Vue d'Ensemble

```
┌─────────────────────────────────────────────────────────┐
│  🔌 MODULE ARDUINO SMART - Surveillance Temps Réel      │
├──────────┬──────────────────────────────────────────────┤
│          │  Connexion Arduino                           │
│ ARDUINO  │  ┌─────────────────────────────────────┐    │
│  SMART   │  │ Port série: [COM5 ▼] [Rafraîchir]  │    │
│          │  │ [Connecter] 🟢 Connecté             │    │
│ ┌──────┐ │  └─────────────────────────────────────┘    │
│ │📊 TB │ │                                              │
│ └──────┘ │  Données en Temps Réel                      │
│ ┌──────┐ │  ┌──────────────┬──────────────┐           │
│ │🔌 Con│ │  │ 💧 HUMIDITÉ  │ 🌡️ TEMPÉRATURE│           │
│ └──────┘ │  │   45.2 %     │   28.5 °C    │           │
│ ┌──────┐ │  └──────────────┴──────────────┘           │
│ │📈 Gra│ │  [████████░░░░░░░░░░] 45%                  │
│ └──────┘ │                                              │
│ ┌──────┐ │  ✅ Conditions optimales pour le stockage   │
│ │📜 His│ │                                              │
│ └──────┘ │  Graphique Temps Réel (50 dernières mesures)│
│ ┌──────┐ │  [Graphique avec courbes]                   │
│ │⚠️ Ale│ │                                              │
│ └──────┘ │  📊 Statistiques de Session                 │
│          │  [Tableau avec Min/Max/Moy]                  │
│          │  [Exporter Historique CSV]                   │
└──────────┴──────────────────────────────────────────────┘
```

### Barre de Navigation (Gauche)

| Bouton | Fonction |
|--------|----------|
| **📊 Tableau de Bord** | Affiche les statistiques de session |
| **🔌 Connexion** | Guide de connexion Arduino |
| **📈 Graphiques** | Explications sur les graphiques |
| **📜 Historique** | Accès à l'historique et exports |
| **⚠️ Alertes** | Configuration des seuils d'alerte |

---

## 🔧 Fonctionnalités Détaillées

### 1. 📊 Tableau de Bord

**Accès** : Cliquez sur "📊 Tableau de Bord"

**Contenu** :
- **Humidité** : Min, Max, Moyenne
- **Température** : Min, Max, Moyenne
- **Session** : Nombre de mesures, Alertes, Durée

**Utilité** :
- Suivi de la qualité des conditions
- Identification des tendances
- Aide à la décision pour maintenance

**Exemple** :
```
💧 HUMIDITÉ
Min: 28.5%  |  Moy: 45.2%  |  Max: 62.8%

🌡️ TEMPÉRATURE
Min: 22.1°C  |  Moy: 28.5°C  |  Max: 34.2°C

📈 SESSION
Mesures: 150  |  Alertes: 3  |  Durée: 00:05:00
```

### 2. 🔌 Connexion Arduino

**Ports Série** :
- **COM1-COM9** : Ports USB standard
- **Rafraîchir** : Actualise la liste des ports

**États de Connexion** :
- 🟢 **Connecté** : Données en temps réel
- ⚫ **Déconnecté** : En attente
- 🔴 **Erreur** : Problème de connexion

**Reconnexion Automatique** :
- Tentatives : 10 fois
- Intervalle : 5 secondes
- Message : Affiché dans les alertes

### 3. 📈 Graphiques Temps Réel

**Caractéristiques** :
- **Historique** : 50 dernières mesures (~100 secondes)
- **Courbe bleue** : Humidité (%)
- **Courbe rouge** : Température (°C)
- **Mise à jour** : Toutes les 2 secondes

**Interprétation** :
- **Ligne horizontale** : Conditions stables ✅
- **Pics** : Variations soudaines ⚠️
- **Tendance montante** : Augmentation progressive 📈
- **Tendance descendante** : Diminution progressive 📉

### 4. 📜 Historique

**Sauvegarde Automatique** :
- **Fréquence** : Toutes les 30 secondes
- **Base de données** : Oracle (table `historique_capteur`)
- **Données** : Date, Humidité, Température, Alerte

**Structure de la Table** :
```sql
CREATE TABLE historique_capteur (
    id NUMBER PRIMARY KEY,
    date_mesure TIMESTAMP,
    humidite NUMBER(5,2),
    temperature NUMBER(5,2),
    alerte VARCHAR2(200)
);
```

### 5. ⚠️ Alertes

**Seuils par Défaut** :
- Humidité Min : **30%**
- Humidité Max : **70%**
- Température Max : **35°C**

**Types d'Alertes** :
| Alerte | Condition | Couleur | Action |
|--------|-----------|---------|--------|
| 🟡 Air trop sec | H < 30% | Orange | Humidifier |
| 🔴 Humidité élevée | H > 70% | Rouge | Déshumidifier |
| 🔥 Température critique | T > 35°C | Rouge | Refroidir |
| ✅ Conditions optimales | 30% ≤ H ≤ 70% et T ≤ 35°C | Vert | RAS |

---

## 📊 Interprétation des Données

### Humidité

#### Trop Sec (< 30%)
**Symptômes** :
- Cuir rigide et cassant
- Craquelures en surface
- Perte de souplesse

**Actions** :
1. Installer un humidificateur
2. Placer des récipients d'eau
3. Vérifier la ventilation

#### Optimal (30-70%)
**Caractéristiques** :
- Cuir souple et résistant
- Couleur stable
- Pas de moisissure

**Maintien** :
- Surveillance régulière
- Ventilation contrôlée
- Température stable

#### Trop Humide (> 70%)
**Symptômes** :
- Apparition de moisissures
- Décoloration
- Odeurs désagréables

**Actions** :
1. Installer un déshumidificateur
2. Améliorer la ventilation
3. Vérifier les fuites d'eau

### Température

#### Normale (< 35°C)
- Conditions idéales
- Pas d'action nécessaire

#### Critique (> 35°C)
**Risques** :
- Déformation du cuir
- Perte de qualité
- Accélération du vieillissement

**Actions** :
1. Activer la climatisation
2. Ouvrir les fenêtres (si extérieur plus frais)
3. Éloigner des sources de chaleur

---

## ⚙️ Configuration Avancée

### Modifier les Seuils d'Alerte

**Accès** : Cliquez sur "⚠️ Alertes" puis "Configuration"

**Paramètres Modifiables** :
1. **Humidité Min** : Seuil bas (défaut 30%)
2. **Humidité Max** : Seuil haut (défaut 70%)
3. **Température Max** : Seuil critique (défaut 35°C)

**Exemple de Configuration** :
```
Pour du cuir de vachette :
- Humidité Min : 35%
- Humidité Max : 65%
- Température Max : 30°C

Pour du cuir exotique :
- Humidité Min : 40%
- Humidité Max : 60%
- Température Max : 28°C
```

### Personnalisation par Type de Cuir

| Type de Cuir | Humidité Optimale | Température Max |
|--------------|-------------------|-----------------|
| Vachette | 30-70% | 35°C |
| Agneau | 35-65% | 30°C |
| Crocodile | 40-60% | 28°C |
| Daim | 35-60% | 32°C |

---

## 📄 Export et Rapports

### Export CSV

**Procédure** :
1. Cliquez sur **"Exporter Historique CSV"**
2. Choisissez l'emplacement
3. Nommez le fichier (ex: `historique_2026-04-27.csv`)
4. Cliquez sur **"Enregistrer"**

**Contenu du Fichier** :
```csv
Date et Heure,Humidité (%),Température (°C),Alerte
2026-04-27 14:30:00,45.20,28.50,Conditions optimales
2026-04-27 14:30:30,45.50,28.60,Conditions optimales
2026-04-27 14:31:00,72.10,29.00,Humidité élevée
```

**Utilisation** :
- Ouvrir avec Excel
- Créer des graphiques personnalisés
- Analyser les tendances
- Archiver les données

### Export PDF (Préparé)

**Contenu du Rapport** :
- En-tête avec logo FIL D'OR
- Période de surveillance
- Statistiques complètes
- Graphiques d'évolution
- Liste des alertes
- Recommandations

---

## 🔧 Dépannage

### Problème : Arduino ne se connecte pas

**Causes possibles** :
1. ❌ Arduino non branché
2. ❌ Mauvais port COM sélectionné
3. ❌ Pilote USB manquant
4. ❌ Port déjà utilisé par une autre application

**Solutions** :
1. ✅ Vérifier le branchement USB
2. ✅ Essayer un autre port COM
3. ✅ Installer les pilotes Arduino
4. ✅ Fermer l'IDE Arduino (Serial Monitor)

### Problème : Données incorrectes (0.00)

**Causes** :
- Capteur DHT11 défectueux
- Mauvaise connexion des fils
- Code Arduino non téléversé

**Solutions** :
1. Vérifier les connexions :
   - VCC → 5V
   - GND → GND
   - DATA → Pin 3
2. Téléverser le code Arduino
3. Tester le capteur avec l'IDE Arduino

### Problème : Graphique ne s'affiche pas

**Causes** :
- Pas de données reçues
- Problème d'affichage

**Solutions** :
1. Vérifier la connexion Arduino
2. Attendre 2-3 mesures
3. Faire défiler vers le bas (scrollbar)

---

## ✅ Bonnes Pratiques

### Surveillance Quotidienne

**Matin** (9h00) :
1. Vérifier la connexion Arduino
2. Consulter les statistiques de la nuit
3. Noter les alertes éventuelles

**Midi** (12h00) :
1. Vérifier les conditions actuelles
2. Ajuster si nécessaire (climatisation, humidificateur)

**Soir** (18h00) :
1. Exporter l'historique du jour
2. Analyser les tendances
3. Préparer les ajustements pour le lendemain

### Maintenance Hebdomadaire

**Lundi** :
- Nettoyer le capteur DHT11
- Vérifier les connexions
- Tester la reconnexion automatique

**Vendredi** :
- Exporter l'historique de la semaine
- Analyser les statistiques
- Générer un rapport PDF

### Maintenance Mensuelle

- Calibrer le capteur si nécessaire
- Vérifier l'état du câble USB
- Mettre à jour les seuils selon la saison
- Archiver les données

---

## 📞 Support

### Contacts

- **Support Technique** : support@fildor.tn
- **Urgences** : +216 XX XXX XXX
- **Documentation** : docs.fildor.tn/arduino

### Ressources

- 📘 Guide utilisateur (ce document)
- 🎥 Vidéos tutorielles
- 💬 Forum communautaire
- 📧 Newsletter mensuelle

---

## 📝 Notes de Version

### Version 1.0 (Avril 2026)
- ✅ Surveillance temps réel
- ✅ Graphiques interactifs
- ✅ Statistiques avancées
- ✅ Export CSV
- ✅ Configuration des seuils
- ✅ Sauvegarde automatique en BDD

### Prochaines Fonctionnalités
- 🔔 Notifications système
- 📊 Export PDF complet
- 📱 Application mobile
- 🌐 Accès web distant

---

## 🏆 Conclusion

Le **Module Arduino Smart** est un outil professionnel qui vous permet de :

✅ **Surveiller** en temps réel les conditions de stockage
✅ **Prévenir** les problèmes avant qu'ils n'affectent le stock
✅ **Optimiser** la qualité de conservation du cuir
✅ **Tracer** l'historique complet des conditions
✅ **Décider** sur la base de données objectives

**Résultat** : Réduction des pertes, amélioration de la qualité, satisfaction client ! 🎯

---

*Document créé le : 27 Avril 2026*
*Version : 1.0*
*Auteur : Équipe FIL D'OR*
*Dernière mise à jour : 27 Avril 2026*
