#include <DHT.h>
#include <WiFi.h>
#include <LiquidCrystal.h>
#include <BlynkSimpleEsp32.h> 

#define BLYNK_TEMPLATE_ID           "TMPxxxxxx"
#define BLYNK_TEMPLATE_NAME         "Device"
#define BLYNK_AUTH_TOKEN            "YourAuthToken"

// WiFi + Blynk credentials
char ssid[] = "eco";
char pass[] = "ecoproject001";

// Pin Definitions
#define gasPin 34
#define gasAlarm 32
#define tempPin 17
#define DHTTYPE DHT11
#define acRelay 22
#define heaterRelay 23

#define load1Pot 35   // Air Conditioner
#define load2Pot 32   // Water Heater
#define load3Pot 33   // Lighting & Sockets

#define relay1 25     // AC relay
#define relay2 26     // Heater relay
#define relay3 27     // Lights relay

#define light1 2   
#define light2 4   
#define light3 5  
#define light4 16  

// Initialize objects
DHT dht(tempPin, DHTTYPE);
LiquidCrystal lcd(14, 12, 13, 19, 18, 5); // RS, E, D4, D5, D6, D7

int gasValue;

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Sensor and actuator pins
  pinMode(gasPin, INPUT);
  pinMode(gasAlarm, OUTPUT);
  pinMode(acRelay, OUTPUT);
  pinMode(heaterRelay, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);

  dht.begin();
  lcd.begin(16, 2);

  pinMode(light1, OUTPUT);
  pinMode(light2, OUTPUT);
  pinMode(light3, OUTPUT);
  pinMode(light4, OUTPUT);
}

void loop() {
  Blynk.run(); // Uncomment if using Blynk

  gasCalc();
  tempCalc();
  loadManagement();
}

// -------------------- GAS FUNCTION --------------------
void gasCalc() {
  gasValue = analogRead(gasPin);
  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  if (gasValue > 250) {
    digitalWrite(gasAlarm, HIGH);
  } else {
    digitalWrite(gasAlarm, LOW);  // FIXED: should turn off if below threshold
  }
}

// -------------------- TEMP FUNCTION --------------------
void tempCalc() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT Read failed");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.println("°C");

  if (t > 25) {
    digitalWrite(heaterRelay, LOW);
    digitalWrite(acRelay, HIGH);
  } else if (t < 16) {
    digitalWrite(heaterRelay, HIGH);
    digitalWrite(acRelay, LOW);
  } else {
    digitalWrite(heaterRelay, LOW);
    digitalWrite(acRelay, LOW);
  }
}

// -------------------- DSM FUNCTION --------------------
void loadManagement() {
  int l1 = analogRead(load1Pot);
  int l2 = analogRead(load2Pot);
  int l3 = analogRead(load3Pot);

  int totalLoad = l1 + l2 + l3;

  Serial.print("Load1: "); Serial.print(l1);
  Serial.print(" Load2: "); Serial.print(l2);
  Serial.print(" Load3: "); Serial.print(l3);
  Serial.print(" Total: "); Serial.println(totalLoad);

  // Example threshold
  int threshold = 2000;

  // All ON initially
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);

  // Progressive Load Shedding
  if (totalLoad > threshold) {
    digitalWrite(relay1, LOW); // Cut AC first
    lcd.setCursor(0, 1);
    lcd.print("Cut: AC        ");
  }

  if (totalLoad > threshold + 500) {
    digitalWrite(relay2, LOW); // Then Water Heater
    lcd.setCursor(0, 1);
    lcd.print("Cut: AC & WH   ");
  }

  if (totalLoad > threshold + 1000) {
    digitalWrite(relay3, LOW); // Finally Lights
    lcd.setCursor(0, 1);
    lcd.print("All Loads Cut  ");
  }

  // Display total load
  lcd.setCursor(0, 0);
  lcd.print("Load: ");
  lcd.print(totalLoad);
  lcd.print("    ");
}

BLYNK_WRITE(V0) {
  int state = param.asInt();
  digitalWrite(light1, state);
}

BLYNK_WRITE(V1) {
  int state = param.asInt();
  digitalWrite(light2, state);
}

BLYNK_WRITE(V2) {
  int state = param.asInt();
  digitalWrite(light3, state);
}

BLYNK_WRITE(V3) {
  int state = param.asInt();
  digitalWrite(light4, state);
}
