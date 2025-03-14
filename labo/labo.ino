#define BTN_PIN 2
#include <LCD_I2C.h>
#include <Wire.h>
LCD_I2C lcd(0x27, 16, 2); 

const int THERMISTOR_PIN = A0;
const int LED_PIN = 8;
const int JOYSTICK_X_PIN = A1;
const int JOYSTICK_Y_PIN = A2;
const int JOYSTICK_BTN_PIN = 2;

unsigned long temps_actuel = 0;
unsigned long previousMillis = 0;

byte caractere_personnalise_2[8] = {
  0b00000,
  0b01110,
  0b00100,
  0b01110,
  0b10000,
  0b01110,
  0b00000,
  0b00000
};

byte caractere_personnalise_6[8] = {
  0b00000,
  0b01110,
  0b10000,
  0b01110,
  0b00100,
  0b01110,
  0b00000,
  0b00000
};

int currentPage = 0; 

const float RESISTANCE_REFERENCE = 5100;
float logResistance = 0;
float resistance = 0;
float temperatureKelvin = 0;
float temperatureCelsius = 0;
float temperatureFahrenheit = 0;
const float C1 = 1.129148e-03;
const float C2 = 2.34125e-04;
const float C3 = 8.76741e-08;

int joystickX = 0, joystickY = 0;
int joystickXRaw, joystickYRaw;

void afficherDemarrage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nom: RIO"); 
  lcd.createChar(0, caractere_personnalise_2);
  lcd.createChar(1, caractere_personnalise_6);
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.write(byte(1));
  lcd.setCursor(14, 1);
  lcd.print("26");

  delay(3000);  
  lcd.clear();  
}

float mesurerTemperature() {
  int sensorValue = analogRead(THERMISTOR_PIN);
  resistance = RESISTANCE_REFERENCE * (1023.0 / (float)sensorValue - 1.0);
  logResistance = log(resistance);
  temperatureKelvin = (1.0 / (C1 + C2 * logResistance + C3 * logResistance * logResistance * logResistance));
  temperatureCelsius = temperatureKelvin - 273.15;

  // Envoi des données de température à la transmission série
  Serial.print("Température: ");
  Serial.print(temperatureCelsius);
  Serial.println(" C");

  if (temperatureCelsius > 25) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  
  return temperatureCelsius;
}

void page2(int joystickXRaw, int joystickYRaw) {
  const int UPDATE_INTERVAL = 130;  
  if (temps_actuel - previousMillis >= UPDATE_INTERVAL) {
    previousMillis = temps_actuel;

    lcd.clear();  

    int joystickXMin = 0;
    int joystickXMax = 506;
    int joystickXMappedMin = -25;
    int joystickXMappedMax = 0;
    if (joystickXRaw < joystickXMax) {
      joystickX = joystickXRaw;
      joystickX = map(joystickX, joystickXMin, joystickXMax, joystickXMappedMin, joystickXMappedMax);
      lcd.setCursor(0, 0);
      lcd.print("Recule:");
      lcd.print(joystickX);
      lcd.print(" km/h");

      // Envoi des valeurs du joystick X à la transmission série
      Serial.print("Joystick X: ");
      Serial.println(joystickX);
    } else if (joystickXRaw > joystickXMax) {
      joystickX = joystickXRaw;
      int joystickXMappedMin2 = 0;
      int joystickXMappedMax2 = 120;
      joystickX = map(joystickX, joystickXMax + 1, 1023, joystickXMappedMin2, joystickXMappedMax2);
      lcd.setCursor(0, 0);
      lcd.print("Avance:");
      lcd.print(joystickX);
      lcd.print("km/h");

      // Envoi des valeurs du joystick X à la transmission série
      Serial.print("Joystick X: ");
      Serial.println(joystickX);
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Neutre:0 km/h");

      // Envoi de l'état neutre du joystick à la transmission série
      Serial.println("Joystick X: Neutre");
    }

    int joystickYMin = 0;
    int joystickYMax = 504;
    int joystickYMappedMin = -90;
    int joystickYMappedMax = 0;
    if (joystickYRaw < joystickYMax) {
      joystickY = joystickYRaw;
      joystickY = map(joystickY, joystickYMin, joystickYMax, joystickYMappedMin, joystickYMappedMax);
      lcd.setCursor(0, 1);
      lcd.print("G: ");
      lcd.print(joystickY);
      lcd.print(" Deg");

      // Envoi des valeurs du joystick Y à la transmission série
      Serial.print("Joystick Y: ");
      Serial.println(joystickY);
    } else if (joystickYRaw > joystickYMax) {
      int joystickYMappedMin2 = 0;
      int joystickYMappedMax2 = 90;
      joystickY = joystickYRaw;
      joystickY = map(joystickY, joystickYMax + 1, 1023, joystickYMappedMin2, joystickYMappedMax2);
      lcd.setCursor(0, 1);
      lcd.print("D: ");
      lcd.print(joystickY);
      lcd.print(" Deg");

      // Envoi des valeurs du joystick Y à la transmission série
      Serial.print("Joystick Y: ");
      Serial.println(joystickY);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Direction:q");

      // Envoi de l'état neutre du joystick Y à la transmission série
      Serial.println("Joystick Y: Neutre");
    }
  }
}

void page3() {
  const int UPDATE_INTERVAL = 130;  
  static unsigned long previousMillis = 0;
  if (temps_actuel - previousMillis >= UPDATE_INTERVAL) {
    previousMillis = temps_actuel;
    float temperature = mesurerTemperature();  
    lcd.clear();  
    lcd.setCursor(0, 0);
    lcd.print(F("Temperature:"));
    lcd.print(temperature);
    lcd.print(F(" C"));
    lcd.setCursor(0, 1);
    lcd.print(F("Climatisation:"));
    if (temperature > 25) {
      lcd.print(F("ON"));
    } else {
      lcd.print(F("OFF"));
    }

    // Envoi des informations de climatisation à la transmission série
    Serial.print("Climatisation: ");
    if (temperature > 25) {
      Serial.println("ON");
    } else {
      Serial.println("OFF");
    }
  }
}

void setup() {
  Serial.begin(115200);  // Initialisation de la communication série à 115200 bauds
  pinMode(LED_PIN, OUTPUT);
  pinMode(JOYSTICK_BTN_PIN, INPUT_PULLUP);
  lcd.begin();
  lcd.backlight();
  afficherDemarrage();  
}

void loop() {
  temps_actuel = millis();
  joystickXRaw = analogRead(JOYSTICK_X_PIN);
  joystickYRaw = analogRead(JOYSTICK_Y_PIN);

  if (digitalRead(JOYSTICK_BTN_PIN) == LOW) {
    delay(200); 
    currentPage = (currentPage + 1) % 3; 
  }

  if (currentPage == 0) {
    page2(joystickXRaw, joystickYRaw);  
  } else if (currentPage == 1) {
    page3();  
  }
}
