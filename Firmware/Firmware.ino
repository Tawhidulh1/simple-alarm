#include <Adafruit_GFX.h> // graphics library
#include <Adafruit_ST7789.h> // driver for the ST7789 screen
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

// Fix setColRowStart() by exposing it via a subclass
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

// setup() runs ONCE when the board powers on
void setup() {
  Serial.begin(115200); // lets the board talk to your computer

  pinMode(TFT_BL, OUTPUT); // Set the backlight pin mode, or just wire it to 3.3V
  digitalWrite(TFT_BL, LOW); // Turns the backlight ON, for some reason this screen is active Low, so setting it to LOW is really HIGH

  pinMode(BTN1, INPUT_PULLUP); // Mode cycle
  pinMode(BTN2, INPUT_PULLUP); // +1 hour
  pinMode(BTN3, INPUT_PULLUP); // +1 minute 
  pinMode(BTN4, INPUT_PULLUP); // Alarm snooze/stop

  tft.init(76, 284); // Our panel size (portrait)
  tft.setOffsets(82, 18); // Offsets for the weird resolution
  tft.invertDisplay(false); // Invert the colors (This display is flipped from normal)
  tft.setRotation(1); // Landscape, if it's upside down use 3!

  tft.fillScreen(ST77XX_BLACK); // clear the screen

  drawTime();
  lastTick = millis();
}


void drawTime() {
  int hour = totalSeconds / 3600;
  int minute = totalSeconds % 3600 / 60;
  int second = totalSeconds % 60;
  char buff[9];
  sprintf(buff, "%02d:%02d:%02d", hour, minute, second);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(5);
  tft.setCursor(22,18);
  tft.print(buff);
}

void loop() {
  if (millis() - lastTick >= 1000) {
    lastTick += 1000;
    totalSeconds = (totalSeconds + 1) % 86400; // increments totalSeconds every second and wrap it to midnight.
    drawTime();
  }

  if(digitalRead(BTN1) == LOW) {

  }
  if(digitalRead(BTN2) == LOW) {

  }
  if(digitalRead(BTN3) == LOW) {

  }
  if(digitalRead(BTN4) == LOW) {

  }

}
