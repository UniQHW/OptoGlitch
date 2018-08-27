# Copyright (C) 2018 Patrick Pedersen <ctx.xda@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Description:
# A python script that parses any given image through the analougous and
# erroneous Arduino optocoupler, thereby distorting/corrupting the image
# providing for a gltich effect.

import os
import sys
import serial
import argparse

from tqdm import tqdm
from PIL import Image

# set
# ------------------------------------------
# Sets optocoupler parameters/properties via
# the serial communication "set" command
# ------------------------------------------
def set(arg1, arg2, tty):
    tty.write(str("set " + arg1 + " " + str(arg2)).encode('ASCII'))
    tty.read()

# reset
# --------------------------------------------
# Resets optocoupler parameters/properties via
# the serial communication "reset" command
# --------------------------------------------
def reset():
    tty.write(b"reset")

# initiate_parse
# ---------------------------------------------
# Sets the arduino into parse mode, where it
# accepts a payload of given size to be parsed.
# ---------------------------------------------
def initiate_parse(size, tty, calibration):
    tty.write(str('parse ' + str(size)).encode('ASCII'))

    if calibration:
        print("Waiting for calibration...")

    tty.read() # Response on serial console signals that the calibration is complete

    if calibration:
        print("Calibration completed, awaiting payload")

# parse
# -----------------------------------------------
# Parses an 8 bit integer through the optocoupler
# -----------------------------------------------
def parse(val, tty):
    tty.write(val.to_bytes(1, byteorder='little'))
    return int.from_bytes(tty.read(), byteorder='little')

# parse_pixel
# -----------------------------------------
# Parses PIL pixels through the optocoupler
# -----------------------------------------
def parse_pixel(pxl, tty):
    return (parse(pxl[0], tty), parse(pxl[1], tty), parse(pxl[2], tty))

# Commandline Arguments
arg_parser = argparse.ArgumentParser(description="Parses images through the Arduino optocoupler")
arg_parser.add_argument('image', type = str, metavar = 'img', help = "Path to an image file")
arg_parser.add_argument('-p', '--port', type = str, default = '/dev/ttyACM0', help = "Serial port (Default '/dev/ttyACM0')")
arg_parser.add_argument('-b', '--baud', type = str, default = 115200, help = "Baud rate (Default 115200)")
arg_parser.add_argument('-c', '--calibration', type = str, default = 'stddev', help = "Calibration mode (none, stddev, lookup)")
arg_parser.add_argument('-dt', '--timeout', type = int, default = 0, help = "Device serial communication timeout (Default 0, no timeout)")
arg_parser.add_argument('-ht', '--host_timeout', type = int, default = 0, help = "Host serial communication timeout (Default 0, no timeout)")
arg_parser.add_argument('-t', '--transition', type = int, default = 0, help = "Transition time for LED (Default 0, no transition time)")
arg_parser.add_argument('-s', '--samples', type = int, default = 0, help = "PR input samples to determine a mean reading (Default 0, realy on a single sample only)")
arg_parser.add_argument('-nmax', '--noise_max', type = int, default = 0, help = "Maximum digitally generated noise")
arg_parser.add_argument('-nmin', '--noise_min', type = int, default = 0, help = "Minimum digitally generated noise")

args = arg_parser.parse_args()

# Open image in pillow
img = Image.open(args.image)

if img.mode != 'RGB':
    img = img.convert('RGB')

# Initialize serial console
tty = serial.Serial(args.port, args.baud)

# Set optocoupler parameters
if args.calibration != "stddev":
    set("calibration", args.calibration, tty)

if args.timeout > 0:
    set("timeout", args.timeout, tty)

if args.transition > 0:
    set("transition", args.transition, tty)

if args.samples > 0:
    set("samples", args.samples, tty)

if args.noise_max > 0:
    set("nmax", args.noise_max, tty)

if args.noise_min > 0:
    set("nmin", args.noise_min, tty)

px = img.load()
payload_size = (img.size[0] * img.size[1]) * 3

# Begin parsing!
initiate_parse(payload_size, tty, (args.calibration != "none"))

print("Parsing image...")
progress_bar = tqdm(total=img.size[0] * img.size[1])

for x in range(img.size[0]):
    for y in range(img.size[1]):
        px[x,y] = parse_pixel(px[x,y], tty) # Parse each pixel through optocoupler
        progress_bar.update(1)

# Save and close image
img.save("Parsed.png")
img.close()

# Send reset command
reset()

# Close tty
tty.close()
