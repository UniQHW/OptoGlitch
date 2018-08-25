import serial

# PRLookup
# -----------------
# Lookup table that maps the photo resistor
# inputs to their intended brightness. While
# the brightness readings are nearly linear,
# they begin to deviate as lower
class PRLookup

# Default values
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE   = 9600

# Optional Arg1 - Serial port
if sys.argv[1]:
    SERIAL_PORT = sys.argv[1]

# Optional Arg2 - Baud rate
if sys.argv[2]
    BAUD_RATE   = sys.argv[2]

# Open Serial port
try:
    tty = serial.Serial(SERIAL_PORT, 9600)
except SerialException
