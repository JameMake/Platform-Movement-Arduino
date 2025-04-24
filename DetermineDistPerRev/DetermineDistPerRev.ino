// Determine Dist Per Revolution of the Stepper (FOR TESTING)

#include <Stepper.h>

const int stepsPerRev = 2048;  // Full revolution steps
const int buttonX = 2;         // Button for X-axis motor
const int buttonY = 13;         // Button for Y-axis motor 

int lastButtonX = HIGH;
int lastButtonY = HIGH;

Stepper myStepper1(stepsPerRev, 8, 10, 9, 11);  // X-axis
Stepper myStepper2(stepsPerRev, 3, 5, 4, 6);    // Y-axis

int stepSpeed = 17;

void setup() {
  Serial.begin(9600);

  myStepper1.setSpeed(stepSpeed);
  myStepper2.setSpeed(stepSpeed);

  pinMode(buttonX, INPUT_PULLUP);
  pinMode(buttonY, INPUT_PULLUP);

  Serial.println("Press Button X or Y for one revolution");
}

void loop() {
  int currentButtonX = digitalRead(buttonX);
  int currentButtonY = digitalRead(buttonY);

  // Detect rising edge on buttonX
  if (lastButtonX == HIGH && currentButtonX == LOW) {
    Serial.println("X-axis Motor: One Revolution Forward");
    myStepper1.step(stepsPerRev); // Move 1 revolution
  }

  // Detect rising edge on buttonY
  if (lastButtonY == HIGH && currentButtonY == LOW) {
    Serial.println("Y-axis Motor: One Revolution Forward");
    myStepper2.step(stepsPerRev); // Move 1 revolution
  }

  // Save current button states
  lastButtonX = currentButtonX;
  lastButtonY = currentButtonY;

  delay(50); // Basic debounce
}
