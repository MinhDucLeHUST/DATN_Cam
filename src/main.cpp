// Code này đã fix xong camera. mai develop nốt phần "Firebase"
#include <Arduino.h>
#include <string>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include "./AS608_LCD/AS608_LCD.h"
#include "./Keypad4_4/Keypad4_4.h"
#include "./DeviceStatus/DeviceStatus.h"
#include <WiFi.h>
#include <WebSocketsClient.h>

// #include "twilio.hpp"
using namespace std;

#define MAX_WRONG_PASS 3
#define MAX_WRONG_FINGER 5
#define MAX_FINGER_ID 5
#define INPUT_KEYPAD_DEFAULT "______"
#define TIME_UPDATE_DATA_DEFAULT 2000

const char* ssid = "P407";
const char* password = "17052000";

// const char* ssid = "DucCoding";
// const char* password = "20082k36";

// const char *ssid = "Noi binh yen";
// const char *password = "lucy666666";

DeviceStatus deviceStatus;
Servo myservo;

WebSocketsClient webSocket;
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

int curCursorIndex = 0;
int countWrongPass = 0;
int countWrongFinger = 0;
int deg = 0;
unsigned long loopCount;
unsigned long curTime;

// Servo
int pos = 0; // variable to store the servo position

void doServoOpen(void){
	for (pos = 0; pos <= 45; pos += 1){
		myservo.write(pos); 
		delay(15);
	}
}
void doServoClose(void){
	for (pos = 45; pos >= 0; pos -= 1){
		myservo.write(pos); 
		delay(15);
	}
}

// Twilio
//  static const char *account_sid = "ACd850852d06b64ca20c2d1ba393445bb5";
//  static const char *auth_token = "8299fc64a6daabd951449336426196a3";
//  static const char *from_number = "+1 442 219 8435";
//  static const char *to_number = "+84912372045";

// static const char *account_sid = "AC1fd210b8bcb947be30dc2a79f6a63388";
// static const char *auth_token = "52778b01c067d5a3188ca3c4f8f4cdeb";
// static const char *from_number = "+12705156567";
// static const char *to_number = "+84396606161";

// static const char *message_warning_1 = "[WARNING] Someone near your hour, but no touch";
// static const char *message_warning_2 = "[WARNING] Someone touch your door";
// static const char *message_warning_3 = "[WARNING] Someone unlock your door";
// Twilio *twilio;

// 4 modules
#define NO_MOTION 0
#define DETECT_MOTION 1
#define NO_TOUCH 0
#define DETECT_TOUCH 1

int BUZZER_PIN = 23;
int VIBRATION_PIN = 26;
int PIR_PIN = 27;
int TRIG_HCSR04_PIN = 25;
int ECHO_HCSR04_PIN = 33;

int value_PIR = 0;
int value_VIBRATION = 0;
int send_MSG_NOTOUCH_SHORT = 0;
int send_MSG_TOUCHED_LONG = 0;
int send_MSG_TOUCHED_SHORT = 0;

bool checkLoop = true;
bool checkConfig = true;
bool do_NOTOUCH_SHORT = true;
bool do_TOUCHED_LONG = true;
bool do_TOUCHED_SHORT = true;

unsigned long duration;
int distance_human;

// connect esp32 with esp32-cam
int TRANSMIT_PIN = 32;
bool transmitSignal = false;
int countTransmit = 0;
bool var_sendCam = true;

String hostName = "192.1.1.1";
const string PASSWORD = "123456";
string inputKeypad = "______";
string pass = "______";

int pin_rows1[4] = {19, 18, 5, 17};	 // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
int pin_column1[4] = {16, 4, 2, 15}; // GIOP16, GIOP4, GIOP0, GIOP2 connec

int fingerId[MAX_FINGER_ID][2] = {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}};

struct NodeLock
{
	bool isOpen = true;
	bool hasCameraRequest = false;
	string password = "123456";
	bool isWarning;
	bool isWarning_modeAntiThief;
	bool isAntiThief;
	int status;
};

