//Thermometer
#include <SimpleDHT.h>
#define DHT11PIN 6

//Relay
#define RELAY 5

// Screen
#include <LiquidCrystal.h>
#define RS 12
#define EN 11
#define D4 10
#define D5 9 
#define D6 8
#define D7 7

//Rotenc for setup
#define KNOBSW 4
#define KNOBDT 3
#define KNOBCLK 2
volatile boolean turnDetected, turnDirection;

#define TEMP_MAX 40
#define TEMP_MIN 8
#define DELTA_MAX 10
#define DELTA_MIN 1
int targetTemp = 20; // default, in °C
int deltaTemp = 2;	 // default, in °C

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
SimpleDHT11 tempSensor(DHT11PIN);

// Function to update a global variable whenever the rotary encoder is turned.
void rotEncTurnDetect() 
{
	delay(4); // delay for Debouncing
	if (digitalRead(KNOBCLK))
		turnDirection = digitalRead(KNOBDT);
	else
		turnDirection = !digitalRead(KNOBDT);
	turnDetected = true;
}

// Displays menu to configure 
void configMenu(int* targetTemp, int* deltaTemp)
{
	  unsigned long configMenuEnterTime = millis();
      lcd.print("Confguration");
      delay(1000);
      while (!digitalRead(KNOBSW)) {} // Wait until knob button depressed or 10 minutes elapsed
      lcd.clear();
      attachInterrupt(digitalPinToInterrupt(KNOBCLK), rotEncTurnDetect, FALLING); // Knob turn monitoring
      // two near-identical code blocks to set target temp and delta temp
      do
      { // As long as knobsw not pressed
        lcd.setCursor(0, 0); lcd.print("> Target: ");lcd.print((int)*targetTemp, DEC);lcd.print("*C    ");
        lcd.setCursor(0, 1); lcd.print("Delta: +/- ");lcd.print((int)*deltaTemp, DEC);lcd.print("*C    ");
        if (turnDetected) // turnDetected changed by rotEncTurnDetect on interrupt from KNOBCLK
        {
          if (!turnDirection) *targetTemp = min(TEMP_MAX, *targetTemp + 1);
          else *targetTemp = max(TEMP_MIN, *targetTemp - 1);
          turnDetected = false;
          lcd.setCursor(10, 0);
          lcd.print((int)targetTemp, DEC);
          lcd.print("*C");
        }
        delay(100);
      } while (digitalRead(KNOBSW) || millis() - configMenuEnterTime < 300000); // wait until button is pressed or until 5 minutes have passed
      while (!(digitalRead(KNOBSW))) {} // Button is pressed, wait here until button is depressed (false)
      delay(100);
      lcd.clear();
      do {
		// Display
        lcd.setCursor(0, 0); lcd.print("Target: ");lcd.print((int)*targetTemp, DEC);lcd.print("*C     ");
        lcd.setCursor(0, 1); lcd.print("> Delta: +/- ");lcd.print((int)*deltaTemp, DEC);lcd.print("*C     ");
        if (turnDetected) // turnDetected changed by rotEncTurnDetect on interrupt from KNOBCLK
        {
          if (!turnDirection)
            *deltaTemp = min(DELTA_MAX, *deltaTemp + 1);
          else
            *deltaTemp = max(DELTA_MIN, *deltaTemp - 1);
          turnDetected = false;
          lcd.setCursor(13, 1);
          lcd.print((int)targetTemp, DEC);
          lcd.print("*C     ");
        }
        delay(100);
      } while (digitalRead(KNOBSW) || millis() - configMenuEnterTime < 300000); // wait until button is pressed or until 5 minutes have passed
      while (!(digitalRead(KNOBSW))) {} // Button is pressed, wait here until button is depressed (false)

      // "config exit" routine
      detachInterrupt(digitalPinToInterrupt(KNOBCLK)); // Disable rot enc monitoring
      lcd.clear();
      lcd.print("Config OK !");
      delay(2000);
}

void setup()
{
	lcd.begin(16, 2);
	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, LOW);
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
	if (!(digitalRead(KNOBSW))) configMenu(&targetTemp,&deltaTemp);
  }

	unsigned long changeTime = 0, secondsSinceChangeTime = 0;
	byte temp = 0, humid = 0;
	String state = "OFF", prevstate = "OFF";
	int thermometerErrCode;
  
	while (true)
	{
		lcd.clear();
		if (!(digitalRead(KNOBSW))) configMenu(&targetTemp,&deltaTemp);
		
		thermometerErrCode = tempSensor.read(&temp, &humid, NULL);

		if (thermometerErrCode == SimpleDHTErrSuccess) {
			lcd.print(" ");
			lcd.print((int)temp, DEC);
			lcd.print("*C (T=");
			lcd.print((int)targetTemp, DEC);
			lcd.print("*C)");

			if (temp > targetTemp + deltaTemp) {
				digitalWrite(RELAY, HIGH);
				state = "ON";
			}
			else if (temp < targetTemp - deltaTemp) {
				digitalWrite(RELAY, LOW);
				state = "OFF";
			}

			if (state != prevstate) {
				changeTime = millis();
			}

			lcd.setCursor(0, 1);
			lcd.print(state);
			lcd.print(" since ");

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
		} else { // Thermometer error handling
			lcd.print("TEMP READ FAIL");
			lcd.setCursor(0, 1);
			lcd.print("ERR= ");
			lcd.print(thermometerErrCode, HEX);
			digitalWrite(RELAY, LOW);
			delay(5000);
			state = "OFF";
		}

		prevstate = state;
		delay(2000);
	}
}
