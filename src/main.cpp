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
 * Main code for the the analogous and errornous arduino optocoupler.
 * Initialization and serial commands are handled here.

*/

#include <Arduino.h>
#include "Pins.h"
#include "Optocoupler.h"

#define  ERROR_BLINK_INTERVAL_MS  250

// success
// ------------------------------------------
// Informs of success via the serial console
// ------------------------------------------
void
success()
{
    Serial.write(1);
}

// error
// ------------------------------------------
// Informs of an error via the serial console
// and builting LED blinking
// ------------------------------------------
void
error(String msg)
{
  Serial.println(msg);

  // Blink builtin LED 3 times
  for (int8_t i = 5; i >= 0; i--) {
    digitalWrite(LED_BUILTIN, (i&0x1)); // Shhh.... we AND the LSB of i for 1 (see bitmasking)
    delay(ERROR_BLINK_INTERVAL_MS);
  }
}

/* -- MAIN -- */

Optocoupler *oc;

void
setup()
{
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_TX, OUTPUT);
  pinMode(PHOTO_RESISTOR, INPUT);

  oc = new Optocoupler(LED_TX, PHOTO_RESISTOR);
}

void
loop()
{
  // Await serial commands
  if (Serial.available() > 0)
  {
    String input = Serial.readString();

    // Set command received
    if (input.length() >= 3 && input.substring(0, 3) == "set")
    {
      if (oc->apply_set_cmd(input)) {
        success();
      } else {
        error("Invalid set property given!");
      }
    }

    // Reset command received
    else if (input.length() >= 5 && input.substring(0, 5) == "reset")
    {
      oc->reset_properties();
      success();
    }

    // Parse command received
    else if (input.length() >= 5 && input.substring(0, 5) == "parse")
    {
      if (input.length() > 6)
      {
        payload_size_t payload_size = input.substring(6).toInt();

        if (payload_size)
        {
          if (oc->parse(payload_size)) {
            success();
          } else {
            error("Lost connecton to host!");
          }
        }
        else
        {
          error("Invalid payload size given!");
        }
      }
      else
      {
        error("No payload size given!");
      }
    }

    // Invalid Command received
    else
    {
      error("No such command: " + input);
    }
  }
}
