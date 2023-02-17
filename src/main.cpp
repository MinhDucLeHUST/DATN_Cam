#include <Arduino.h>
#include <string>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include "./AS608_LCD/AS608_LCD.h"
#include "./Keypad4_4/Keypad4_4.h"
#include "./DeviceStatus/DeviceStatus.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>

using namespace std;

#define debug
#define MAX_WRONG_PASS 3
#define MAX_WRONG_FINGER 5
#define MAX_FINGER_ID 5
#define INPUT_KEYPAD_DEFAULT "______"
#define TIME_UPDATE_DATA_DEFAULT 2000
#define BUZZER_PIN 33

DeviceStatus deviceStatus;

int curCursorIndex = 0;
int countWrongPass = 0;
int countWrongFinger = 0;
int deg = 0;
unsigned long loopCount;
unsigned long curTime;

// string PASSWORD = "123456";
string inputKeypad = INPUT_KEYPAD_DEFAULT;
string pass = INPUT_KEYPAD_DEFAULT;

int pin_rows1[4] = {19, 18, 5, 17};	 // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
int pin_column1[4] = {16, 4, 2, 15}; // GIOP16, GIOP4, GIOP0, GIOP2 connect

int fingerId[MAX_FINGER_ID][2] = {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}};

static const int servoPin = 27;

// String hostName = "http://192.1.1.1/data";
String hostName = "192.1.1.1";
struct NodeLock
{
	int nodeIndex = 2;
	bool isLocked = true;
	bool isWarning;
	string password = "123456";
	int status;
};

NodeLock nodeLock;
Servo servo1;

int fingerIdOpenDoor = 0;
bool statusBuzzer = false;
bool statusChangePass = false;

void IRAM_ATTR isr()
{
	deviceStatus.keyPress = true;
}
// Intterrupt

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
void closeDoor();
int checkFinger();

String dataToSend();
String sendData(String dataToSend);
bool handleDataAck(String dataAck);
// void handleDataReceiver();
String getDataGateway();
LiquidCrystal_I2C lcd(0x27, 16, 2);

uint32_t time1;
uint32_t timeUpdateData = TIME_UPDATE_DATA_DEFAULT;
uint32_t timeDelayBuzzer;
String dataNotUpdate();
String dataReceiver = "";
bool isDataReceiverGateway = false;
WebSocketsClient webSocket;
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

uint32_t timeDelayOpen;
void setup()
{
	Serial.begin(9600);
	pinMode(25, OUTPUT);
	pinMode(BUZZER_PIN, OUTPUT);
	digitalWrite(25, LOW);
	digitalWrite(BUZZER_PIN, HIGH);
	lcd.init(); // initialize the lcd
	lcd.backlight();
	fingerInit();
	loopCount = 0;
	curTime = millis();
	// deviceStatus.statusChangeFinger = false;
	servo1.attach(servoPin);
	servo1.write(0);
	initKeyPad();
	WiFi.begin("Gateway", "password");
	// lcd.print("Wait.");
	uint32_t timeOutConnectGateway = millis();
	while (WiFi.status() != WL_CONNECTED && (millis() - timeOutConnectGateway < 30000))
	{
		Serial.print(".");
		delay(500);
	}
	// delay(1000);
	lcdDisplayEnterPass();

	webSocket.begin(hostName, 81);
	webSocket.onEvent(webSocketEvent);
	// handleDataAck(sendData(getDataGateway()));
}

