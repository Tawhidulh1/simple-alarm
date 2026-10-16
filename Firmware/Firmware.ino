#include <Adafruit_GFX.h>     // graphics library
#include <Adafruit_ST7789.h>  // driver for the ST7789 screen
#include <SPI.h>

#define TFT_SCLK D9   // SCL
#define TFT_MOSI D10  // SDA
#define TFT_RST D8    // RST
#define TFT_DC D4     // DC
#define TFT_CS D5     // CS
#define TFT_BL D6     // BL

#define BTN1 D0
#define BTN2 D1
#define BTN3 D2
#define BTN4 D3

#define BUZZER D7  

class MyST7789 : public Adafruit_ST7789 {
public:
  MyST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}
  void setOffsets(uint8_t col, uint8_t row) {
    _colstart = _colstart2 = col;
    _rowstart = _rowstart2 = row;
  }
};

MyST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

long totalSeconds = 0;
unsigned long lastTick = 0;
long alarmSeconds = 0;

const int btnPins[4] = { BTN1, BTN2, BTN3, BTN4 };
bool lastBtn[4] = { HIGH, HIGH, HIGH, HIGH };
unsigned long lastChange[4] = { 0, 0, 0, 0 };

int currentMode = 0;  // (0): Normal, (1): Set Time, (2): Set Alarm

bool isAlarmOn = false;   // ringing right now
bool isAlarmSet = false;  // armed
bool ringPhase = false;   // flips every second while ringing

bool needsRedraw = false;

// true once per physical press, ignores bounce
bool pressed(int i) {
  bool now = digitalRead(btnPins[i]);
  if (now == lastBtn[i]) return false;
  if (millis() - lastChange[i] < 30) return false;
  lastChange[i] = millis();
  lastBtn[i] = now;
  return now == LOW;
}

void drawState() {
  uint16_t bg = ST77XX_BLACK;
  uint16_t fg = ST77XX_WHITE;
  if (isAlarmOn) {
    bg = ringPhase ? ST77XX_RED : ST77XX_BLACK;
    tft.fillScreen(bg);
  }

  long shown = (currentMode == 2 && !isAlarmOn) ? alarmSeconds : totalSeconds;
  int hour = shown / 3600;
  int minute = shown % 3600 / 60;
  int second = shown % 60;
  char buff[9];
  sprintf(buff, "%02d:%02d:%02d", hour, minute, second);

  const char* mode = "NORMAL   ";
  if (isAlarmOn) mode = "WAKE UP!!";
  else if (currentMode == 1) mode = "SET TIME ";
  else if (currentMode == 2) mode = "SET ALARM";

  const char* status = isAlarmSet ? "ALARM ON " : "ALARM OFF";
  if (isAlarmOn) status = "BTN4 STOP";

  tft.setTextColor(fg, bg);

  tft.setTextSize(5);
  tft.setCursor(22, 18);
  tft.print(buff);

  tft.setTextSize(2);
  tft.setCursor(0, 1);
  tft.print(mode);

  tft.setCursor(0, 59);
  tft.print(status);
}

// setup() runs ONCE when the board powers on
void setup() {
  Serial.begin(115200);  // lets the board talk to your computer

  pinMode(TFT_BL, OUTPUT);    // Set the backlight pin mode, or just wire it to 3.3V
  digitalWrite(TFT_BL, LOW);  // Turns the backlight ON, for some reason this screen is active Low, so setting it to LOW is really HIGH

  pinMode(BTN1, INPUT_PULLUP);  // Mode cycle Normal | Set Time | Set Alarm
  pinMode(BTN2, INPUT_PULLUP);  // +1 hour
  pinMode(BTN3, INPUT_PULLUP);  // +1 minute
  pinMode(BTN4, INPUT_PULLUP);  // Alarm stop / arm-disarm button

  pinMode(BUZZER, OUTPUT);

  tft.init(76, 284);         // Our panel size (portrait)
  tft.setOffsets(82, 18);    // Offsets for the weird resolution
  tft.invertDisplay(false);  // Invert the colors (This display is flipped from normal)
  tft.setRotation(1);        // Landscape, if it's upside down use 3!

  tft.fillScreen(ST77XX_BLACK);  // clear the screen

  drawState();
  lastTick = millis();
}

void btn1Pressed() {
  currentMode++;
  if (currentMode > 2) {
    currentMode = 0;
  }
  needsRedraw = true;
}

// increment hour
void btn2Pressed() {
  if (currentMode == 1) {
    totalSeconds = (totalSeconds + 3600) % 86400;
    needsRedraw = true;
  }
  if (currentMode == 2) {
    alarmSeconds = (alarmSeconds + 3600) % 86400;
    needsRedraw = true;
  }
}

// increment minute
void btn3Pressed() {
  if (currentMode == 1) {
    totalSeconds = (totalSeconds + 60) % 86400;
    needsRedraw = true;
  }
  if (currentMode == 2) {
    alarmSeconds = (alarmSeconds + 60) % 86400;
    needsRedraw = true;
  }
}

// stop alarm if ringing, otherwise arm/disarm
void btn4Pressed() {
  if (isAlarmOn) {
    isAlarmOn = false;
    noTone(BUZZER);
    tft.fillScreen(ST77XX_BLACK);
  } else {
    isAlarmSet = !isAlarmSet;
  }
  needsRedraw = true;
}

void loop() {
  if (millis() - lastTick >= 1000) {
    lastTick += 1000;
    totalSeconds = (totalSeconds + 1) % 86400;

    if (isAlarmOn) {
      ringPhase = !ringPhase;
      if (ringPhase) tone(BUZZER, 1000);
      else noTone(BUZZER);
    } else if (isAlarmSet && totalSeconds == alarmSeconds) {
      isAlarmOn = true;
      ringPhase = true;
      tone(BUZZER, 1000);
      Serial.println("ALARM!");
    }

    if (currentMode != 2 || isAlarmOn) {
      drawState();
    }
  }

  if (pressed(0)) btn1Pressed();
  if (pressed(1)) btn2Pressed();
  if (pressed(2)) btn3Pressed();
  if (pressed(3)) btn4Pressed();

  if (needsRedraw) {
    needsRedraw = false;
    drawState();
  }
}
