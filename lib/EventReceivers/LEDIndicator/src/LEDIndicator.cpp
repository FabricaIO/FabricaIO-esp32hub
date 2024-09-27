#include "LEDIndicator.h"

/// @brief Creates and LED indicator
/// @param LEDPin The LED pin
/// @param LEDCount The number of LEDs
/// @param RGB True to use an RGB LED, False to use a single color LED
LEDIndicator::LEDIndicator(uint8_t LEDPin, int LEDCount, bool RGB) : leds(LEDCount, LEDPin, NEO_GRB + NEO_KHZ800) {
	rgb = RGB;
	led_pin = LEDPin;
}

/// @brief Initializes the LEDs
/// @return True on success
bool LEDIndicator::begin() {
	// Start LEDs
	if (rgb) {
		leds.begin();
		leds.fill(); // Clear LEDs
		leds.show();
	} else {
		pinMode(led_pin, OUTPUT);
		digitalWrite(led_pin, LOW);
	}
	return true;
}

/// @brief Shows a color on the LED indicator, or blinks the LED
/// @param color The color to show
/// @return True on success
bool LEDIndicator::receiveEvent(int event) {
	if (rgb) {
		leds.fill(color_map[(int)event]);
		leds.show();
	} else {
		for (int i = 0; i < (int)event; i++) {
			digitalWrite(led_pin, HIGH);
			delay(250);
			digitalWrite(led_pin, LOW);
			delay(250);
		}
	}
	return true;
}