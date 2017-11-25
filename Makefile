# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

USER_LIB_PATH = ./libraries
ARDUINO_LIBS  = TimerOne
ARDUINO_DIR   = ../arduino/arduino-1.8.5
BOARD_TAG     = uno

include ../arduino/Arduino-Makefile-1.6.0/Arduino.mk

