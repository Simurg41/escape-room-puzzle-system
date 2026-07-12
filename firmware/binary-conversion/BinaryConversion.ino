/*
  Binary Conversion Puzzle — Public Showcase Version

  This sanitized firmware demonstrates a three-stage number-conversion puzzle.
  The original production answers are intentionally excluded.

  Hardware:
  - Arduino Nano
  - TM1637 four-digit display
  - Rotary encoder with push button
  - Three status LEDs
  - Servo-controlled lock
  - Enable input from the previous module
  - Enable output for the next module
*/

#include <Encoder.h>
#include <Servo.h>
#include <TM1637Display.h>

// -------------------- Pins --------------------
constexpr uint8_t PIN_DISPLAY_CLK = 5;
constexpr uint8_t PIN_DISPLAY_DIO = 4;

constexpr uint8_t PIN_ENCODER_DT = 2;
constexpr uint8_t PIN_ENCODER_CLK = 3;
constexpr uint8_t PIN_ENCODER_BUTTON = 10;

constexpr uint8_t PIN_STAGE_LED_1 = 8;
constexpr uint8_t PIN_STAGE_LED_2 = 7;
constexpr uint8_t PIN_STAGE_LED_3 = 6;

constexpr uint8_t PIN_SERVO = 11;
constexpr uint8_t PIN_ENABLE_NEXT = 12;
constexpr uint8_t PIN_ENABLE_LED = 13;
constexpr uint8_t PIN_ENABLE_INPUT = A5;

// The original hardware used an active-low module-enable signal.
constexpr uint8_t NEXT_MODULE_IDLE = HIGH;
constexpr uint8_t NEXT_MODULE_ACTIVE = LOW;

// -------------------- Demonstration targets --------------------
// These values are examples and are not the original production answers.
constexpr int DEMO_TARGET_BINARY = 6;
constexpr int DEMO_TARGET_OCTAL = 12;
constexpr int DEMO_TARGET_DECIMAL = 24;

constexpr int MAX_BINARY_VALUE = 15;
constexpr int MAX_OCTAL_VALUE = 4095;
constexpr int MAX_DECIMAL_VALUE = 4095;

// -------------------- Servo --------------------
constexpr int SERVO_LOCK_POSITION = 90;
constexpr int SERVO_UNLOCK_POSITION = 0;

// -------------------- State --------------------
TM1637Display display(PIN_DISPLAY_CLK, PIN_DISPLAY_DIO);
Servo lockServo;
Encoder encoder(PIN_ENCODER_CLK, PIN_ENCODER_DT);

int currentValue = 0;
uint8_t completedStages = 0;
bool previousButtonState = LOW;
long previousEncoderPosition = -999;

// -------------------- Helpers --------------------
void showValueInBase(int value, int base) {
  long displayedValue = 0;
  long decimalPlace = 1;

  do {
    displayedValue += (value % base) * decimalPlace;
    value /= base;
    decimalPlace *= 10;
  } while (value > 0);

  display.showNumberDec(displayedValue);
}

void moveServoToLock() {
  lockServo.write(SERVO_LOCK_POSITION);
}

void moveServoToUnlock() {
  lockServo.write(SERVO_UNLOCK_POSITION);
}

void resetEncoder() {
  encoder.write(0);
  previousEncoderPosition = 0;
  currentValue = 0;
}

void completeCurrentStage() {
  switch (completedStages) {
    case 0:
      digitalWrite(PIN_STAGE_LED_1, HIGH);
      resetEncoder();
      break;

    case 1:
      digitalWrite(PIN_STAGE_LED_2, HIGH);
      resetEncoder();
      break;

    case 2:
      digitalWrite(PIN_STAGE_LED_3, HIGH);
      moveServoToUnlock();
      digitalWrite(PIN_ENABLE_NEXT, NEXT_MODULE_ACTIVE);
      break;

    default:
      break;
  }

  completedStages++;
}

void verifyCurrentValue(int expectedValue) {
  if (currentValue == expectedValue) {
    completeCurrentStage();
  }
}

void updateEncoder(int maximumValue) {
  long newPosition = encoder.read();

  if (newPosition > maximumValue * 4L) {
    newPosition = 0;
    encoder.write(newPosition);
  } else if (newPosition < 0) {
    newPosition = maximumValue * 4L;
    encoder.write(newPosition);
  }

  if (newPosition != previousEncoderPosition) {
    previousEncoderPosition = newPosition;
    currentValue = static_cast<int>(newPosition / 4L);
  }
}

bool buttonWasPressed() {
  const bool currentButtonState = digitalRead(PIN_ENCODER_BUTTON);

  if (previousButtonState == LOW && currentButtonState == HIGH) {
    delay(20);
    previousButtonState = currentButtonState;
    return true;
  }

  previousButtonState = currentButtonState;
  return false;
}

bool moduleIsEnabled() {
  return analogRead(PIN_ENABLE_INPUT) <= 500;
}

void resetPuzzle() {
  completedStages = 0;
  resetEncoder();

  digitalWrite(PIN_STAGE_LED_1, LOW);
  digitalWrite(PIN_STAGE_LED_2, LOW);
  digitalWrite(PIN_STAGE_LED_3, LOW);
  digitalWrite(PIN_ENABLE_NEXT, NEXT_MODULE_IDLE);
  digitalWrite(PIN_ENABLE_LED, LOW);

  moveServoToLock();
  showValueInBase(currentValue, 2);
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(9600);

  display.setBrightness(0x0F);

  pinMode(PIN_ENCODER_BUTTON, INPUT);
  pinMode(PIN_STAGE_LED_1, OUTPUT);
  pinMode(PIN_STAGE_LED_2, OUTPUT);
  pinMode(PIN_STAGE_LED_3, OUTPUT);
  pinMode(PIN_ENABLE_NEXT, OUTPUT);
  pinMode(PIN_ENABLE_LED, OUTPUT);
  pinMode(PIN_ENABLE_INPUT, INPUT_PULLUP);

  lockServo.attach(PIN_SERVO);
  previousButtonState = digitalRead(PIN_ENCODER_BUTTON);

  resetPuzzle();
}

void loop() {
  if (!moduleIsEnabled()) {
    digitalWrite(PIN_ENABLE_LED, LOW);
    delay(5);
    return;
  }

  digitalWrite(PIN_ENABLE_LED, HIGH);
  const bool pressed = buttonWasPressed();

  switch (completedStages) {
    case 0:
      updateEncoder(MAX_BINARY_VALUE);
      showValueInBase(currentValue, 2);
      if (pressed) {
        verifyCurrentValue(DEMO_TARGET_BINARY);
      }
      break;

    case 1:
      updateEncoder(MAX_OCTAL_VALUE);
      showValueInBase(currentValue, 8);
      if (pressed) {
        verifyCurrentValue(DEMO_TARGET_OCTAL);
      }
      break;

    case 2:
      updateEncoder(MAX_DECIMAL_VALUE);
      showValueInBase(currentValue, 10);
      if (pressed) {
        verifyCurrentValue(DEMO_TARGET_DECIMAL);
      }
      break;

    default:
      delay(50);
      break;
  }

  delay(2);
}
