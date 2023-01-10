#ifndef AS608_LCD_H
#define AS608_LCD_H
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include "keypad4x4.h"


void fingerInit();
void changeFinger();
uint8_t getFingerprintEnroll();
int getFingerprintIDez();

#endif