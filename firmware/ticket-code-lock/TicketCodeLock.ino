/*
  Ticket Code Lock — Public Showcase Version

  This sanitized firmware demonstrates a keypad-based puzzle with:
  - A 4x4 matrix keypad
  - A 16x2 I2C LCD
  - A servo-controlled lock
  - A next-module enable output

  The original production access code is intentionally excluded.
*/

#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Wire.h>

// -------------------- Keypad --------------------
constexpr byte ROW_COUNT = 4;
constexpr byte COLUMN_COUNT = 4;

char keyMap[ROW_COUNT][COLUMN_COUNT] = {
  {'0', '1', '2', '3'},
  {'4', '5', '6', '7'},
  {'8', '9', 'A', 'B'},
  {'C', 'D', 'E', 'F'}
};

byte rowPins[ROW_COUNT] = {7, 8, 9, 10};
byte columnPins[COLUMN_COUNT] = {3, 4, 5, 6};

Keypad keypad = Keypad(
  makeKeymap(keyMap),
  rowPins,
  columnPins,
  ROW_COUNT,
  COLUMN_COUNT
);

// -------------------- Outputs --------------------
constexpr uint8_t PIN_SERVO = 11;
constexpr uint8_t PIN_ENABLE_NEXT = 12;
constexpr uint8_t PIN_ENABLE_LED = 13;

// The original hardware used an active-low next-module signal.
constexpr uint8_t NEXT_MODULE_IDLE = HIGH;
constexpr uint8_t NEXT_MODULE_ACTIVE = LOW;

constexpr int SERVO_LOCK_POSITION = 90;
constexpr int SERVO_UNLOCK_POSITION = 0;

// -------------------- Demonstration code --------------------
constexpr uint8_t CODE_LENGTH = 4;
constexpr char DEMO_TARGET_CODE[CODE_LENGTH + 1] = "1A2B";
constexpr unsigned long DEBOUNCE_MILLISECONDS = 35;

// -------------------- LCD --------------------
uint8_t detectedLcdAddress = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool lcdAvailable = false;

// -------------------- State --------------------
Servo lockServo;

char enteredCode[CODE_LENGTH + 1] = {0};
uint8_t enteredIndex = 0;
bool puzzleSolved = false;

// -------------------- Helpers --------------------
uint8_t detectLcdAddress() {
  constexpr uint8_t commonAddresses[] = {0x27, 0x3F};

  for (uint8_t address : commonAddresses) {
    Wire.beginTransmission(address);

    if (Wire.endTransmission() == 0) {
      return address;
    }
  }

  return 0;
}

void printCentered(uint8_t row, const char* message) {
  if (!lcdAvailable) {
    return;
  }

  lcd.setCursor(0, row);
  lcd.print(F("                "));

  const uint8_t length = strlen(message);
  const uint8_t startColumn = length < 16 ? (16 - length) / 2 : 0;

  lcd.setCursor(startColumn, row);
  lcd.print(message);
}

void showCodeEntry() {
  if (!lcdAvailable) {
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print(F("Enter Code:     "));

  lcd.setCursor(0, 1);
  lcd.print(F("Code: "));

  for (uint8_t index = 0; index < CODE_LENGTH; index++) {
    lcd.print(index < enteredIndex ? enteredCode[index] : '_');
  }

  lcd.print(F("      "));
}

void resetEntry() {
  enteredIndex = 0;

  for (uint8_t index = 0; index <= CODE_LENGTH; index++) {
    enteredCode[index] = '\0';
  }

  showCodeEntry();
}

bool enteredCodeIsCorrect() {
  for (uint8_t index = 0; index < CODE_LENGTH; index++) {
    if (enteredCode[index] != DEMO_TARGET_CODE[index]) {
      return false;
    }
  }

  return true;
}

void unlockPuzzle() {
  puzzleSolved = true;

  digitalWrite(PIN_ENABLE_NEXT, NEXT_MODULE_ACTIVE);
  lockServo.write(SERVO_UNLOCK_POSITION);

  if (lcdAvailable) {
    lcd.clear();
    printCentered(0, "SOLVED!");
    printCentered(1, "Unlocked");
  }

  Serial.println(F("Puzzle solved."));
}

void showWrongCodeFeedback() {
  Serial.println(F("Wrong demonstration code."));

  if (lcdAvailable) {
    printCentered(0, "WRONG CODE!");
    printCentered(1, "Try again...");
  }

  delay(1000);
  resetEntry();
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(115200);
  Wire.begin();

  keypad.setDebounceTime(DEBOUNCE_MILLISECONDS);

  pinMode(PIN_ENABLE_NEXT, OUTPUT);
  pinMode(PIN_ENABLE_LED, OUTPUT);

  digitalWrite(PIN_ENABLE_NEXT, NEXT_MODULE_IDLE);
  digitalWrite(PIN_ENABLE_LED, HIGH);

  lockServo.attach(PIN_SERVO);
  lockServo.write(SERVO_LOCK_POSITION);

  detectedLcdAddress = detectLcdAddress();

  if (detectedLcdAddress != 0) {
    lcd = LiquidCrystal_I2C(detectedLcdAddress, 16, 2);
    lcd.init();
    lcd.backlight();
    lcdAvailable = true;

    lcd.clear();
    printCentered(0, "Code Lock");
    printCentered(1, "Ready");
    delay(700);

    lcd.clear();
    showCodeEntry();
  } else {
    Serial.println(F("LCD not found. Check SDA, SCL and I2C address."));
  }

  Serial.println(F("Ticket code-lock puzzle ready."));
}

void loop() {
  if (puzzleSolved) {
    delay(50);
    return;
  }

  const char pressedKey = keypad.getKey();

  if (pressedKey == NO_KEY) {
    delay(5);
    return;
  }

  if (enteredIndex >= CODE_LENGTH) {
    resetEntry();
  }

  enteredCode[enteredIndex] = pressedKey;
  enteredIndex++;
  enteredCode[enteredIndex] = '\0';

  showCodeEntry();

  if (enteredIndex == CODE_LENGTH) {
    delay(500);

    if (enteredCodeIsCorrect()) {
      unlockPuzzle();
    } else {
      showWrongCodeFeedback();
    }
  }

  delay(30);
}
