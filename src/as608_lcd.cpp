#include "as608_lcd.h"
#define TX_PIN 14
#define RX_PIN 13
#define FINGER_SERIAL Serial1

LiquidCrystal_I2C lcdDisplay(0x27, 16, 2);
// HardwareSerial Serial2(2) = HardwareSerial Serial2(2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&FINGER_SERIAL);
uint8_t id;

void fingerInit()
{

	lcdDisplay.backlight();
	Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
	//  set the data rate for the sensor serial port
	// Serial1.setPins(RX_PIN, TX_PIN);
	// finger = Adafruit_Fingerprint(&FINGER_SERIAL);
	finger.begin(57600);
	if (finger.verifyPassword())
	{
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		Serial.println("Found fingerprint!");
		lcdDisplay.print("Found fingerprint!");
	}
	else
	{
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		Serial.println("Did not find");
		lcdDisplay.println("Did not find : ");
		lcdDisplay.setCursor(0, 1);
		lcdDisplay.print("fingerprint sensor");
		while (1)
		{
			delay(1);
		}
	}
	delay(5000);
}

void changeFinger()
{
	Serial.println("Ready to enroll a fingerprint!");
	lcdDisplay.clear();
	lcdDisplay.setCursor(0, 0);
	lcdDisplay.print("Please type ID(1-3)");
	lcdDisplay.setCursor(5, 1);
	Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
	id = readnumber();
	// id = 1;
	//  Serial.println(id);
	lcdDisplay.print(id);
	if (id == 0)
	{ // ID #0 not allowed, try again!
		return;
	}

	delay(1000);

	lcdDisplay.clear();
	lcdDisplay.setCursor(0, 0);
	lcdDisplay.print("Enrolling ID:");
	lcdDisplay.print(id);
	Serial.println(id);
	while (getFingerprintEnroll() != FINGERPRINT_OK)
		;
}

