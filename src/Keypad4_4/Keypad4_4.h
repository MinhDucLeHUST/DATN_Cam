#ifndef KEYPAD4X4_H
#define KEYPAD4X4_H

#include <Arduino.h>

#define ROW_NUM 4    // four rows
#define COLUMN_NUM 4 // four columns

char getKey();
uint8_t readnumber(void);

#endif