void loop()
{

	if (deviceStatus.keyPress == true)
	{
		handleKeyPress();
	}
	int check = checkFinger();
	if (check == 1)
	{
		if (handleFinger() == true)
		{
			openDoor();
			countWrongFinger = 0;
		}
	}
	else
	{
		if (check == 0)
		{
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("ERROR Finger!");
			lcd.setCursor(0, 1);
			lcd.print("Try again..");
			delay(2000);
			countWrongFinger++;
			lcdDisplayEnterPass();
		}
	}
	if (countWrongFinger >= MAX_WRONG_FINGER || countWrongPass >= MAX_WRONG_PASS)
	{

		lcd.clear();
		lcd.print("  WARNING!!!!");
		nodeLock.isWarning = true;
		if (deviceStatus.warning == false)
		{
			String data = dataToSend();
			webSocket.sendTXT(data);
		}
		deviceStatus.warning = true;
		if (millis() - timeDelayBuzzer > 500)
		{
			statusBuzzer = !statusBuzzer;
			digitalWrite(BUZZER_PIN, statusBuzzer);
		}
		// Serial.println("SOS");
	}
	if (nodeLock.isLocked == false && deviceStatus.openDoor == false)
	{
		// handleDataAck(sendData(dataToSend()));
		openDoor();
		// lcdDisplayEnterPass();
	}
	if (deviceStatus.openDoor == true && millis() - timeDelayOpen > 10000)
	{
		closeDoor();
		deviceStatus.openDoor = false;
		timeDelayOpen = millis();
	}
	if (deviceStatus.statusChangeFinger == true)
	{
		curTime = millis();
		while (deviceStatus.statusChangeFinger == true && millis() - curTime < 10000)
		{
			deviceStatus.statusChangeFinger = changeFinger(deviceStatus.statusChangeFinger); /* code */
		}
		// lcdDisplayEnterPass();
		curTime = millis();
		deviceStatus.statusChangeFinger = false;
		// lcdDisplayEnterPass();
	}

	webSocket.loop();
	if (isDataReceiverGateway == true)
	{
		handleDataAck(dataReceiver.c_str());
		isDataReceiverGateway = false;
		dataReceiver = "";
	}
	if (WiFi.status() != WL_CONNECTED)
	{
		WiFi.disconnect();
		WiFi.reconnect();
		uint32_t timeOutConnectGateway = millis();
		while (WiFi.status() != WL_CONNECTED && (millis() - timeOutConnectGateway < 30000))
		{
			Serial.print(".");
			delay(500);
		}
	}
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
			if(deviceStatus.openDoor == true)
			{
				break;
				//openDoor();
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
				if (pass == nodeLock.password)
				{
					lcd.clear();
					lcd.setCursor(5, 0);
					lcd.print("CORRECT!");
					Serial.println("CORRECT");
					if (deviceStatus.mode == NORMAL)
					{
						// deviceStatus.openDoor = true;
						//deviceStatus.openDoor = true;
						nodeLock.isLocked = false;
					}
					else if (deviceStatus.mode == CHANGE_FINGER)
					{
						deviceStatus.statusChangeFinger = true;
					}
					// delay(2000);
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
			if (deviceStatus.mode == CHANGE_FINGER)
			{
				deviceStatus.mode = NORMAL;
				deviceStatus.statusChangeFinger = false;
			}
			else
			{
				deviceStatus.mode = CHANGE_FINGER;
				lcd.clear();
				lcd.print("Mode: 1");
				pass = "______";
				inputKeypad = "______";
				curCursorIndex = 0;
				delay(2000);
				lcdDisplayEnterPass();
			}
			pass = INPUT_KEYPAD_DEFAULT;
			inputKeypad = INPUT_KEYPAD_DEFAULT;
			return false;
			break;
		case 'B':
			statusChangePass = true;
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
	nodeLock.isLocked = false;
	String data = dataToSend();
	webSocket.sendTXT(data);
	deviceStatus.openDoor = true;
	// sendData(dataToSend());
	Serial.println("Open");
	lcdDisplayOpenDoor();
	for (int i = 0; i < 30; i++)
	{
		deg += 3;
		if (deg >= 90)
			deg = 90;
		servo1.write(deg);
		delay(20);
	}
	// delay(2000);
	timeDelayOpen = millis();
	// delay(5000);
	// closeDoor();
}

void closeDoor()
{
	nodeLock.isLocked = true;
	String data = dataToSend();
	webSocket.sendTXT(data);
	Serial.println("Close");
	lcd.clear();
	lcd.print("  Close!!");
	for (int i = 0; i < 30; i++)
	{
		deg -= 3;
		if (deg <= 0)
			deg = 0;
		servo1.write(deg);
		delay(20);
	}
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
		return 1;
	}
	else
	{
		if (fingerStatus == -2)
		{
			for (int ii = 0; ii < 5; ii++)
			{
				Serial.print("Not Match");
			}
			return 0;
		}
	}
	return -1;
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
		// deviceStatus.openDoor = true;
		nodeLock.isLocked = false;
		// openDoor();
		fingerIdOpenDoor = 0;
		return true;
	}
	return false;
}

