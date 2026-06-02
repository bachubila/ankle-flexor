/********************************************************************************

 * Servo motor control with Arduino
 * Potentiometer connected to Arduino is used to set Servo's position.

/********************************************************************************/

#include <Servo.h>

Servo servo;

#define PotPin A0
#define ServoPin 9

// Safe movement limits
const int MIN_ANGLE = 30;
const int MAX_ANGLE = 120;

// Current servo position
int currentAngle = 75;  // Start near the middle

void setup() {
  servo.attach(ServoPin);
  servo.write(currentAngle);
}

void loop() {
  // Read potentiometer
  uint16_t an = analogRead(PotPin);

  // Map potentiometer to the safe range
  int targetAngle = map(an, 0, 1023, MIN_ANGLE, MAX_ANGLE);

  // Smooth movement: move 1 degree at a time
  if (abs(targetAngle - currentAngle) > 2)  // the servo ignores those tiny changes and only moves when the difference reaches at least 2°. This reduces jitter and unnecessary servo activity.
  {

    if (currentAngle < targetAngle) {
      currentAngle++;
      servo.write(currentAngle);
    } else if (currentAngle > targetAngle) {
      currentAngle--;
      servo.write(currentAngle);
    }
  }

  // Controls movement speed
  delay(20);
}
// end of code.