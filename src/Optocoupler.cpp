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
 * Defines all methods provided by the Optocoupler class.

*/

#include <Arduino.h>
#include "Pins.h"
#include "Optocoupler.h"

// trim_int_arg
// ----------------------------------------------
// Trims and converts an numeric command argument
// into an integer
// ----------------------------------------------
int
trim_int_arg(String cmd, uint8_t int_arg_pos)
{
  String arg_str = cmd.substring(int_arg_pos);
  int arg_int = arg_str.toInt();

  if (arg_str != "0\n" && arg_int == 0) {
    return -1;
  }

  return arg_int;
}

/** NoiseGenerator **/

/* Public */

// NoiseGenerator
NoiseGenerator::NoiseGenerator()
{
  disable();
}

// disable()
// ----------------------------
// Disables the noise generator
// ----------------------------
void
NoiseGenerator::disable()
{
  enabled = false;
}

// enabled()
// -----------------------------------------
// Returns if the noise generator is enabled
// -----------------------------------------
bool
NoiseGenerator::is_enabled()
{
  return enabled;
}

// set_max_noise()
// -------------------------------------------------
// Sets the maximum noise that can be generated
//  (and minimum noise to 0 if the object has been
// enabled through this call)
// -------------------------------------------------
void
NoiseGenerator::set_max_noise(uint8_t max)
{
  if (!max)
  {
    enabled = false;
    return;
  }

  if (!enabled)
  {
    min_noise = 0;
    enabled = true;
  }

  max_noise = max;
}

// set_max_noise()
// -------------------------------------------------
// Sets the minimum noise that can be generated
//  (and maximum noise to 255 if the object has been
// enabled through this call)
// -------------------------------------------------
void
NoiseGenerator::set_min_noise(uint8_t min)
{
  if (!enabled)
  {
    max_noise = 255;
    enabled = true;
  }

  min_noise = min;
}

// generate()
// -------------------------------------------
// Generates a random value with a random sign
// in the range of min_noise to max_noise
// -------------------------------------------
int
NoiseGenerator::generate()
{
  return ((int) random(2) * 2 - 1) * random(min_noise, max_noise+1);
}

/** Optocoupler **/

/* PUBLIC */

// Optocoupler
// -------------------------------------------------------------
// Optocoupler class constructor. Takes the transmission LED pin
// and receiver PR pin as parameters
// -------------------------------------------------------------
Optocoupler::Optocoupler(uint8_t tx, uint8_t rx) :
  tx(tx),
  rx(rx)
{
  reset_properties();
};

// reset_properties
// ------------------------------------------------------
// Sets all class properties back to their default values
// ------------------------------------------------------
void
Optocoupler::reset_properties()
{
  timeout            = DEFAULT_TIMEOUT;
  transmission_time  = DEFAULT_TRANSMISSION_TIME;
  mean_samples       = DEFAULT_MEAN_SAMPLES;
  calibration_mode   = DEFAULT_CALIBRATION_MODE;
  noise_generator.disable();
}

