#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "as608_lcd.h"

// #include "keypad4x4.h"

#define MAX_WRONG_PASS 3

int count = 0;
int countWrongPass = 0;
unsigned long loopCount;
unsigned long time;
bool checkPass = false;
bool checkOpenDoor = false;
String msg;
String Password = "123456";
bool statusChangeFinger = false;
String inputKeypad = "______";

char getKey();
void display();
bool enterPassword(char c);
bool checkPassword();
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
	// Serial2.setPins(RX_PIN, TX_PIN);
	fingerInit();
	loopCount = 0;
	time = millis();
	msg = "";
	statusChangeFinger = true;
	// display();
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

	if (statusChangeFinger == true)
	{
		while (statusChangeFinger == true)
		{
			changeFinger(); /* code */
		}
	}
	// if (checkFinger() == true)
	// {
	// 	openDoor();
	// }

	delay(100);
}
void display()
{
	// Serial.print("Enter Password: ");
	lcd.clear();
	lcd.setCursor(1, 0);
	lcd.print("Enter Password");
	lcd.setCursor(5, 1);
	lcd.print(inputKeypad);
	// lcd.print("______");
}
bool checkPassword()
{
	char c = getKey();
	if (c != '!')
	{
		enterPassword(c);
	}
	if (checkPass == true)
	{
		return true;
	}

	return false;
}

bool enterPassword(char c)
{
	if (count <= 6)
	{
		switch (c)
		{
		case '*': // Clear
			count--;

			// lcd.setCursor(5 + count, 1);
			// lcd.print("_");
			inputKeypad[count] = '-';
			break;
		case '#': // Enter
			if (count == 6)
			{
				// Serial.println(pass);
				if (Password.indexOf(inputKeypad) != -1)
				{
					lcd.clear();
					lcd.setCursor(5, 0);
					lcd.print("CORRECT!");
					checkPass = true;
					countWrongPass = 0;
					return true;
				}
				else
				{
					lcd.clear();
					lcd.setCursor(5, 0);
					lcd.print("WRONG!!!");
					delay(1000);
					countWrongPass++;
					count = 0;
					display();
					return false;
				}
			}
			break;
		case 'A':
			if (statusChangeFinger == false)
			{
				statusChangeFinger = true;
			}
			else
			{
				statusChangeFinger = false;
			}
			break;
		case 'B':
			break;
		case 'C':
			break;
		case 'D':
			break;
		default:
			if (count < 6)
			{
				inputKeypad[count] = c;
				lcd.setCursor(5 + count, 1);
				lcd.print(c);
				delay(500);
				lcd.setCursor(5 + count, 1);
				lcd.print("*");
				// lcd.blink();
				count++;
			}
			break;
		}
	}
	return false;
}

void openDoor()
{
	// opendoor

	checkOpenDoor = false;
	count = 0;
	checkPass = false;
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
