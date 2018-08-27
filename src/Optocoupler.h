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
 * Provides the main optocoupler class.

*/

#include <Arduino.h>

#pragma once

#define APPEND_SPACE(STR)             STR " "

// Set Arguments
#define SET_ARG_CALIBRATION_MODE      "cmode"                          // Set calibration mode
#define SET_ARG_TIMEOUT               "timeout"                        // Set communication timeout
#define SET_ARG_TRANSITION_TIME       "transition"                     // Set LED transition time
#define SET_ARG_MEAN_SAMPLES          "samples"                        // Set size of PR sample pool to determine a mean value
#define SET_ARG_NOISE_GENERATOR_MAX   "nmax"                           // Maximum random noise that can be generated
#define SET_ARG_NOISE_GENERATOR_MIN   "nmin"                           // Minimum random noise that must be generated
#define MIN_SET_ARG_SIZE              sizeof(SET_ARG_NOISE_GENERATOR_MAX) // Option with least characters

// Calibartion mode arguments
#define CALIBARTION_MODE_ARG_NONE     "none"   // No calibration
#define CALIBARTION_MODE_ARG_STDDEV   "stddev" // Standard deviation
#define CALIBARTION_MODE_ARG_LOOKUP   "lookup" // Lookup table

// Default properties
#define DEFAULT_TIMEOUT           0
#define DEFAULT_TRANSMISSION_TIME 0
#define DEFAULT_MEAN_SAMPLES      0
#define DEFAULT_CALIBRATION_MODE  stddev

// Typedefs
typedef unsigned long payload_size_t;

// Calibration Modes
enum CalibrationMode
{
  none,    // NO CALIBRATION (Colors will be very inacurate)
  stddev,  // Standard deviation
  lookup   // Mean Lookup table
};

// Optocoupler
// -----------------------------------------
// A pseudo-random noise generator that adds
// or subtracts random values from provided
// data
// -----------------------------------------
class NoiseGenerator
{
 public:
   NoiseGenerator();

   bool is_enabled();
   void set_max_noise(uint8_t max);
   void set_min_noise(uint8_t min);

   int generate();
   void disable();

 private:
   int16_t max_noise;
   int16_t min_noise;
   bool enabled;
};

// Optocoupler
// -----------------------------------------------------
// A hardware abstraction interface to communicate
// and controll the analogous and errornous optocoupler.
// -----------------------------------------------------
class Optocoupler
{
 public:
   Optocoupler(uint8_t tx, uint8_t rx);
   void reset_properties();         // Resets object properties to their default values
   bool apply_set_cmd(String cmd);  // Sets parameters via a provded serial "set" command
   bool parse(payload_size_t payload_size); // Accepts and parses a serial data payload through the optocoupler

 private:
   uint8_t tx; // LED
   uint8_t rx; // Photo Resistor

   // Input Accuracy
   unsigned int timeout;
   unsigned int transmission_time;
   unsigned int mean_samples;

   int mean_reading();

   // Calibration
   CalibrationMode calibration_mode           = stddev;
   unsigned int calibration_transmission_time = 10;
   unsigned int calibration_mean_samples      = 255;

   int calibration_mean_reading();
   int calibration_std_deviation();

   // Digital Randomization
   NoiseGenerator noise_generator;
};