NodeLock nodeLock;
// Servo servo1;

int fingerID_OpenDoor = 0;
// int fingerIdOpenDoor = 0;
bool statusBuzzer = false;
bool statusChangePass = false;
bool reconnect = false;

void IRAM_ATTR isr()
{
	deviceStatus.keyPress = true;
}

void initWiFi();

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

void sendSignalToCam(void);
void configHCSR(void);
void antiThiefMode(void);

void handleDataAck(String dataAck);
String dataToSend();
// Firebase

void sendMessage_1(void);
void sendMessage_2(void);
void sendMessage_3(void);

LiquidCrystal_I2C lcd(0x27, 16, 2);

uint32_t timeUpdateData = TIME_UPDATE_DATA_DEFAULT;
uint32_t timeDelayBuzzer;
uint32_t timeReconnectWiFi;
uint32_t timeDelaySendDataCam;

String dataReceiver = "";
bool isDataReceiverGateway = false;

uint32_t timeDelayOpen;

void setup()
{
	Serial.begin(9600);

	initWiFi();
	// initFirebase();
	lcd.init(); // initialize the lcd
	lcd.backlight();
	pinMode(PIR_PIN, INPUT);
	pinMode(ECHO_HCSR04_PIN, INPUT);
	pinMode(TRIG_HCSR04_PIN, OUTPUT);
	pinMode(VIBRATION_PIN, INPUT);
	pinMode(BUZZER_PIN, OUTPUT);
	myservo.attach(12);
	myservo.write(0);
	pinMode(TRANSMIT_PIN, OUTPUT);
	digitalWrite(BUZZER_PIN, HIGH);
	fingerInit();
	loopCount = 0;
	curTime = millis();
	// deviceStatus.statusChangeFinger = false;
	initKeyPad();
	lcdDisplayEnterPass();

	webSocket.begin(hostName, 81);
	webSocket.onEvent(webSocketEvent);
	//  twilio = new Twilio(account_sid, auth_token);
}

void loop()
{
	if (nodeLock.isAntiThief == true)
	{
		configHCSR();
		antiThiefMode();
	}

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
		nodeLock.hasCameraRequest = true;
		if (nodeLock.hasCameraRequest == true && millis() - timeDelaySendDataCam > 120000)
		{
			sendSignalToCam();
			delay(2000);
			digitalWrite(TRANSMIT_PIN, LOW);
			timeDelaySendDataCam = millis();
		}			
		String data = dataToSend();
		webSocket.sendTXT(data);

		delay(2000);
		nodeLock.hasCameraRequest = false;
		data = dataToSend();
		webSocket.sendTXT(data);

		countWrongFinger = 0;
		countWrongPass = 0;		
		Serial.println("SOS\n");
	}
	if (nodeLock.isOpen == false && deviceStatus.openDoor == false)
	{
		openDoor();
		timeDelayOpen = millis();
		// if (nodeLock.isAntiThief == true)
		// {
		// 	nodeLock.isAntiThief = false;
		// 	String data = dataToSend();
		// 	webSocket.sendTXT(data);
		// }
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
		initKeyPad();
		lcdDisplayEnterPass();
	}

	if (isDataReceiverGateway == true)
	{
		handleDataAck(dataReceiver);
		isDataReceiverGateway = false;
		dataReceiver = "";
	}
	if (nodeLock.isWarning == true)
	{
		Serial.println("Buzzer");
		for (int i = 0; i < 2; i++)
		{
			digitalWrite(BUZZER_PIN, LOW);
			delay(200);
			digitalWrite(BUZZER_PIN, HIGH);
			delay(200);
		}
	}
	else
	{
		digitalWrite(BUZZER_PIN, HIGH);
	}

	webSocket.loop();
}

