#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Pin Definitions
const int sensorPin = A0;    // Analog pin connected to the moisture sensor
const int greenLED = D2;     // Green LED pin
const int yellowLED = D1;    // Yellow LED pin
const int redLED = D0;       // Red LED pin
const int buttonPin = D3;    // Button pin

// Moisture level thresholds
const int maxMoisture = 1024; // Air is completely dry
const int minMoisture = 300;  // Sensor in water (calibrate this if necessary)

// Variables for button press and state
int buttonState = 0;
int lastButtonState = 0;
unsigned long lastPressTime = 0;
unsigned long displayStartTime = 0;
bool isCustomDisplay = false;
int customDisplayState = 0;
int updatedcustomDisplayState = 0;
int lastlevel = -1;

// Variables for debounce
const unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;

// WiFi credentials
const char* ssid = "Hotsiepotsie";
const char* password = "FAN1234Y";

// Web server
ESP8266WebServer server(80);

// Function to handle the root page
void handleRoot() {
  server.send(200, "text/plain", "Hello from ESP8266!");
}

// Function to handle LED control via web server
void handleControl() {
  if (server.hasArg("led")) {
    String led = server.arg("led");

    if (led == "green") {
      digitalWrite(greenLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
    } else if (led == "yellow") {
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, LOW);
    } else if (led == "red") {
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, HIGH);
    } else if (led == "off") {
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
    } else if (led == "on") {
      digitalWrite(greenLED, HIGH);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, HIGH);
    } else {
      server.send(400, "text/plain", "Invalid LED command");
      return;
    }
    server.send(200, "text/plain", "LED state updated");
  } else {
    server.send(400, "text/plain", "Missing LED parameter");
  }
}

// Function to update the LEDs based on custom display state
void updateCustomDisplay() {
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);

  if (customDisplayState == -1) {
    // Do nothing
  } else if (customDisplayState == 0) {
    digitalWrite(redLED, HIGH); // Red
  } else if (customDisplayState == 1) {
    digitalWrite(yellowLED, HIGH); // Yellow
  } else if (customDisplayState == 2) {
    digitalWrite(greenLED, HIGH); // Green
  }
}

// Function to display moisture status
void showMoistureStatus() {
  int sensorValue = analogRead(sensorPin);
  int moisturePercent = map(sensorValue, maxMoisture, minMoisture, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  if (moisturePercent != lastlevel) {
    lastlevel = moisturePercent;
    Serial.print("Moisture Level: ");
    Serial.print(moisturePercent);
    Serial.println("%");
  }

  if (moisturePercent > 70) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
  } else if (moisturePercent > 40) {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
  } else {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}

void setup() {
  Serial.println("yeah.");
  Serial.begin(9600);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.begin();
  Serial.println("Server started.");
}

void loop() {
  server.handleClient();

  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        if (!isCustomDisplay) {
          customDisplayState = -1;
          updateCustomDisplay();
          delay(200);
        }
        customDisplayState = (updatedcustomDisplayState + 1) % 3;
        updatedcustomDisplayState = customDisplayState;
        isCustomDisplay = true;
        displayStartTime = millis();
        updateCustomDisplay();
      }
    }
  }

  lastButtonState = reading;

  if (isCustomDisplay && millis() - displayStartTime > 3000) {
    customDisplayState = -1;
    updateCustomDisplay();
    isCustomDisplay = false;
  }

  if (!isCustomDisplay) {
    showMoistureStatus();
  }
}
