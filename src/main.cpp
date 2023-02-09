#include <Arduino.h>
#include <string>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "./AS608_LCD/AS608_LCD.h"
#include "./Keypad4_4/Keypad4_4.h"
#include "./DeviceStatus/DeviceStatus.h"

using namespace std;

#define debug
#define MAX_WRONG_PASS 3
#define MAX_FINGER_ID 5
#define INPUT_KEYPAD_DEFAULT "______"
DeviceStatus deviceStatus;

int curCursorIndex = 0;
int countWrongPass = 0;

unsigned long loopCount;
unsigned long curTime;

const string PASSWORD = "123456";
string inputKeypad = INPUT_KEYPAD_DEFAULT;
string pass = INPUT_KEYPAD_DEFAULT;

int pin_rows1[4] = {19, 18, 5, 17};	 // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
int pin_column1[4] = {16, 4, 2, 15}; // GIOP16, GIOP4, GIOP0, GIOP2 connect

int fingerId[MAX_FINGER_ID][2] = {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}};

static const int servoPin = 27;

int fingerIdOpenDoor = 0;
bool statusBuzzer = false;

void IRAM_ATTR isr()
{
	deviceStatus.keyPress = true;
}
// Intterrupt
Servo servo1;
void initKeyPad();
void deInitKeyPad();

// Intterrupt
char getKeyInterrupt();
void lcdDisplayEnterPass();
void lcdDisplayOpenDoor();

void handleKeyPress();
bool handleKeyPad(char c);
bool handleFinger();

void warnWrongPassword();
void openDoor();
int checkFinger();

bool receiverFromGetway();
void handleDataReceiver();

LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup()
{
	Serial.	begin(9600);
	pinMode(33,OUTPUT);
	pinMode(25,OUTPUT);
	digitalWrite(33,LOW);
	digitalWrite(25,LOW);
	lcd.init(); // initialize the lcd
	lcd.backlight();
	fingerInit();
	loopCount = 0;
	curTime = millis();
	// deviceStatus.statusChangeFinger = false;
	servo1.attach(servoPin);
	initKeyPad();
	lcdDisplayEnterPass();
	servo1.write(0);
}

void loop()
{

	if (deviceStatus.keyPress == true)
	{
		handleKeyPress();
	}

	if (checkFinger() != false)
	{
		handleFinger();
	}

	if (deviceStatus.openDoor == true)
	{
		openDoor();
		lcdDisplayEnterPass();
	}

	if (deviceStatus.statusChangeFinger == true)
	{
		curTime = millis();
		while (deviceStatus.statusChangeFinger == true && millis() - curTime < 10000)
		{
			deviceStatus.statusChangeFinger = changeFinger(deviceStatus.statusChangeFinger); /* code */
		}
		curTime = millis();
		deviceStatus.statusChangeFinger = false;
		// lcdDisplayEnterPass();
	}

	if (receiverFromGetway)
	{
		handleDataReceiver();
	}

	delay(10);
}
void lcdDisplayEnterPass()
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

void lcdDisplayOpenDoor()
{
	lcd.clear();
	lcd.setCursor(0, 5);
	lcd.print("OPEN!!!");
}

void handleKeyPress()
{
	curTime = millis();
	deInitKeyPad();
	char c = '!';
	do
	{
		c = '!';
		while (c == '!')
		{
			if (millis() - curTime < 5000)
			{
				c = getKey();
				delay(10);
			}
			else
			{
				curCursorIndex = 0;
				// countWrongPass = 0;
				inputKeypad = INPUT_KEYPAD_DEFAULT;
				pass = INPUT_KEYPAD_DEFAULT;
				break;
			}
		}
		curTime = millis();
	} while (handleKeyPad(c) == true);
	initKeyPad();
	deviceStatus.keyPress = false;
}

bool handleKeyPad(char c)
{
	if (curCursorIndex <= 6)
	{
		switch (c)
		{
		case '*':
			curCursorIndex--;
			if (curCursorIndex < 0)
			{
				break;
			}
			pass[curCursorIndex] = '_';
			inputKeypad[curCursorIndex] = '_';
			lcdDisplayEnterPass();
			break;
		case '#': // Enter
			if (curCursorIndex == 6)
			{

				if (pass == PASSWORD)
				{
					lcd.clear();
					lcd.setCursor(5, 0);
					lcd.print("CORRECT!");
					Serial.println("CORRECT");
					deviceStatus.openDoor = true;
					//delay(2000);
					pass = INPUT_KEYPAD_DEFAULT;
					inputKeypad = INPUT_KEYPAD_DEFAULT;
					// checkPass = true;
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
					pass = INPUT_KEYPAD_DEFAULT;
					inputKeypad = INPUT_KEYPAD_DEFAULT;
					delay(2000);
					countWrongPass++;
					curCursorIndex = 0;
					lcdDisplayEnterPass();
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
			pass = INPUT_KEYPAD_DEFAULT;
			inputKeypad = INPUT_KEYPAD_DEFAULT;
			return false;
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
			lcdDisplayEnterPass();
			return false;
		default:
			Serial.print(c);
			if (curCursorIndex < 6)
			{
				pass[curCursorIndex] = c;
				inputKeypad[curCursorIndex] = c;
				lcdDisplayEnterPass();
				delay(200);
				inputKeypad[curCursorIndex] = '*';
				lcdDisplayEnterPass();
				curCursorIndex++;
			}
			break;
		}
	}
	return true;
}

void openDoor()
{
	servo1.write(90);
	Serial.println("Open");
	lcdDisplayOpenDoor();
	delay(2000);
	uint32_t timeDelay = millis();
	for(int i =0; i < 6;i++)
	{

		statusBuzzer =!statusBuzzer;
		digitalWrite(33,statusBuzzer);
		delay(500);
	}
	delay(10000);
	Serial.println("Close");
	lcd.clear();
	lcd.print("  Close!!");
	servo1.write(0);
	deviceStatus.openDoor = false;
	lcdDisplayEnterPass();
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

int checkFinger()
{
	int fingerStatus = -1;
	fingerStatus = getFingerprintIDez();
	if (fingerStatus != -1 and fingerStatus != -2)
	{
		Serial.print("Match");
		Serial.println(fingerIdOpenDoor);
		fingerIdOpenDoor = fingerStatus;
		return true;
	}
	else
	{
		if (fingerStatus == -2)
		{
			for (int ii = 0; ii < 5; ii++)
			{
				Serial.print("Not Match");
			}
			return false;
		}
	}
	return false;
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

bool handleFinger()
{
	if (fingerIdOpenDoor != 0)
	{
		deviceStatus.openDoor = true;
		// openDoor();
		fingerIdOpenDoor = 0;
		return true;
	}
	return false;
}

void sendDataToGetway()
{
	// deviceId = SmartLock
	// deviceStatus.openDoor = ?
	// deviceStatus.warning = ?
	// deviceStatus.unlockOption
	// fingerIdOpendoor;
}
bool receiverFromGetway()
{
	// deviceStatus.statusChangeFinger
	// deviceStatus.chageFinger = add/sub
	// deviceStatus.chageFingerId = number ID
	// deviceStatus.unlockOption = MobileApp
	return true;
}

void handleDataReceiver()
{
}
