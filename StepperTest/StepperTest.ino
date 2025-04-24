#include <Stepper.h>

const int stepsPerRev = 2048;
const int buttonPin = 2;
const int buttonPin2 = 13;
int buttonState = HIGH;
int prevState = HIGH;
int buttonState2 = HIGH;
int prevState2 = HIGH;
int activeMotor = 0; // 0 = none, 1 = first motor, 2 = second motor
int mode1 = 0; // 0 = stopped, 1 = forward, 2 = reverse
int mode2 = 0; // 0 = stopped, 1 = forward, 2 = reverse

Stepper myStepper(stepsPerRev, 8, 10, 9, 11);
Stepper myStepper2(stepsPerRev, 3, 5, 4, 6);

void setup() {
  myStepper.setSpeed(15);
  myStepper2.setSpeed(15);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (prevState == HIGH && buttonState == LOW) { // Detect button press
    if (activeMotor != 1) {
      activeMotor = 1;
      mode1 = 1; // Start first motor forward
      mode2 = 0; // Stop second motor
    } else {
      mode1 = (mode1 + 1) % 3; // Cycle through 0 (stopped), 1 (forward), 2 (reverse)
    }
  }
  prevState = buttonState;
  
  buttonState2 = digitalRead(buttonPin2);
  if (prevState2 == HIGH && buttonState2 == LOW) { // Detect button press for second motor
    if (activeMotor != 2) {
      activeMotor = 2;
      mode2 = 1; // Start second motor forward
      mode1 = 0; // Stop first motor
    } else {
      mode2 = (mode2 + 1) % 3; // Cycle through 0 (stopped), 1 (forward), 2 (reverse)
    }
  }
  prevState2 = buttonState2;
  
  if (activeMotor == 1) {
    if (mode1 == 1) {
      myStepper.step(stepsPerRev / 10); // Move first motor forward
    } else if (mode1 == 2) {
      myStepper.step(-stepsPerRev / 10); // Move first motor in reverse
    }
  }
  
  if (activeMotor == 2) {
    if (mode2 == 1) {
      myStepper2.step(stepsPerRev / 10); // Move second motor forward
    } else if (mode2 == 2) {
      myStepper2.step(-stepsPerRev / 10); // Move second motor in reverse
    }
  }
}
