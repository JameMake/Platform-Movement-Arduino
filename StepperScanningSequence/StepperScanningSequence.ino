#include <Stepper.h>

// === CONFIGURABLE PARAMETERS ===
const float stepDistance_cm = .2;
const float totalDistanceX_cm = 1;
const float totalDistanceY_cm = 1;
const float distancePerRevolution_cm = 2.0 / 13.0;

const int stepsPerRevolution = 2048;
const int stepperSpeed = 17;
const int delayBetweenSteps = 200;  // ms

// === BUTTONS ===
const int startButtonPin = 2;
const int homeButtonPin = 13;

int lastStartButtonState = HIGH;
int lastHomeButtonState = HIGH;

// === STEPPERS ===
Stepper stepperX(stepsPerRevolution, 8, 10, 9, 11);
Stepper stepperY(stepsPerRevolution, 3, 5, 4, 6);

// === STATE VARIABLES ===
bool scanning = false;
bool homingRequested = false;

int stepsPerCM;
int stepsPerMove;
int totalStepsX;
int totalStepsY;
int currentXSteps = 0;
int currentYSteps = 0;

void setup() {
  Serial.begin(9600);

  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(homeButtonPin, INPUT_PULLUP);

  stepperX.setSpeed(stepperSpeed);
  stepperY.setSpeed(stepperSpeed);

  stepsPerCM = stepsPerRevolution / distancePerRevolution_cm;
  stepsPerMove = stepDistance_cm * stepsPerCM;

  totalStepsX = totalDistanceX_cm / stepDistance_cm;
  totalStepsY = totalDistanceY_cm / stepDistance_cm;

  Serial.println("System ready. Press START button to begin.");
}

void loop() {
  int startButton = digitalRead(startButtonPin);
  int homeButton = digitalRead(homeButtonPin);

  if (lastStartButtonState == HIGH && startButton == LOW) {
    if (!scanning) {
      Serial.println("Starting scan...");
      scanning = true;
      homingRequested = false;
    }
  }
  lastStartButtonState = startButton;

  if (lastHomeButtonState == HIGH && homeButton == LOW) {
    Serial.println("Homing sequence requested...");
    homingRequested = true;
    scanning = false;
  }
  lastHomeButtonState = homeButton;

  if (homingRequested) {
    performHoming();
    homingRequested = false;
    scanning = false;
  }

  if (scanning) {
    performScan();
    performHoming(); // Return to origin
    scanning = false;
  }
}

void performScan() {
  bool forward = true;

  for (int y = 0; y < totalStepsY; y++) {
    if (checkHomingInterrupt()) return;

    Serial.print("Scanning Row ");
    Serial.print(y + 1);
    Serial.print(" of ");
    Serial.println(totalStepsY);

    for (int x = 0; x < totalStepsX; x++) {
      if (checkHomingInterrupt()) return;

      int stepDir = forward ? stepsPerMove : -stepsPerMove;
      stepperX.step(stepDir);
      currentXSteps += stepDir;
      delay(delayBetweenSteps);
    }

    forward = !forward;

    if (y < totalStepsY - 1 && !checkHomingInterrupt()) { // Check to see if home button is being pushed!
      // If homing button is not being pushed then the y will move on to the next row
      stepperY.step(stepsPerMove);
      currentYSteps += stepsPerMove;
      delay(delayBetweenSteps);
    }
  }

  Serial.println("Scan complete.");
}

void performHoming() {
  Serial.println("Returning to origin...");

  if (currentYSteps != 0) {
    stepperY.step(-currentYSteps);
    delay(500);
    currentYSteps = 0;
  }

  if (currentXSteps != 0) {
    stepperX.step(-currentXSteps);
    delay(500);
    currentXSteps = 0;
  }

  Serial.println("Homing complete.");
}

bool checkHomingInterrupt() {
  if (digitalRead(homeButtonPin) == LOW) {
    Serial.println("Homing interrupted scan...");
    homingRequested = true;
    performHoming();
    return true;
  }
  return false;
}
