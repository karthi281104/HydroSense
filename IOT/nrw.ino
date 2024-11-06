#include <WiFi.h>
#include <FirebaseESP32.h>

// Replace with your WiFi network credentials
#define WIFI_SSID "KARTHI2657"
#define WIFI_PASSWORD "12345678"

// Define Firebase configuration
FirebaseConfig config;
FirebaseAuth auth;

// Define pins for flow sensors
const int sensor1Pin = 34;  // GPIO pin for flow sensor 1
const int sensor2Pin = 35;  // GPIO pin for flow sensor 2

// Variables for pulse counting and flow rate calculation
volatile int pulseCount1 = 0;
volatile int pulseCount2 = 0;
float flowRate1 = 0.0;
float flowRate2 = 0.0;
float threshold = 1.0; // Adjusted threshold for higher accuracy
String leakStatus = "Not Detected";

// Firebase setup
FirebaseData firebaseData;

// ISR for flow sensor 1
void IRAM_ATTR pulseCounter1() {
  pulseCount1++;
}

// ISR for flow sensor 2
void IRAM_ATTR pulseCounter2() {
  pulseCount2++;
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Firebase configuration
  config.host = "https://hydrsense-default-rtdb.asia-southeast1.firebasedatabase.app/"; // Firebase host URL
  config.signer.tokens.legacy_token = "AIzaSyD3t1XScGrfq7zZJm2j3mqTe9tsPIrPEPw"; // Firebase database secret or token

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Attach interrupts for pulse counting
  pinMode(sensor1Pin, INPUT_PULLUP);
  pinMode(sensor2Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensor1Pin), pulseCounter1, RISING);
  attachInterrupt(digitalPinToInterrupt(sensor2Pin), pulseCounter2, RISING);
}

void loop() {
  // Reset pulse counts and measure over a 5-second interval
  pulseCount1 = 0;
  pulseCount2 = 0;
  delay(1000); // Increase delay for more accurate pulse counts

  // Calculate flow rate in liters per minute (L/min)
  // Dividing by 5 since we extended the delay to 5 seconds
  flowRate1 = (pulseCount1 / 7.5);
  flowRate2 = (pulseCount2 / 7.5);

  // Leak detection logic
  if (abs(flowRate1 - flowRate2) > threshold) {
    leakStatus = "Leak Detected";
  } else {
    leakStatus = "Not Detected";
  }

  // Send data to Firebase
  if (Firebase.ready()) {
    Firebase.setFloat(firebaseData, "/sensorData/flow_meter1", flowRate1);
    Firebase.setFloat(firebaseData, "/sensorData/flow_meter2", flowRate2);
    Firebase.setString(firebaseData, "/sensorData/leak_status", leakStatus);
    
    if (firebaseData.dataAvailable()) {
      Serial.print("Flow Meter 1: ");
      Serial.println(flowRate1);
      Serial.print("Flow Meter 2: ");
      Serial.println(flowRate2);
      Serial.print("Leak Status: ");
      Serial.println(leakStatus);
    }
  } else {
    Serial.println("Failed to send data to Firebase");
  }
}
