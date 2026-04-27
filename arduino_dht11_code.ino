/*
 * ========================================
 * FIL D'OR - Module Arduino Smart
 * Surveillance des Conditions de Stockage
 * ========================================
 * 
 * Capteur : DHT11 (Température + Humidité)
 * Microcontrôleur : Arduino Uno
 * Communication : Port Série (9600 bauds)
 * 
 * Format de sortie : H:xx.xx;T:yy.yy
 * Exemple : H:45.50;T:22.30
 * 
 * Auteur : Équipe FIL D'OR
 * Date : 26 avril 2026
 * Version : 1.0.0
 */

// ========== BIBLIOTHÈQUES ==========
#include <DHT.h>

// ========== CONFIGURATION ==========
#define DHTPIN 3          // Pin de données du DHT11 (Digital Pin 3)
#define DHTTYPE DHT11     // Type de capteur (DHT11 ou DHT22)

// Intervalle de lecture (millisecondes)
#define INTERVALLE_LECTURE 2000  // 2 secondes

// ========== INITIALISATION ==========
DHT dht(DHTPIN, DHTTYPE);

// Variables globales
unsigned long derniereLecture = 0;
int nombreErreurs = 0;
const int MAX_ERREURS_CONSECUTIVES = 5;

// ========== SETUP (Exécuté une seule fois au démarrage) ==========
void setup() {
  // Initialisation de la communication série
  Serial.begin(9600);
  
  // Attendre que le port série soit prêt
  while (!Serial) {
    ; // Attente (nécessaire pour certains boards)
  }
  
  // Initialisation du capteur DHT11
  dht.begin();
  
  // Message de démarrage
  Serial.println("========================================");
  Serial.println("FIL D'OR - Module Arduino Smart");
  Serial.println("Capteur DHT11 - Surveillance Stockage");
  Serial.println("========================================");
  Serial.println("Initialisation...");
  
  // Attendre 2 secondes pour stabiliser le capteur
  delay(2000);
  
  Serial.println("Capteur pret !");
  Serial.println("Format: H:xx.xx;T:yy.yy");
  Serial.println("========================================");
}

// ========== LOOP (Exécuté en boucle infinie) ==========
void loop() {
  // Vérifier si l'intervalle de lecture est écoulé
  unsigned long maintenant = millis();
  
  if (maintenant - derniereLecture >= INTERVALLE_LECTURE) {
    derniereLecture = maintenant;
    
    // Lire les données du capteur
    lireCapteur();
  }
}

// ========== FONCTION DE LECTURE DU CAPTEUR ==========
void lireCapteur() {
  // Lecture de l'humidité (%)
  float humidite = dht.readHumidity();
  
  // Lecture de la température (°C)
  float temperature = dht.readTemperature();
  
  // Vérifier si les lectures sont valides
  if (isnan(humidite) || isnan(temperature)) {
    // Erreur de lecture
    nombreErreurs++;
    
    Serial.print("ERREUR de lecture du capteur ! (");
    Serial.print(nombreErreurs);
    Serial.println(" erreurs consecutives)");
    
    // Si trop d'erreurs consécutives, signaler un problème matériel
    if (nombreErreurs >= MAX_ERREURS_CONSECUTIVES) {
      Serial.println("ALERTE: Verifiez le cablage du capteur DHT11 !");
      Serial.println("  - VCC -> 5V");
      Serial.println("  - GND -> GND");
      Serial.println("  - DATA -> Pin 3");
    }
    
    return;
  }
  
  // Réinitialiser le compteur d'erreurs si lecture réussie
  nombreErreurs = 0;
  
  // Vérifier les plages de valeurs réalistes
  if (!valeurValide(humidite, temperature)) {
    Serial.println("ERREUR: Valeurs hors plage realiste !");
    return;
  }
  
  // Envoyer les données au format attendu par l'application Qt
  // Format : H:xx.xx;T:yy.yy
  Serial.print("H:");
  Serial.print(humidite, 2);  // 2 décimales
  Serial.print(";T:");
  Serial.println(temperature, 2);  // 2 décimales
  
  // Optionnel : Afficher aussi en format lisible (commentez si non nécessaire)
  // afficherDonneesLisibles(humidite, temperature);
}

