# diy-fridge-temp-controller
Arduino software to control your fridge's temperature using a DHT11 thermometer and a relay.

This code is intended to drive a relay that turns on or off power to a fridge/freezer in order to reach a given temperature, with target temperature that can be changed at run-time, without reflashing the arduino.

This program will read the temperature of your fridge and turn it on if it is above the set temperature band, then turn it off when below this band.

***Disclaimer:*** This project is intended as a relay driver which manipulates **mains power** to a home appliance. You are responsible for the safety of your HV wiring and for configuring this software properly in order to avoid damage to your appliance (see advice below).

# Setup

- Wire your hardware: DHT11 temp sensor and relay, optionally display and rotenc. Wire the mains to the NO (normally open) port of the relay,
- Change the pins used in the `#define` blocks line 1-21 to match your setup,
- (optional) Change targetTemp and deltaTemp line 28-29 to change the default settings on boot,
- Upload the code to your arduino,
- Set your fridge to the maximum setting (lowest temperature),
- Put the thermometer in the fridge,
- Plug the fridge into the relay,
- Turn on the arduino,
- Press the rotary encoder button (KNOBSW) to enter config
- Set your target temperature ("target") and temperature band size ("delta")
- Press the rotary encoder button (KNOBSW) to confirm settings and exit config

# Configuration guidelines

Configuring the size of your temperature band ("Delta") is important to the health of your appliance. If it is set too low, the driver will turn on and off your appliance repeatedly, potentially damaging the compressor. The lower the thermal mass and temperature you're trying to cool, the bigger delta you want to set. This will make sure power is turned off in long intervals allowing your compressor to rest.

I've found a delta of +/- 2°C to be reasonable for 10L of liquid at 10°C in a half-size fridge, giving me "off-times" of around 20 minutes. If you want to set a smaller temperature band than that, edit the `DELTA_MIN` constant in the code.

The program is set to not allow temperatures below 8°C as this is the temperature where the fridge's internal circuitry will start to kick in.
