#include <EEPROM.h>
#include <FastLED.h>

#define NUM_LEDS 64
#define BRIGHTNESS 30
#define R 150
#define G 150
#define B 150
#define LED_VCC 3
#define LED_GND 4
#define LED_PIN 5

CRGB leds[NUM_LEDS];


int led_style = 25;                         // 25 or 64 bits
int current_gear = 0;                       //current gear
int randomcolor = 0;                        //random color
int gear_mode = 6;                          // 5 speeds or 6 speeds
int analogPins[] = { A4, A3, A5, A6, A7 };  //U1,U2,U3,U4,U5
int dtime = 500;
int eeprom_r;
int eeprom_g;
int eeprom_b;

void setup() {
  Serial.begin(9600);  // open the serial port at 9600 bps:

  // read the data from eeprom
  eeprom_r = EEPROM.read(0);
  eeprom_g = EEPROM.read(1);
  eeprom_b = EEPROM.read(2);

  if (eeprom_r == 255 && randomcolor == 1) {
    EEPROM.update(0, 0);    //r
    EEPROM.update(1, 255);  //g
    EEPROM.update(2, 0);    //b
  } else if (eeprom_g == 255 && randomcolor == 1) {
    EEPROM.update(0, 0);    //r
    EEPROM.update(1, 0);    //g
    EEPROM.update(2, 255);  //b
  } else if (randomcolor == 1) {
    EEPROM.update(0, 255);  //r
    EEPROM.update(1, 0);    //g
    EEPROM.update(2, 0);    //b
  }

  if (randomcolor == 1) {
    // read the color data from eeprom
    eeprom_r = EEPROM.read(0);
    eeprom_g = EEPROM.read(1);
    eeprom_b = EEPROM.read(2);

  } else {
    // fixed color data
    EEPROM.update(0, 0);    //r
    EEPROM.update(1, 255);  //g
    EEPROM.update(2, 0);    //b
  }

  pinMode(LED_VCC, OUTPUT);
  pinMode(LED_GND, OUTPUT);

  digitalWrite(LED_VCC, HIGH);
  digitalWrite(LED_GND, LOW);

  FastLED.addLeds<WS2812, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  // calculate the current gear
  // hall sensors are arranged like this
  //
  //   A4(U1)         A6(U4)
  //      1     3     5
  //      │     │     │
  //      ├─────┼─────┼─────┼
  //      │     │     │
  //      2     4     6     R
  //   A3(U2)         A5(U3)    A7(U5)
  int hall_1_value = analogRead(analogPins[0]);  // read the analog value on pin A4, value will be between 0 - 1023
  int hall_2_value = analogRead(analogPins[1]);  // read the analog value on pin A1, value will be between 0 - 1023
  int hall_3_value = analogRead(analogPins[2]);  // read the analog value on pin A2, value will be between 0 - 1023
  int hall_4_value = analogRead(analogPins[3]);  // read the analog value on pin A3, value will be between 0 - 1023
  int hall_5_value = analogRead(analogPins[4]);  // read the analog value on pin A3, value will be between 0 - 1023

  int percentage_value_1 = round(abs(hall_1_value - 512) / 5.12);  // convert hall value to range 0-100%
  int percentage_value_2 = round(abs(hall_2_value - 512) / 5.12);  // convert hall value to range 0-100%
  int percentage_value_3 = round(abs(hall_3_value - 512) / 5.12);  // convert hall value to range 0-100%
  int percentage_value_4 = round(abs(hall_4_value - 512) / 5.12);  // convert hall value to range 0-100%
  int percentage_value_5 = round(abs(hall_5_value - 512) / 5.12);  // convert hall value to range 0-100%

  if (percentage_value_1 > 22 && percentage_value_5 > 8) {  // 1 gear
    current_gear = 1;
  } else if (percentage_value_1 > 20 && percentage_value_4 > 5) {  // 3 gear
    current_gear = 3;
  } else if (percentage_value_4 > 10) {  // 5 gear
    current_gear = 5;
  } else if (percentage_value_2 > 5 && percentage_value_3 < 5) {  // 2 gear
    current_gear = 2;
  } else if (percentage_value_2 > 4 && percentage_value_2 < 10 && percentage_value_3 > 4 && percentage_value_3 < 8) {  // 4 gear
    current_gear = 4;
  } else if (percentage_value_1 > 10 && percentage_value_5 > 10) {  // R gear
    current_gear = 7;
  } else if (percentage_value_2 < 8 && percentage_value_3 > 5) {  // 6 gear
    if (gear_mode == 6) {
      current_gear = 6;
    } else {
      current_gear = 7;
    }
  } else {  // n gear
    current_gear = 0;
  }

  Serial.println("==============================");
  Serial.print("Current Gear=");
  Serial.println(current_gear);
  Serial.print("U1=");
  Serial.println(percentage_value_1);
  Serial.print("U2=");
  Serial.println(percentage_value_2);
  Serial.print("U3=");
  Serial.println(percentage_value_3);
  Serial.print("U4=");
  Serial.println(percentage_value_4);
  Serial.print("U5=");
  Serial.println(percentage_value_5);
  Serial.println("==============================");

  boolean debug = 0;
  if (debug) {
    delay(1000);
  }

  int* gear = nullptr;
  int size = 0;

  switch (current_gear) {
    case 0:
      if (led_style == 25) {
        static int gear25[] = { 0, 4, 5, 6, 9, 10, 12, 14, 15, 18, 19, 20, 24 };  // 5 x 5
        gear = gear25;
        size = sizeof(gear25) / sizeof(gear25[0]);
      } else {
        static int gear64[] = { 1, 6, 9, 10, 14, 17, 19, 22, 25, 27, 30, 33, 36, 38, 41, 44, 46, 49, 53, 54, 57, 62 };  // 8 x 8
        gear = gear64;
        size = sizeof(gear64) / sizeof(gear64[0]);
      }
      break;

    case 1:
      if (led_style == 25) {
        static int gear25[] = { 1, 2, 3, 7, 12, 17, 18, 22 };  // 5 x 5
        gear = gear25;
        size = sizeof(gear25) / sizeof(gear25[0]);
      } else {
        static int gear64[] = { 2, 3, 4, 11, 19, 27, 35, 43, 51, 52, 59 };  // 8 x 8
        gear = gear64;
        size = sizeof(gear64) / sizeof(gear64[0]);
      }
      break;

    case 2:
      if (led_style == 25) {
        static int gear25[] = { 1, 2, 3, 8, 11, 12, 13, 16, 21, 22, 23 };  // 5 x 5
        gear = gear25;
        size = sizeof(gear25) / sizeof(gear25[0]);
      } else {
        static int gear64[] = { 1, 2, 3, 4, 5, 6, 14, 21, 26, 27, 28, 33, 41, 49, 54, 58, 59, 60, 61 };  // 8 x 8
        gear = gear64;
        size = sizeof(gear64) / sizeof(gear64[0]);
      }
      break;

    case 3:
      if (led_style == 25) {
        static int gear25[] = { 1, 2, 3, 6, 11, 12, 13, 16, 21, 22, 23 };  // 5 x 5
        gear = gear25;
        size = sizeof(gear25) / sizeof(gear25[0]);
      } else {
        static int gear64[] = { 2, 3, 4, 5, 9, 14, 17, 25, 34, 35, 36, 41, 49, 54, 58, 59, 60, 61 };  // 8 x 8
        gear = gear64;
        size = sizeof(gear64) / sizeof(gear64[0]);
      }
      break;

    case 4:
      if (led_style == 25) {
        static int gear25[] = { 3, 6, 11, 12, 13, 16, 18, 21, 23 };  // 5 x 5
        gear = gear25;
        size = sizeof(gear25) / sizeof(gear25[0]);
      } else {
        static int gear64[] = { 2, 10, 17, 18, 19, 20, 21, 22, 26, 30, 34, 37, 42, 44, 50, 51, 58 };  // 8 x 8
        gear = gear64;
        size = sizeof(gear64) / sizeof(gear64[0]);
      }
      break;

    case 5:
      if (led_style == 25) {
        static int gear25[] = { 1, 2, 3, 6, 11, 12, 13, 18, 21, 22, 23 };  // 5 x 5
        gear = gear25;
        size = sizeof(gear25) / sizeof(gear25[0]);
      } else {
        static int gear64[] = { 2, 3, 4, 5, 9, 14, 17, 25, 34, 35, 36, 37, 38, 46, 54, 57, 58, 59, 60, 61, 62 };  // 8 x 8
        gear = gear64;
        size = sizeof(gear64) / sizeof(gear64[0]);
      }
      break;

    case 6:
      if (led_style == 25) {
        static int gear25[] = { 1, 2, 3, 6, 8, 11, 12, 13, 18, 21 };  // 5 x 5
        gear = gear25;
        size = sizeof(gear25) / sizeof(gear25[0]);
      } else {
        static int gear64[] = { 2, 3, 4, 5, 9, 14, 17, 22, 25, 30, 34, 35, 36, 37, 38, 46, 49, 54, 58, 59, 60, 61 };  // 8 x 8
        gear = gear64;
        size = sizeof(gear64) / sizeof(gear64[0]);
      }
      break;

    case 7:
      if (led_style == 25) {
        static int gear25[] = { 0, 4, 6, 9, 10, 11, 12, 13, 15, 19, 20, 21, 22, 23 };  // 5 x 5
        gear = gear25;
        size = sizeof(gear25) / sizeof(gear25[0]);
      } else {
        static int gear64[] = { 1, 6, 10, 14, 19, 22, 28, 30, 34, 35, 36, 37, 38, 41, 46, 49, 54, 58, 59, 60, 61, 62 };  // 8 x 8
        gear = gear64;
        size = sizeof(gear64) / sizeof(gear64[0]);
      }
      break;
      // Add more cases here for other gears

    default:
      // Optionally, you can add a default case to handle unexpected values
      break;
  }

    FastLED.clear();


    // Set LED colors based on the current gear pattern
    for (int i = 0; i < size; i++) {
      if (gear[i] < NUM_LEDS) {
        leds[gear[i]] = CRGB(eeprom_g, eeprom_r, eeprom_b);
      }
    }

    FastLED.show();
 
  delay(dtime);

}
