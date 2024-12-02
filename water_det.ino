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
int buttonState = 0;         // Current button state
int lastButtonState = 0;     // Previous button state
unsigned long lastPressTime = 0; // Time of the last button press
unsigned long displayStartTime = 0; // Time display mode started
bool isCustomDisplay = false; // Flag for custom display mode
int customDisplayState = 0;   // Current state in the custom display cycle
int updatedcustomDisplayState = 0;
int lastlevel;

void setup() {
  // Set up the LED pins as outputs
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Set up the button pin as input with pullup resistor
  pinMode(buttonPin, INPUT_PULLUP);

  // Start serial communication for debugging
  Serial.begin(115200);
}

void loop() {
  // Read button state
  buttonState = digitalRead(buttonPin);

  // Check for button press
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Button was pressed, change custom display state
    if(!isCustomDisplay)
    {
    customDisplayState = -1;
    updateCustomDisplay();
    delay(200);
    }
    customDisplayState = (updatedcustomDisplayState + 1) % 3; // Cycle through 0, 1, 2
    updatedcustomDisplayState = customDisplayState;
    isCustomDisplay = true;
    displayStartTime = millis(); // Record the time custom mode starts
    updateCustomDisplay();
  }
  lastButtonState = buttonState;

  // If in custom display mode and 3 seconds have passed, return to moisture display
  if (isCustomDisplay && millis() - displayStartTime > 3000) {
    customDisplayState = -1;
    updateCustomDisplay();
    delay(200);
    isCustomDisplay = false; // Exit custom display mode
  }

  // Show moisture level if not in custom display mode
  if (!isCustomDisplay) {
    showMoistureStatus();
  }
}

// Function to update the LEDs based on custom display state
void updateCustomDisplay() {
  // Turn off all LEDs
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);

  // Turn on the corresponding LED for the current state
  if(customDisplayState == -1)
  {

  }
  else if (customDisplayState == 0) {
    digitalWrite(redLED, HIGH); // Red
  } else if (customDisplayState == 1) {
    digitalWrite(yellowLED, HIGH); // Yellow
  } else if (customDisplayState == 2) {
    digitalWrite(greenLED, HIGH); // Green
  }
}

// Function to display moisture status
void showMoistureStatus() {
  // Read moisture sensor value
  int sensorValue = analogRead(sensorPin);
  
  // Map sensor value to percentage (0% = air, 100% = wet)
  int moisturePercent = map(sensorValue, maxMoisture, minMoisture, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100); // Ensure percentage is within bounds
  if(moisturePercent != lastlevel)
  { 
   lastlevel = moisturePercent;
   // Debugging output
   Serial.print("Moisture Level: ");
   Serial.print(moisturePercent);
   Serial.println("%");
   Serial.println(lastlevel);
  }
  // Determine LED status
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