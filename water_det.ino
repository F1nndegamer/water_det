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

// Moisture thresholds for presets
struct Preset {
  int Greenrequired;
  int Yellowreq;
};

Preset presets[3] = {
  {70, 40},  // Preset 1
  {80, 50},  // Preset 2
  {60, 30}   // Preset 3
};

int currentPreset = 0; // Index of the current preset

// Initialize the web server
ESP8266WebServer server(80);

// Global variables
int lastMoisture = 0;          // Last moisture level
unsigned long lastReadTime = 0; // Last time the sensor was read
const unsigned long readInterval = 500; // Interval between reads (ms)

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
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP()); // Print the ESP8266's IP address

  // Define web server routes
  server.on("/getMoisture", HTTP_GET, handleMoistureRequest);
  server.on("/setPreset", HTTP_GET, handleSetPreset);
  server.on("/updatePreset", HTTP_POST, handleUpdatePreset);

  server.begin();
}

void loop() {
  // Handle incoming client requests
  server.handleClient();

  // Read sensor only at specified intervals
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime > readInterval) {
    lastReadTime = currentTime;
    lastMoisture = MoisturePercent(); // Update the global moisture level

    // Update LED states based on the current preset
    updateLEDs();
  }

  // Handle button press for preset switching
  static bool buttonPressed = false;
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    switchPreset();
  } else if (digitalRead(buttonPin) == HIGH) {
    buttonPressed = false;
  }
}

int MoisturePercent() {
  int moistureValue = analogRead(sensorPin);

  // Map moisture level to percentage
  int moisturePercent = map(moistureValue, 1024, 340, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100); // Ensure it's between 0 and 100
  return moisturePercent;
}

void updateLEDs() {
  Preset current = presets[currentPreset];
  if (lastMoisture > current.Greenrequired) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
  } else if (lastMoisture > current.Yellowreq) {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
  } else {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}

void handleMoistureRequest() {
  Preset current = presets[currentPreset];
  String response = "{\"moisture\":" + String(lastMoisture) + 
                    ",\"ledStatus\":\"";
  if (lastMoisture > current.Greenrequired) {
    response += "green";
  } else if (lastMoisture > current.Yellowreq) {
    response += "yellow";
  } else {
    response += "red";
  }
  response += "\",\"preset\":" + String(currentPreset + 1);
  responce += current.Greenrequired;
  responce += current.Yellowreq;
  server.send(200, "application/json", response);
}

void handleSetPreset() {
  if (server.hasArg("preset")) {
    int preset = server.arg("preset").toInt() - 1;
    if (preset >= 0 && preset < 3) {
      currentPreset = preset;
      server.send(200, "text/plain", "Preset switched to " + String(preset + 1));
    } else {
      server.send(400, "text/plain", "Invalid preset number");
    }
  } else {
    server.send(400, "text/plain", "Preset parameter is missing");
  }
}

void handleUpdatePreset() {
  if (server.hasArg("preset") && server.hasArg("green") && server.hasArg("yellow")) {
    int preset = server.arg("preset").toInt() - 1;
    int green = server.arg("green").toInt();
    int yellow = server.arg("yellow").toInt();
    if (preset >= 0 && preset < 3 && green > yellow) {
      presets[preset].Greenrequired = green;
      presets[preset].Yellowreq = yellow;
      server.send(200, "text/plain", "Preset updated successfully");
    } else {
      server.send(400, "text/plain", "Invalid parameters");
    }
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

void switchPreset() {
  currentPreset = (currentPreset + 1) % 3; // Cycle through presets
  Serial.println("Switched to preset " + String(currentPreset + 1));
}
