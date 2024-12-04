#include <ESP8266WiFi.h>

// WiFi credentials
const char* ssid = "Hotsiepotsie";
const char* password = "FAN1234Y";

// Web server
WiFiServer server(80);

// Pin Definitions
const int sensorPin = A0;    // Analog pin connected to the moisture sensor

// Moisture level thresholds
const int maxMoisture = 1024; // Air is completely dry
const int minMoisture = 300;  // Sensor in water (calibrate this if necessary)

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

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
  // Handle HTTP requests
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected.");
    // HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<h1>Moisture Level</h1>");

    // Calculate and display moisture level
    int sensorValue = analogRead(sensorPin);
    int moisturePercent = map(sensorValue, maxMoisture, minMoisture, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    client.println("<p>Moisture Level: " + String(moisturePercent) + "%</p>");
    client.println("</html>");
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}