// ========== VALIDATION DES VALEURS ==========
bool valeurValide(float humidite, float temperature) {
  // Plages réalistes pour DHT11
  // Humidité : 20-90% (plage typique)
  // Température : -40 à 80°C (plage du capteur)
  
  if (humidite < 0.0 || humidite > 100.0) {
    Serial.print("Humidite invalide: ");
    Serial.println(humidite);
    return false;
  }
  
  if (temperature < -40.0 || temperature > 80.0) {
    Serial.print("Temperature invalide: ");
    Serial.println(temperature);
    return false;
  }
  
  return true;
}

// ========== AFFICHAGE LISIBLE (OPTIONNEL) ==========
void afficherDonneesLisibles(float humidite, float temperature) {
  Serial.println("----------------------------------------");
  Serial.print("Humidite    : ");
  Serial.print(humidite, 1);
  Serial.println(" %");
  
  Serial.print("Temperature : ");
  Serial.print(temperature, 1);
  Serial.println(" °C");
  
  // Analyse des conditions
  if (humidite < 30) {
    Serial.println("⚠️  ALERTE: Air trop sec !");
  } else if (humidite > 70) {
    Serial.println("❌ ALERTE: Humidite elevee !");
  } else {
    Serial.println("✅ Conditions optimales");
  }
  
  if (temperature > 35) {
    Serial.println("🔥 ALERTE: Temperature critique !");
  }
  
  Serial.println("----------------------------------------");
}

// ========== NOTES TECHNIQUES ==========
/*
 * CÂBLAGE DHT11 :
 * ---------------
 * DHT11 Pin 1 (VCC)  -> Arduino 5V
 * DHT11 Pin 2 (DATA) -> Arduino Pin 3
 * DHT11 Pin 3 (NC)   -> Non connecté
 * DHT11 Pin 4 (GND)  -> Arduino GND
 * 
 * Note : Une résistance pull-up de 10kΩ entre VCC et DATA
 *        peut améliorer la stabilité (optionnelle).
 * 
 * INSTALLATION BIBLIOTHÈQUE DHT :
 * -------------------------------
 * 1. Ouvrir l'IDE Arduino
 * 2. Aller dans : Croquis > Inclure une bibliothèque > Gérer les bibliothèques
 * 3. Rechercher "DHT sensor library" par Adafruit
 * 4. Installer "DHT sensor library" ET "Adafruit Unified Sensor"
 * 
 * TÉLÉVERSEMENT :
 * ---------------
 * 1. Brancher l'Arduino Uno via USB
 * 2. Sélectionner : Outils > Type de carte > Arduino Uno
 * 3. Sélectionner : Outils > Port > COMx (Windows) ou /dev/ttyUSBx (Linux)
 * 4. Cliquer sur le bouton "Téléverser" (flèche droite)
 * 
 * TEST :
 * ------
 * 1. Ouvrir le Moniteur Série : Outils > Moniteur série
 * 2. Régler la vitesse à 9600 bauds
 * 3. Vérifier que les données s'affichent au format : H:xx.xx;T:yy.yy
 * 
 * DÉPANNAGE :
 * -----------
 * - "ERREUR de lecture" : Vérifier le câblage
 * - Valeurs à 0.00 : Capteur défectueux ou mal alimenté
 * - Pas de données : Vérifier le port série et la vitesse (9600 bauds)
 * - Valeurs erratiques : Ajouter une résistance pull-up 10kΩ
 * 
 * OPTIMISATIONS POSSIBLES :
 * -------------------------
 * - Ajouter un filtre de moyenne mobile pour lisser les valeurs
 * - Implémenter un mode veille pour économiser l'énergie
 * - Ajouter un LED pour indiquer l'état (vert=OK, rouge=erreur)
 * - Stocker les données sur carte SD en cas de perte de connexion
 * - Ajouter un capteur de luminosité (LDR) pour détecter l'ouverture du stock
 */
