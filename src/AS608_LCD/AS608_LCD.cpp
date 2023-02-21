#include "AS608_LCD.h"

LiquidCrystal_I2C lcdDisplay(0x27, 16, 2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&FINGER_SERIAL);
int id = 0;

void fingerInit()
{

	lcdDisplay.backlight();
	Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
	finger.begin(57600);
	if (finger.verifyPassword())
	{
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		Serial.println("Found fingerprint!");
	}
	else
	{
		while (1)
		{
			delay(1);
		}
	}
	delay(5000);
}

bool changeFinger(bool &statusFinger, int timeOut)
{
	Serial.println("Ready to enroll a fingerprint!");
	lcdDisplay.clear();
	lcdDisplay.setCursor(0, 0);
	lcdDisplay.print("Enter ID(1-5): ");
	lcdDisplay.setCursor(5, 1);
	Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
	char c = getKey();
	uint32_t countTimeOut = millis();
	while (millis() - countTimeOut < 15000)
	{
		c = getKey();
		delay(10);
		Serial.print(c);
		id = c - '0';
		Serial.print(id);
		if (id >= 1 && id <= 5)
		{
			break;
		}
	}
	if (id == -1)
	{
		return false;
	}

	lcdDisplay.print(id);
	Serial.println(id);
	lcdDisplay.print(id);

	if (id < 10 && id > 0)
	{
		delay(1000);
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		lcdDisplay.print("Enrolling ID:");
		lcdDisplay.print(id);
		Serial.println(id);
		countTimeOut = millis();
		while (getFingerprintEnroll() != FINGERPRINT_OK && millis() - countTimeOut < 30000)
		{
			lcdDisplay.clear();
			lcdDisplay.setCursor(0, 0);
			lcdDisplay.print("ADD FINGER ERO0R!");
			lcdDisplay.setCursor(0, 1);
			lcdDisplay.print("Retry after 3s");
			delay(3000);
		}
	}
	statusFinger = false;
	return false;
}

uint8_t getFingerprintEnroll()
{
	int p = -1;
	Serial.print("Waiting for valid finger to enroll as #");
	Serial.println(id);
	unsigned long timeOut = millis();

	lcdDisplay.clear();
	lcdDisplay.setCursor(0, 0);
	lcdDisplay.print("Enter Finger");

	while (p != FINGERPRINT_OK)
	{
		// Lay van tay lan 1
		p = finger.getImage();
		switch (p)
		{
		case FINGERPRINT_OK:
			Serial.println("Image taken");
			break;
		case FINGERPRINT_NOFINGER:
			break;
		default:
			delay(500);
			Serial.println("Unknown error");
			break;
		}
		if (millis() - timeOut > 30000)
		{
			return p;
		}
	}

#ifdef DEBUG
	Serial.println("Done get Image");
#endif
	// OK success!

	p = finger.image2Tz(1);

	if (p == FINGERPRINT_OK)
	{
		Serial.println("Image converted");
		Serial.println(" Please press finger");
	}
	else
	{
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
	lcdDisplay.print("Enter finger ");
	lcdDisplay.setCursor(0, 1);
	lcdDisplay.print("again");

	// Lay lai van tay lan 2
	timeOut = millis();
	while (p != FINGERPRINT_OK)
	{
		p = finger.getImage();
		if (p == FINGERPRINT_OK)
		{
			Serial.println("Image taken");
			lcdDisplay.clear();
			lcdDisplay.setCursor(0, 0);
		}
		else if (p == FINGERPRINT_NOFINGER)
		{
			continue;
		}

		if (millis() - timeOut > 10000)
		{
			return p;
		}
	}

	// OK success!

	p = finger.image2Tz(2);
	if (p == FINGERPRINT_OK)
	{
		Serial.println("Image converted");
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		// lcdDisplay.print("Image converted!");
	}
	else
	{
		Serial.println("ERROR!!!");
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		lcdDisplay.print("ERROR!!");
		return p;
	}

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

	Serial.print("ID ");
	Serial.println(id);
	p = finger.storeModel(id);
	if (p == FINGERPRINT_OK)
	{
		Serial.println("Stored!");
		lcdDisplay.clear();
		lcdDisplay.setCursor(0, 0);
		lcdDisplay.print("Stored!");
		p = finger.getImage();
		while (p != FINGERPRINT_NOFINGER)
		{
			// Lay van tay lan 1
			p = finger.getImage();
		}
		return FINGERPRINT_OK;
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