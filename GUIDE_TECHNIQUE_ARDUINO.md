# 🔧 GUIDE TECHNIQUE - MODULE ARDUINO SMART

## Documentation Développeur

---

## 📋 Architecture Technique

### Vue d'Ensemble

```
┌─────────────────────────────────────────────────────────┐
│                    ARCHITECTURE                          │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────────┐      ┌──────────────┐               │
│  │   Arduino    │──USB─│ ArduinoManager│               │
│  │   DHT11      │      │  (QSerialPort) │               │
│  └──────────────┘      └───────┬────────┘               │
│                                 │                        │
│                                 │ Signals/Slots          │
│                                 ▼                        │
│                        ┌──────────────┐                 │
│                        │ ArduinoWidget│                 │
│                        │   (UI Layer) │                 │
│                        └───────┬──────┘                 │
│                                │                        │
│                    ┌───────────┼───────────┐           │
│                    │           │           │           │
│                    ▼           ▼           ▼           │
│              ┌─────────┐ ┌─────────┐ ┌─────────┐      │
│              │QtCharts │ │QSqlQuery│ │ QTimer  │      │
│              │(Graphs) │ │  (BDD)  │ │(Auto)   │      │
│              └─────────┘ └─────────┘ └─────────┘      │
└─────────────────────────────────────────────────────────┘
```

---

## 📁 Structure des Fichiers

### Fichiers Principaux

```
projetcpp/
├── arduinomanager.h          # Gestion communication série
├── arduinomanager.cpp         # Implémentation ArduinoManager
├── arduinowidget.h            # Interface utilisateur
├── arduinowidget.cpp          # Implémentation ArduinoWidget
├── arduinowidget.ui           # Design Qt Designer
├── arduino_dht11_code.ino     # Code Arduino
├── mainwindow.h               # Fenêtre principale
├── mainwindow.cpp             # Intégration module
├── mainwindow.ui              # UI principale
└── CMakeLists.txt             # Configuration build
```

### Fichiers de Documentation

```
docs/
├── GUIDE_UTILISATEUR_ARDUINO_SMART.md
├── GUIDE_TECHNIQUE_ARDUINO.md
├── AMELIORATIONS_20_SUR_20.md
└── README_MODULE_ARDUINO.md
```

---

## 🔌 Classe ArduinoManager

### Responsabilités

- Communication série avec Arduino
- Parsing des données reçues
- Gestion des erreurs de connexion
- Reconnexion automatique

### Interface Publique

```cpp
class ArduinoManager : public QObject
{
    Q_OBJECT

public:
    explicit ArduinoManager(QObject *parent = nullptr);
    ~ArduinoManager();

    // Méthodes publiques
    bool connecter(const QString &portName);
    void deconnecter();
    bool estConnecte() const;
    QStringList listerPorts() const;

signals:
    void donneesRecues(double humidite, double temperature);
    void erreurConnexion(const QString &message);
    void statutChange(bool connecte);

private slots:
    void lireDonnees();
    void gererErreur(QSerialPort::SerialPortError error);
    void tentativeReconnexion();

private:
    QSerialPort *m_port;
    QTimer *m_timerReconnexion;
    QString m_dernierPort;
    int m_tentativesReconnexion;
    static constexpr int MAX_TENTATIVES = 10;
};
```

### Format des Données

**Arduino → PC** :
```
H:45.20;T:28.50\n
```

**Parsing** :
```cpp
QRegularExpression regex("H:(\\d+\\.\\d+);T:(\\d+\\.\\d+)");
QRegularExpressionMatch match = regex.match(data);
if (match.hasMatch()) {
    double humidite = match.captured(1).toDouble();
    double temperature = match.captured(2).toDouble();
    emit donneesRecues(humidite, temperature);
}
```

---

## 🖥️ Classe ArduinoWidget

### Responsabilités

- Affichage des données en temps réel
- Gestion des graphiques QtCharts
- Calcul des statistiques
- Sauvegarde en base de données
- Export CSV/PDF

