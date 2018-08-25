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
 * Provides a set functions to statistically calibrate
 * the photo resistor input.

*/

#pragma once

// Time given for LED to change its brightness
#define TRANSITION_TIME_MS      10

// SAMPLES_PER_BRIGHTNESS
// Photo resistor input samples per LED brightness to be tested for
// an average value. As more samples are collected, the accuraccy
// and time consumption per avrg_read excection rises. It should be
// noted that the impact on the outcoming average value exponentially
// decreases with every new sample added, hence, higher values will
// not show a notable change in accuraccy, but rather only more
// processing power and time.

// The following equation approximates the averge time in ms required
// for a avrg_read execution:
//  t = SAMPLES_PER_BRIGHTNESS * 0.112

// For preciser results, it is highly recommended to rely on lookup tables
// rather than utilizing an average deviation!
#define SAMPLES_PER_BRIGHTNESS  255

// CalibrationAverage.cpp
int mean_reading(uint8_t rx, unsigned int samples); // Reads the average input from a pool of samples
int std_deviation(uint8_t rx, unsigned long transition_delay);  // Computes the average deviation between LED output and PR input
