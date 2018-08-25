/*

* Copyright (C) 2018 Patrick Pedersen <ctx.xda@gmail.com>

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

 * Description:
 * Defines a set functions to statistically calibrate
 * the photo resistor input.

*/

#include <Arduino.h>
#include "Pins.h"
#include "Calibration.h"

// mean_reading
// -------------------------------------------
// Calculates the average value from a pool of
// photo resistor inputs
// -------------------------------------------
int
mean_reading(uint8_t rx, unsigned int samples)
{
  unsigned long sum = 0;

  for (unsigned int i = 0; i < samples; i++) {
    sum += analogRead(rx);
  }

  return sum / samples;
}

// std_deviantion
// ------------------------------------------------------
// Computes the standard deviation between LED output and
// mean photo resistor input
// ------------------------------------------------------
int
std_deviation(uint8_t rx, unsigned long transition_delay)
{
  unsigned long sum = 0;

  for (int16_t i = 255; i >= 0; i--)
  {
      analogWrite(LED_TX, i);
      delay(transition_delay);

      sum += pow(abs(mean_reading(rx, SAMPLES_PER_BRIGHTNESS) - i), 2);
  }

  return sqrt(sum / 256);
}
