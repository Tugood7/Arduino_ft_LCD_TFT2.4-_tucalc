#include <Arduino.h>
#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>

#define YP A1  
#define XM A2  
#define YM 7   
#define XP 6

// Calibrate values
#define TS_MINX 125
#define TS_MINY 15
#define TS_MAXX 965
#define TS_MAXY 905

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define MINPRESSURE 10
#define MAXPRESSURE 1000

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

String Key[4][4] = {
  { "7", "8", "9", "/" },
  { "4", "5", "6", "*" },
  { "1", "2", "3", "-" },
  { "C", "0", "=", "+" }
};

String N1, N2, ShowSC, opt;
bool updata = false;
float answers = -1;



// Fungsi ditaruh di sini agar loop() mengenalnya
TSPoint waitTouch() {

  TSPoint p;

  // Tunggu sampai layar disentuh
  do {
    p = ts.getPoint();

    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

  } while (p.z < MINPRESSURE || p.z > MAXPRESSURE);

  // Mapping koordinat
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);

  // Batasi agar tidak keluar layar
  p.x = constrain(p.x, 0, 239);
  p.y = constrain(p.y, 0, 319);

  // Tunggu sampai jari dilepas
  while (true) {

    TSPoint r = ts.getPoint();

    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (r.z < MINPRESSURE)
      break;

    delay(10);
  }

  return p;
}

void setup() {
  Serial.begin(9600);

  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(2);

  // ==========================
  // SPLASH SCREEN TUCALC
  // ==========================
  tft.fillScreen(BLACK);

  // Border
  tft.drawRect(5, 5, 230, 310, CYAN);

  // Logo
  tft.setTextColor(CYAN);
  tft.setTextSize(4);
  tft.setCursor(30, 60);
  tft.print("TuCalc");

  // Garis pemisah
  tft.drawFastHLine(25, 110, 190, CYAN);

  // Slogan
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(25, 140);
  tft.print("Smart Calculator");

  // Versi
  tft.setTextColor(YELLOW);
  tft.setCursor(55, 180);
  tft.print("Version 1.0");

  // Author
  tft.setTextColor(GREEN);
  tft.setCursor(35, 220);
  tft.print("by Tubagus DF");

  // Loading Bar
  tft.drawRect(30, 270, 180, 20, WHITE);

  for (int i = 0; i <= 178; i += 6) {
    tft.fillRect(31, 271, i, 18, CYAN);
    delay(50);
  }

  delay(800);

  // ==========================
  // HALAMAN KALKULATOR
  // ==========================
  tft.fillScreen(BLACK);

  // Area Display
  tft.fillRect(0, 0, 240, 80, BLACK);

  // Tombol
  tft.fillRect(0, 80, 240, 240, WHITE);

  // Garis Horizontal
  tft.drawFastHLine(0, 80, 240, BLACK);
  tft.drawFastHLine(0, 140, 240, BLACK);
  tft.drawFastHLine(0, 200, 240, BLACK);
  tft.drawFastHLine(0, 260, 240, BLACK);
  tft.drawFastHLine(0, 319, 240, BLACK);

  // Garis Vertical
  tft.drawFastVLine(0, 80, 240, BLACK);
  tft.drawFastVLine(60, 80, 240, BLACK);
  tft.drawFastVLine(120, 80, 240, BLACK);
  tft.drawFastVLine(180, 80, 240, BLACK);
  tft.drawFastVLine(239, 80, 240, BLACK);

  // Judul kecil di atas display
  tft.setTextColor(CYAN);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.print("TuCalc");

  // Tombol angka
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {

      tft.setCursor(22 + (60 * x), 100 + (60 * y));
      tft.setTextSize(3);

      // Operator diberi warna berbeda
      if (Key[y][x] == "+" ||
          Key[y][x] == "-" ||
          Key[y][x] == "*" ||
          Key[y][x] == "/" ||
          Key[y][x] == "=") {
        tft.setTextColor(BLUE);
      }
      else if (Key[y][x] == "C") {
        tft.setTextColor(RED);
      }
      else {
        tft.setTextColor(BLACK);
      }

      tft.print(Key[y][x]);
    }
  }
}
  
void loop() {
  TSPoint p = waitTouch();
  updata = false;

  for (int i1 = 0; i1 < 4; i1++) {
    for (int i2 = 0; i2 < 4; i2++) {

      const int margin = 5;

        if ((p.y >= 240 - ((i1 + 1) * 60) + margin &&
        p.y <= 240 - (i1 * 60) - margin) &&
        (p.x >= (i2 * 60) + margin &&
        p.x <= ((i2 + 1) * 60) - margin)) {

        String key = Key[i1][i2];

        // =========================
        // TOMBOL ANGKA
        // =========================
        if ((i1 <= 2 && i2 <= 2) || (i1 == 3 && i2 == 1)) {

          // Jika sebelumnya baru selesai menghitung
          if (answers != -1 && N1 == "" && N2 == "" && opt == "") {
            answers = -1;
            ShowSC = "";
          }

          if (opt == "") {
            N1 += key;
            ShowSC = N1;
          } else {
            N2 += key;
            ShowSC = N1 + " " + opt + " " + N2;
          }

          updata = true;
        }

        // =========================
        // CLEAR
        // =========================
        else if (key == "C") {

          N1 = "";
          N2 = "";
          opt = "";
          ShowSC = "";
          answers = -1;

          updata = true;
        }

        // =========================
        // OPERATOR
        // =========================
        else if (key == "+" || key == "-" ||
                 key == "*" || key == "/") {

          // Gunakan hasil sebelumnya
          if (N1 == "" && answers != -1) {

            if (answers == (int)answers)
              N1 = String((int)answers);
            else
              N1 = String(answers, 2);
          }

          if (N1 != "") {
            opt = key;
            ShowSC = N1 + " " + opt + " ";
            updata = true;
          }
        }

        // =========================
        // EQUAL
        // =========================
        else if (key == "=") {

          if (N1 != "" && N2 != "" && opt != "") {

            float num1 = N1.toFloat();
            float num2 = N2.toFloat();

            if (opt == "+")
              answers = num1 + num2;

            else if (opt == "-")
              answers = num1 - num2;

            else if (opt == "*")
              answers = num1 * num2;

            else if (opt == "/") {
              if (num2 != 0)
                answers = num1 / num2;
              else
                answers = 0;
            }

            // Tampilkan hasil
            if (answers == (int)answers)
              ShowSC = String((int)answers);
            else
              ShowSC = String(answers, 2);

            // Reset operand
            N1 = "";
            N2 = "";
            opt = "";

            updata = true;
          }
        }
      }
    }
  }

  // =========================
  // UPDATE LCD
  // =========================
  if (updata) {

    tft.fillRect(0, 0, 240, 80, BLACK);

    tft.setCursor(10, 20);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print(ShowSC);
  }

  delay(250);
}