/*

apply_set_cmd
------------------------------------------------------------------------
  Sets parameters via a provded serial "set" command

 * Options:

 #  set cmode <stddev | lookup> - Sets calibration mode
    stddev - calibration by standard deviation
    lookup - calibration by mean lookup table

 #  set timeout <TIMEOUT_MS> - Sets maximum serial communication timeout

* Example:
   set timeout 1000
-------------------------------------------------------------------------

*/
bool
Optocoupler::apply_set_cmd(String cmd)
{
  String set_args = cmd.substring(4); // Trim "set " from the command

  if (set_args.length() < MIN_SET_ARG_SIZE + 1) {
    return false;
  }

  // Set calibration mode
  if (set_args.substring(0, sizeof(SET_ARG_CALIBRATION_MODE)) == APPEND_SPACE(SET_ARG_CALIBRATION_MODE))
  {
    String calibration_mode_str = set_args.substring(sizeof(SET_ARG_CALIBRATION_MODE));

    // No calibration
    if (calibration_mode_str == CALIBARTION_MODE_ARG_NONE) {
      calibration_mode = none;
    }

    // Standard deviation
    else if (calibration_mode_str == CALIBARTION_MODE_ARG_STDDEV) {
      calibration_mode = stddev;
    }

    // Lookup table
    else if (calibration_mode_str == CALIBARTION_MODE_ARG_LOOKUP) {
      calibration_mode = lookup;
    }

    // Invalid command
    else {
      return false; // Invalid calibration mode
    }
  }

  // Set timeout
  else if (set_args.substring(0, sizeof(SET_ARG_TIMEOUT)) == APPEND_SPACE(SET_ARG_TIMEOUT))
  {
    int ret = trim_int_arg(set_args, sizeof(SET_ARG_TIMEOUT));

    if (ret >= 0 ) {
      timeout = ret;
    } else {
      return false;
    }
  }

  // Set transition time
  else if (set_args.substring(0, sizeof(SET_ARG_TRANSITION_TIME)) == APPEND_SPACE(SET_ARG_TRANSITION_TIME))
  {
    int ret = trim_int_arg(set_args, sizeof(SET_ARG_TRANSITION_TIME));

    if (ret >= 0 ) {
      transmission_time = ret;
    } else {
      return false;
    }
  }

  // Set mean sample rate
  else if (set_args.substring(0, sizeof(SET_ARG_MEAN_SAMPLES)) == APPEND_SPACE(SET_ARG_MEAN_SAMPLES))
  {
    int ret = trim_int_arg(set_args, sizeof(SET_ARG_MEAN_SAMPLES));

    if (ret >= 0 ) {
      mean_samples = ret;
    } else {
      return false;
    }
  }

  // Set max noise generation
  else if (set_args.substring(0, sizeof(SET_ARG_NOISE_GENERATOR_MAX)) == APPEND_SPACE(SET_ARG_NOISE_GENERATOR_MAX))
  {
    int ret = trim_int_arg(set_args, sizeof(SET_ARG_NOISE_GENERATOR_MAX));

    if (ret >= 0 && ret < 255) {
      noise_generator.set_max_noise(ret);
    } else {
      return false;
    }
  }

  // Set min noise generation
  else if (set_args.substring(0, sizeof(SET_ARG_NOISE_GENERATOR_MIN)) == APPEND_SPACE(SET_ARG_NOISE_GENERATOR_MIN))
  {
    int ret = trim_int_arg(set_args, sizeof(SET_ARG_NOISE_GENERATOR_MIN));

    if (ret >= 0 && ret < 255) {
      noise_generator.set_min_noise(ret);
    } else {
      return false;
    }
  }

  return true;
}

// parse
// -----------------------------------------------------------------------------
// Accepts and analogously parses a serial data payload through the optocoupler,
// thereby analogously distorting/corrupting the data that is being provided.
// -----------------------------------------------------------------------------
bool
Optocoupler::parse(payload_size_t payload_size)
{
  // Get standard deviation
  int reading;
  int std_dev;

  if (calibration_mode == stddev) {
    std_dev = calibration_std_deviation();
  }

  // Tell host that we're ready for the payload!
  Serial.write(1);

  // Accept and parse payload
  for (payload_size_t i = 0; i < payload_size; i++)
  {
    unsigned long tstamp = millis();

    while(!Serial.available())
    {
      if (timeout != 0 && millis() - tstamp >= timeout) {
        analogWrite(tx, 0);
        return false;
      }
    }

    uint8_t byte = Serial.read();

    analogWrite(tx, byte);
    delay(transmission_time);

    if (mean_samples) {
      reading = mean_reading();
    } else {
      reading = analogRead(rx);
    }

    if (calibration_mode == std_dev) {
      if (reading > byte) {
        reading -= std_dev;
      } else if (reading < byte) {
        reading += std_dev;
      }
    }

    if (noise_generator.is_enabled()) {
      reading += noise_generator.generate();
    }

    // Return parsed data to host
    if (reading > 255) {
      Serial.write(255);
    } else if (reading < 0) {
      Serial.write((uint8_t)0);
    } else {
      Serial.write(reading);
    }
  }

  analogWrite(tx, 0);
  return true;
}

/* PRIVATE */

// mean_reading
// -------------------------------------------
// Calculates the average value from a pool of
// photo resistor inputs
// -------------------------------------------
int
Optocoupler::mean_reading()
{
  unsigned long sum = 0;

  for (unsigned int i = 0; i < mean_samples; i++) {
    sum += analogRead(rx);
  }

  return sum / mean_samples;
}

// calibration_mean_reading
// ----------------------------------------
// Same as mean_reading with the exception
// that this function utilizes the private
// calibration_* member variables
// ----------------------------------------
int
Optocoupler::calibration_mean_reading()
{
  unsigned long sum = 0;

  for (unsigned int i = 0; i < calibration_mean_samples; i++) {
    sum += analogRead(rx);
  }

  return sum / calibration_mean_samples;
}

// calibration_std_deviantion
// ------------------------------------------------------
// Computes the standard deviation between LED output and
// mean photo resistor input
// ------------------------------------------------------
int
Optocoupler::calibration_std_deviation()
{
  unsigned long sum = 0;

  for (int16_t i = 255; i >= 0; i--)
  {
      analogWrite(LED_TX, i);
      delay(calibration_transmission_time);

      sum += pow(abs(calibration_mean_reading() - i), 2);
  }

  return sqrt(sum / 256);
}
