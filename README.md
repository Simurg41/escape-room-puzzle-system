# Escape-Room Puzzle System

Selected Arduino firmware modules from a collaborative physical-computing project developed as part of a university course.

The complete system consisted of multiple interconnected escape-room puzzles. Each module received an enable signal from the previous puzzle and activated the next module after successful completion.

This repository focuses on the firmware modules developed as my contribution to the project.

## Project overview

The system combined:

- Arduino-based puzzle logic
- physical buttons, switches and rotary controls
- LCD and seven-segment displays
- servo-controlled locking mechanisms
- I2C input expanders
- visual status feedback
- communication between sequential puzzle modules

When a puzzle was solved, its firmware:

1. verified the player input,
2. activated the visual feedback,
3. moved a servo to unlock the mechanism,
4. enabled the next puzzle in the sequence.

## My contribution

I developed and tested the Arduino firmware for three puzzle modules:

### Binary Conversion

A multi-stage number-conversion puzzle controlled with a rotary encoder.

Players select values that are displayed using different number systems. Each correctly completed stage activates a status LED, and completion of all stages unlocks the mechanism.

Main components:

- Arduino Nano
- TM1637 four-digit display
- rotary encoder with push button
- three status LEDs
- servo motor
- next-module enable output

### Logic Gate Puzzle

A configurable digital-logic puzzle using physical input switches and gate selectors.

The firmware reads:

- five binary input switches,
- nine gate-selector configurations,
- two MCP23X17 I2C input expanders.

The puzzle is solved only when both the input pattern and the complete logic-gate configuration match the expected state.

Main components:

- Arduino Nano
- two MCP23X17 I2C expanders
- physical toggle inputs
- gate-selector inputs
- status LEDs
- servo-controlled lock
- sequential puzzle enable signal

### Ticket Code Lock

A keypad-based code-entry puzzle with visual feedback through an I2C LCD.

Players enter a sequence using the keypad. The firmware validates the input, displays success or retry feedback, and unlocks the mechanism when the correct sequence is entered.

Main components:

- Arduino Nano
- 4×4 matrix keypad
- 16×2 I2C LCD
- automatic LCD address detection
- servo-controlled lock
- input debouncing
- next-module enable output

## System architecture

Each puzzle module follows the same basic state flow:

```text
WAITING
   ↓
ENABLED BY PREVIOUS MODULE
   ↓
READ PLAYER INPUT
   ↓
VALIDATE PUZZLE STATE
   ↓
UNLOCK SERVO
   ↓
ENABLE NEXT MODULE
```

This allowed multiple independent puzzle boards to operate as one sequential escape-room system.

## Repository structure

```text
escape-room-puzzle-system/
├── firmware/
│   ├── binary-conversion/
│   │   └── BinaryConversion.ino
│   ├── logic-gate/
│   │   └── LogicGatePuzzle.ino
│   └── ticket-code-lock/
│       └── TicketCodeLock.ino
├── docs/
│   └── images/
├── .gitignore
└── README.md
```

## Hardware and libraries

### Hardware

- Arduino Nano / ATmega328P
- servo motors
- TM1637 seven-segment display
- rotary encoder
- matrix keypad
- 16×2 I2C LCD
- MCP23X17 I/O expanders
- LEDs, buttons and toggle switches

### Arduino libraries

Depending on the module, the following libraries are used:

- `Servo`
- `Wire`
- `Encoder`
- `TM1637Display`
- `Adafruit_MCP23X17`
- `Keypad`
- `LiquidCrystal_I2C`

## Running a module

1. Open the required `.ino` file in the Arduino IDE.
2. Install the libraries required by that module.
3. Select the appropriate Arduino Nano board and processor.
4. Review the pin configuration in the source file.
5. Calibrate the servo lock and unlock positions for the physical mechanism.
6. Upload the firmware to the board.
7. Use the Serial Monitor for testing and diagnostics.

## Public repository scope

This repository contains selected and sanitized firmware modules developed as my contribution to a six-person university group project.

To protect the original puzzle experience and the work of the other contributors, the public version intentionally excludes:

- original production puzzle answers,
- final access codes,
- story and hint documents,
- full PCB and manufacturing files,
- private university information,
- personal information belonging to other team members,
- assets developed exclusively by other contributors.

Example values used in the public firmware are demonstration values and are not the original production solutions.

## Project status

The original physical system was completed and presented as a working interconnected escape-room installation.

This repository serves as a technical showcase of the embedded programming, hardware integration and puzzle-state logic developed for the project.

## Skills demonstrated

- embedded C++ and Arduino development
- hardware/software integration
- digital input processing
- I2C communication
- state-machine design
- input validation and debouncing
- servo and display control
- modular system integration
- collaborative hardware development

## Author

Firmware modules presented in this repository were developed by [Ahmet Kislali](https://github.com/Simurg41) as part of a collaborative university project.

## License

No explicit open-source license has been assigned.

The repository is published for portfolio and educational presentation purposes.