uint8_t getFingerprintEnroll()
{
	int p = -1;
	Serial.print("Waiting for valid finger to enroll as #");
	Serial.println(id);
	while (p != FINGERPRINT_OK)
	{
		// Lay van tay lan 1
		p = finger.getImage();
		// switch (p)
		// {
		// case FINGERPRINT_OK:
		// Serial.println("Image taken");
		// 	break;
		// case FINGERPRINT_NOFINGER:
		// 	break;
		// case FINGERPRINT_PACKETRECIEVEERR:
		// 	Serial.println("Communication error");
		// 	break;
		// case FINGERPRINT_IMAGEFAIL:
		// 	Serial.println("Imaging error");
		// 	break;
		// default:
		// Serial.println("Unknown error");
		// 	break;
		// }

		switch (p)
		{
		case FINGERPRINT_OK:
			Serial.println("Image taken");
			break;
		case FINGERPRINT_NOFINGER:
			break;
		default:
			lcdDisplay.clear();
			lcdDisplay.setCursor(0, 0);
			lcdDisplay.print("ERROR");
			Serial.println("Unknown error");
			Serial.println("Unknown error");
			break;
		}
		// if (p == FINGERPRINT_OK)
		// {
		// 	lcdDisplay.clear();
		// 	lcdDisplay.setCursor(0, 0);
		// 	lcdDisplay.print("Image taken");// OK lan 1
		// 	Serial.print("Image taken");
		// 	// break;
		// }
		// else if (p == FINGERPRINT_NOFINGER)
		// {
		// 	continue;
		// }
		// else
		// {
		// 	lcdDisplay.clear();
		// 	lcdDisplay.setCursor(0, 0);
		// 	lcdDisplay.print("ERROR");
		// 	Serial.println("Unknown error");
		// 	// break;
		// }
	}

	// OK success!

	p = finger.image2Tz(1);
	// switch (p)
	// {
	// case FINGERPRINT_OK:
	//		Serial.println("Image converted");
	// 	break;
	// case FINGERPRINT_IMAGEMESS:
	// 	Serial.println("Image too messy");
	// 	return p;
	// case FINGERPRINT_PACKETRECIEVEERR:
	// 	Serial.println("Communication error");
	// 	return p;
	// case FINGERPRINT_FEATUREFAIL:
	// 	Serial.println("Could not find fingerprint features");
	// 	return p;
	// case FINGERPRINT_INVALIDIMAGE:
	// 	Serial.println("Could not find fingerprint features");
	// 	return p;
	// default:
	// 	Serial.println("Unknown error");
	// 	return p;
	// }

	if (p == FINGERPRINT_OK)
	{
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		Serial.println("Image converted");
		lcdDisplay.print("Image taken");
		Serial.println(" Please press finger");
	}
	else
	{
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		lcdDisplay.print("ERROR");
		//		Serial.println("Image converted");
		Serial.println("Unknown error");
		return p;
	}

	Serial.println("Remove finger");
	delay(2000);
	p = 0;
	while (p != FINGERPRINT_NOFINGER)
	{
		p = finger.getImage();
	}
	Serial.print("ID ");
	Serial.println(id);
	p = -1;
	Serial.println("Place same finger again");

	lcdDisplay.clear();
	lcdDisplay.setCursor(0, 0);
	lcdDisplay.print("Enter finger again");
	lcdDisplay.setCursor(0, 1);
	lcdDisplay.print("ID:");
	//	uint8_t number = readnumber();
	//	lcdDisplay.print(number);

	// Lay lai van tay lan 2
	while (p != FINGERPRINT_OK)
	{
		p = finger.getImage();
		if (p == FINGERPRINT_OK)
		{
			Serial.println("Image taken");
			lcdDisplay.clear();
			lcdDisplay.setCursor(0, 0);
			lcdDisplay.print("Image taken");
		}
		else if (p == FINGERPRINT_NOFINGER)
		{
			continue;
		}
		else
		{
			/* code */
			Serial.println("ERORR!");
			lcdDisplay.clear();
			lcdDisplay.setCursor(0, 0);
			lcdDisplay.println("ERORR!");
		}

		// switch (p)
		// {
		// case FINGERPRINT_OK:
		// 	Serial.println("Image taken");
		// 	break;
		// case FINGERPRINT_NOFINGER:
		// 	break;
		// case FINGERPRINT_PACKETRECIEVEERR:
		// 	Serial.println("Communication error");
		// 	break;
		// case FINGERPRINT_IMAGEFAIL:
		// 	Serial.println("Imaging error");
		// 	break;
		// default:
		// 	Serial.println("Unknown error");
		// 	break;
		// }
	}

	// OK success!

	p = finger.image2Tz(2);
	if (p == FINGERPRINT_OK)
	{
		Serial.println("Image converted");
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		lcdDisplay.print("Image converted!");
	}
	else
	{
		Serial.println("ERROR!!!");
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		lcdDisplay.print("ERROR!!");
		return p;
	}
	// switch (p)
	// {
	// case FINGERPRINT_OK:
	// 	Serial.println("Image converted");
	// 	break;
	// case FINGERPRINT_IMAGEMESS:
	// 	Serial.println("Image too messy");
	// 	return p;
	// case FINGERPRINT_PACKETRECIEVEERR:
	// 	Serial.println("Communication error");
	// 	return p;
	// case FINGERPRINT_FEATUREFAIL:
	// 	Serial.println("Could not find fingerprint features");
	// 	return p;
	// case FINGERPRINT_INVALIDIMAGE:
	// 	Serial.println("Could not find fingerprint features");
	// 	return p;
	// default:
	// 	Serial.println("Unknown error");
	// 	return p;
	// }

	// OK converted!
	Serial.print("Creating model for #");
	Serial.println(id);

	p = finger.createModel();

	if (p == FINGERPRINT_OK)
	{
		Serial.println("Prints matched!");
	}
	else
	{
		Serial.println("ERROR!!");
		return p;
	}

	// if (p == FINGERPRINT_OK)
	// {
	// 	Serial.println("Prints matched!");
	// }
	// else if (p == FINGERPRINT_PACKETRECIEVEERR)
	// {
	// 	Serial.println("Communication error");
	// 	return p;
	// }
	// else if (p == FINGERPRINT_ENROLLMISMATCH)
	// {
	// 	Serial.println("Fingerprints did not match");
	// 	return p;
	// }
	// else
	// {
	// 	Serial.println("Unknown error");
	// 	return p;
	// }

	Serial.print("ID ");
	Serial.println(id);
	p = finger.storeModel(id);
	if (p == FINGERPRINT_OK)
	{
		Serial.println("Stored!");
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		lcdDisplay.print("Stored!");
		return p;
	}
	else
	{
		Serial.println("Unknown error");
		return p;
	}
}

int getFingerprintIDez()
{
	uint8_t p = finger.getImage();
	if (p != 2)
	{
		Serial.println(p);
	}
	if (p != FINGERPRINT_OK)
		return -1;

	p = finger.image2Tz();
	if (p != 2)
	{
		Serial.println(p);
	}
	if (p != FINGERPRINT_OK)
		return -1;

	p = finger.fingerFastSearch();
	if (p != FINGERPRINT_OK)
		return -2;

	// found a match!
	Serial.print("Found ID #");
	Serial.print(finger.fingerID);
	Serial.print(" with confidence of ");
	Serial.println(finger.confidence);
	return finger.fingerID;
}
// else if (p == FINGERPRINT_PACKETRECIEVEERR)
// {
// 	Serial.println("Communication error");
// 	return p;
// }
// else if (p == FINGERPRINT_BADLOCATION)
// {
// 	Serial.println("Could not store in that location");
// 	return p;
// }
// else if (p == FINGERPRINT_FLASHERR)
// {
// 	Serial.println("Error writing to flash");
// 	return p;
// }