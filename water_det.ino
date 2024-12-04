#include <ESP8266WiFi.h>

// WiFi credentials
const char* ssid = "Hotsiepotsie";
const char* password = "FAN1234Y";

// Web server
WiFiServer server(80);

// Pin Definitions
const int sensorPin = A0;    // Analog pin connected to the moisture sensor
const int buttonPin = D3;    // Button pin
const int greenLED = D2;     // Green LED pin
const int yellowLED = D1;    // Yellow LED pin
const int redLED = D0;       // Red LED pin

// Moisture level thresholds
const int maxMoisture = 1024; // Air is completely dry
const int minMoisture = 300;  // Sensor in water (calibrate this if necessary)

// Variables for button press and debounce
int buttonState = 0;         // Current button state
int lastButtonState = 0;     // Previous button state
unsigned long lastDebounceTime = 0; // Last time the button was debounced
const unsigned long debounceDelay = 50; // Debounce delay in milliseconds

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Set up the LED pins as outputs
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Set up the button pin as input with pullup resistor
  pinMode(buttonPin, INPUT_PULLUP);

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
  // Read the current state of the button
  int reading = digitalRead(buttonPin);

  // Debounce logic
  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // Reset the debounce timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has stabilized
    if (reading != buttonState) {
      buttonState = reading;

      // Check for a button press
      if (buttonState == LOW) {
        Serial.println("Button Pressed!");
        // Add custom behavior for the button press here (e.g., toggle mode, change display)
      }
    }
  }

  lastButtonState = reading; // Save the current reading as the last state

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
