#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "TMNL-8F0095";
const char* password = "WMFF9F9VCG45BJMJ";

// Pin Definitions
const int sensorPin = A0;    // Analog pin connected to the moisture sensor
const int greenLED = D2;     // Green LED pin
const int yellowLED = D1;    // Yellow LED pin
const int redLED = D0;       // Red LED pin
const int buttonPin = D3;    // Button pin

// Moisture level thresholds
const int maxMoisture = 1024; // Air is completely dry
const int minMoisture = 340;  // Sensor in water (calibrate this if nece
const int Greenrequired = 70;
const int Yellowreq = 40;
// Initialize the web server
ESP8266WebServer server(80);

void setup() {
  // Set up the LED pins as outputs
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Start Serial for debugging
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Define web server routes
  server.on("/getMoisture", HTTP_GET, handleMoistureRequest);
  server.begin();
  Serial.println(WiFi.localIP());
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
   int moisture = MoisturePercent();
  if (moisture > Greenrequired) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
  } else if (moisture > Yellowreq) {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
  } else {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}
int MoisturePercent()
{
  int moistureValue = analogRead(sensorPin);

  // Map moisture level to percentage
  int moisturePercent = map(moistureValue, maxMoisture, minMoisture, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100); // Ensure it's between 0 and 100
  return(moisturePercent);
}
void handleMoistureRequest() {
  int moisture = MoisturePercent();
  // Send moisture and LED status as JSON response
  String response = "{\"moisture\":" + String(moisture) + ",\"ledStatus\":\"";
  if (moisture > 70) {
    response += "green";
  } else if (moisture > 40) {
    response += "yellow";
  } else {
    response += "red";
  }
  response += "\"}";
  
  server.send(200, "application/json", response);
}