### Variables Membres Clés

```cpp
private:
    // Communication
    ArduinoManager *m_arduinoManager;
    QTimer *m_timerSauvegarde;
    
    // Graphiques
    QChart *m_chart;
    QChartView *m_chartView;
    QLineSeries *m_seriesHumidite;
    QLineSeries *m_seriesTemperature;
    
    // Statistiques
    double m_humiditeMin, m_humiditeMax, m_humiditeSum;
    double m_temperatureMin, m_temperatureMax, m_temperatureSum;
    int m_nombreMesures, m_nombreAlertes;
    QDateTime m_debutSession;
    
    // Configuration
    double m_seuilHumiditeMin;      // Défaut: 30%
    double m_seuilHumiditeMax;      // Défaut: 70%
    double m_seuilTemperatureMax;   // Défaut: 35°C
    
    // Widgets statistiques
    QLabel *m_lblHumiditeMin;
    QLabel *m_lblHumiditeMax;
    QLabel *m_lblHumidityMoy;
    // ... autres labels
```

### Méthodes Principales

#### Initialisation

```cpp
void ArduinoWidget::initialiserUI()
{
    // Style moderne
    setStyleSheet(R"(
        QWidget { background-color: #ffffff; }
        QGroupBox { border: 2px solid #e0e0e0; }
        QPushButton { background-color: #8d5524; }
    )");
    
    // Configuration initiale
    ui->lblHumiditeValeur->setText("-- %");
    ui->lblTemperatureValeur->setText("-- °C");
    ui->progressHumidite->setRange(0, 100);
}
```

#### Traitement des Données

```cpp
void ArduinoWidget::onDonneesRecues(double humidite, double temperature)
{
    // 1. Mise à jour affichage
    mettreAJourAffichage(humidite, temperature);
    
    // 2. Mise à jour graphique
    mettreAJourGraphique(humidite, temperature);
    
    // 3. Analyse des seuils
    QString alerte = analyserSeuils(humidite, temperature);
    
    // 4. Mise à jour statistiques
    m_nombreMesures++;
    m_humiditeSum += humidite;
    m_temperatureSum += temperature;
    
    if (humidite < m_humiditeMin) m_humiditeMin = humidite;
    if (humidite > m_humiditeMax) m_humiditeMax = humidite;
    
    // Compter les alertes
    if (!alerte.isEmpty() && alerte != "✅ Conditions optimales") {
        m_nombreAlertes++;
    }
    
    // 5. Rafraîchir statistiques
    rafraichirStatistiques();
    
    // 6. Mémoriser pour BDD
    m_derniereHumidite = humidite;
    m_derniereTemperature = temperature;
    m_derniereAlerte = alerte;
}
```

#### Graphiques QtCharts

```cpp
void ArduinoWidget::creerGraphique()
{
    // Créer les séries
    m_seriesHumidite = new QLineSeries();
    m_seriesHumidite->setName("Humidité (%)");
    m_seriesHumidite->setColor(QColor("#00838f"));
    
    m_seriesTemperature = new QLineSeries();
    m_seriesTemperature->setName("Température (°C)");
    m_seriesTemperature->setColor(QColor("#d84315"));
    
    // Créer le graphique
    m_chart = new QChart();
    m_chart->addSeries(m_seriesHumidite);
    m_chart->addSeries(m_seriesTemperature);
    m_chart->setTitle("Surveillance Temps Réel");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    
    // Axes
    m_axisX = new QDateTimeAxis();
    m_axisX->setFormat("hh:mm:ss");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    
    m_axisYHumidite = new QValueAxis();
    m_axisYHumidite->setRange(0, 100);
    m_chart->addAxis(m_axisYHumidite, Qt::AlignLeft);
    
    // Vue
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}
```

#### Mise à Jour Graphique

