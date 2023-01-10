#include <Arduino.h>
#include <string>
#include <LiquidCrystal_I2C.h>
#include "as608_lcd.h"
#include "keypad4x4.h"
#include "./DeviceStatus/DeviceStatus.h"

using namespace std;

#define MAX_WRONG_PASS 3

DeviceStatus deviceStatus;

int curCursorIndex = 0;
int countWrongPass = 0;

unsigned long loopCount;
unsigned long curTime;


const string PASSWORD = "123456";
string inputKeypad = "______";
string pass = "______";

int pin_rows1[4] = {19, 18, 5, 17};	 // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
int pin_column1[4] = {16, 4, 2, 15}; // GIOP16, GIOP4, GIOP0, GIOP2 connec

void IRAM_ATTR isr()
{
	deviceStatus.keyPress = true;
}
// Intterrupt
void initKeyPad();
void deInitKeyPad();

// Intterrupt
char getKeyInterrupt();
void display();

bool handleKeyPad(char c);

void warnWrongPassword();
void openDoor();
bool checkFinger();

void task1();
void task2();
void task3();
void task4();
void task5();
LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup()
{
	Serial.begin(9600);
	lcd.init(); // initialize the lcd
	lcd.backlight();
	fingerInit();
	loopCount = 0;
	curTime = millis();
	deviceStatus.statusChangeFinger = false;
	initKeyPad();
	display();
}

void loop()
{
	// if (checkPassword() == true)
	// {
	// 	openDoor();
	// }

	// if (countWrongPass >= MAX_WRONG_PASS)
	// {
	// 	warnWrongPassword();
	// }
	if (deviceStatus.keyPress == true)
	{
		curTime = millis();
		deInitKeyPad();
		char c = '!';
		do
		{
			c = '!';
			while (c == '!' && millis() - curTime < 5000)
			{
				c = getKey();
				delay(10);
			}
			curTime = millis();
		} while (handleKeyPad(c) == true);
		initKeyPad();
		deviceStatus.keyPress = false;
	}

	//
	if (deviceStatus.statusChangeFinger == true)
	{
		while (deviceStatus.statusChangeFinger == true)
		{
			changeFinger(deviceStatus.statusChangeFinger); /* code */
		}
	}

	checkFinger();

	delay(10);
}
void display()
{
	lcd.clear();
	lcd.setCursor(1, 0);
	lcd.print("Enter Password");
	lcd.setCursor(5, 1);
	for (char i : inputKeypad)
	{
		lcd.print(i);
	}
}

bool handleKeyPad(char c)
{
	if (curCursorIndex <= 6)
	{
		switch (c)
		{
		case '*': // Clear
			curCursorIndex--;
			// lcd.setCursor(5 + count, 1);
			// lcd.print("_");
			pass[curCursorIndex] = '_';
			inputKeypad[curCursorIndex] = '_';
			display();
			break;
		case '#': // Enter
			if (curCursorIndex == 6)
			{
				// Serial.println("Haidz");
				// // Serial.println(pass[5]);
				// Serial.print(pass.substr);

				if (pass == PASSWORD)
				{
					lcd.clear();
					lcd.setCursor(5, 0);
					lcd.print("CORRECT!");
					Serial.println("CORRECT");
					delay(2000);
					pass = "______";
					inputKeypad = "______";
					//checkPass = true;
					curCursorIndex = 0;
					countWrongPass = 0;
					return true;
				}
				else
				{
					lcd.clear();
					lcd.setCursor(5, 0);
					lcd.print("WRONG!!!");
					Serial.println("WRONG!!!");
					pass = "______";
					inputKeypad = "______";
					delay(2000);
					countWrongPass++;
					curCursorIndex = 0;
					display();
					return false;
				}
			}
			break;
		case 'A':
			if (deviceStatus.statusChangeFinger == false)
			{
				deviceStatus.statusChangeFinger = true;
			}
			else
			{
				deviceStatus.statusChangeFinger = false;
			}
			pass = "______";
			inputKeypad = "______";
			break;
		case 'B':
			break;
		case 'C':
			break;
		case 'D':
			break;
		case '!':
			pass = "______";
			inputKeypad = "______";
			display();
			return false;
		default:
			Serial.print(c);
			if (curCursorIndex < 6)
			{
				pass[curCursorIndex] = c;
				inputKeypad[curCursorIndex] = c;
				display();
				delay(200);
				inputKeypad[curCursorIndex] = '*';
				display();
				curCursorIndex++;
			}
			break;
		}
	}
	return true;
}

void openDoor()
{
	// opendoor
	deviceStatus.openDoor = false;
	curCursorIndex = 0;
	//checkPass = false;
	display();
}
void warnWrongPassword()
{
	// warn
	lcd.clear();
	lcd.setCursor(3, 0);
	lcd.print("WARNING!!!");
	lcd.setCursor(1, 1);
	lcd.print("Try after 30s!");
}

bool checkFinger()
{
	int finger_status = -1;
	finger_status = getFingerprintIDez();
	if (finger_status != -1 and finger_status != -2)
	{
		Serial.print("Match");
	}
	else
	{
		if (finger_status == -2)
		{
			for (int ii = 0; ii < 5; ii++)
			{
				Serial.print("Not Match");
			}
		}
	}
	return true;
}

void initKeyPad()
{
	for (int i = 0; i < 4; i++)
	{
		pinMode(pin_column1[i], OUTPUT);
		digitalWrite(pin_column1[i], LOW);
	}
	for (int i = 0; i < 4; i++)
	{
		pinMode(pin_rows1[i], INPUT_PULLUP);
		attachInterrupt(pin_rows1[i], isr, FALLING);
	}
}

void deInitKeyPad()
{
	for (int i = 0; i < 4; i++)
	{
		pinMode(pin_rows1[i], INPUT_PULLUP);
		detachInterrupt(pin_rows1[i]);
	}
}
char getKeyInterrupt()
{
	unsigned int timeOut = 5000;
	unsigned long newTime = millis();
	deInitKeyPad();
	char c = getKey();
	while (c == '!' && (millis() - newTime) < timeOut)
	{
		c = getKey();
		delay(10);
	}
	initKeyPad();
	delay(100);
	return c;
}