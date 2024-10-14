#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PS2Keyboard.h>
#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 60  // Nombre de LEDs dans le ruban
#define LED_PIN 1    // Broche de connexion du ruban

//#define NUM_LEDS_couloir 1   // Nombre de LEDs dans le ruban sur la broche 25
//#define LED_PIN_couloir 25   // Broche de connexion du ruban sur la broche 25

const int led = 3;
const int chipSelect = 5;
const int ps2DataPin = 14;
const int ps2ClockPin = 12;


PS2Keyboard keyboard;
LiquidCrystal_I2C lcd(0x27, 20, 4); // Utiliser l'adresse 0x27 pour l'écran LCD 20x4
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip_couloir(NUM_LEDS_couloir, LED_PIN_couloir, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  keyboard.begin(ps2DataPin, ps2ClockPin);
  pinMode(led, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(4, 1);
  lcd.print("Bienvenue");

  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.print("Erreur SD ");
    while (1);
  }

  lcd.clear();
  lcd.print("SD initialisee");
  delay(1000);
  lcd.clear();

  // Initialiser le ruban de LEDs
  strip.begin();
  strip.show(); // Éteindre toutes les LEDs au démarrage
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("Veuillez choisir :");
  lcd.setCursor(0, 1);
  lcd.print("1 : Chercher Ref");
  lcd.setCursor(0, 2);
  lcd.print("2 : Ajouter Ref");
  lcd.setCursor(0, 3);
  lcd.print("3 : Supprimer Ref");

  while (!keyboard.available()) {
    // Attendre que l'utilisateur choisisse une option
  }

  char c = keyboard.read();

  switch (c) {
    case 'a': c = 'q'; break;
    case 'q': c = 'a'; break;
    case 'z': c = 'w'; break;
    case 'w': c = 'z'; break;
    case ';': c = 'm'; break;
    case 'm': c = ';'; break;
  }

  if (c >= 'a' && c <= 'z') {
    c = c - 'a' + 'A';
  }

  if (c == '1') {
    chercherReference();
  } else if (c == '2') {
    ajouterReference();
  } else if (c == '3') {
    supprimerReference();
  } else {
    lcd.clear();
    lcd.print("Option invalide ");
    delay(2000);
    lcd.clear();
  }
}

void chercherReference() {
  lcd.clear();
  lcd.print("Entrer une Ref :    ");
  lcd.setCursor(0, 1);
  while (!keyboard.available()) {
    // Attendre que l'utilisateur entre une reference
  }

  String reference = readKeyboardString();

  // Ouvrir le fichier en mode lecture
  File file = SD.open("/Data.txt", FILE_READ);
  if (file) {
    int lineNumber = 0; // Variable pour suivre le numero de ligne
    // Lire le contenu du fichier ligne par ligne
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();

      // Diviser la ligne en valeurs séparées par des virgules
      int pos = line.indexOf(',');
      if (pos != -1) {
        String currentReference = line.substring(0, pos);

        // Vérifier si la référence entrée par l'utilisateur existe dans le fichier
        if (currentReference.equals(reference)) {
          int lastPos = pos + 1;
          pos = line.indexOf(',', lastPos);
          String description = line.substring(lastPos, pos);
          lastPos = pos + 1;
          pos = line.indexOf(',', lastPos);
          String Couloir = line.substring(lastPos, pos);
          lcd.clear();
          lcd.print("Ref trouvee ");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Couloir: ");
          lcd.print(Couloir);
          lcd.setCursor(0, 1);
          lcd.print("Description:");
          // Afficher la description avec défilement horizontal
          int textLength = description.length();
          int lcdWidth = 20; // Nombre de colonnes de l'écran LCD (20 pour un écran 20x4)
          for (int i = 0; i <= textLength - lcdWidth; i++) {
          lcd.setCursor(0, 2);
          lcd.print(description.substring(i, i + lcdWidth));
          delay(400); // Délai pour le défilement (ajustez selon votre préférence)
          }
          delay(4000);
          int CouloirValue = line.substring(lastPos).toInt(); // Lire la valeur de la troisième colonne et la convertir en entier
          if (CouloirValue == 2) {
//          clignotercouloir(1, 0, 255, 0, 5); 
            digitalWrite(led, HIGH); 
            }
          delay(4000);
          digitalWrite(led, LOW); 
          delay(4000);
          // Référence trouvée, clignoter la LED correspondante
          clignoterLed(lineNumber - 1);
          lcd.clear();
          file.close();
          return;
        }
      }

      lineNumber++;
    }
    file.close();

    // Si la référence n'a pas été trouvée dans le fichier
    lcd.clear();
    lcd.print("Ref non trouvee ");
    delay(2000);
    lcd.clear();
  } else {
    lcd.clear();
    lcd.print("Erreur ouverture fichier ");
    delay(2000);
    lcd.clear();
  }
}

