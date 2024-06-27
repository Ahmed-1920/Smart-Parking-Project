#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Keypad.h>
#include <math.h>

#define WIFI_SSID "Wifi"
#define WIFI_PASSWORD "thewifi192092"
#define API_KEY "AIzaSyCKddWGVzR_pUZXHJzixfdyXWFtifmV3oM"
#define USER_EMAIL "admin@project.com"
#define USER_PASSWORD "192092"
#define DATABASE_URL "https://iot-project-eu-default-rtdb.europe-west1.firebasedatabase.app/"

char keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pin_rows[4] = {12, 14, 27, 26};
byte pin_cols[4] = {25, 4, 16, 17};

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_cols, 4, 4);

Servo myServo[2];

LiquidCrystal_I2C lcd(0x27, 16, 2);

enum GateState {
  NO_REQ,
  WAITING_FOR_AUTHEN,
  AUTHEN_FAILED,
  CAR_ENTERING,
  FIREALARM
};

GateState gateState = NO_REQ;
bool carDetected = false;
bool openExit = false;
bool openEntrance = false;
bool fireAlarm = false;
unsigned long reqInTime = 0;
unsigned long getDataPrevMillis = 0;
int carCount = 0;
unsigned long entPressTime;
bool entHeld;
unsigned long exPressTime;
bool exHeld;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
String databasePath;
FirebaseJson json;
bool signupOK = false;

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("\nConnected Successfully\nIP is: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  pinMode(13, INPUT);  // Entrance IR
  pinMode(34, INPUT); // car detection IR
  pinMode(15, INPUT); // car exit IR
  pinMode(35, INPUT); // Fire Detector
  pinMode(2, OUTPUT);  // BUZZER

  myServo[0].attach(33); // Entrance Gate
  myServo[1].attach(23); // Exit Gate

  myServo[0].write(90);
  myServo[1].write(90);

  lcd.init();    // Initialize the LCD (assuming a 16x2 LCD)
  lcd.noBacklight();   // Turn off backlight initially
  lcd.clear();         // Clear the LCD initially
  lcd.setCursor(0, 0);

  initWiFi();
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  config.max_token_generation_retry = 5;

  Serial.println("Starting Firebase...");
  Firebase.begin(&config, &auth);
  Serial.println("Firebase started");

  if (Firebase.RTDB.getInt(&fbdo, "/INFO/carCount")) {
    carCount = fbdo.intData();
    Serial.print("Initial car count: ");
    Serial.println(carCount);
  } else {
    Serial.println("Failed to get initial car count");
  }
}

unsigned long lastFirebaseAttemptMillis = 0; // Track the last Firebase connection attempt

void reconnectFirebase() {
  if (!Firebase.ready() && (millis() - lastFirebaseAttemptMillis >= 5000)) {
    Serial.println("Reconnecting to Firebase...");
    Firebase.begin(&config, &auth);
    lastFirebaseAttemptMillis = millis();
  }
}