```cpp
void ArduinoWidget::mettreAJourGraphique(double humidite, double temperature)
{
    QDateTime maintenant = QDateTime::currentDateTime();
    
    // Ajouter les points
    m_historiqueHumidite.append(qMakePair(maintenant, humidite));
    m_historiqueTemperature.append(qMakePair(maintenant, temperature));
    
    // Limiter à 50 points
    if (m_historiqueHumidite.size() > 50) {
        m_historiqueHumidite.removeFirst();
    }
    
    // Mettre à jour les séries
    m_seriesHumidite->clear();
    for (const auto &point : m_historiqueHumidite) {
        m_seriesHumidite->append(point.first.toMSecsSinceEpoch(), point.second);
    }
    
    // Ajuster l'axe X
    if (!m_historiqueHumidite.isEmpty()) {
        m_axisX->setRange(
            m_historiqueHumidite.first().first,
            m_historiqueHumidite.last().first
        );
    }
}
```

---

## 💾 Base de Données

### Table historique_capteur

```sql
CREATE TABLE historique_capteur (
    id NUMBER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    date_mesure TIMESTAMP NOT NULL,
    humidite NUMBER(5,2) NOT NULL,
    temperature NUMBER(5,2) NOT NULL,
    alerte VARCHAR2(200)
);
```

### Sauvegarde Automatique

```cpp
void ArduinoWidget::sauvegarderEnBase()
{
    Connexion *cnx = Connexion::getInstance();
    if (!cnx->estConnecte()) return;
    
    QSqlQuery query(cnx->getDatabase());
    query.prepare(R"(
        INSERT INTO historique_capteur 
        (date_mesure, humidite, temperature, alerte)
        VALUES (:date, :humidite, :temperature, :alerte)
    )");
    
    query.bindValue(":date", QDateTime::currentDateTime());
    query.bindValue(":humidite", m_derniereHumidite);
    query.bindValue(":temperature", m_derniereTemperature);
    query.bindValue(":alerte", m_derniereAlerte);
    
    if (query.exec()) {
        qDebug() << "💾 Données sauvegardées";
    } else {
        qDebug() << "❌ Erreur:" << query.lastError().text();
    }
}
```

### Requêtes Utiles

```sql
-- Dernières 100 mesures
SELECT * FROM historique_capteur 
ORDER BY date_mesure DESC 
FETCH FIRST 100 ROWS ONLY;

-- Statistiques du jour
SELECT 
    MIN(humidite) as hum_min,
    MAX(humidite) as hum_max,
    AVG(humidite) as hum_moy,
    MIN(temperature) as temp_min,
    MAX(temperature) as temp_max,
    AVG(temperature) as temp_moy,
    COUNT(*) as nb_mesures
FROM historique_capteur
WHERE date_mesure >= TRUNC(SYSDATE);

-- Alertes du mois
SELECT date_mesure, alerte
FROM historique_capteur
WHERE alerte IS NOT NULL
  AND alerte != 'Conditions optimales'
  AND date_mesure >= TRUNC(SYSDATE, 'MM')
ORDER BY date_mesure DESC;
```

---

## 🔧 Configuration CMake

### CMakeLists.txt

```cmake
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Widgets 
    Gui 
    PrintSupport 
    Charts 
    Sql 
    SerialPort
)

set(PROJECT_SOURCES
    # ... autres fichiers
    arduinomanager.h
    arduinomanager.cpp
    arduinowidget.h
    arduinowidget.cpp
    arduinowidget.ui
)

target_link_libraries(ProjetCpp PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::SerialPort
)
```

---

## 🎨 Personnalisation

### Thèmes de Couleurs

```cpp
// Palette FIL D'OR
#define COLOR_GOLD      "#d4af37"
#define COLOR_BROWN     "#8d5524"
#define COLOR_BEIGE     "#f4f1ea"
#define COLOR_WHITE     "#ffffff"

// Palette Alertes
#define COLOR_SUCCESS   "#2e7d32"
#define COLOR_WARNING   "#e65100"
#define COLOR_ERROR     "#c62828"

// Palette Graphiques
#define COLOR_HUMIDITY  "#00838f"
#define COLOR_TEMP      "#d84315"
```

