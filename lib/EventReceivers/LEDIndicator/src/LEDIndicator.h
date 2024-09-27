/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* Adafruit_NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <EventReceiver.h>

class LEDIndicator : public EventReceiver {
	public:
		LEDIndicator(uint8_t LEDPin, int LEDCount, bool RGB = true);
		bool begin();
		bool receiveEvent(int event);
	
	private:
		/// @brief LED  driver
		Adafruit_NeoPixel leds;

		/// @brief Indicates if the LED is an RGB LED
		bool rgb;

		/// @brief The pin connected to the LED(s)
		int led_pin;

		/// @brief Array of color RGB hex codes
		int color_map[8] = {
		0x000000,     // Off    (0 blinks)
		0xFF6000,     // Yellow (1 blinks)
		0x00B0FF,     // Cyan	(2 blinks)
		0xFF2800,     // Orange (3 blinks)
		0xFF00C4,     // Purple (4 blinks)
		0x007F00,     // Green  (5 blinks)
		0x0000FF,     // Blue   (6 blinks)
		0xFF0000,     // Red    (7 blinks)
	};
};