#include "Keypad4_4.h"

char keys[ROW_NUM][COLUMN_NUM] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
int pin_rows[ROW_NUM] = {19, 18, 5, 17};     // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
int pin_column[COLUMN_NUM] = {16, 4, 2, 15}; // GIOP16, GIOP4, GIOP0, GIOP2 connec

uint8_t readnumber(void)
{
    uint8_t num = 0;

    while (num == 0)
    {
        while (!Serial.available())
            ;
        num = Serial.parseInt();
    }
    return num;
}

char getKey()
{
    delay(20);
    for (int i = 0; i < ROW_NUM; i++)
    {
        pinMode(pin_rows[i], INPUT_PULLUP);
        pinMode(pin_column[i], OUTPUT);
    }
    for (int i = 0; i < COLUMN_NUM; i++)
    {
        for (int j = 0; j < COLUMN_NUM; j++)
        {
            digitalWrite(pin_column[j], HIGH);
        }
        digitalWrite(pin_column[i], LOW);
        for (int j = 0; j < ROW_NUM; j++)
        {
            if (digitalRead(pin_rows[j]) == 0)
            {
                while (digitalRead(pin_rows[j]) == 0)
                    ;
                return keys[i][j];
            }
        }
    }
    return '!';
}
