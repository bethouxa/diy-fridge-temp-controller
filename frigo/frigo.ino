#include <LiquidCrystal.h>
#include <SimpleDHT.h>

#define TEMP_SETTING_MAX 40
#define TEMP_SETTING_MIN 8
#define DELTA_MAX 9
#define DELTA_MIN 1

const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
const int dht11pin = 6;
const int relay = 5;

const int RS = 12, EN = 11, D4 = 10, D5 = 9, D6 = 8, D7 = 7;
const int DHT11 = 6;
const int RELAY = 5;
const int KNOBSW = 4, KNOBDT = 3, KNOBCLK = 2;
volatile boolean turnDetected, turnDirection;

int targetTemp = 20; // °C
int deltaTemp = 2;	 // °C

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
SimpleDHT11 tempSensor(dht11pin);

void isr()
{
	delay(4); // delay for Debouncing
	if (digitalRead(KNOBCLK))
		turnDirection = digitalRead(KNOBDT);
	else
		turnDirection = !digitalRead(KNOBDT);
	turnDetected = true;
}

void configMenu(int* targetTemp, int* deltaTemp)
{
    // "greeting"
      lcd.print("Confguration");
      delay(1000);
      while (!digitalRead(KNOBSW)) {} // Wait until knob button depressed
      lcd.clear();
      attachInterrupt(digitalPinToInterrupt(KNOBCLK), isr, FALLING); // Knob turn monitoring
      // two near-identical code blocks to set target temp and delta temp
      do
      { // As long as knobsw not pressed
        lcd.setCursor(0, 0); lcd.print("> Target: ");lcd.print((int)*targetTemp, DEC);lcd.print("*C");
        lcd.setCursor(0, 1); lcd.print("Delta: +/- ");lcd.print((int)*deltaTemp, DEC);lcd.print("*C");
        if (turnDetected)
        {
          if (!turnDirection)
            *targetTemp = min(TEMP_SETTING_MAX, *targetTemp + 1);
          else
            *targetTemp = max(TEMP_SETTING_MIN, *targetTemp - 1);
          turnDetected = false;
          lcd.setCursor(10, 0);
          lcd.print((int)targetTemp, DEC);
          lcd.print("*C");
        }
        delay(100);
      } while ((digitalRead(KNOBSW))); // continue as long as button is depresseds (true)
      while (!(digitalRead(KNOBSW))) {} // Wait here as long as knob button pressed (false)
      delay(100);
      lcd.clear();
      do {
        lcd.setCursor(0, 0); lcd.print("Target: ");lcd.print((int)*targetTemp, DEC);lcd.print("*C");
        lcd.setCursor(0, 1); lcd.print("> Delta: +/- ");lcd.print((int)*deltaTemp, DEC);lcd.print("*C");
        if (turnDetected)
        {
          if (!turnDirection)
            *deltaTemp = min(DELTA_MAX, *deltaTemp + 1);
          else
            *deltaTemp = max(DELTA_MIN, *deltaTemp - 1);
          turnDetected = false;
          lcd.setCursor(13, 1);
          lcd.print((int)targetTemp, DEC);
          lcd.print("*C  ");
        }
        delay(100);
      } while ((digitalRead(KNOBSW))); // continue as long as button is depressed (true)
      while (!(digitalRead(KNOBSW))) {} // Wait here as long as knob button pressed (false)

      // "config exit" routine
      detachInterrupt(digitalPinToInterrupt(KNOBCLK)); // Knob turn monitoring
      lcd.clear();
      lcd.print("Config OK !");
      delay(2000);
}

void setup()
{
	lcd.begin(16, 2);
	pinMode(relay, OUTPUT);
	digitalWrite(relay, LOW);
	
	Serial.begin(9600);
}


void loop()
{

  lcd.print("Demarrage...");
  unsigned int countdown = 30;
  while (countdown > 0)
  {
    lcd.setCursor(0, 1);
    lcd.print(countdown, DEC);
    lcd.print(" ");
    countdown--;
    delay(1000);
  }

	unsigned long changeTime = 0;
	unsigned long secondsSinceChangeTime;
	byte temp = 0, humid = 0;
	String state = "OFF", prevstate = "OFF";
	int readResult;
  
	while (true)
	{
		lcd.clear();
		if (!(digitalRead(KNOBSW))) /* CONFIG MODE */
		{
		  configMenu(&targetTemp,&deltaTemp);
		}

		readResult = tempSensor.read(&temp, &humid, NULL);
		if (readResult != SimpleDHTErrSuccess) {
			lcd.print("TEMP READ FAIL");
			lcd.setCursor(0, 1);
			lcd.print("ERR= ");
			lcd.print(readResult, HEX);
			digitalWrite(relay, LOW);
			delay(5000);
			state = "OFF";
		} else {

			Serial.println(temp);

			lcd.print(" ");
			lcd.print((int)temp, DEC);
			lcd.print("*C (T=");
			lcd.print((int)targetTemp, DEC);
			lcd.print("*C)");

			if (temp > targetTemp + deltaTemp) {
				digitalWrite(relay, HIGH);
				state = "ON";
			}
			else if (temp < targetTemp - deltaTemp) {
				digitalWrite(relay, LOW);
				state = "OFF";
			}
			if (state != prevstate) {
				changeTime = millis();
			}

			lcd.setCursor(0, 1);
			lcd.print(state);
			lcd.print(" depuis ");

			secondsSinceChangeTime = (millis() - changeTime) / 1000;
			if (secondsSinceChangeTime < 60) {
				lcd.print(secondsSinceChangeTime, DEC);
				lcd.print(" sec");
			} else if (secondsSinceChangeTime >= 60 && secondsSinceChangeTime < 3600) {
				lcd.print(secondsSinceChangeTime / 60, DEC);
				lcd.print(" min");
			} else if (secondsSinceChangeTime >= 3600) {
				lcd.print(secondsSinceChangeTime / 3600, DEC);
				lcd.print(" hr");
			}
		}

		prevstate = state;
		delay(2000);
	}
}
