#include <ESP8266WiFi.h>

// WiFi credentials
const char* ssid = "TMNL-8F0095"; // Replace with your WiFi SSID
const char* password = "WMFF9F9VCG45BJMJ"; // Replace with your WiFi Password

// Web server
WiFiServer server(80);

// Pin Definitions
const int sensorPin = A0;    // Analog pin connected to the moisture sensor
const int greenLED = D2;     // Green LED pin
const int yellowLED = D1;    // Yellow LED pin
const int redLED = D0;       // Red LED pin

// Moisture level thresholds
const int maxMoisture = 1024; // Air is completely dry
const int minMoisture = 300;  // Sensor in water (calibrate this as needed)

// Variables for sensor reading
unsigned long lastSensorReadTime = 0;
const unsigned long sensorReadInterval = 1000; // Read sensor every 1 second
int cachedSensorValue = 0;

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Set up the LED pins as outputs
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
}

void loop() {
  // Periodically read sensor value
  if (millis() - lastSensorReadTime >= sensorReadInterval) {
    lastSensorReadTime = millis();
    cachedSensorValue = analogRead(sensorPin);
  }

  // Handle HTTP requests
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected.");
    handleRequest(client);
    client.stop();
    Serial.println("Client disconnected.");
  }

  // Update LED status based on moisture level
  updateLEDs();
}

void updateLEDs() {
  int moisturePercent = map(cachedSensorValue, maxMoisture, minMoisture, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  if (moisturePercent > 70) {
    // Green: Moisture level is good
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
  } else if (moisturePercent > 40) {
    // Yellow: Moisture is getting low
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
  } else {
    // Red: Too dry, needs water
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}

void handleRequest(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.flush();

  // Handle LED control requests
  if (request.indexOf("/setLED") != -1) {
    if (request.indexOf("?green=on") != -1) digitalWrite(greenLED, HIGH);
    else if (request.indexOf("?green=off") != -1) digitalWrite(greenLED, LOW);
    else if (request.indexOf("?yellow=on") != -1) digitalWrite(yellowLED, HIGH);
    else if (request.indexOf("?yellow=off") != -1) digitalWrite(yellowLED, LOW);
    else if (request.indexOf("?red=on") != -1) digitalWrite(redLED, HIGH);
    else if (request.indexOf("?red=off") != -1) digitalWrite(redLED, LOW);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<p>LED Updated</p>");
    return;
  }

  // Handle status requests
  if (request.indexOf("/getStatus") != -1) {
    int moisturePercent = map(cachedSensorValue, maxMoisture, minMoisture, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print("{\"moisture\":");
    client.print(moisturePercent);
    client.print(",\"green\":");
    client.print(digitalRead(greenLED));
    client.print(",\"yellow\":");
    client.print(digitalRead(yellowLED));
    client.print(",\"red\":");
    client.print(digitalRead(redLED));
    client.println("}");
    return;
  }

  // Default response
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<p>Invalid Request</p>");
}
