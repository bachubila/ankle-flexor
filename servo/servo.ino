/************************************************************
 * Ankle Flexor Servo Controller
 *
 * Features:
 * 1. Potentiometer Control Mode
 *    - Potentiometer controls servo position.
 *    - Motion is limited to a safe angle range.
 *    - Servo movement is smoothed to reduce sudden jumps.
 *
 * 2. Therapy Cycle Mode
 *    - Servo automatically moves between a minimum
 *      and maximum angle.
 *    - Movement speed is configurable.
 *
 * To switch modes:
 * - In loop(), call either:
 *     potentiometerControl();
 *   or
 *     therapyCycle(...);
 *
 ************************************************************/

#include <Servo.h>   // Arduino Servo library

// Create servo object
Servo servo;

// Pin definitions
#define PotPin    A0
#define ServoPin  9

// Safe operating range for potentiometer mode
const int MIN_ANGLE = 30;
const int MAX_ANGLE = 120;

// Track current servo position for smooth movement
int currentAngle = 75;

/************************************************************
 * SETUP
 ************************************************************/
void setup()
{
    // Attach servo to control pin
    servo.attach(ServoPin);

    // Move servo to starting position
    servo.write(currentAngle);

    // Initialize serial communication
    Serial.begin(9600);

    Serial.println("Ankle Flexor Controller Started");
}

/************************************************************
 * POTENTIOMETER CONTROL MODE
 *
 * Reads the potentiometer and maps its value to the
 * configured angle range.
 *
 * Smoothly moves the servo toward the target angle.
 * Includes a deadband to reduce jitter.
 ************************************************************/
void potentiometerControl()
{
    // Read potentiometer value (0–1023)
    uint16_t potValue = analogRead(PotPin);

    // Convert potentiometer value into angle range
    int targetAngle = map(
        potValue,
        0,
        1023,
        MIN_ANGLE,
        MAX_ANGLE
    );

lcdPrint(currentAngle, targetAngle);
    // Deadband: ignore tiny changes
    if (abs(targetAngle - currentAngle) > 1)
    {
        // Move gradually toward target
        if (currentAngle < targetAngle)
        {
            currentAngle++;
        }
        else if (currentAngle > targetAngle)
        {
            currentAngle--;
        }

        servo.write(currentAngle);
    }

    // Controls movement speed
    delay(20);
}

/************************************************************
 * THERAPY CYCLE MODE
 *
 * Automatically moves the servo:
 *   minAngle -> maxAngle
 *   maxAngle -> minAngle
 *
 * Parameters:
 *   minAngle  = minimum exercise angle
 *   maxAngle  = maximum exercise angle
 *   speedDelay = delay between steps (ms)
 *
 * Smaller delay = faster movement
 * Larger delay = slower movement
 ************************************************************/
void therapyCycle(
    int minAngle,
    int maxAngle,
    int speedDelay)
{
  lcdPrint(minAngle, speedDelay);
    // Flexion movement
    for (int pos = minAngle;
         pos <= maxAngle;
         pos++)
    {
        servo.write(pos);
        delay(speedDelay);
    }

    // Extension movement
    for (int pos = maxAngle;
         pos >= minAngle;
         pos--)
    {
        servo.write(pos);
        delay(speedDelay);
    }
}

/************************************************************
 * MAIN LOOP
 *
 * Select ONE control mode.
 ************************************************************/
void loop()
{
    // ---------------------------------
    // MODE 1: Potentiometer Control
    // ---------------------------------
    // potentiometerControl();

    // ---------------------------------
    // MODE 2: Automatic Therapy Cycle
    // Uncomment this and comment out
    // potentiometerControl() above
    // ---------------------------------

    therapyCycle(
        30,   // minimum angle
        120,  // maximum angle
        20    // speed (ms per step)
    );
}

void lcdPrint(int angle, int target)
{
    // Clear-like behavior (depends on module firmware)
    Serial.write(0xFE);  // command prefix (common serial LCD command)
    Serial.write(0x01);  // clear display

    Serial.print("Angle:");
    Serial.print(angle);

    Serial.write(0xFE);
    Serial.write(0xC0);  // move to line 2

    Serial.print("Target:");
    Serial.print(target);
}