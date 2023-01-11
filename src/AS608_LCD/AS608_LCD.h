#ifndef AS608_LCD_H
#define AS608_LCD_H

#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include "../Keypad4_4/Keypad4_4.h"

#define FINGER_SERIAL Serial1
#define DEBUG

void fingerInit();
bool changeFinger(bool &statusFinger, int timeOut = 10000);
uint8_t getFingerprintEnroll();
int getFingerprintIDez();

#endif