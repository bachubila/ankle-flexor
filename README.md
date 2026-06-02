# Ankle Flexor Servo Controller

Arduino-based servo controller for ankle rehabilitation therapy, simulated in Proteus. The system moves a servo motor through a configurable range of motion to assist with dorsiflexion/plantarflexion exercises.

## Hardware

| Component | Pin |
|-----------|-----|
| Servo motor (signal) | D9 |
| Potentiometer (wiper) | A0 |
| LCD RS | D7 |
| LCD E | D8 |
| LCD D4 | D10 |
| LCD D5 | D11 |
| LCD D6 | D12 |
| LCD D7 | D13 |
| Serial (USB) | 9600 baud |

> **Note:** The LCD is a standard 16x2 character display in 4-bit mode. A 10k pot provides contrast on pin 3. Backlight powered via 5V and GND (with 220Ω resistor on anode).

## Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software) (v1.8.x or 2.x)
- [Proteus 8 Professional](https://www.labcenter.com/) (for simulation)
- Arduino AVR Boards platform (install via Boards Manager)

## Setup & Installation

### 1. Clone or download the project

```
git clone <repo-url> ankle_flexor
cd ankle_flexor
```

### 2. Open the Proteus simulation

Double-click `ankle_flexor.pdsprj` (or `servo/ServoMotor.pdsprj`) in Proteus.

### 3. Compile and load the firmware

1. Open `servo/servo.ino` in Arduino IDE
2. Select **Tools → Board → Arduino Uno**
3. Select **Tools → Port** (if uploading to real hardware)
4. Click **Verify** (✓) to compile
5. In Proteus, double-click the Arduino MCU and load the generated `.hex` file:
   - Path: `servo/build/atmel-avr-xminis.avr.atmega328p_xplained_mini/servo.ino.hex`
6. Run the simulation

> **Tip:** If building in the Arduino IDE produces the hex in a temporary folder, enable **File → Preferences → Show verbose output during: ☑ compilation**, then copy the hex path from the output.

## Usage

### Therapy Cycle Mode (default)

The servo automatically oscillates between `therapyMin` and `therapyMax` at a speed determined by `therapySpeed` (ms per step).

On startup, the serial monitor prompts for configuration:

```
Ankle Flexor Controller Started
Enter MIN angle [30-120] (or wait 5s for default 30):
```

Send three integer values over serial (one per prompt) or wait 5 seconds at the first prompt to accept defaults.

The LCD shows the configured parameters for 1.5 seconds, then displays real-time position and phase (FLEX / EXTEND).

### Potentiometer Control Mode

Comment out `therapyCycle()` and uncomment `potentiometerControl()` in `loop()`. The servo follows the potentiometer position with smoothing and a 1° deadband to reduce jitter.

## Project Structure

```
ankle_flexor/
├── servo/
│   ├── servo.ino              # Arduino firmware
│   ├── ServoMotor.pdsprj      # Proteus schematic (servo focus)
│   ├── 2x16LCDtoARDUINO.txt   # LCD wiring reference
│   ├── build/                 # Compiled hex output
│   ├── build-01/
│   ├── build-02/
│   └── Project Backups/
├── ankle_flexor.pdsprj        # Proteus project (top level)
└── README.md
```

## TODO

### High Priority

- [ ] **Start / Stop button** — physical button to pause/resume therapy cycle without resetting. Hardware: add button on A1 with 10k pull-down.
- [ ] **Angle adjustment buttons** — increase/decrease `therapyMin` and `therapyMax` on the fly. Hardware: buttons on A2 (angle+) and A3 (angle-).
- [ ] **Speed adjustment buttons** — increase/decrease `therapySpeed` during therapy. Hardware: buttons on A4 (speed+) and A5 (speed-).
- [ ] **Debounced button input** — all physical buttons need software debouncing (or hardware RC filter).
- [ ] **Non-blocking therapy loop** — convert `delay()` calls to `millis()`-based timing so the loop can poll buttons and update the LCD in real time.

### Medium Priority

- [ ] **Configuration mode toggle** — switch between potentiometer control and therapy cycle with a mode button (no code changes needed).
- [ ] **EEPROM parameter storage** — save the last-used therapy parameters so they persist across power cycles.
- [ ] **LCD update throttle** — prevent flicker by updating the LCD at max 10 Hz regardless of servo step rate.
- [ ] **Parameter bounds display** — show valid ranges on the LCD when adjusting values with buttons.

### Low Priority

- [ ] **Rotary encoder for angle/speed** — replace individual +/- buttons with a single rotary encoder for a cleaner UI.
- [ ] **Buzzer / audio feedback** — beep at cycle completion or when parameter limits are reached.
- [ ] **Data logging** — log angle over time over serial for therapy progress tracking.
- [ ] **Emergency stop** — hardware killswitch that parks the servo at the minimum angle immediately.
- [ ] **Adjustable min/max hard limits** — make `MIN_ANGLE` / `MAX_ANGLE` configurable at compile time via `#define` or EEPROM.
