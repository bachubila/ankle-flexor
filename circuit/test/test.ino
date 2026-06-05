#include <Servo.h>

Servo servo;

#define SERVO_PIN  9
#define POT_PIN    A0

const int MIN_ANGLE = 30;
const int MAX_ANGLE = 120;
const int MIN_SPEED = 5;
const int MAX_SPEED_MS = 200;

int therapyMin = 30;
int therapyMax = 120;
int therapySpeed = 20;

int currentAngle = therapyMin;

void setup()
{
    servo.attach(SERVO_PIN);
    servo.write(currentAngle);
}

void loop()
{
  //runFromPOT();
  therapyCycle();
    delay(15);
}

void runFromPOT(){
    int potValue = analogRead(POT_PIN);
    int targetAngle = map(potValue, 0, 1023, 0, 180);

    if (abs(targetAngle - currentAngle) > 1)
    {
        if (currentAngle < targetAngle) currentAngle++;
        else currentAngle--;
        servo.write(currentAngle);
    }
}

void therapyCycle()
{
    for (int pos = therapyMin; pos <= therapyMax; pos++)
    {
        servo.write(pos);
        currentAngle = pos;
        // updateLcdMotion(pos, "FLEX");
        delay(therapySpeed);
    }

    for (int pos = therapyMax; pos >= therapyMin; pos--)
    {
        servo.write(pos);
        currentAngle = pos;
        // updateLcdMotion(pos, "EXTEND");
        delay(therapySpeed);
    }
}