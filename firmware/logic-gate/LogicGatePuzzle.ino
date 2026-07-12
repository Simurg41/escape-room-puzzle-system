/*
  Logic Gate Puzzle — Public Showcase Version

  This sanitized firmware reads:
  - Five physical binary inputs
  - Nine three-bit gate selectors
  - Two MCP23X17 I2C expanders

  The original production input pattern and gate configuration are intentionally
  excluded. The values below are demonstration targets only.
*/

#include <Adafruit_MCP23X17.h>
#include <Servo.h>
#include <Wire.h>

// -------------------- Configuration --------------------
constexpr bool INPUT_ACTIVE_LOW = false;
constexpr bool SELECTOR_ACTIVE_LOW = false;

// -------------------- Arduino pins --------------------
constexpr uint8_t PIN_ENABLE_INPUT = 4;
constexpr uint8_t PIN_ENABLE_NEXT = 5;
constexpr uint8_t PIN_STATUS_LED_1 = 10;
constexpr uint8_t PIN_STATUS_LED_2 = 11;
constexpr uint8_t PIN_SERVO = 12;

constexpr int SERVO_LOCK_POSITION = 0;
constexpr int SERVO_UNLOCK_POSITION = 90;

// This module uses an active-high next-module signal.
constexpr uint8_t NEXT_MODULE_IDLE = LOW;
constexpr uint8_t NEXT_MODULE_ACTIVE = HIGH;

// -------------------- I2C expanders --------------------
constexpr uint8_t I2C_ADDRESS_INPUTS = 0x20;
constexpr uint8_t I2C_ADDRESS_SELECTORS = 0x21;

Adafruit_MCP23X17 inputExpander;
Adafruit_MCP23X17 selectorExpander;

// -------------------- Demonstration input target --------------------
// Example only — not the original production solution.
constexpr bool DEMO_INPUT_PATTERN[5] = {true, false, true, false, true};

constexpr uint8_t INPUT_PINS[5] = {0, 1, 2, 3, 4};

// -------------------- Gate configuration --------------------
enum class GateFunction : uint8_t {
  Nand,
  Or,
  And,
  Nor,
  Xor
};

constexpr uint8_t CODE_NAND = 0b000;
constexpr uint8_t CODE_OR = 0b001;
constexpr uint8_t CODE_AND = 0b010;
constexpr uint8_t CODE_NOR = 0b011;
constexpr uint8_t CODE_XOR = 0b100;

struct BitReference {
  Adafruit_MCP23X17* expander;
  uint8_t pin;
};

struct GateConfiguration {
  BitReference bits[3];
  GateFunction target;
};

#define BIT_REF(expanderObject, expanderPin) { &(expanderObject), (expanderPin) }

// The physical pin mapping remains representative of the original module.
// The target functions below are demonstration values.
GateConfiguration gates[9] = {
  {{{BIT_REF(inputExpander, 5), BIT_REF(inputExpander, 6), BIT_REF(inputExpander, 7)}}, GateFunction::And},
  {{{BIT_REF(inputExpander, 8), BIT_REF(inputExpander, 9), BIT_REF(inputExpander, 10)}}, GateFunction::Xor},
  {{{BIT_REF(inputExpander, 11), BIT_REF(inputExpander, 12), BIT_REF(inputExpander, 13)}}, GateFunction::Or},
  {{{BIT_REF(inputExpander, 14), BIT_REF(inputExpander, 15), BIT_REF(selectorExpander, 0)}}, GateFunction::Nand},
  {{{BIT_REF(selectorExpander, 1), BIT_REF(selectorExpander, 2), BIT_REF(selectorExpander, 3)}}, GateFunction::Nor},
  {{{BIT_REF(selectorExpander, 4), BIT_REF(selectorExpander, 5), BIT_REF(selectorExpander, 6)}}, GateFunction::And},
  {{{BIT_REF(selectorExpander, 7), BIT_REF(selectorExpander, 8), BIT_REF(selectorExpander, 9)}}, GateFunction::Or},
  {{{BIT_REF(selectorExpander, 10), BIT_REF(selectorExpander, 11), BIT_REF(selectorExpander, 12)}}, GateFunction::Xor},
  {{{BIT_REF(selectorExpander, 13), BIT_REF(selectorExpander, 14), BIT_REF(selectorExpander, 15)}}, GateFunction::Nand},
};

