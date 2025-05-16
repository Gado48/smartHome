#include <DHT.h>
#include <WiFi.h>
#include <LiquidCrystal.h>
#include <TimeLib.h>

#define BLYNK_TEMPLATE_ID           "TMPL2wc_ezJ4A"
#define BLYNK_TEMPLATE_NAME         "first project"
#define BLYNK_AUTH_TOKEN            "84PQFAHTugaI3l47l78hbKiMxSBjZ4i9"

#include <BlynkSimpleEsp32.h> 

// WiFi + Blynk credentials
char ssid[] = "tel";
char pass[] = "11110000";

// Pin Definitions
#define gasPin 34
#define gasAlarm 13
#define tempPin 17
#define DHTTYPE DHT11
#define acRelay 22
#define heaterRelay 23
#define lightRelay 27 

#define load1Pot 35   // Air Conditioner
#define load2Pot 32   // Water Heater
#define load3Pot 33   // Lighting & Sockets

#define light1 26   
#define light2 25   
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
  pinMode(lightRelay, OUTPUT);

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
  autoLightning()
  loadManagement();
  nightLight();
  delay(10);
}

// -------------------- GAS FUNCTION --------------------
void gasCalc() {
  gasValue = analogRead(gasPin);
  Serial.print("Gas Value: ");
  Serial.println(gasValue);
  Blynk.virtualWrite(V6, gasValue);

  if (gasValue > 3000) {
    digitalWrite(gasAlarm, HIGH);
    Blynk.logEvent("gas_alert", "Gas Leak Detected!");
  } else {
    digitalWrite(gasAlarm, LOW);
    Serial.println("led off");
  }
}
// -------------------- LIGHTNING FUNCTION ---------------
void nightLight(){
  if (hour() >= 19 || hour() < 6) {
  digitalWrite(light4, HIGH); // Night Light ON
  } else {
  digitalWrite(light4, LOW);  // OFF in daytime
  }
}

// -------------------- TEMP FUNCTION --------------------
void tempCalc() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Blynk.virtualWrite(V4, t);
  Blynk.virtualWrite(V5, h);

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

// -------------------- AUTO LIGHTNING ------------------
void autoLightning(){
  if (hour() >= 19 || hour() < 6) {
  digitalWrite(light4, HIGH); // Night Light ON
  } else {
  digitalWrite(light4, LOW);  // OFF in daytime
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
  digitalWrite(acRelay, HIGH);
  digitalWrite(heaterRelay, HIGH);
  digitalWrite(lightRelay, HIGH);

  // Progressive Load Shedding
  if (totalLoad > threshold) {
    digitalWrite(acRelay, LOW); // Cut AC first
    lcd.setCursor(0, 1);
    lcd.print("Cut: AC        ");
  }

  if (totalLoad > threshold + 500) {
    digitalWrite(heaterRelay, LOW); // Then Water Heater
    lcd.setCursor(0, 1);
    lcd.print("Cut: AC & WH   ");
  }

  if (totalLoad > threshold + 1000) {
    digitalWrite(lightRelay, LOW); // Finally Lights
    lcd.setCursor(0, 1);
    lcd.print("All Loads Cut  ");
  }

  // Display total load
  lcd.clear()
  lcd.setCursor(0, 0);
  lcd.print("Load: ");
  lcd.print(totalLoad);
  lcd.print("    ");
}

BLYNK_WRITE(V0) {
  int state = param.asInt();
  analogWrite(light1, state);
}

BLYNK_WRITE(V1) {
  int state = param.asInt();
  analogWrite(light2, state);
}

BLYNK_WRITE(V2) {
  int state = param.asInt();
  analogWrite(light3, state);
}

BLYNK_WRITE(V3) {
  int state = param.asInt();
  Serial.println(state);
  analogWrite(light4, state);
  // Serial.println("working");
  if(state){
    Serial.println("on");
  }
}
