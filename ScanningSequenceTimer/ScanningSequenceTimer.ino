#include <Stepper.h>

// === CONFIGURABLE PARAMETERS ===
const float totalDistanceX_cm = 0.5;     // Total travel in X (cm)
const float totalDistanceY_cm = 0.5;     // Total travel in Y (cm)
const float distancePerRevolution_cm = 2.0 / 13.0; // Leadscrew distance per turn (cm)

const int stepsPerRevolution = 2048;
const int stepperSpeed = 15;  // RPM
const int timeBetweenImages_ms = 2000; // Movement time for each image pause (milliseconds)
const int delayAfterMove_ms = 500;     // Delay after each move (for imaging)
const int numYImageStepsPerRowMove = 3;      // How many image-like steps the Y makes per row change

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
float distancePerStep_cm;
float currentX_cm = 0;
float currentY_cm = 0;
int progressCount = 0;
int totalStepsPlanned = 0;

void setup() {
  Serial.begin(9600);

  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(homeButtonPin, INPUT_PULLUP);

  stepperX.setSpeed(stepperSpeed);
  stepperY.setSpeed(stepperSpeed);

  stepsPerCM = stepsPerRevolution / distancePerRevolution_cm;
  distancePerStep_cm = 1.0 / stepsPerCM; // cm moved per one step

  // === Correct steps calculation by simulation ===
  int simulatedProgressCount = 0;
  float simCurrentX_cm = 0;
  float simCurrentY_cm = 0;
  bool simForward = true;

  while (simCurrentY_cm < totalDistanceY_cm) {
    simCurrentX_cm = 0;

    while (simCurrentX_cm < totalDistanceX_cm) {
      unsigned long moveTime = 0;
      while (moveTime < timeBetweenImages_ms && simCurrentX_cm < totalDistanceX_cm) {
        simCurrentX_cm += distancePerStep_cm;
        simulatedProgressCount++;
        moveTime += 1000.0 * (distancePerStep_cm / (stepperSpeed * distancePerRevolution_cm / 60.0)); // time per step
      }
    }

    // Y movement after X row
    for (int yStep = 0; yStep < numYImageStepsPerRowMove; yStep++) {
      if (simCurrentY_cm < totalDistanceY_cm) {
        unsigned long yMoveTime = 0;
        while (yMoveTime < timeBetweenImages_ms) {
          simCurrentY_cm += distancePerStep_cm;
          simulatedProgressCount++;
          yMoveTime += 1000.0 * (distancePerStep_cm / (stepperSpeed * distancePerRevolution_cm / 60.0));
        }
      }
    }
  }

  totalStepsPlanned = simulatedProgressCount;

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
    performHoming();
    scanning = false;
  }
}

void performScan() {
  bool forward = true;
  int direction = forward ? 1 : -1;
  progressCount = 0;  // Reset progress counter

  for (int yStepGroup = 0; (currentY_cm < totalDistanceY_cm) && !checkHomingInterrupt(); yStepGroup++) {
    Serial.print("Row group ");
    Serial.println(yStepGroup + 1);

    currentX_cm = 0;

    while (currentX_cm < totalDistanceX_cm && !checkHomingInterrupt()) {
      unsigned long moveStartTime = millis();
      while (millis() - moveStartTime < timeBetweenImages_ms && currentX_cm < totalDistanceX_cm) {
        stepperX.step(direction);
        currentX_cm += distancePerStep_cm;
        progressCount++;
      }

      printProgressBar();
      Serial.print("Paused X at (cm): ");
      Serial.println(currentX_cm);
      delay(delayAfterMove_ms);
    }

    forward = !forward;
    direction = forward ? 1 : -1;

    // === Now Y movement after finishing row ===
    if (currentY_cm < totalDistanceY_cm && !checkHomingInterrupt()) {
      for (int yStep = 0; yStep < numYImageStepsPerRowMove; yStep++) {
        if (currentY_cm < totalDistanceY_cm) {
          unsigned long yMoveStart = millis();
          while (millis() - yMoveStart < timeBetweenImages_ms) {
            stepperY.step(1); // Y always forward
            currentY_cm += distancePerStep_cm;
          }
          progressCount++;
          printProgressBar();
          Serial.print("Paused Y at (cm): ");
          Serial.println(currentY_cm);
          delay(delayAfterMove_ms);
        }
      }
    }
  }

  Serial.println("Reached end of Y travel.");
  Serial.println("Scan complete.");
  progressCount = totalStepsPlanned;
  printProgressBar();
}

void performHoming() {
  Serial.println("Returning to origin...");

  int stepsToHomeX = -(currentX_cm * stepsPerCM);
  int stepsToHomeY = -(currentY_cm * stepsPerCM);

  if (currentX_cm != 0) {
    stepperX.step(stepsToHomeX);
    delay(500);
    currentX_cm = 0;
  }

  if (currentY_cm != 0) {
    stepperY.step(stepsToHomeY);
    delay(500);
    currentY_cm = 0;
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

// === Progress Bar Printer ===
void printProgressBar() {
  const int barWidth = 20;
  float percentage = (float)progressCount / totalStepsPlanned;
  if (percentage > 1.0) percentage = 1.0;  // Cap at 100%

  int filledBars = percentage * barWidth;

  Serial.print("[");
  for (int i = 0; i < barWidth; i++) {
    if (i < filledBars) Serial.print("#");
    else Serial.print("-");
  }
  Serial.print("] ");
  Serial.print((int)(percentage * 100));
  Serial.print("% (Progress: ");
  Serial.print(progressCount);
  Serial.print("/");
  Serial.print(totalStepsPlanned);
  Serial.println(")");
}
