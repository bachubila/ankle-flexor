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
 *   Servo      -> D6
 *   Potentiometer -> A0
 *   LCD (16x2, 4-bit mode)
 *     RS       -> D13
 *     E        -> D12
 *     D4       -> D11
 *     D5       -> D10
 *     D6       -> D9
 *     D7       -> D8
 *
 *   Button (A1 to GND) toggles therapy pause/resume.
 *     Uses INPUT_PULLUP — no external resistor needed.
 *
 * Serial (9600 baud) is used at startup to configure
 * therapy parameters. If nothing is sent within 5 seconds,
 * default values are used.
 ************************************************************/

#include <Servo.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
Servo servo;

#define PotPin   A0
#define ServoPin 6
#define BTN_START A1

const int MIN_ANGLE = 30;
const int MAX_ANGLE = 120;
const int MIN_SPEED = 5;
const int MAX_SPEED_MS = 200;

int therapyMin = 30;
int therapyMax = 120;
int therapySpeed = 20;

// Therapy cycle state
int currentAngle = therapyMin;
bool therapyPaused = false;
int cyclePos = therapyMin;
bool cycleForward = true;
unsigned long lastStepTime = 0;

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

    pinMode(BTN_START, INPUT_PULLUP);

    Serial.begin(9600);

    cyclePos = therapyMin;
    lcdPrintTherapy(therapyMin, therapyMax, therapySpeed);
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
 * MAIN LOOP
 *
 * Non-blocking therapy cycle using millis() timing.
 * Button is checked every iteration for instant response.
 ************************************************************/
void loop()
{
    // therapyCycle();
    potentiometerControl();

    handleButton();

    // Display pause state and skip stepping
    if (therapyPaused)
    {
        lcd.setCursor(0, 0);
        lcd.print("  ** PAUSED **  ");
        return;
    }

    // Non-blocking step
    if ((millis() - lastStepTime) >= (unsigned long)therapySpeed)
    {
        if (cycleForward)
        {
            cyclePos++;
            if (cyclePos >= therapyMax)
            {
                cyclePos = therapyMax;
                cycleForward = false;
            }
        }
        else
        {
            cyclePos--;
            if (cyclePos <= therapyMin)
            {
                cyclePos = therapyMin;
                cycleForward = true;
            }
        }

        servo.write(cyclePos);
        currentAngle = cyclePos;

        if (cycleForward)
            updateLcdMotion(cyclePos, "FLEX");
        else
            updateLcdMotion(cyclePos, "EXTEND");

        lastStepTime = millis();
    }
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
 * BUTTON HANDLING
 ************************************************************/
void handleButton()
{
    static bool lastState = HIGH;
    static unsigned long lastDebounce = 0;

    bool reading = digitalRead(BTN_START);

    if (reading != lastState)
        lastDebounce = millis();

    if ((millis() - lastDebounce) > 50)
    {
        if (lastState == HIGH && reading == LOW)
            therapyPaused = !therapyPaused;
    }

    lastState = reading;
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