void initWiFi()
{
	WiFi.begin("Gateway", "password");
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		Serial.println("Connecting to WiFi...");
	}
	if (WiFi.status() == WL_CONNECTED)
	{
		Serial.println("Connected to WiFi - OK");
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
			if (deviceStatus.openDoor == true)
			{
				break;
				// openDoor();
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
					if (deviceStatus.mode == NORMAL)
					{
						lcd.setCursor(5, 0);
						lcd.print("CORRECT!");
						Serial.println("CORRECT");
						delay(1);
						nodeLock.isOpen = false;
					}
					else if (deviceStatus.mode == CHANGE_FINGER)
					{
						lcd.setCursor(0, 0);
						lcd.print("Change Finger");
						Serial.println("CORRECT");
						deviceStatus.statusChangeFinger = true;
					}
					else if (deviceStatus.mode == CHANGE_PASSWORD)
					{
						lcd.setCursor(0, 0);
						lcd.print("Enter new pass");
						statusChangePass = true;
						pass = INPUT_KEYPAD_DEFAULT;
						inputKeypad = INPUT_KEYPAD_DEFAULT;
						// checkPass = true;
						curCursorIndex = 0;
						countWrongPass = 0;
						return true;
					}
					// delay(2000);
					deviceStatus.mode = NORMAL;
					pass = INPUT_KEYPAD_DEFAULT;
					inputKeypad = INPUT_KEYPAD_DEFAULT;
					// checkPass = true;
					curCursorIndex = 0;
					countWrongPass = 0;
					return true;
				}
				else if (deviceStatus.mode == CHANGE_PASSWORD && statusChangePass == true)
				{
					lcd.clear();
					lcd.setCursor(0, 0);
					lcd.print("Change Pass");
					lcd.setCursor(0, 1);
					lcd.print("Success");
					nodeLock.password = pass;
					// String data = dataToSend();
					delay(500);
					statusChangePass = false;
					deviceStatus.mode = NORMAL;
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
					delay(1);
					// sendMessage_2();
					delay(2000);
					deviceStatus.mode = NORMAL;
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
			//
			// statusChangePass = true;
			deviceStatus.mode = CHANGE_PASSWORD;
			lcd.setCursor(0, 0);
			lcd.print(" Enter Old Pass    ");
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
	nodeLock.isOpen = false;
	// String data = dataToSend();
	deviceStatus.openDoor = true;
	// sendData(dataToSend());
	Serial.println("Open");
	if (nodeLock.isAntiThief == true)
	{
		nodeLock.isAntiThief = false;
	}
	String data = dataToSend();
	webSocket.sendTXT(data);
	lcdDisplayOpenDoor();

	doServoOpen();
	timeDelayOpen = millis();
}

void closeDoor()
{
	nodeLock.isOpen = true;
	String data = dataToSend();
	webSocket.sendTXT(data);
	// String data = dataToSend();
	doServoClose();
	Serial.println("Close");
	lcd.clear();
	lcd.print("  Close!!");
	// for (int i = 0; i < 30; i++)
	// {
	// 	deg -= 3;
	// 	if (deg <= 0)
	// 		deg = 0;
	// 	servo1.write(deg);
	// 	delay(20);
	// }
	// servo1.write(0);
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

int count_fingerWrong = 0;

int checkFinger()
{
	int fingerStatus = -1;
	fingerStatus = getFingerprintIDez();
	if (fingerStatus != -1 and fingerStatus != -2)
	{
		Serial.print("Match");
		Serial.println(fingerID_OpenDoor);
		fingerID_OpenDoor = fingerStatus;
		return 1;
	}
	else
	{
		if (fingerStatus == -2)
		{
			count_fingerWrong ++;
			Serial.printf("Not Match, time: %d\n", count_fingerWrong);
			if(count_fingerWrong >=5)
			{
				count_fingerWrong = 0;
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
	if (fingerID_OpenDoor != 0)
	{
		// deviceStatus.openDoor = true;
		nodeLock.isOpen = false;
		// openDoor();
		fingerID_OpenDoor = 0;
		return true;
	}
	return false;
}

void configHCSR(void)
{
	digitalWrite(TRIG_HCSR04_PIN, 0);
	delayMicroseconds(2);
	digitalWrite(TRIG_HCSR04_PIN, 1);
	delayMicroseconds(10);
	digitalWrite(TRIG_HCSR04_PIN, 0);

	duration = pulseIn(ECHO_HCSR04_PIN, HIGH);
	distance_human = int(duration / 2 / 29.412);
	value_PIR = digitalRead(PIR_PIN);
	value_VIBRATION = digitalRead(VIBRATION_PIN);
	if (checkConfig)
	{
		Serial.println("Config done!\n");
		checkConfig = false;
	}
}

void sendSignalToCam(void)
{
	Serial.println("Da gui tin hieu cho Camera\n");
	digitalWrite(TRANSMIT_PIN, HIGH);
}

void antiThiefMode(void)
{
	digitalWrite(BUZZER_PIN, HIGH);
	switch (value_PIR)
	{
	case NO_MOTION:
		if (distance_human > 50)
		{
			Serial.printf("[No motion] [No touch] [Long] - Distance: %d (cm)\n", distance_human);
			delay(800);
		}
		else
		{
			Serial.printf("[No motion] [No touch] [Short] - Distance: %d (cm)\n", distance_human);
			delay(800);
		}
		break;

	case DETECT_MOTION:
		switch (value_VIBRATION)
		{
		case NO_TOUCH:
			if (distance_human > 50)
			{
				Serial.printf("[Motion] [No touch] [Long] - Distance: %d (cm)\n", distance_human);
				delay(500);
			}
			else if (distance_human < 50 && distance_human > 0)
			{
				Serial.printf("[Motion] [No touch] [Short] - Distance: %d (cm)\n", distance_human);
				do_NOTOUCH_SHORT = true;
				if (do_NOTOUCH_SHORT)
				{
					send_MSG_NOTOUCH_SHORT++;
					delay(200);
					if (send_MSG_NOTOUCH_SHORT >= 20)
					{
						send_MSG_NOTOUCH_SHORT = 0;
						nodeLock.isWarning = true;
						nodeLock.hasCameraRequest = true;
						if (nodeLock.hasCameraRequest == true && millis() - timeDelaySendDataCam > 120000)
						{
							sendSignalToCam();
							delay(2000);
							digitalWrite(TRANSMIT_PIN, LOW);
							timeDelaySendDataCam = millis();
						}

						String data = dataToSend();
						webSocket.sendTXT(data);
				
						delay(2000);
						nodeLock.hasCameraRequest = false;
						data = dataToSend();
						webSocket.sendTXT(data);
					}
				}
			}
			break;
		case DETECT_TOUCH:
			if (distance_human > 50)
			{
				Serial.printf("[Motion] [Touch] [Long] - Distance: %d (cm)\n", distance_human);
				do_TOUCHED_LONG = true;
				if (do_TOUCHED_LONG)
				{
					send_MSG_TOUCHED_LONG++;
					delay(10);
					if (send_MSG_TOUCHED_LONG >= 5)
					{
						// sendMessage_2();
						send_MSG_TOUCHED_LONG = 0;
						nodeLock.isWarning = true;
						nodeLock.hasCameraRequest = true;
						if (nodeLock.hasCameraRequest == true && millis() - timeDelaySendDataCam > 120000)
						{
							sendSignalToCam();
							delay(2000);
							digitalWrite(TRANSMIT_PIN, LOW);
							timeDelaySendDataCam = millis();
						}
						String data = dataToSend();
						webSocket.sendTXT(data);
				
						delay(2000);
						nodeLock.hasCameraRequest = false;
						data = dataToSend();
						webSocket.sendTXT(data);

						do_TOUCHED_LONG = false;
					}
				}
			}
			else if (distance_human < 50 && distance_human > 0)
			{
				Serial.printf("[Motion] [Touch] [Short] - Distance: %d (cm)\n", distance_human);
				do_TOUCHED_SHORT = true;

				if (do_TOUCHED_SHORT)
				{
					send_MSG_TOUCHED_SHORT++;
					delay(10);
					if(send_MSG_TOUCHED_SHORT >= 3)
					{
						delay(1);
						send_MSG_TOUCHED_LONG = 0;
						
						nodeLock.isWarning = true;
						nodeLock.hasCameraRequest = true;
						if (nodeLock.hasCameraRequest == true && millis() - timeDelaySendDataCam > 120000)
						{
							sendSignalToCam();
							delay(2000);
							digitalWrite(TRANSMIT_PIN, LOW);
							timeDelaySendDataCam = millis();
						}
						String data = dataToSend();
						webSocket.sendTXT(data);
				
						delay(2000);
						nodeLock.hasCameraRequest = false;
						data = dataToSend();
						webSocket.sendTXT(data);

						do_TOUCHED_SHORT = false;
					}
					
				}
			}
			break;
		}
		break;
	default:
		Serial.print("No sensor is installed !");
		break;
	}
}

// void //sendMessage_1 (void)
// {
//   String response;
//   bool success = twilio->send_message(to_number, from_number, message_warning_1, response);
//   if (success) {
//     Serial.println("Sent MESSAGE 1 ok!");
//   }
//   else
//   {
//     Serial.println(response);
//   }
// }
// void //sendMessage_2 (void)
// {
//   String response;
//   bool success = twilio->send_message(to_number, from_number, message_warning_2, response);
//   if (success) {
//     Serial.println("Sent MESSAGE 2 ok!");
//   }
//   else
//   {
//     Serial.println(response);
//   }
// }
// void //sendMessage_3 (void)
// {
//   String response;
//   bool success = twilio->send_message(to_number, from_number, message_warning_3, response);
//   if (success) {
//     Serial.println("Sent MESSAGE 3 ok!");
//   }
//   else
//   {
//     Serial.println(response);
//   }
// }

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
	}
	break;
	case WStype_TEXT:
		// case WStype_FRAGMENT:
		Serial.println("Message: " + String((char *)payload));
		dataReceiver = String((char *)payload);
		isDataReceiverGateway = true;
		break;
	}
}

void handleDataAck(String dataAck)
{
	DynamicJsonDocument dataReceiverJson(1024);
	deserializeJson(dataReceiverJson, dataAck);

	if (dataAck.indexOf("hasCameraRequest") != -1)
	{
		nodeLock.hasCameraRequest = dataReceiverJson[F("hasCameraRequest")].as<bool>();

		if (nodeLock.hasCameraRequest == true && millis() - timeDelaySendDataCam > 120000)
		{
			sendSignalToCam();
			delay(2000);
			digitalWrite(TRANSMIT_PIN, LOW);
			timeDelaySendDataCam = millis();
		}
	}
	else if (dataAck.indexOf("isAntiThief") != -1)
	{
		nodeLock.isAntiThief = dataReceiverJson[F("isAntiThief")].as<bool>();
	}
	else if (dataAck.indexOf("isOpen") != -1)
	{
		nodeLock.isOpen = dataReceiverJson[F("isOpen")].as<bool>();
	}
	else if (dataAck.indexOf("password") != -1)
	{
		Serial.println(dataReceiverJson["password"].as<String>());
		nodeLock.password = dataReceiverJson[F("password")].as<string>();
		Serial.println("ChangePassWord");
	}
	else if (dataAck.indexOf("isWarning") != -1)
	{
		nodeLock.isWarning = dataReceiverJson[F("isWarning")].as<bool>();
	}
}

String dataToSend()
{
	DynamicJsonDocument dataToSend(1024);

	dataToSend[F("isOpen")] = nodeLock.isOpen;
	dataToSend[F("isWarning")] = nodeLock.isWarning;
	dataToSend[F("password")] = nodeLock.password;
	dataToSend[F("isAntiThief")] = nodeLock.isAntiThief;
	dataToSend[F("hasCameraRequest")] = nodeLock.hasCameraRequest;
	// dataToSend[F("status")] = nodeLock.status;
	String output;
	serializeJson(dataToSend, output);
	return output;
}