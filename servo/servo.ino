/************************************************************
 * Ankle Flexor Servo Controller
 *
 * Controls a servo motor for ankle rehabilitation therapy.
 *
 * Modes:
 *   - Therapy Cycle (default)
 *   - Potentiometer Control (optional)
 *
 * LCD shows:
 *   Dorsi-flexing
 *   Plantar-flexing
 *   Idle
 ************************************************************/

#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD (I2C address 0x27, 20 cols, 4 rows)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Servo
Servo servo;

// Pin Definitions
#define PotPin A0
#define ServoPin 6
#define LED_PIN 2
#define BUZZER_PIN 3
#define BTN_START A1

// MG995 Servo Configuration
const int SERVO_CENTER = 90;
const int SERVO_RANGE  = 60;

const int MIN_ANGLE = SERVO_CENTER - SERVO_RANGE;   //30°
const int MAX_ANGLE = SERVO_CENTER + SERVO_RANGE;   //150°

const int MIN_SPEED = 5;
const int MAX_SPEED_MS = 200;

// Default therapy settings
int therapyMin = 50;
int therapyMax = 130;
int therapySpeed = 20;

int currentAngle = SERVO_CENTER;

bool therapyPaused = false;
int btnState = HIGH;

bool alertActive = false;
unsigned long alertStartTime = 0;

int lastPotValue = 0;
const int POT_THRESHOLD = 10;  // Minimum ADC change to count as movement

/************************************************************
 * SETUP
 ************************************************************/
void setup() {
  currentAngle = SERVO_CENTER;

  servo.attach(ServoPin);
  servo.write(currentAngle);

  lcd.init();
  lcd.backlight();
  lcd.print("Ankle Therapy");
  lcd.setCursor(0, 1);
  lcd.print("Initializing");

  pinMode(BTN_START, INPUT_PULLUP);

  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  lastPotValue = analogRead(PotPin);

  getTherapyParameters();

  delay(1500);
  lcd.clear();
}

/************************************************************
 * POTENTIOMETER MODE
 ************************************************************/
void potentiometerControl() {
  uint16_t potValue = analogRead(PotPin);

  int targetAngle = map(
    potValue,
    0,
    1023,
    MIN_ANGLE,
    MAX_ANGLE);

  if (abs(targetAngle - currentAngle) > 1) {
    if (currentAngle < targetAngle)
      currentAngle++;
    else
      currentAngle--;

    servo.write(currentAngle);
  }

  lcd.setCursor(0, 0);
  lcd.print("Manual Control ");

  lcd.setCursor(0, 1);
  lcd.print("Positioning    ");

  delay(15);
}

/************************************************************
 * THERAPY MODE
 ************************************************************/
void moveAndWait(int pos, const char* phase) {
  servo.write(pos);
  currentAngle = pos;
  updateLcdMotion(phase);

  unsigned long t0 = millis();
  while (millis() - t0 < (unsigned long)therapySpeed) {
    handleButton();
    checkPotMovement();
    updateAlert();
    while (therapyPaused)
      pauseLoop();
    delay(5);
  }
}

void therapyCycle() {
  lcdPrintTherapy();
  lcdPrintTherapyInfo();

  for (int pos = therapyMin; pos <= therapyMax; pos++)
    moveAndWait(pos, "DORSI");

  for (int pos = therapyMax; pos >= therapyMin; pos--)
    moveAndWait(pos, "PLANTAR");
}

/************************************************************
 * LOOP
 ************************************************************/
void loop() {
  therapyCycle();

  // Uncomment for manual potentiometer mode
  // potentiometerControl();
}

/************************************************************
 * SERIAL CONFIGURATION
 ************************************************************/
// Reads a line from Serial with timeout (ms). Returns empty string on timeout.
String readSerialLine(unsigned long timeoutMs) {
  unsigned long start = millis();
  String line = "";
  while (millis() - start < timeoutMs) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') {
        if (line.length() > 0)
          break;
      } else if (c != '\r') {
        line += c;
      }
    }
  }
  return line;
}