void loop() {
  Serial.println("Loop start");
  reconnectFirebase();

  if (Firebase.ready() && (millis() - getDataPrevMillis > 2000 || getDataPrevMillis == 0)) {
    getDataPrevMillis = millis();
    Serial.println("Firebase is ready, fetching data...");
    if (Firebase.RTDB.getBool(&fbdo, "/INFO/openExit")) {
      openExit = fbdo.boolData();
      Serial.print("openExit: ");
      Serial.println(openExit);
    } else {
      Serial.println("Failed to get openExit");
    }
    if (Firebase.RTDB.getBool(&fbdo, "/INFO/openEntrance")) {
      openEntrance = fbdo.boolData();
      Serial.print("openEntrance: ");
      Serial.println(openEntrance);
    } else {
      Serial.println("Failed to get openEntrance");
    }
    if (Firebase.RTDB.getBool(&fbdo, "/INFO/fireAlarm")) {
      fireAlarm = fbdo.boolData();
      Serial.print("fireAlarm: ");
      Serial.println(fireAlarm);
    } else {
      Serial.println("Failed to get fireAlarm");
    }
  }

if (openEntrance && carCount < 4 && gateState != AUTHEN_FAILED) {
    if (gateState != CAR_ENTERING) { // Only update the state and timing if we're not already in CAR_ENTERING
        myServo[0].write(45);
        gateState = CAR_ENTERING;
        reqInTime = millis() + 4000; // Set the timeout for 4 seconds
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.clear();
        lcd.print("Welcome!!");
        Serial.println("Opening entrance gate");
    }
} else {
    myServo[0].write(90);
}

  if (openExit) {
    myServo[1].write(134);  // Open the gate
    Serial.println("Opening exit gate");

    if (digitalRead(15) == LOW) {
      myServo[1].write(90);
      carCount = max(0, carCount - 1);
      Firebase.RTDB.setInt(&fbdo, "/INFO/carCount", carCount);
      openExit = false;
      Firebase.RTDB.setBool(&fbdo, "/INFO/openExit", false);
      Serial.println("Car exited, closing exit gate");
    }
  } else {
    myServo[1].write(90);
  }

  carDetected = (digitalRead(13) == LOW);
  Serial.print("carDetected: ");
  Serial.println(carDetected);

  if (digitalRead(35) == LOW || fireAlarm) {
    Firebase.RTDB.setBool(&fbdo, "/INFO/fireAlarm", true);
    gateState = FIREALARM;
    lcd.clear();
    lcd.backlight();   // Turn on backlight initially
    lcd.print("FIRE");
    Serial.println("Fire alarm triggered");
  }

  switch (gateState) {
    case NO_REQ:
      Serial.println("State: NO_REQ");
      if (carCount == 4 && carDetected) {
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.print("Parking is full");
        Serial.println("Parking is full");
      } else if (carDetected) {
        myServo[0].write(90);
        gateState = WAITING_FOR_AUTHEN;
        reqInTime = millis() + 5000;
        lcd.backlight();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Authenticating...");
        Serial.println("Car detected, waiting for authentication");
      }
      break;

    case WAITING_FOR_AUTHEN:
      Serial.println("State: WAITING_FOR_AUTHEN");
      if (millis() > reqInTime) {
        gateState = AUTHEN_FAILED;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Authentication");
        lcd.setCursor(0, 1);
        lcd.print("Failed!");
        Serial.println("Authentication failed");
      }
      break;

    case AUTHEN_FAILED:
      Serial.println("State: AUTHEN_FAILED");
      if (digitalRead(13) == HIGH) {
        gateState = NO_REQ;
        lcd.clear();
        lcd.noBacklight();
        Serial.println("Authentication failed, car left");
      }
      break;

case CAR_ENTERING:
    Serial.println("State: CAR_ENTERING");
    if ((digitalRead(34) == LOW) && (digitalRead(13) == HIGH)) {
        carCount = min(4, carCount + 1);
        myServo[0].write(90);
        openEntrance = false;
        Firebase.RTDB.setBool(&fbdo, "/INFO/openEntrance", false);
        Firebase.RTDB.setInt(&fbdo, "/INFO/carCount", carCount);
        gateState = NO_REQ;
        lcd.clear();
        lcd.noBacklight();
        Serial.println("Car entered, closing entrance gate");
    }

    if (millis() > reqInTime) {
        myServo[0].write(90);
        gateState = NO_REQ;
        openEntrance = false;
        Firebase.RTDB.setBool(&fbdo, "/INFO/openEntrance", false);
        lcd.clear();
        lcd.noBacklight();
        Serial.println("Entrance gate closed due to timeout");
    }
    break;

    case FIREALARM:
      Serial.println("State: FIREALARM");
      while (true) {
        myServo[1].write(134);
        myServo[0].write(45);
        digitalWrite(2, HIGH);
        delay(2000);
        digitalWrite(2, LOW);
        Serial.println("Fire alarm active, gates open and buzzer sounding");

        if (Firebase.RTDB.getBool(&fbdo, "/INFO/fireAlarm")) {
          if (!(fbdo.boolData())) {
            myServo[1].write(90);
            myServo[0].write(90);
            gateState = NO_REQ;
            lcd.clear();
            lcd.noBacklight();
            Serial.println("Fire alarm deactivated, closing gates");
            break;
          }
        }
      }
      delay(2000);
      break;
  }

  char key = keypad.getKey();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    if (key != '#') { // New key press
      entPressTime = millis();
      entHeld = false;
    } else if ((millis() - entPressTime) >= 2500 && !entHeld) { // Key held for 2.5 seconds
      Serial.println("Key held: #");
      entHeld = true;
      Firebase.RTDB.setBool(&fbdo, "/INFO/openEntrance", !openEntrance);
    }

    if (key != '*') { // New key press
      exPressTime = millis();
      exHeld = false;
    } else if ((millis() - exPressTime) >= 2500 && !exHeld) { // Key held for 2.5 seconds
      Serial.println("Key held: *");
      exHeld = true;
      Firebase.RTDB.setBool(&fbdo, "/INFO/openExit", !openExit);
    }
  }

  Serial.println("Loop end");
}