Servo lockServo;
bool puzzleSolved = false;

// -------------------- Helpers --------------------
bool readActive(Adafruit_MCP23X17& expander, uint8_t pin, bool activeLow) {
  const bool rawValue = expander.digitalRead(pin);
  return activeLow ? !rawValue : rawValue;
}

uint8_t codeForGate(GateFunction function) {
  switch (function) {
    case GateFunction::Nand:
      return CODE_NAND;
    case GateFunction::Or:
      return CODE_OR;
    case GateFunction::And:
      return CODE_AND;
    case GateFunction::Nor:
      return CODE_NOR;
    case GateFunction::Xor:
      return CODE_XOR;
  }

  return 0;
}

uint8_t readGateCode(const GateConfiguration& gate) {
  uint8_t code = 0;

  for (uint8_t bitIndex = 0; bitIndex < 3; bitIndex++) {
    const bool active = readActive(
      *gate.bits[bitIndex].expander,
      gate.bits[bitIndex].pin,
      SELECTOR_ACTIVE_LOW
    );

    if (active) {
      code |= (1U << bitIndex);
    }
  }

  return code;
}

bool gateConfigurationIsCorrect() {
  for (uint8_t index = 0; index < 9; index++) {
    if (readGateCode(gates[index]) != codeForGate(gates[index].target)) {
      return false;
    }
  }

  return true;
}

bool inputPatternIsCorrect() {
  for (uint8_t index = 0; index < 5; index++) {
    const bool currentValue = readActive(
      inputExpander,
      INPUT_PINS[index],
      INPUT_ACTIVE_LOW
    );

    if (currentValue != DEMO_INPUT_PATTERN[index]) {
      return false;
    }
  }

  return true;
}

void setLockedState() {
  puzzleSolved = false;
  lockServo.write(SERVO_LOCK_POSITION);

  digitalWrite(PIN_ENABLE_NEXT, NEXT_MODULE_IDLE);
  digitalWrite(PIN_STATUS_LED_1, LOW);
  digitalWrite(PIN_STATUS_LED_2, LOW);
}

void playSolvedFeedback() {
  for (uint8_t iteration = 0; iteration < 5; iteration++) {
    digitalWrite(PIN_STATUS_LED_1, HIGH);
    digitalWrite(PIN_STATUS_LED_2, LOW);
    delay(120);

    digitalWrite(PIN_STATUS_LED_1, LOW);
    digitalWrite(PIN_STATUS_LED_2, HIGH);
    delay(120);
  }

  digitalWrite(PIN_STATUS_LED_1, HIGH);
  digitalWrite(PIN_STATUS_LED_2, HIGH);
}

void solvePuzzle() {
  puzzleSolved = true;
  lockServo.write(SERVO_UNLOCK_POSITION);
  digitalWrite(PIN_ENABLE_NEXT, NEXT_MODULE_ACTIVE);
  playSolvedFeedback();

  Serial.println(F("Puzzle solved."));
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(PIN_ENABLE_INPUT, INPUT);
  pinMode(PIN_ENABLE_NEXT, OUTPUT);
  pinMode(PIN_STATUS_LED_1, OUTPUT);
  pinMode(PIN_STATUS_LED_2, OUTPUT);

  lockServo.attach(PIN_SERVO);
  setLockedState();

  const bool inputsReady = inputExpander.begin_I2C(I2C_ADDRESS_INPUTS);
  const bool selectorsReady = selectorExpander.begin_I2C(I2C_ADDRESS_SELECTORS);

  if (!inputsReady || !selectorsReady) {
    Serial.println(F("MCP23X17 initialization failed. Check addresses and wiring."));
  }

  for (uint8_t pin = 0; pin < 16; pin++) {
    inputExpander.pinMode(pin, INPUT);
    selectorExpander.pinMode(pin, INPUT);
  }

  Serial.println(F("Logic gate puzzle ready."));
}

void loop() {
  const bool enabled = digitalRead(PIN_ENABLE_INPUT) == HIGH;

  if (!enabled) {
    setLockedState();
    delay(5);
    return;
  }

  digitalWrite(PIN_STATUS_LED_1, HIGH);
  digitalWrite(PIN_STATUS_LED_2, HIGH);

  if (!puzzleSolved && gateConfigurationIsCorrect() && inputPatternIsCorrect()) {
    solvePuzzle();
  }

  delay(5);
}