String dataToSend()
{
	DynamicJsonDocument dataToSend(1024);
	dataToSend[F("nodeIndex")] = nodeLock.nodeIndex;
	dataToSend[F("isLocked")] = nodeLock.isLocked;
	dataToSend[F("isWarning")] = nodeLock.isWarning;
	dataToSend[F("password")] = nodeLock.password;
	// dataToSend[F("status")] = nodeLock.status;
	String output;
	serializeJson(dataToSend, output);
	return output;
}

String sendData(String dataToSend)
{
	if (WiFi.status() == WL_CONNECTED)
	{
		WiFiClient client;
		HTTPClient http;
		http.begin(client, hostName);
		http.addHeader("Content-Type", "application/json");
		int httpResponseCode = http.POST(dataToSend);
		if (httpResponseCode == 200)
		{
			// Serial.println(http.getString());
			String dataAck = http.getString();
			Serial.println(dataAck);
			// serialPrintstring(dataAck.c_str());
			return dataAck;
		}
		else
		{
			return "!";
		}
		Serial.print("HTTP Response code: ");
		Serial.println(httpResponseCode);
		http.end();
	}
	else
	{
		Serial.println("WiFi Disconnected");
		ESP.restart();
		// WiFi.reconnect();
		//  reconnectNetwork = true;
	}
	// return 0;
	return "";
}

bool handleDataAck(String dataAck)
{
	DynamicJsonDocument dataAckJson(1024);
	deserializeJson(dataAckJson, dataAck);
	if (dataAck == "" || dataAck.length() < 20)
	{
		return false;
	}
	if (dataAck.indexOf("isWarning") != -1)
	{
		nodeLock.isWarning = dataAckJson[F("isWarning")].as<bool>();
		deviceStatus.warning = nodeLock.isWarning;
		if (nodeLock.isWarning == false)
		{
			countWrongFinger = 0;
			countWrongPass = 0;
			digitalWrite(BUZZER_PIN, HIGH);
		}
	}
	if (dataAck.indexOf("password") != -1)
	{
		nodeLock.password = dataAckJson[F("password")].as<string>();
		// if (nodeLock.password == p)
		// {
		// 	timeUpdateData = TIME_UPDATE_DATA_DEFAULT;
		// }
	}
	if (dataAck.indexOf("isLocked") != -1)
	{
		nodeLock.isLocked = dataAckJson[F("isLocked")].as<bool>();
		if (nodeLock.isLocked == false)
		{
			openDoor();
		}
		else
		{
			closeDoor();
		}
	}
	return true;
}

String dataNotUpdate()
{
	DynamicJsonDocument dataToSend(1024);
	dataToSend[F("nodeIndex")] = nodeLock.nodeIndex;
	dataToSend[F("statusUpdater")] = "Not update";
	// dataToSend[F("status")] = nodeLock.status;
	String output;
	serializeJson(dataToSend, output);
	return output;
}

String getDataGateway()
{
	DynamicJsonDocument doc(1024);
	String getData;
	doc["name"] = "nodeLock";
	doc["isFirstConnected"] = true;
	// doc["name"]
	serializeJson(doc, getData);
	return getData;
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
	switch (type)
	{
	case WStype_DISCONNECTED:
		Serial.println("Disconnected");
		break;
	case WStype_CONNECTED:
	{
		Serial.println("Connected");
		// webSocket.sendTXT("name:\"nodeControl\"");
		String dataToSend = getDataGateway();
		webSocket.sendTXT(dataToSend);
	}
	break;
	case WStype_TEXT:
		// case WStype_FRAGMENT:
		Serial.println("Message: " + String((char *)payload));
		dataReceiver = String((char *)payload);
		isDataReceiverGateway = true;
		// webSocket.sendTXT("Hello gateway");
		break;
	}
}