/************************************************************
 * Ankle Flexor Servo Controller
 *
 * Controls a servo motor for ankle rehabilitation therapy.
 * Supports:
 *   - Potentiometer Control Mode: manual position via pot
 *   - Therapy Cycle Mode: automatic oscillation between
 *     configurable min/max angles at configurable speed
 *
 * Pin Assignment:
 *   Servo      -> D9
 *   Potentiometer -> A0
 *   LCD (16x2, 4-bit mode)
 *     RS       -> D7
 *     E        -> D8
 *     D4       -> D10
 *     D5       -> D11
 *     D6       -> D12
 *     D7       -> D13
 *
 * Serial (9600 baud) is used at startup to configure
 * therapy parameters. If nothing is sent within 5 seconds,
 * default values are used.
 ************************************************************/

#include <Servo.h>
#include <LiquidCrystal.h>

Servo servo;
LiquidCrystal lcd(7, 8, 10, 11, 12, 13);

#define PotPin   A0
#define ServoPin 9

const int MIN_ANGLE = 30;
const int MAX_ANGLE = 120;
const int MIN_SPEED = 5;
const int MAX_SPEED_MS = 200;

int therapyMin = 30;
int therapyMax = 120;
int therapySpeed = 20;

int currentAngle = therapyMin;

/************************************************************
 * SETUP
 ************************************************************/
void setup()
{
    servo.attach(ServoPin);
    servo.write(currentAngle);

    lcd.begin(16, 2);
    lcd.print("Ankle Flexor");
    lcd.setCursor(0, 1);
    lcd.print("Controller v2");

    Serial.begin(9600);
    getTherapyParameters();

    delay(1500);
    lcd.clear();
}

/************************************************************
 * POTENTIOMETER CONTROL MODE
 ************************************************************/
void potentiometerControl()
{
    uint16_t potValue = analogRead(PotPin);
    int targetAngle = map(potValue, 0, 1023, MIN_ANGLE, MAX_ANGLE);

    if (abs(targetAngle - currentAngle) > 1)
    {
        if (currentAngle < targetAngle) currentAngle++;
        else if (currentAngle > targetAngle) currentAngle--;
        servo.write(currentAngle);
        updateLcdPot(targetAngle);
    }

    delay(15);
}

/************************************************************
 * THERAPY CYCLE MODE
 ************************************************************/
void therapyCycle()
{
    lcdPrintTherapy(therapyMin, therapyMax, therapySpeed);

    for (int pos = therapyMin; pos <= therapyMax; pos++)
    {
        servo.write(pos);
        currentAngle = pos;
        updateLcdMotion(pos, "FLEX");
        delay(therapySpeed);
    }

    for (int pos = therapyMax; pos >= therapyMin; pos--)
    {
        servo.write(pos);
        currentAngle = pos;
        updateLcdMotion(pos, "EXTEND");
        delay(therapySpeed);
    }
}

/************************************************************
 * MAIN LOOP
 *
 * Select one mode below.
 ************************************************************/
void loop()
{
    therapyCycle();
    // potentiometerControl();
}

/************************************************************
 * CONFIGURATION OVER SERIAL
 *
 * Prompts user for therapy parameters with a 5-second
 * timeout. If no input is received, defaults are kept.
 ************************************************************/
void getTherapyParameters()
{
    unsigned long timeout = millis() + 5000;

    Serial.println("Ankle Flexor Controller Started");
    Serial.print("Enter MIN angle [");
    Serial.print(MIN_ANGLE);
    Serial.print("-");
    Serial.print(MAX_ANGLE);
    Serial.print("] (or wait 5s for default ");
    Serial.print(therapyMin);
    Serial.println("):");

    while (Serial.available() == 0)
    {
        if (millis() > timeout)
        {
            Serial.println("Timeout - using defaults");
            printParameters();
            return;
        }
    }
    therapyMin = constrain(Serial.parseInt(), MIN_ANGLE, therapyMax);

    Serial.print("Enter MAX angle [");
    Serial.print(therapyMin);
    Serial.print("-");
    Serial.print(MAX_ANGLE);
    Serial.print("] (default ");
    Serial.print(therapyMax);
    Serial.println("):");
    while (Serial.available() == 0) {}
    therapyMax = constrain(Serial.parseInt(), therapyMin, MAX_ANGLE);

    Serial.print("Enter SPEED (ms per step) [");
    Serial.print(MIN_SPEED);
    Serial.print("-");
    Serial.print(MAX_SPEED_MS);
    Serial.print("] (default ");
    Serial.print(therapySpeed);
    Serial.println("):");
    while (Serial.available() == 0) {}
    therapySpeed = constrain(Serial.parseInt(), MIN_SPEED, MAX_SPEED_MS);

    printParameters();
}

void printParameters()
{
    Serial.print("Parameters set - Min: ");
    Serial.print(therapyMin);
    Serial.print(" Max: ");
    Serial.print(therapyMax);
    Serial.print(" Speed: ");
    Serial.print(therapySpeed);
    Serial.println(" ms");
}

/************************************************************
 * LCD HELPERS
 ************************************************************/
void updateLcdPot(int target)
{
    lcd.setCursor(0, 0);
    lcd.print("Angle: ");
    lcd.print(currentAngle);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    lcd.print("Target: ");
    lcd.print(target);
    lcd.print("   ");
}

void updateLcdMotion(int angle, const char* phase)
{
    lcd.setCursor(0, 0);
    lcd.print("Pos: ");
    lcd.print(angle);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    lcd.print(phase);
    lcd.print("   ");
}

void lcdPrintTherapy(int minAngle, int maxAngle, int speed)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Min:");
    lcd.print(minAngle);
    lcd.print(" Max:");
    lcd.print(maxAngle);

    lcd.setCursor(0, 1);
    lcd.print("Speed:");
    lcd.print(speed);
    lcd.print("ms");

    delay(1500);
}