void ajouterReference() {
  lcd.clear();
  lcd.print("Entrer une nouv ref:");
  lcd.setCursor(0, 1);
  while (!keyboard.available()) {
    // Attendre que l'utilisateur entre une nouvelle référence
  }

  String newReference = readKeyboardString();

  // Vérifier si la nouvelle référence existe déjà dans le fichier
  File file = SD.open("/Data.txt", FILE_READ);
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      int pos = line.indexOf(',');
      if (pos != -1) {
        String currentReference = line.substring(0, pos);
        if (currentReference.equals(newReference)) {
          lcd.clear();
          lcd.print("Ref deja existante ");
          delay(2000);
          lcd.clear();
          file.close();
          return;
        }
      }
    }
    file.close();
  } else {
    lcd.clear();
    lcd.print("Erreur ouverture fichier ");
    delay(2000);
    lcd.clear();
    return;
  }

  // Demander la description pour la nouvelle référence
  String newDescription;
  lcd.setCursor(0, 2);
  lcd.print("Entrer la descr:");
  lcd.setCursor(0, 3);
  while (!keyboard.available()) {
    // Attendre que l'utilisateur entre la description
  }
  newDescription = readKeyboardString();

    // Demander la Couloir pour la nouvelle référence
  String newCouloir;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Entrer num Couloir :");
  lcd.setCursor(0, 1);
  while (!keyboard.available()) {
    // Attendre que l'utilisateur entre la description
  }
  newCouloir = readKeyboardString();

  // Ajouter la nouvelle référence au fichier
  file = SD.open("/Data.txt", FILE_APPEND);
  if (file) {
    file.print(newReference);
    file.print(",");
    file.println(newDescription);
    file.print(",");
    file.println(newCouloir);
    file.close();

    lcd.clear();
    lcd.print("Nouvelle ref ajoutee ");
    delay(2000);
    lcd.clear();
  } else {
    lcd.clear();
    lcd.print("Erreur ajout fichier ");
    delay(2000);
    lcd.clear();
  }
}

void supprimerReference() {
  lcd.clear();
  lcd.print("Entrer ref a supp:  ");
  lcd.setCursor(0, 1);
  while (!keyboard.available()) {
    // Attendre que l'utilisateur entre la référence à supprimer
  }

  String referenceToDelete = readKeyboardString();

  // Créer un fichier temporaire pour stocker les lignes qui ne contiennent pas la référence à supprimer
  File tempFile = SD.open("/temp.txt", FILE_WRITE);

  if (tempFile) {
    // Ouvrir le fichier en mode lecture
    File file = SD.open("/Data.txt", FILE_READ);
    if (file) {
      // Lire le contenu du fichier ligne par ligne
      boolean found = false; // Variable pour indiquer si la référence a été trouvée
      while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();

        // Diviser la ligne en valeurs séparées par des virgules
        int pos = line.indexOf(',');
        if (pos != -1) {
          String currentReference = line.substring(0, pos);

          // Vérifier si la référence de la ligne correspond à celle à supprimer
          if (!currentReference.equals(referenceToDelete)) {
            // Écrire la ligne dans le fichier temporaire
            tempFile.println(line);
          } else {
            found = true;
          }
        }
      }
      file.close();
      tempFile.close();

      if (found) {
        // Supprimer le fichier original
        SD.remove("/Data.txt");

        // Renommer le fichier temporaire en "Data.txt"
        SD.rename("/temp.txt", "/Data.txt");

        lcd.clear();
        lcd.print("Ref supprimee ");
        delay(2000);
        lcd.clear();
      } else {
        lcd.clear();
        lcd.print("Ref non trouvee ");
        delay(2000);
        lcd.clear();
      }
    } else {
      lcd.clear();
      lcd.print("Erreur ouverture fichier ");
      delay(2000);
      lcd.clear();
      tempFile.close();
      return;
    }
  } else {
    lcd.clear();
    lcd.print("Erreur creation fichier ");
    delay(2000);
    lcd.clear();
  }
}




//void clignotercouloir(int index, byte r, byte g, byte b, int repetitions) {
//  for (int i = 0; i < repetitions; i++) {
//    strip_couloir.setPixelColor(index, strip.Color(r, g, b));
//    strip_couloir.show();
//    delay(500);   // Attendre 0.5 seconde
//    strip_couloir.setPixelColor(index, strip.Color(0, 0, 0));
//    strip_couloir.show();
//    delay(500);   // Attendre 0.5 seconde
//  }
//}


void clignoterLed(int ledIndex) {
  if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
    for (int i = 0; i < 5; i++) { // Clignoter la LED 5 fois
      strip.setPixelColor(ledIndex, strip.Color(255, 0, 0)); // Allumer la LED en rouge
      strip.show();
      delay(500);
      strip.setPixelColor(ledIndex, strip.Color(0, 0, 0)); // Éteindre la LED
      strip.show();
      delay(500);
    }
  }
}




// Fonction pour lire une chaîne de caractères depuis le clavier PS2
String readKeyboardString() {
  String result = "";
  while (true) {
    if (keyboard.available()) {
      char c = keyboard.read();

      if (c == PS2_ENTER) {
        break; // Fin de la saisie
      } else if (c == PS2_DELETE) {
        // Effacer le dernier caractère s'il y en a un
        if (result.length() > 0) {
          result = result.substring(0, result.length() - 1);
          lcd.setCursor(result.length(), 1);
          lcd.print(" ");
          lcd.setCursor(result.length(), 1);
        }
      } else {
        // Ajouter le caractère à la chaîne de résultat
        result += c;
        lcd.print(c);
      }
    }
  }
  return result;
}
