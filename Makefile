# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

USER_LIB_PATH = ./libraries
ARDUINO_LIBS  = USB_Host_Shield_2.0-1.3.2 SPI Keyboard HID
ARDUINO_DIR   = ../arduino/Arduino-1.8.7.app/Contents/Java
BOARD_TAG     = leonardo
# BOARD_TAG     = uno

include ../arduino/Arduino-Makefile-1.6.0/Arduino.mk