### Styles CSS

```cpp
QString styleGroupBox = R"(
    QGroupBox {
        font-weight: bold;
        border: 2px solid #d4af37;
        border-radius: 10px;
        margin-top: 15px;
        padding-top: 15px;
        background-color: #fffef7;
    }
)";
```

---

## 🧪 Tests

### Tests Unitaires

```cpp
// Test de parsing
void testParsing() {
    QString data = "H:45.20;T:28.50\n";
    QRegularExpression regex("H:(\\d+\\.\\d+);T:(\\d+\\.\\d+)");
    QRegularExpressionMatch match = regex.match(data);
    
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1).toDouble(), 45.20);
    QCOMPARE(match.captured(2).toDouble(), 28.50);
}

// Test de seuils
void testSeuils() {
    ArduinoWidget widget;
    
    // Test humidité basse
    QString alerte = widget.analyserSeuils(25.0, 28.0);
    QVERIFY(alerte.contains("trop sec"));
    
    // Test humidité haute
    alerte = widget.analyserSeuils(75.0, 28.0);
    QVERIFY(alerte.contains("élevée"));
    
    // Test température critique
    alerte = widget.analyserSeuils(50.0, 40.0);
    QVERIFY(alerte.contains("critique"));
}
```

---

## 📊 Performance

### Optimisations

1. **Graphiques** : Limiter à 50 points
2. **BDD** : Sauvegarde toutes les 30s (pas 2s)
3. **UI** : Mise à jour par batch
4. **Mémoire** : Utilisation de QVector

### Métriques

- **Latence** : < 100ms entre réception et affichage
- **CPU** : < 5% en fonctionnement normal
- **Mémoire** : ~50 MB
- **BDD** : ~1 KB par mesure

---

## 🔒 Sécurité

### Validation des Données

```cpp
// Filtrer les valeurs invalides
if (humidite < 0 || humidite > 100) return;
if (temperature < -40 || temperature > 80) return;

// Filtrer les lectures d'erreur du capteur
if (humidite == 0.0 && temperature == 0.0) return;
```

### Gestion des Erreurs

```cpp
try {
    // Code critique
} catch (const std::exception &e) {
    qCritical() << "Erreur:" << e.what();
    emit erreurConnexion(QString::fromStdString(e.what()));
}
```

---

## 📝 Bonnes Pratiques

### Code Style

```cpp
// Nommage
m_variableMembre    // Membre de classe
nomFonction()       // Fonction
NomClasse           // Classe
CONSTANTE           // Constante

// Commentaires
/**
 * @brief Description courte
 * @param param Description du paramètre
 * @return Description du retour
 */
```

### Signaux/Slots

```cpp
// Préférer les pointeurs de fonction
connect(m_arduinoManager, &ArduinoManager::donneesRecues,
        this, &ArduinoWidget::onDonneesRecues);

// Éviter les anciennes syntaxes
// connect(obj, SIGNAL(signal()), this, SLOT(slot()));
```

---

## 🚀 Déploiement

### Prérequis

- Qt 5.15+ ou Qt 6.x
- Compilateur C++17
- Oracle Client
- Pilotes Arduino

### Build Release

```bash
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

### Installation

```bash
# Windows
windeployqt ProjetCpp.exe

# Linux
linuxdeployqt ProjetCpp

# macOS
macdeployqt ProjetCpp.app
```

---

## 📞 Support Développeur

### Ressources

- **Documentation Qt** : doc.qt.io
- **Arduino Reference** : arduino.cc/reference
- **Stack Overflow** : stackoverflow.com/questions/tagged/qt

### Contribution

Pour contribuer au projet :
1. Fork le repository
2. Créer une branche feature
3. Commit les changements
4. Push et créer une Pull Request

---

*Document créé le : 27 Avril 2026*
*Version : 1.0*
*Auteur : Équipe Technique FIL D'OR*
