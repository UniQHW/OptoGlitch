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
#define MIN_SET_ARG_SIZE              sizeof(SET_ARG_CALIBRATION_MODE) // Option with least characters

// Calibartion mode arguments
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
  stddev,  // Standard deviation
  lookup   // Mean Lookup table
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

 // private:
   uint8_t tx; // LED
   uint8_t rx; // Photo Resistor

   int timeout;
   int transmission_time;
   int mean_samples;
   CalibrationMode calibration_mode = stddev;
};