void getTherapyParameters() {
  Serial.println("Ankle Therapy Controller");
  delay(100);
  while (Serial.available()) Serial.read();  // flush stale data

  // --- MIN ---
  Serial.print("Enter MIN angle (");
  Serial.print(MIN_ANGLE);
  Serial.print("-");
  Serial.print(MAX_ANGLE);
  Serial.print(") Default=");
  Serial.println(therapyMin);

  String input = readSerialLine(5000);
  if (input.length() > 0)
    therapyMin = constrain(input.toInt(), MIN_ANGLE, therapyMax);

  // --- MAX ---
  Serial.print("Enter MAX angle (");
  Serial.print(therapyMin);
  Serial.print("-");
  Serial.print(MAX_ANGLE);
  Serial.print(") Default=");
  Serial.println(therapyMax);

  input = readSerialLine(5000);
  if (input.length() > 0)
    therapyMax = constrain(input.toInt(), therapyMin, MAX_ANGLE);

  // --- SPEED ---
  Serial.print("Enter Speed (");
  Serial.print(MIN_SPEED);
  Serial.print("-");
  Serial.print(MAX_SPEED_MS);
  Serial.print(" ms) Default=");
  Serial.println(therapySpeed);

  input = readSerialLine(5000);
  if (input.length() > 0)
    therapySpeed = constrain(input.toInt(), MIN_SPEED, MAX_SPEED_MS);

  printParameters();
}

void printParameters() {
  Serial.println();

  Serial.print("Therapy Minimum : ");
  Serial.println(therapyMin);

  Serial.print("Therapy Maximum : ");
  Serial.println(therapyMax);

  Serial.print("Therapy Speed   : ");
  Serial.print(therapySpeed);
  Serial.println(" ms");

  Serial.println();
}

/************************************************************
 * BUTTON HANDLING
 ************************************************************/
void handleButton() {
  int reading = digitalRead(BTN_START);

  if (reading == LOW && btnState == HIGH) {
    therapyPaused = !therapyPaused;
    Serial.print("Therapy ");
    Serial.println(therapyPaused ? "PAUSED" : "RESUMED");
  }

  btnState = reading;
}

void pauseLoop() {
  servo.write(currentAngle);
  checkPotMovement();
  updateAlert();
  lcd.setCursor(0, 0);
  lcd.print("* THERAPY STOP *");

  lcd.setCursor(0, 1);
  lcd.print("by therapist   ");

  handleButton();

  delay(10);
}

/************************************************************
 * LCD FUNCTIONS
 ************************************************************/
void updateLcdMotion(const char* phase) {
  lcd.setCursor(0, 0);
  lcd.print("Ankle Therapy  ");

  lcd.setCursor(0, 1);

  if (strcmp(phase, "DORSI") == 0) {
    lcd.print("Dorsi-flexing ");
  } else if (strcmp(phase, "PLANTAR") == 0) {
    lcd.print("Plantar-flex  ");
  } else {
    lcd.print("Idle          ");
  }
}

void lcdPrintTherapy() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Ankle Therapy   ");
  lcd.setCursor(0, 1);
  lcd.print("Starting...     ");

  for (int i = 0; i < 150; i++) {
    updateAlert();
    delay(10);
  }
}

void lcdPrintTherapyInfo() {
  lcd.setCursor(0, 2);
  lcd.print("Speed - ");
  lcd.print(therapySpeed);
  lcd.print(" ms         ");

  lcd.setCursor(0, 3);
  lcd.print("min-");
  lcd.print(therapyMin);
  lcd.print(" max-");
  lcd.print(therapyMax);
  lcd.print("       ");
}

void checkPotMovement() {
  int potValue = analogRead(PotPin);

  if (!alertActive && abs(potValue - lastPotValue) > POT_THRESHOLD) {
    alertActive = true;
    alertStartTime = millis();

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  }

  lastPotValue = potValue;
}

void updateAlert() {
  if (alertActive && millis() - alertStartTime >= 1000) {
    alertActive = false;

    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}